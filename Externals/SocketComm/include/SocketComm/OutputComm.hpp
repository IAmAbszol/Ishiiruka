/*
 * Enables input communication from external server to Dolphin
 * controller interface.
 */

#pragma once

#include <cstdint>
#include <mutex>
#include <thread>
#include <tuple>

#include "Common/CommonTypes.h"
#include "SFML/Network.hpp"
#include <json.hpp>

#define DEFAULT_PORT 55080

using json = nlohmann::json;

namespace SocketComm
{
    class OutputComm
    {
    public:

        /**
         * OutputComm
         */
        OutputComm();

        /** 
         * ~OutputComm
         * Deconstructor
         */
        ~OutputComm();

        /** 
         * SendUpdate
         * @param json_message message update provided by Slippi.
         */
        static void SendUpdate(std::vector<u8> &json_message);
        /**
         * TODO: Create second function for SendUpdate to have (JSON string update, frame data);
         * - Figure out data to Python PNG, or some type of format Python may use.
         * 1. Each update received will be placed into a heap, sorted by time.
         * 2. When polled, the two oldest should be grabbed. This might be swapped to newest but 
         *    the number of items in each list will become an issue at that point. Same with old.
         * 3. Send the pairing to output.
         * 4. Frequency of these are equal to the input coming back through.
         */
    protected:
    /**
     * HandleClients
     */
    void HandleClients();
    /**
     * UpdateClient
     */
    void UpdateClient();
    
    /** Rate at which the outputs should occur at. */
    uint64_t tick_rate = 50;
    /** Time difference to allow for correlation. */
    uint64_t sync_difference = 5;
    /** Current socket status */
    bool mConnected;
    /** Sending socket for the UDP outgoing updates */
    sf::UdpSocket mSenderSocket;
    /** Sending thread to house the server on */
    std::thread mSenderThread;
    /** Connecting clients thread **/
    std::thread mClientsThread;
    /** Vector of tuple'd pair (Address, Port) of clients */
    std::vector<std::tuple<std::string, uint16_t>> mConnectedClients;
    /** Mutex lock for accessing thread */
    std::mutex mLock;
    };
}
