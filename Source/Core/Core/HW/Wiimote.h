// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include "Common/Common.h"
#include "Common/CommonTypes.h"
#include "InputCommon/ControllerEmu.h"

class InputConfig;
class PointerWrap;
namespace WiimoteEmu
{
enum class WiimoteGroup;
enum class NunchukGroup;
enum class ClassicGroup;
enum class GuitarGroup;
enum class DrumsGroup;
enum class TurntableGroup;
}

enum
{
	WIIMOTE_CHAN_0 = 0,
	WIIMOTE_CHAN_1,
	WIIMOTE_CHAN_2,
	WIIMOTE_CHAN_3,
	WIIMOTE_BALANCE_BOARD,
	MAX_WIIMOTES = WIIMOTE_BALANCE_BOARD,
	MAX_BBMOTES = 5,
};

#define WIIMOTE_INI_NAME "WiimoteNew"

enum
{
	WIIMOTE_SRC_NONE = 0,
	WIIMOTE_SRC_EMU = 1,
	WIIMOTE_SRC_REAL = 2,
	WIIMOTE_SRC_HYBRID = 3,  // emu + real
};

extern unsigned int g_wiimote_sources[MAX_BBMOTES];

namespace Wiimote
{
enum class InitializeMode
{
	DO_WAIT_FOR_WIIMOTES,
	DO_NOT_WAIT_FOR_WIIMOTES,
};

void Shutdown();
void Initialize(InitializeMode init_mode);
void ResetAllWiimotes();
void LoadConfig();
void Resume();
void Pause();

unsigned int GetAttached();
void DoState(PointerWrap& p);
void EmuStateChange(EMUSTATE_CHANGE newState);
InputConfig* GetConfig();
ControllerEmu::ControlGroup* GetWiimoteGroup(int number, WiimoteEmu::WiimoteGroup group);
ControllerEmu::ControlGroup* GetNunchukGroup(int number, WiimoteEmu::NunchukGroup group);
ControllerEmu::ControlGroup* GetClassicGroup(int number, WiimoteEmu::ClassicGroup group);
ControllerEmu::ControlGroup* GetGuitarGroup(int number, WiimoteEmu::GuitarGroup group);
ControllerEmu::ControlGroup* GetDrumsGroup(int number, WiimoteEmu::DrumsGroup group);
ControllerEmu::ControlGroup* GetTurntableGroup(int number, WiimoteEmu::TurntableGroup group);

void ControlChannel(int _number, u16 _channelID, const void* _pData, u32 _Size);
void InterruptChannel(int _number, u16 _channelID, const void* _pData, u32 _Size);
void Update(int _number, bool _connected);
}

namespace WiimoteReal
{
void Initialize(::Wiimote::InitializeMode init_mode);
void Stop();
void Shutdown();
void Resume();
void Pause();
void Refresh();

void LoadSettings();
}
