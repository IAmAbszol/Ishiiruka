#include <chrono>
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <stdio.h>

#include "SocketComm/OutputComm.hpp"

#include "Common/MsgHandler.h"
#include "Core/ConfigManager.h"
#include <json.hpp>

#include <chrono>

using json = nlohmann::json;

namespace SocketComm
{

	OutputComm::OutputComm(OutputType m_type) :
		sending_address(IP_ADDRESS)
	{
		switch (m_type)
		{
		case OutputType::CONTROLLER_BACKEND:
			mPort = CONTROLLER_PORT;
			mUdpSocket.setBlocking(false);
			mConnected = true;
			break;

		case OutputType::SLIPPI_BACKEND:
			mPort = SLIPPI_PORT;
			mUdpSocket.setBlocking(false);
			mConnected = true;
			break;

		case OutputType::VIDEO_FRONTEND:
			mPort = VIDEO_PORT;
			if (mListenerSocket.listen(mPort) != sf::Socket::Done)
			{
				std::cout << "OutputComm(OutputType m_type) video initialization failed to listen on designated port." << std::endl;
			}
			mListenerSocket.setBlocking(false);
			mHandleConnectThread = std::thread(&OutputComm::HandleConnect, this);
			break;
		}
	}

	OutputComm::~OutputComm()
	{
		if (mConnected)
		{
			mConnected = false;
		}
		if (mPort == VIDEO_PORT)
		{
			mHandleConnections = false;
			mClientSocket.disconnect();
			mListenerSocket.close();
			mHandleConnectThread.join();
		}
	}

	uint64_t OutputComm::GetTimeSinceEpoch()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(
			mClock.now().time_since_epoch()).count();
	}

	const bool OutputComm::IsConnected() const
	{
		return mConnected;
	}

	void OutputComm::HandleConnect()
	{
		while (mHandleConnections)
		{
			if (!mConnected)
			{
				sf::Socket::Status send_status;
				if ((send_status = mListenerSocket.accept(mClientSocket)) == sf::Socket::Done)
				{
					#ifdef _WIN32
						mClientSocket.setBlocking(true);
					#else
						mClientSocket.setBlocking(true);
					#endif
					mConnected = true;
				}
				else if (send_status == sf::Socket::Error)
				{
					std::cout << "HandleConnect() failed to accept client socket." << std::endl;
				}
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	void OutputComm::PngWriteCallback(png_structp  png_ptr, png_bytep data, png_size_t length) {
		auto p = (std::vector<uint8_t>*)png_get_io_ptr(png_ptr);
		p->insert(p->end(), data, data + length);
	}

	void OutputComm::WritePngToMemory(size_t width, size_t height, const uint8_t *dataRGBA, std::vector<uint8_t> out) {
		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		TPngDestructor destroyPng(png_ptr);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		png_set_IHDR(png_ptr, info_ptr, width, height, 8,
			PNG_COLOR_TYPE_RGBA,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
		std::vector<uint8_t*> rows(height);
		for (size_t y = 0; y < height; ++y)
			rows[y] = (uint8_t*)dataRGBA + y * width * 4;
		png_set_rows(png_ptr, info_ptr, &rows[0]);
		png_set_compression_level(png_ptr, 6);
		png_set_write_fn(png_ptr, &out, PngWriteCallback, NULL);
		png_set_rgb_to_gray_fixed(png_ptr, 3,-1,-1);
		png_read_update_info(png_ptr, info_ptr);
		png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	}

	void OutputComm::SendUpdate(const u8* data, int row_stride, int width,
		int height, bool saveAlpha, bool frombgra)
	{
		if (mConnected)
		{

			sf::Packet data_packet;
			std::vector<u8> buffer;
			uint8_t segment_index = 0;
			if (!saveAlpha)
				buffer.resize(width * 4);

			// TODO: Clean this up
			// Write image data, right from ImageWrite.cpp
			for (auto y = 0; y < height; ++y)
			{
				if (y % mRowSize == 0 || (y + 1) >= height)
				{
					if (y != 0)
					{
						if (SendTcpMessage(data_packet) != sf::Socket::Done)
						{
							std::cout << "SendUpdate(const u8* data, int row_stride, int width, int height, bool saveAlpha, bool frombgra) failed to send." << std::endl;
						}
						segment_index++;
					}
					/*
					data_packet.clear();
					data_packet << mFrameCount;
					data_packet << segment_index;
					data_packet << mRowSize;
					data_packet << width;
					data_packet << height;
					data_packet << saveAlpha;
					data_packet << frombgra;
					data_packet << (sf::Uint64) GetTimeSinceEpoch();
					*/
				}

				const u8* row_ptr = data + y * row_stride;
				if (!saveAlpha || frombgra)
				{
					int src_r = frombgra ? 2 : 0;
					int src_b = frombgra ? 0 : 2;
					for (int x = 0; x < width; x++)
					{
						buffer[4 * x + 0] = row_ptr[4 * x + src_r];
						buffer[4 * x + 1] = row_ptr[4 * x + 1];
						buffer[4 * x + 2] = row_ptr[4 * x + src_b];
						buffer[4 * x + 3] = saveAlpha ? row_ptr[4 * x + 3] : 0xff;
					}
					row_ptr = buffer.data();
				}
				for (int x = 0; x < width * 4; x++)
				{
					data_packet.append(&row_ptr[x], sizeof(u8));
				}
			}
			std::vector<uint8_t> out;
			WritePngToMemory(width, height, const_cast<uint8_t *>(&buffer[0]), out);
			data_packet.append(&out[0], sizeof(out.size()));
		}
        if (mFrameCount >= std::numeric_limits<uint32_t>::max())
        {
            mFrameCount = 0;
        }
        else
        {
            mFrameCount++;
        }
	}

	void OutputComm::SendUpdate(std::vector<u8> &json_message)
	{
		if (mConnected)
		{
			sf::Packet packet;
			// TODO: Make this more stream lined
			std::string data(json_message.begin(), json_message.end());
			packet << data;
			if (SendUpdMessage(packet) != sf::Socket::Done)
			{
				std::cout << "SendUpdate(std::vector<u8> &json_message) failed to send." << std::endl;
			}
		}
	}

	void OutputComm::SendUpdate(u32 m_device_number, GCPadStatus &pad_status)
	{
		if (mConnected)
		{
			sf::Packet packet;
			packet.append(&m_device_number, sizeof(m_device_number));
			packet.append(&pad_status, sizeof(pad_status));
			if (SendUpdMessage(packet) != sf::Socket::Done)
			{
				std::cout << "SendUpdate(32 m_device_number, GCPadStatus &pad_status) failed to send." << std::endl;
			}
		}
	}
	int OutputComm::SendTcpMessage(sf::Packet &packet)
	{
		sf::Socket::Status send_status;
		if ((send_status = mClientSocket.send(packet)) != sf::Socket::Done)
		{
			mConnected = false;
			mClientSocket.disconnect();
			return send_status;
		}
		return send_status;
	}

	int OutputComm::SendUpdMessage(sf::Packet &packet)
	{
		return mUdpSocket.send(packet, sending_address, mPort);
	}

}
