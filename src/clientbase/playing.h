/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * main game loop stuff
 */

#ifndef BZF_PLAYING_H
#define BZF_PLAYING_H

#include "common.h"

// system includes
#include <string>
#include <vector>

// common headers
#include "DirectoryNames.h"
#include "FileManager.h"
#include "ServerLink.h"
#include "StartupInfo.h"
#include "WordFilter.h"
#include "bz_md5.h"

// common client headers
#include "FlashClock.h"
#include "RobotPlayer.h"
#include "WorldBuilder.h"
#include "WorldDownLoader.h"

#include "motd.h"

// FIXME: The following should not be in the clientbase
#include "ControlPanel.h"
#include "CommandCompleter.h"
#include "ThirdPersonVars.h"


class NetMessage;

void warnAboutMainFlags();
void warnAboutRadarFlags();
void warnAboutRadar();
void warnAboutConsole();

void handleFlagDropped(Player* tank);
void setTarget();
bool shouldGrabMouse();

void setSceneDatabase();

void selectNextRecipient(bool forward, bool robotIn);

extern const float FlagHelpDuration;
extern CommandCompleter completer;
extern ThirdPersonVars thirdPersonVars;
// END FIXME

// FIXME: Any code surrounded by "if (!headless)" is unsafely assuming that
// it's operating in a context where graphics and sound are available.
extern bool   headless;

#define MAX_DT_LIMIT 0.1f
#define MIN_DT_LIMIT 0.001f


typedef void (*JoinGameCallback)(bool success, void* data);
typedef void (*ConnectStatusCallback)(std::string& str);
typedef void (*PlayingCallback)(void*);
typedef void (*ChangePlayerTeamCallback)(Player*, TeamColor oldTeam,
                                         TeamColor newTeam);

struct PlayingCallbackItem {
  public:
    PlayingCallback cb;
    void*   data;
};

void showError(const char* msg, bool flush = false);
void showMessage(const std::string& line);
void showMessage(const std::string& line, ControlPanel::MessageModes mode);

int curlProgressFunc(void* clientp,  double dltotal, double dlnow,  double ultotal, double ulnow);

bool gotBlowedUp(BaseLocalPlayer* tank, BlowedUpReason reason, PlayerId killer,
                 const ShotPath* hit = NULL, int physicsDriver = -1);

void suicide(int sig);
void hangup(int sig);

void joinInternetGame(const struct in_addr* inAddress);
void handlePendingJoins();
bool checkSquishKill(const Player* victim, const Player* killer, bool localKiller = false);
bool dnsLookupDone(struct in_addr& inAddress);
void setTankFlags();
void updateShots(const float dt);
void enteringServer(void* buf);
void updateNumPlayers();
void joinInternetGame2();

extern void setChangePlayerTeamCallback(ChangePlayerTeamCallback);

#ifdef ROBOT
void addObstacle(std::vector<BzfRegion*> &rgnList, const Obstacle& obstacle);
void makeObstacleList();
void setRobotTarget(RobotPlayer* robot);
void updateRobots(float dt);
void sendRobotUpdates();
void addRobots();
#endif

void initGlobalAres();
void killGlobalAres();

void doNetworkStuff();

StartupInfo* getStartupInfo();

void initPlaying();
void startPlaying();

void addPlayingCallback(PlayingCallback, void* data);
void removePlayingCallback(PlayingCallback, void* data);
void callPlayingCallbacks();

void updateEvents();

void joinGame(JoinGameCallback, void* userData);
extern void joinGame();
void leaveGame();

void handleAddPlayer(void* msg, bool& checkScores);
void handleAdminInfo(void* msg);
void handleAliveMessage(void* msg);
void handleAllowSpawn(uint16_t len, void* msg);
void handleAllow(void* msg);
void handleAccept(void* msg);
void handleAutoPilot(void* msg);
void handleCacheURL(void* msg, uint16_t len);
void handleCaptureFlag(void* msg, bool& checkScores);
void handleCustomSound(void* msg);
void handleDropFlag(void* msg);
void handleFlagNegotiation(void* msg, uint16_t len);
void handleFlagTransferred(Player* from, Player* to, int flagIndex, ShotType);
void handleFlagType(void* msg);
void handleFlagUpdate(void* msg, size_t len);
void handleGameSettings(void* msg);
void handleGameTime(void* msg);
void handleGetWorld(void* msg, uint16_t len);
void handleQueryGL(void* msg);
void handleQueryOS(void* msg);
void handleGMUpdate(void* msg);
void handleGrabFlag(void* msg);
void handleHandicap(void* msg);
void handleJoinServer(void* msg);
void handleKilledMessage(void* msg, bool human, bool& checkScores);
void handleLimboMessage(void* msg);
void handleMessage(void* msg);
void handlePlayerUpdate(uint16_t code, void* msg);
void handleMsgSetVars(void* msg);
void handleNearFlag(void* msg);
void handleNewPlayer(void* msg);
void handleNewRabbit(void* msg);
void handlePlayerData(void* msg);
void handlePlayerInfo(void* msg);
void handleRejectMessage(void* msg);
void handleRemovePlayer(void* msg, bool& checkScores);
void handleReplayReset(void* msg, bool& checkScores);
void handleResourceFetch(void* msg);
void handleScoreOver(void* msg);
void handleScore(void* msg);
void handleSetShotType(void* msg);
void handleSetTeam(void* msg, uint16_t len);
void handleShotBegin(bool human, void* msg);
void handleShotEnd(void* msg);
void handleSuperKill(void* msg);
void handleTangReset();
void handleTangUpdate(uint16_t len, void* msg);
void handleTeamUpdate(void* msg, bool& checkScores);
void handleTeleport(void* msg);
void handleTimeUpdate(void* msg);
void handleTransferFlag(void* msg);
void handleWantHash(void* msg, uint16_t len);
void handleWShotBegin(void* msg);

void updateHighScores();

void addMessage(const Player* player,
                const std::string& msg,
                ControlPanel::MessageModes mode = ControlPanel::MessageMisc,
                bool highlight = false,
                const char* oldColor = NULL);

void doMessages();

extern StartupInfo startupInfo;
extern FlashClock pulse;
extern ServerLink* serverLink;
extern WordFilter* wordFilter;

extern WorldBuilder* worldBuilder;
extern WorldDownLoader* worldDownLoader;
extern MessageOfTheDay* motd;
extern PlayerId msgDestination;

extern bool downloadingData;

extern bool serverDied;
extern bool joinRequested;
extern bool waitingDNS;
extern bool serverError;
extern bool entered;
extern bool joiningGame;

extern double epochOffset;
extern double lastEpochOffset;
extern double userTimeEpochOffset;

extern bool justJoined;
extern bool canSpawn;
extern bool gameOver;

extern int numFlags;

extern float clockAdjust;

extern bool  pausedByUnmap;
extern bool  pauseWaiting;
extern float pauseCountdown;

extern float destructCountdown;

extern const char* blowedUpMessage[];


#endif // BZF_PLAYING_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab expandtab
