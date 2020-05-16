#include <chrono>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>

#include "Core/SocketComm/InputComm.hpp"
#include "Common/MsgHandler.h"
#include <json.hpp>

using json = nlohmann::json;

namespace SocketComm
{
InputComm::InputComm(u32 device_number) : 
                            mOutputComm(OutputType::CONTROLLER_BACKEND),
                            mDeviceNumber(device_number),
                            mPort(DEFAULT_PORT + device_number)
{
    if(mPort)
    {
        // Binding failed, panic! 
        if(mListenerSocket.bind(mPort) == sf::Socket::Done)
        {
            mListenerSocket.setBlocking(false);
            mConnected = true;
            mListenerThread = std::thread(&InputComm::ReadSocket, this);
            return;
        }
        else
        {
            PanicAlertT("Listener socket failed to initialize!");
            exit(EXIT_FAILURE);
        }
    }
}

InputComm::~InputComm()
{
    mListenerSocket.unbind();
    if(mConnected)
    {
        mConnected = false;
        mListenerThread.join();
    }
}

void InputComm::ReadSocket()
{
    while(mConnected)
    {
        char receiving_size[1024];
        std::size_t received;
        sf::IpAddress sender;
        unsigned short senderPort;
        if (mListenerSocket.receive(receiving_size, sizeof(receiving_size), received, sender, senderPort) == sf::Socket::Done)
        {
            try 
            {
                std::string json_string(receiving_size, received);
		        json json_body = json::parse(json_string);
                GCPadStatus pad;
                pad.button = json_body["button"].get<u16>();
                pad.stickX = json_body["stickX"].get<u8>();
                pad.stickY = json_body["stickY"].get<u8>();
                pad.substickX = json_body["substickX"].get<u8>();
                pad.substickY = json_body["substickY"].get<u8>();
                pad.triggerLeft = json_body["triggerLeft"].get<u8>();
                pad.triggerRight = json_body["triggerRight"].get<u8>();
                pad.analogA = json_body["analogA"].get<u8>();
                pad.analogB = json_body["analogB"].get<u8>();
                pad.err = json_body["err"].get<u8>();
                mPadBuffer.insert(mPadBuffer.begin(), pad);
            } 
            catch(std::exception const& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool InputComm::GetUpdate(GCPadStatus &pad_status)
{
    std::lock_guard<std::mutex> lock(mLock);
    mOutputComm.SendUpdate(mDeviceNumber, pad_status);
    if(mPadBuffer.size() > 0)
    {
        pad_status = mPadBuffer.back();
        mPadBuffer.pop_back();
        return true;
    }
    return false; 
}

}
