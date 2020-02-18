#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "SocketComm/OutputComm.hpp"

#include "Common/MsgHandler.h"
#include <json.hpp>

using json = nlohmann::json;

namespace SocketComm
{
OutputComm::OutputComm()
{
    if(mSenderSocket.bind(DEFAULT_PORT) == sf::Socket::Done)
    {
        mSenderSocket.setBlocking(false);
        mConnected = true;
        mSenderThread = std::thread(&OutputComm::UpdateClient, this);
        return;
    }
    else
    {
        PanicAlertT("Sender socket failed to initialize!");
        exit(EXIT_FAILURE);
    }
    
}

OutputComm::~OutputComm()
{
    mSenderSocket.unbind();
    if(mConnected)
    {
        mConnected = false;
        mSenderThread.join();
    }
}

void OutputComm::SendUpdate(std::vector<u8> &json_message)
{
    /*
    - Get current timestamp
    - Store timestamp into some type of PQ
    */
}

void OutputComm::UpdateClient()
{
    while(mConnected)
    {
        char in[1028];
        std::size_t received;
        sf::IpAddress sending_address("192.168.1.7");
        unsigned short sender_port = DEFAULT_PORT;
        /*
        if (mSenderSocket.receive(in, sizeof(in), received, sending_address, sender_port) == sf::Socket::Done)
        {
            std::string data;
            for(const auto &piece : in)
            {
                data += piece;
            }
            std::cout << data << std::endl;
        }
        */
        char out[] = "Hello";
        if(mSenderSocket.send(out, sizeof(out), sending_address, sender_port))
        {
            std::cout << "SEND IT" << std::endl;
        }
    }
}

}
