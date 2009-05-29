/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

#ifndef	BZF_PLAYING_H
#define	BZF_PLAYING_H

#include "common.h"

// system includes
#include <string>
#include <vector>

// common headers
#include "ServerLink.h"
#include "StartupInfo.h"
#include "WordFilter.h"


// FIXME: These headers should not be needed in the clientbase
#include "CommandCompleter.h"
#include "ControlPanel.h"
#include "HUDRenderer.h"
#include "ThirdPersonVars.h"
// END FIXME

#define MAX_DT_LIMIT 0.1f
#define MIN_DT_LIMIT 0.001f


typedef void (*JoinGameCallback)(bool success, void* data);
typedef void (*ConnectStatusCallback)(std::string& str);
typedef void (*PlayingCallback)(void*);

struct PlayingCallbackItem {
  public:
    PlayingCallback	cb;
    void*		data;
};


// FIXME: These should not need to be linked in the clientbase
//        note: most of these are from clientCommands and shot strategies
void warnAboutMainFlags();
void warnAboutRadarFlags();
void warnAboutRadar();
void warnAboutConsole();

void handleFlagDropped(Player* tank);
void setTarget();
bool shouldGrabMouse();

bool addExplosion(const fvec3& pos, float size, float duration);
void addTankExplosion(const fvec3& pos);
void addShotExplosion(const fvec3& pos);
void addShotPuff(const fvec3& pos, const fvec3& vel);

void setSceneDatabase();

void selectNextRecipient(bool forward, bool robotIn);
// END FIXME


void showError(const char *msg, bool flush = false);
void showMessage(const std::string& line);
void showMessage(const std::string& line, ControlPanel::MessageModes mode);

StartupInfo* getStartupInfo();

void initPlaying();
void startPlaying();

void addPlayingCallback(PlayingCallback, void* data);
void removePlayingCallback(PlayingCallback, void* data);

void updateEvents();

void joinGame(JoinGameCallback, void* userData);
extern void joinGame();
void leaveGame();

void handleSetShotType(BufferedNetworkMessage *msg);
void handleWhatTimeIsIt(void *msg);
void handleNearFlag(void *msg);
void handleSetTeam(void *msg, uint16_t len);
void handleResourceFetch(void *msg);
void handleCustomSound(void *msg);
void handleJoinServer(void *msg);
void handleSuperKill(void *msg);
void handleRejectMessage(void *msg);
void handleFlagNegotiation(void *msg, uint16_t len);
void handleFlagType(void *msg);
void handleGameSettings(void *msg);
void handleCacheURL(void *msg, uint16_t len);
void handleWantHash(void *msg, uint16_t len);
void handleGetWorld(void *msg, uint16_t len);
void handleGameTime(void *msg);
void handleTimeUpdate(void *msg);
void handleScoreOver(void *msg);
void handleAddPlayer(void *msg, bool &checkScores);
void handleRemovePlayer(void *msg, bool &checkScores);
void handleFlagUpdate(void *msg, size_t len);
void handleTeamUpdate(void *msg, bool &checkScores);
void handleAliveMessage(void *msg);
void handleAutoPilot(void *msg);
void handleAllow(void *msg);
void handleKilledMessage(void *msg, bool human, bool &checkScores);
void handleGrabFlag(void *msg);
void handleDropFlag(void *msg);
void handleCaptureFlag(void *msg, bool &checkScores);
void handleNewRabbit(void *msg);
void handleShotBegin(bool human, void *msg);
void handleWShotBegin(void *msg);
void handleShotEnd(void *msg);
void handleHandicap(void *msg);
void handleScore(void *msg);
void handleMsgSetVars(void *msg);
void handleTeleport(void *msg);
void handleTransferFlag(void *msg);
void handleMessage(void *msg);
void handleReplayReset(void *msg, bool &checkScores);
void handleAdminInfo(void *msg);
void handlePlayerInfo(void *msg);
void handleNewPlayer(void *msg);
void handleMovementUpdate(uint16_t code, void *msg);
void handleGMUpdate(void *msg);
void handleTangUpdate(uint16_t len, void *msg);
void handleTangReset();
void handleAllowSpawn(uint16_t len, void *msg);
void handleLimboMessage(void *msg);
void handlePlayerData(void *msg);

void updateHighScores();

void addMessage(const Player* player,
                const std::string& msg,
                ControlPanel::MessageModes mode = ControlPanel::MessageMisc,
                bool highlight = false,
                const char* oldColor = NULL);

bool handleServerMessage(bool human, BufferedNetworkMessage *msg);
void handleServerMessage(bool human, uint16_t code, uint16_t len, void *msg);


// FIXME: Any code surrounded by "if (!headless)" is unsafely assuming that
// it's operating in a context where graphics and sound are available.
extern bool		headless;


// FIXME: Variables like 'hud' should not need to be linked in the client base
//        note: many of these are linked only in clientCommands.cxx
extern StartupInfo startupInfo;
extern BzfDisplay* display;
extern ControlPanel* controlPanel;
extern HUDRenderer* hud;
extern MainWindow* mainWindow;
extern CommandCompleter	completer;
extern ThirdPersonVars thirdPersonVars;
extern float roamDZoom;
extern int savedVolume;
extern bool fireButton;
extern bool roamButton;
// End FIXME

extern ServerLink *serverLink;
extern WordFilter* wordFilter;
extern PlayerId msgDestination;

extern bool canSpawn;
extern bool gameOver;

extern int numFlags;

extern float clockAdjust;
extern bool pausedByUnmap;
extern float pauseCountdown;
extern float destructCountdown;

extern std::string customLimboMessage;


#endif // BZF_PLAYING_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
