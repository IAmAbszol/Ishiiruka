#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <chrono>

#include "SocketComm/OutputComm.hpp"

#include "Common/MsgHandler.h"
#include <json.hpp>

using json = nlohmann::json;

namespace SocketComm
{
OutputComm::OutputComm(OutputType m_type) :
    sending_address(IP_ADDRESS)
{
    switch(m_type)
    {
        case OutputType::CONTROLLER_BACKEND:
            mPort = CONTROLLER_PORT;
            break;
        
        case OutputType::SLIPPI_BACKEND:
            mPort = SLIPPI_PORT;
            break;

        case OutputType::VIDEO_FRONTEND:
            mPort = VIDEO_PORT;
            break;
    }
    mSenderSocket.setBlocking(false);
    mConnected = true;
}

OutputComm::~OutputComm()
{
    mSenderSocket.unbind();
    if(mConnected)
    {
        mConnected = false;
    }
}

uint64_t OutputComm::GetTimeSinceEpoch()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        mClock.now().time_since_epoch()).count();
}

void OutputComm::SendUpdate(std::vector<u8> &json_message)
{
    if(mConnected)
    {
        sf::Packet packet;
        // TODO: Make this more stream lined
        std::string data(json_message.begin(), json_message.end());
        packet << data;
        if(!SendMessage(packet))
        {
            std::cout << "SendUpdate(std::vector<u8> &json_message) failed to send." << std::endl;
        }
    }
}

void OutputComm::SendUpdate(GCPadStatus &pad_status)
{
    sf::Packet packet;
    packet.append(&pad_status, sizeof(pad_status));
    if(!SendMessage(packet))
    {
        std::cout << "SendUpdate(GCPadStatus &pad_status) failed to send." << std::endl;
    }
}

bool OutputComm::SendMessage(sf::Packet &packet)
{
    return (mSenderSocket.send(packet, sending_address, mPort) == sf::Socket::Done);
}

}
