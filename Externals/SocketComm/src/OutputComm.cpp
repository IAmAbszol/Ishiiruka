#include <chrono>
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <stdio.h>

#include "SocketComm/OutputComm.hpp"

#include "Common/MsgHandler.h"
#include "Core/ConfigManager.h"

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
			if(mSegments > 1)
			{
				std::cout << "WARNING! mSegments > 1 causes the jpeg reconstruction to fail. Still not 100% why but buffer out == buffer in. Wack" << std::endl;
			}
			mPort = VIDEO_PORT;
			//if (mListenerSocket.listen(mPort) != sf::Socket::Done)
			//{
			//	std::cout << "OutputComm(OutputType m_type) video initialization failed to listen on designated port." << std::endl;
			//}
			//mListenerSocket.setBlocking(false);
			//mHandleConnectThread = std::thread(&OutputComm::HandleConnect, this);
			mUdpSocket.setBlocking(false);
			mConnected = true;
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
					
					mClientSocket.setBlocking(true);
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

	void OutputComm::SendUpdate(const u8* data, int row_stride, int width,
		int height, bool saveAlpha, bool frombgra)
	{
		if (mConnected)
		{
			// Data
			sf::Packet data_packet;
			std::vector<u8> buffer;
			u8 *jpeg_buffer;
			JSAMPROW jpeg_row_pointer[1];
			
			// Setup the buffer to be written to
			jpeg_compress_struct jpeg_cinfo;
			jpeg_error_mgr jerr;
			jpeg_cinfo.err = jpeg_std_error(&jerr);
			jerr.trace_level = 10;
			jpeg_create_compress(&jpeg_cinfo);
			uint64_t outbuffer_size = 0;
			jpeg_mem_dest(&jpeg_cinfo, &jpeg_buffer, &outbuffer_size);

			// Setup the struct info
			jpeg_cinfo.image_width = width;
			jpeg_cinfo.image_height = height;
			jpeg_cinfo.input_components = 3;
			jpeg_cinfo.in_color_space = JCS_RGB;
			jpeg_set_defaults(&jpeg_cinfo);
			jpeg_set_quality(&jpeg_cinfo, JPEG_QUALITY, true);
			jpeg_start_compress(&jpeg_cinfo, false);

			buffer.resize(width * 3);

			for (auto y = 0; y < height; ++y)
			{
				const u8* row_ptr = data + y * row_stride;
				// TODO : Always ensure RGB is used
				if (!saveAlpha || frombgra)
				{
					int src_r = frombgra ? 2 : 0;
					int src_b = frombgra ? 0 : 2;
					for (int x = 0; x < width; x++)
					{
						buffer[3 * x + 0] = row_ptr[4 * x + src_r];
						buffer[3 * x + 1] = row_ptr[4 * x + 1];
						buffer[3 * x + 2] = row_ptr[4 * x + src_b];
					}
					row_ptr = buffer.data();
				}
				if(jpeg_cinfo.next_scanline >= jpeg_cinfo.image_height)
				{
					std::cout << "SendUpdate(const u8* data, int row_stride, int width, int height, bool saveAlpha, bool frombgra) JPEG Conversion Error, height out of bounds." << std::endl;
					return;
				}
				jpeg_row_pointer[0] = (uint8_t *) buffer.data();
				jpeg_write_scanlines(&jpeg_cinfo, jpeg_row_pointer, 1);
			}
			jpeg_finish_compress(&jpeg_cinfo);
			jpeg_destroy_compress(&jpeg_cinfo);

			uint8_t m_current_pos = 0;
			uint32_t block_size = outbuffer_size / mSegments;
			// WARNING! mSegments > 1 causes the jpeg reconstruction to fail. Still not 100% why but buffer out == buffer in. Wack.
			for(uint8_t segment = 0; segment < mSegments; segment++)
			{
				if((m_current_pos + block_size) > outbuffer_size)
				{
					block_size -= ((m_current_pos + block_size) - outbuffer_size);
				}
				// Total header bytes = 35
				data_packet.clear();
				data_packet << mFrameCount;
				data_packet << segment;
				data_packet << mSegments;
				data_packet << block_size;
				data_packet << width;
				data_packet << height;
				data_packet << (sf::Uint64) outbuffer_size;
				data_packet << (sf::Uint64) GetTimeSinceEpoch();
				data_packet.append(reinterpret_cast<const char*>(&jpeg_buffer[m_current_pos]), block_size);
				if (SendUdpMessage(data_packet) != sf::Socket::Done)
				{
					std::cout << "SendUpdate(const u8* data, int row_stride, int width, int height, bool saveAlpha, bool frombgra) failed to send." << std::endl;
				}
				
				m_current_pos += block_size;
			}
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
			if (SendUdpMessage(packet) != sf::Socket::Done)
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
			if (SendUdpMessage(packet) != sf::Socket::Done)
			{
				std::cout << "SendUpdate(32 m_device_number, GCPadStatus &pad_status) failed to send." << std::endl;
			}
			else
			{
									std::cout << "Sent to " << sending_address.toString() << " Port " << mPort << std::endl;
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

	int OutputComm::SendUdpMessage(sf::Packet &packet)
	{
		return mUdpSocket.send(packet, sending_address, mPort);
	}

}
