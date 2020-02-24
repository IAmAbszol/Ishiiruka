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
    json slippi_data = json::parse(json_message.begin(), json_message.end());
    slippi_data["received_time"] = GetTimeSinceEpoch();
    if(!SendMessage(slippi_data.dump()))
    {
        std::cout << "SendUpdate(std::vector<u8> &json_message) failed to send." << std::endl;
    }
}

bool OutputComm::SendMessage(std::string message)
{
    return (mSenderSocket.send(message.c_str(), sizeof(message.c_str()), sending_address, mPort) == sf::Socket::Done);
}

/*
void OutputComm::UpdateClients()
{
    while(mConnected)
    {
        sf::IpAddress sending_address("127.0.0.1");

        std::vector<std::uint8_t> v = {'t', 'r', 'u', 'e'};
        json j = json::parse(v.begin(), v.end());
        std::string data = j.dump();
        const char * c = data.c_str();
        if(mSenderSocket.send(c, sizeof(c), sending_address, DEFAULT_PORT) != sf::Socket::Done)
        {
            std::cout << "ERROR! Send to client failed!" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(mTickRate));
    }
}
*/
}
