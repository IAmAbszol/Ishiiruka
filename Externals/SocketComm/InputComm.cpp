#include <chrono>
#include <iostream>
#include <stdlib.h>

#include "InputComm.hpp"
#include "Common/MsgHandler.h"
#include "yaml-cpp/yaml.h"

namespace SocketComm
{
InputComm::InputComm(u32 device_number, u16 port) : 
                            mDeviceNumber(device_number),
                            mPort(port + device_number)
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
    if(mConnected)
    {
        mConnected = false;
        mListenerSocket.unbind();
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
                YAML::Node json_body = YAML::Load(json_string);
                // TODO: Update pads to use CommonTypes.h
                // TODO: Evaluate slippi and view what parts of the struct are used.
                // TODO:Update SI_DEVICE to poll from the buffer.
                GCPadStatus pad;
                pad.button = json_body["button"].as<u16>();
                pad.stickX = (u8) json_body["stickX"].as<u16>();
                pad.stickY = (u8) json_body["stickY"].as<u16>();
                pad.substickX = (u8) json_body["substickX"].as<u16>();
                pad.substickY = (u8) json_body["substickY"].as<u16>();
                pad.triggerLeft = (u8) json_body["triggerLeft"].as<u16>();
                pad.triggerRight = (u8) json_body["triggerRight"].as<u16>();
                pad.analogA = (u8) json_body["analogA"].as<u16>();
                pad.analogB = (u8) json_body["analogB"].as<u16>();
                pad.err = (u8) json_body["err"].as<int16_t>();
                mPadBuffer.insert(mPadBuffer.begin(), pad);
                
            } 
            catch(std::exception const& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

bool InputComm::GetUpdate(GCPadStatus &pad_status)
{
    std::lock_guard<std::mutex> lock(mLock);
    if(mPadBuffer.size() > 0)
    {
        pad_status = mPadBuffer.back();
        mPadBuffer.pop_back();
        return true;
    }
    return false; 
}

}