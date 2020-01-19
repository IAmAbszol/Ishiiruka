/*
 * Enables input communication from external server to Dolphin
 * controller interface.
 */

#pragma once

#include <mutex>
#include <thread>

#include "InputCommon/GCPadStatus.h"

#include "Common/CommonTypes.h"
#include "SFML/Network.hpp"

namespace SocketComm
{
    class InputComm
    {
    public:

        /**
         * InputComm
         * @param mPort port to listen on for incoming messages.
         */
        InputComm(uint32_t device_number, uint16_t port);

        /** 
         * ~InputComm
         * Deconstructor
         */
        ~InputComm();

        /** 
         * GetUpdate
         * @param pad status update
         * @return true if an update was found
         */
        bool GetUpdate(GCPadStatus &pad_status);

    protected:

        void ReadSocket();

        /** TODO Remove and check status directly with mListenerSocket */
        bool mConnected;
        /** Stream of controller inputs */
        std::vector<GCPadStatus> mPadBuffer;
        /** Listener socket for the UDP incoming requests */
        sf::UdpSocket mListenerSocket;
        /** Listener thread to house the server on */
        std::thread mListenerThread;
        /** Mutex lock for accessing thread */
        std::mutex mLock;
        /** Device number of the controller associated with this comm */
        u32 mDeviceNumber;
        /** Port associated with the comm, will be incremented based on mDeviceNumber to avoid conflicts */
        u16 mPort;
    };
}