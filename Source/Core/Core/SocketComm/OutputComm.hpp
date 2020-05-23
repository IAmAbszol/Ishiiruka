/*
 * Enables input communication from external server to Dolphin
 * controller interface.
 */

#pragma once

#include <cstdint>
#include <jpeglib.h>
#include <json.hpp>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>

#include "Common/CommonTypes.h"
#include "InputCommon/GCPadStatus.h"
#include "SFML/Network.hpp"

using json = nlohmann::json;

// TODO: Create a better schema rather than falling into the static trap for
//       updating the queue's.
namespace SocketComm
{
	enum class OutputType
	{
		CONTROLLER_BACKEND,
		SLIPPI_BACKEND,
		VIDEO_FRONTEND
	};

	/** Designated ports per interface. */
	static const uint16_t 		CONTROLLER_PORT 	= 55079;
	static const uint16_t 		SLIPPI_PORT 		= 55080;
	static const uint16_t 		VIDEO_PORT 			= 55081;
	/** Defaulted IP address for this client. */
	static const std::string 	IP_ADDRESS 			= "127.0.0.1";
	/** JPEG image quality being sent over (100 = 215KB, 25 = 31KB). */
	static const uint8_t		JPEG_QUALITY		= 25;

	class OutputComm
	{
	public:

		/**
		 * OutputComm
		 */
		OutputComm(OutputType m_type);

		/**
		 * ~OutputComm
		 * Deconstructor
		 */
		~OutputComm();

		/**
		 * IsConnected
		 * @return true if connected
		 */
		const bool IsConnected() const;

		/**
		  * HandleConnect
		  **/
		void HandleConnect();

		/**
		 * SendUpdate
		 * @copydoc TextureToPng
		 */
		void SendUpdate(const u8* data, int row_stride, int width,
			int height, bool saveAlpha, bool frombgra);

		/**
		 * SendUpdate
		 * @param json_message message update provided by Slippi.
		 */
		void SendUpdate(std::vector<u8> &json_message);

		/**
		 * SendUpdate
		 * @param pad_status gamecube pad status struct
		 */
		void SendUpdate(u32 m_device_number, GCPadStatus &pad_status);

	protected:
		/**
		 * GetTimeSinceEpoch
		 */
		uint64_t GetTimeSinceEpoch();
		/**
		 * SendTcpMessage
		 * @param packet data message to send to local socket.
		 * @return sf::SocketStatus enum value.
		 */
		int SendTcpMessage(sf::Packet &packet);
		/**
		 * SendUdpMessage
		 * @param packet data message to send to local socket.
		 * @return sf::SocketStatus enum value.
		 */
		int SendUdpMessage(sf::Packet &packet);

		/** Number of segments */
		uint16_t mSegments = 1; 
		/** Number of frames that have passed */
		uint32_t mFrameCount = 0;
		/** Port to send on, either Slippi or Video */
		uint16_t mPort = SLIPPI_PORT;
		/** Address to send on */
		sf::IpAddress sending_address;
		/** Clock */
		std::chrono::high_resolution_clock mClock;
		/** Current socket status */
		bool mConnected = false;
		/** Handle socket connection status */
		bool mHandleConnections = true;
		/** Sending socket for the UDP out going updates. */
		sf::UdpSocket mUdpSocket;
		/** Sending socket for the TCP out going updates. */
		sf::TcpListener mListenerSocket;
		/** Sending socket for the TCP out going updates. */
		sf::TcpSocket mClientSocket;
		/** Tcp listening socket thread, rip boost post. */
		std::thread mHandleConnectThread;
		/** Mutex lock for accessing thread. */
		std::mutex mLock;
	};
}
