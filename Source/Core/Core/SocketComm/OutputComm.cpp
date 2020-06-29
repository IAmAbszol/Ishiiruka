#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>

#include "Core/SocketComm/OutputComm.hpp"

#include "Common/MsgHandler.h"
#include "Core/ConfigManager.h"

using json = nlohmann::json;

namespace SocketComm
{

OutputComm::OutputComm(OutputType m_type)
    : sending_address(IP_ADDRESS)
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
		if (mSegments > 1)
		{
			std::cout << "WARNING! mSegments > 1 causes the jpeg reconstruction to fail. Still not 100% why but buffer "
			             "out == buffer in. Wack"
			          << std::endl;
		}
		mPort = VIDEO_PORT;
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
	if (mProcessingThread.joinable())
	{
		mProcessingThread.join();
	}
}

std::tuple<uint32_t, uint32_t> OutputComm::GetTimeSinceEpoch()
{
	uint64_t current_time =
	    std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	double seconds = 0;
	uint32_t microseconds = (uint32_t)(std::modf((double)(current_time / pow(10, 6)), &seconds) * pow(10, 6));
	
	return std::make_tuple((uint32_t)seconds, (uint32_t)microseconds);
}

const bool OutputComm::IsReady() const
{
	return mConnected & !mProcessingVideo;
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

void OutputComm::ProcessVideo(const u8 *data, int row_stride, int width, int height, bool saveAlpha, bool frombgra)
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
	unsigned long outbuffer_size = 0;
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
		const u8 *row_ptr = data + y * row_stride;
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
		if (jpeg_cinfo.next_scanline >= jpeg_cinfo.image_height)
		{
			std::cout << "SendUpdate(const u8* data, int row_stride, int width, int height, bool saveAlpha, bool "
			             "frombgra) JPEG Conversion Error, height out of bounds."
			          << std::endl;
			return;
		}
		jpeg_row_pointer[0] = (uint8_t *)buffer.data();
		jpeg_write_scanlines(&jpeg_cinfo, jpeg_row_pointer, 1);
	}
	jpeg_finish_compress(&jpeg_cinfo);
	jpeg_destroy_compress(&jpeg_cinfo);

	uint32_t reduced_outbuffer_size = static_cast<uint32_t>(outbuffer_size);
	uint32_t m_current_pos = 0;
	uint32_t block_size = reduced_outbuffer_size / mSegments;
	// WARNING! mSegments > 1 causes the jpeg reconstruction to fail. Still not 100% why but buffer out == buffer
	// in. Wack.
	for (uint8_t segment = 0; segment < mSegments; segment++)
	{
		if ((m_current_pos + block_size) > reduced_outbuffer_size)
		{
			block_size -= ((m_current_pos + block_size) - reduced_outbuffer_size);
		}
		// Total header bytes = 30
		data_packet.clear();
		auto current_time = GetTimeSinceEpoch();
		data_packet << mFrameCount;
		data_packet << segment;
		data_packet << mSegments;
		data_packet << width;
		data_packet << height;
		data_packet << block_size;
		data_packet << reduced_outbuffer_size;
		data_packet << std::get<0>(current_time);
		data_packet << std::get<1>(current_time);
		data_packet.append(reinterpret_cast<const char *>(&jpeg_buffer[m_current_pos]), block_size);
		if (SendUdpMessage(data_packet) != sf::Socket::Done)
		{
			std::cout << "SendUpdate(const u8* data, int row_stride, int width, int height, bool saveAlpha, bool "
			             "frombgra) failed to send."
			          << std::endl;
		}

		m_current_pos += block_size;
	}
	buffer.clear();
	data_packet.clear();
	free(jpeg_buffer);
	jpeg_buffer = NULL;

	mProcessingVideo = false;
}

void OutputComm::SendUpdate(const u8 *data, int row_stride, int width, int height, bool saveAlpha, bool frombgra)
{
	if (mConnected & !mProcessingVideo)
	{
		std::lock_guard<std::mutex> lock(mLock);
		mProcessingVideo = true;
		if (mProcessingThread.joinable())
		{
			mProcessingThread.join();
		}
		mProcessingThread =
		    std::thread(&OutputComm::ProcessVideo, this, data, row_stride, width, height, saveAlpha, frombgra);
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
		auto current_time = GetTimeSinceEpoch();
		packet << std::get<0>(current_time);
		packet << std::get<1>(current_time);
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
		auto current_time = GetTimeSinceEpoch();
		sf::Packet packet;
		packet.append(&m_device_number, sizeof(m_device_number));
		packet.append(&std::get<0>(current_time), sizeof(uint32_t));
		packet.append(&std::get<1>(current_time), sizeof(uint32_t));
		packet.append(&pad_status, sizeof(pad_status));
		if (SendUdpMessage(packet) != sf::Socket::Done)
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

int OutputComm::SendUdpMessage(sf::Packet &packet)
{
	return mUdpSocket.send(packet, sending_address, mPort);
}

} // namespace SocketComm
