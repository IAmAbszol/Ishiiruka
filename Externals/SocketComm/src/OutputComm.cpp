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
    if(m_type == OutputType::SLIPPI_BACKEND)
    {
        mPort = SLIPPI_PORT;
    }
    else if (m_type == OutputType::VIDEO_FRONTEND)
    {
        mPort = VIDEO_PORT;
    }
    else
    {
        PanicAlertT("ERROR! Invalid output type selected.");
        exit(1);
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
        std::string data(json_message.begin(), json_message.end());
        if(!SendMessage(data))
        {
            std::cout << "SendUpdate(std::vector<u8> &json_message) failed to send." << std::endl;
        }
    }
}

bool OutputComm::SendMessage(std::string message)
{
    sf::Packet packet;
    packet << message;
    return (mSenderSocket.send(packet, sending_address, mPort) == sf::Socket::Done);
}

}
