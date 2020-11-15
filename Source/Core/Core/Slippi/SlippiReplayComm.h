#pragma once

#include <SlippiGame.h>
#include <string>
#include <queue>

#include <json.hpp>
using json = nlohmann::json;

class SlippiReplayComm
{
  public:
	typedef struct {
		std::string path;
		int startFrame = Slippi::GAME_FIRST_FRAME;
		int endFrame = INT_MAX;
		std::string gameStartAt = "";
		std::string gameStation = "";
	} WatchSettings;

	// Loaded file contents
	typedef struct {
		std::string mode;
		std::string replayPath;
		int startFrame = Slippi::GAME_FIRST_FRAME;
		int endFrame = INT_MAX;
		bool outputOverlayFiles;
		bool isRealTimeMode;
		std::string commandId;
		std::queue<WatchSettings> queue;
	} CommSettings;

	SlippiReplayComm();
	~SlippiReplayComm();

	WatchSettings current;

	CommSettings getSettings();
	void nextReplay();
	bool isNewReplay();
	Slippi::SlippiGame *loadGame();

  private:
	void loadFile();
	std::string getReplayPath();

	std::string configFilePath;
	json fileData;
	std::string previousReplayLoaded;
	std::string previousCommandId;

	// Queue stuff
	bool isFirstLoad = true;
	bool provideNew = false;
	int queuePos = 0;

	CommSettings commFileSettings;
};
