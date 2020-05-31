// Copyright 2014 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.


// Copyright 2014 Tony Wasserka
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the owner nor the names of its contributors may
//       be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#pragma once

#include <limits>
#include <type_traits>

#include "Common.h"

/*
 * Abstract bitfield class
 *
 * Allows endianness-independent access to individual bitfields within some raw
 * integer value. The assembly generated by this class is identical to the
 * usage of raw bitfields, so it's a perfectly fine replacement.
 *
 * For BitField<X,Y,Z>, X is the distance of the bitfield to the LSB of the
 * raw value, Y is the length in bits of the bitfield. Z is an integer type
 * which determines the sign of the bitfield. Z must have the same size as the
 * raw integer.
 *
 *
 * General usage:
 *
 * Create a new union with the raw integer value as a member.
 * Then for each bitfield you want to expose, add a BitField member
 * in the union. The template parameters are the bit offset and the number
 * of desired bits.
 *
 * Changes in the bitfield members will then get reflected in the raw integer
 * value and vice-versa.
 *
 *
 * Sample usage:
 *
 * union SomeRegister
 * {
 *     u32 hex;
 *
 *     BitField<0,7,u32> first_seven_bits;     // unsigned
 *     BitField<7,8,u32> next_eight_bits;      // unsigned
 *     BitField<3,15,s32> some_signed_fields;  // signed
 * };
 *
 * This is equivalent to the little-endian specific code:
 *
 * union SomeRegister
 * {
 *     u32 hex;
 *
 *     struct
 *     {
 *         u32 first_seven_bits : 7;
 *         u32 next_eight_bits : 8;
 *     };
 *     struct
 *     {
 *         u32 : 3; // padding
 *         s32 some_signed_fields : 15;
 *     };
 * };
 *
 *
 * Caveats:
 *
 * 1)
 * BitField provides automatic casting from and to the storage type where
 * appropriate. However, when using non-typesafe functions like printf, an
 * explicit cast must be performed on the BitField object to make sure it gets
 * passed correctly, e.g.:
 * printf("Value: %d", (s32)some_register.some_signed_fields);
 *
 * 2)
 * Not really a caveat, but potentially irritating: This class is used in some
 * packed structures that do not guarantee proper alignment. Therefore we have
 * to use #pragma pack here not to pack the members of the class, but instead
 * to break GCC's assumption that the members of the class are aligned on
 * sizeof(StorageType).
 * TODO(neobrain): Confirm that this is a proper fix and not just masking
 * symptoms.
 */
#pragma pack(1)
template<std::size_t position, std::size_t bits, typename T>
struct BitField
{
private:
	// This constructor might be considered ambiguous:
	// Would it initialize the storage or just the bitfield?
	// Hence, delete it. Use the assignment operator to set bitfield values!
	BitField(T val) = delete;

public:
	// Force default constructor to be created
	// so that we can use this within unions
	BitField() = default;

	// We explicitly delete the copy assignment operator here, because the
	// default copy assignment would copy the full storage value, rather than
	// just the bits relevant to this particular bit field.
	// Ideally, we would just implement the copy assignment to copy only the
	// relevant bits, but this requires compiler support for unrestricted
	// unions.
	// TODO: Implement this operator properly once all target compilers
	// support unrestricted unions.
	BitField& operator=(const BitField&) = delete;

	__forceinline BitField& operator=(T val)
	{
		storage = (storage & ~GetMask()) | ((val << position) & GetMask());
		return *this;
	}

	__forceinline T Value() const
	{
		if (std::numeric_limits<T>::is_signed)
		{
			std::size_t shift = 8 * sizeof(T) - bits;
			return (T)((storage << (shift - position)) >> shift);
		}
		else
		{
			return (T)((storage & GetMask()) >> position);
		}
	}

	__forceinline operator T() const
	{
		return Value();
	}

private:
	// StorageType is T for non-enum types and the underlying type of T if
	// T is an enumeration. Note that T is wrapped within an enable_if in the
	// former case to workaround compile errors which arise when using
	// std::underlying_type<T>::type directly.
	typedef typename std::conditional<std::is_enum<T>::value,
		std::underlying_type<T>,
		std::enable_if<true, T>>::type::type StorageType;

	// Unsigned version of StorageType
	typedef typename std::make_unsigned<StorageType>::type StorageTypeU;

	__forceinline StorageType GetMask() const
	{
		return (((StorageTypeU)~0) >> (8 * sizeof(T) - bits)) << position;
	}

	StorageType storage;

	static_assert(bits + position <= 8 * sizeof(T), "Bitfield out of range");

	// And, you know, just in case people specify something stupid like bits=position=0x80000000
	static_assert(position < 8 * sizeof(T), "Invalid position");
	static_assert(bits <= 8 * sizeof(T), "Invalid number of bits");
	static_assert(bits > 0, "Invalid number of bits");
};
#pragma pack()
