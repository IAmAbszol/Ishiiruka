// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <SlippiGame.h>
#include <string>
#include <unordered_map>
#include <deque>
#include <ctime>

#include <Core/SocketComm/OutputComm.hpp>

#include "Common/CommonTypes.h"
#include "Common/FileUtil.h"
#include "Core/HW/EXI_Device.h"
#include "Core/Slippi/SlippiReplayComm.h"

// Acts
class CEXISlippi : public IEXIDevice
{
public:
	CEXISlippi();
  	virtual ~CEXISlippi();

	void DMAWrite(u32 _uAddr, u32 _uSize) override;
	void DMARead(u32 addr, u32 size) override;

	bool IsPresent() const override;

private:
	enum {
		CMD_UNKNOWN = 0x0,
		CMD_RECEIVE_COMMANDS = 0x35,
		CMD_RECEIVE_GAME_INFO = 0x36,
		CMD_RECEIVE_POST_FRAME_UPDATE = 0x38,
		CMD_RECEIVE_GAME_END = 0x39,
		CMD_PREPARE_REPLAY = 0x75,
		CMD_READ_FRAME = 0x76,
		CMD_GET_LOCATION = 0x77,
		CMD_IS_FILE_READY = 0x88,
		CMD_IS_STOCK_STEAL = 0x89,
		CMD_GET_FRAME_COUNT = 0x90,
	};

	enum {
		FRAME_RESP_WAIT = 0,
		FRAME_RESP_CONTINUE = 1,
		FRAME_RESP_TERMINATE = 2,
		FRAME_RESP_FASTFORWARD = 3,
	};

	std::unordered_map<u8, u32> payloadSizes = {
		// The actual size of this command will be sent in one byte
		// after the command is received. The other receive command IDs
		// and sizes will be received immediately following
		{ CMD_RECEIVE_COMMANDS, 1},

		// The following are all commands used to play back a replay and
		// have fixed sizes
		{ CMD_PREPARE_REPLAY, 0 },
		{ CMD_READ_FRAME, 4 },
		{ CMD_IS_STOCK_STEAL, 5 },
		{ CMD_GET_LOCATION, 6 },
		{ CMD_IS_FILE_READY, 0 },
		{ CMD_GET_FRAME_COUNT, 0 }
	};

	// Communication with Launcher
	SlippiReplayComm* replayComm;

	// .slp File creation stuff
	u32 writtenByteCount = 0;

	// vars for metadata generation
	time_t gameStartTime;
	int32_t lastFrame;
	std::unordered_map<u8, std::unordered_map<u8, u32>> characterUsage;

	void updateMetadataFields(u8* payload, u32 length);
	void configureCommands(u8* payload, u8 length);
	void writeToFile(u8* payload, u32 length, std::string fileOption);
	std::vector<u8> generateMetadata();
	void createNewFile();
	void closeFile();
	std::string generateFileName();
	bool checkFrameFullyFetched(int32_t frameIndex);
	bool shouldFFWFrame(int32_t frameIndex);

	//std::ofstream log;

	File::IOFile m_file;
	std::vector<u8> m_payload;

	// replay playback stuff
	void prepareGameInfo();
	void prepareCharacterFrameData(int32_t frameIndex, u8 port, u8 isFollower);
	void prepareFrameData(u8* payload);
	void prepareIsStockSteal(u8 *payload);
	void prepareFrameCount();
	void prepareIsFileReady();

	std::unordered_map<u8, std::string> getNetplayNames();

	bool isSoftFFW = false;
	bool isHardFFW = false;
	int32_t lastFFWFrame = INT_MIN;
	std::vector<u8> m_read_queue;
	Slippi::SlippiGame* m_current_game = nullptr;
	SocketComm::OutputComm m_output_comm;

protected:
	void TransferByte(u8& byte) override;
};
