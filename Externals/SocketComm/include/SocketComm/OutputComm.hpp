/*
 * Enables input communication from external server to Dolphin
 * controller interface.
 */

#pragma once

#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>

#include "Common/CommonTypes.h"
#include "InputCommon/GCPadStatus.h"
#include "SFML/Network.hpp"
#include <json.hpp>

#define CONTROLLER_PORT 55079
#define SLIPPI_PORT 55080
#define VIDEO_PORT 55081
#define IP_ADDRESS "127.0.0.1"

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
    void SendUpdate(GCPadStatus &pad_status);
        
    protected:
    /**
     * GetTimeSinceEpoch
     */
    uint64_t GetTimeSinceEpoch();
    /**
     * SendMessage
     * @param packet data message to send to local socket.
     * @return true if message sent.
     */
    bool SendMessage(sf::Packet &packet);

    /** Port to send on, either Slippi or Video */
    uint16_t mPort = SLIPPI_PORT;
    /** Address to send on */
    sf::IpAddress sending_address;
    /** Clock */
    std::chrono::high_resolution_clock mClock;
    /** Current socket status */
    bool mConnected;
    /** Sending socket for the UDP outgoing updates. */
    sf::UdpSocket mSenderSocket;
    /** Mutex lock for accessing thread. */
    std::mutex mLock;
    };
}
