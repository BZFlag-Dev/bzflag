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
#ifndef __BZFS_H__
#define __BZFS_H__

#include "common.h"

// system headers
// fstream must be before network.h because that defines a close()
// macro which messes up fstreams. luckily, we don't need to call the
// close() method on any fstream.
#include <fstream>
#include <string>
#include <vector>
#include <list>

// must be before windows.h
#include "net/network.h"

// common interface headers
#include "net/Address.h"
#include "common/BzTime.h"
#include "game/Flag.h"
#include "net/Ping.h"
#include "vectors.h"

// bzfs specific headers
#include "TeamBases.h"
#include "CmdLineOptions.h"
#include "GameKeeper.h"
#include "FlagInfo.h"
#include "WorldInfo.h"
#include "RejoinList.h"
#include "game/VotingArbiter.h"

#include "bzfs/bzfsAPI.h"


// to dump score info to stdout
#define SERVERLOGINMSG true

extern const std::string& getPublicOwner();
extern void setPublicOwner(const std::string& owner);

extern void sendMessage(int  playerIndex,
                        PlayerId    dstPlayer,
                        const char* message,
                        uint8_t    type = ChatMessage);
extern void removePlayer(int   playerIndex,
                         const char* reason,
                         bool notify = true);
extern void sendPlayerMessage(GameKeeper::Player* playerData,
                              PlayerId dstPlayer,
                              const char* message);

extern void sendChatMessage(PlayerId srcPlayer, PlayerId dstPlayer, const char* message, uint8_t type = ChatMessage);
extern void pausePlayer(int playerIndex, bool paused);
extern void sendFlagUpdate(FlagInfo& flag);
extern void dropFlag(FlagInfo& flag);
extern void sendIPUpdate(int targetPlayer = -1, int playerIndex = -1);
extern void sendPlayerInfo();

extern NetHandler* getPlayerNetHandler(int playerIndex);

extern int  getCurMaxPlayers();
extern PingPacket getTeamCounts();
extern void zapFlagByPlayer(int playerIndex);
extern void resetFlag(FlagInfo& flag);
extern void dropFlag(FlagInfo& flag, const fvec3& dropPos);
extern void publicize();
extern TeamColor  whoseBase(float x, float y, float z);
extern void checkGameOn();
extern PlayerId getNewPlayerID();
extern PlayerId getNewPlayer(NetHandler* netHandler);
extern PlayerId getNewBot(PlayerId hostPlayer, int botID);
extern bool validPlayerCallsign(int playerIndex);
extern void addPlayer(int playerIndex, GameKeeper::Player* playerData);
extern bool updatePlayerState(GameKeeper::Player* playerData, PlayerState& state, BzTime const& timeStamp, bool shortState);
extern void rejectPlayer(int playerIndex, uint16_t code, const char* reason);
extern bool worldStateChanging();
extern void searchFlag(GameKeeper::Player& playerData);
extern void playerAlive(int playerIndex);
extern int  lookupPlayer(const PlayerId& id);
extern void playerKilled(int victimIndex, int killerIndex, BlowedUpReason reason, int16_t shotIndex, const FlagType* flagType, int phydrv, bool respawnOnBase = false);
extern void dropPlayerFlag(GameKeeper::Player& playerData, const fvec3& dropPos);
extern void captureFlag(int playerIndex, TeamColor teamCaptured);
extern bool invalidPlayerAction(PlayerInfo& p, int t, const char* action);
extern bool allowTeams();
extern void pauseCountdown(const char* pausedBy);
extern void resumeCountdown(const char* resumedBy);
extern void resetTeamScores();
extern void startCountdown(int delay, float limit, const char* buyWho);
extern bool defineWorld();
extern bool saveWorldCache(const char* fileName = NULL);
extern void rescanForBans(const char* callsign = NULL, int playerID = -1);
extern void zapFlag(FlagInfo& flag);
extern void anointNewRabbit(int killerId = NoPlayer);
extern void spawnPlayer(int playerIndex);

extern void lagKick(int playerIndex);
extern void jitterKick(int playerIndex);
extern void packetLossKick(int playerIndex);

typedef struct _CheatProtectionOptions {
  bool doHeightChecks;
  bool doSpeedChecks;
  _CheatProtectionOptions() {
    doHeightChecks = true;
    doSpeedChecks = true;
  }
} CheatProtectionOptions;
extern CheatProtectionOptions cheatProtectionOptions;

// initialize permission groups
extern void initGroups();

extern BasesList  bases;
extern CmdLineOptions* clOptions;
extern uint16_t   curMaxPlayers;
extern bool   serverDone;
extern bool   gameOver;
extern BzTime   gameStartTime;
extern TeamInfo   teamInfos[NumTeams];
extern int    numFlags;
extern bool   countdownActive;
extern int    countdownDelay;
extern BzTime countdownPauseStart;
extern int    countdownResumeTime;
extern char   hexDigest[50];
extern WorldInfo*  world;
extern char*   worldDatabase;
extern uint32_t   worldDatabaseSize;
extern char   worldSettings[WorldSettingsSize];
extern uint8_t    rabbitIndex;
extern float    speedTolerance;
extern bool   handlePings;
extern uint16_t   maxPlayers;
extern uint16_t   maxRealPlayers;
extern float    pluginWorldSize;
extern float    pluginWorldHeight;
extern std::map<std::string, float> APIWaitTimes;
extern bool   isIdentifyFlagIn;
extern bool   worldWasSentToAPlayer;
extern RejoinList rejoinList;
extern bool   publiclyDisconnected;
extern VotingArbiter*  votingArbiter;
extern PingPacket pingReply;
extern Address    serverAddress;

// FIXME - this should not be hangin out here
extern bool dontWait;
extern float maxWorldHeight;

extern unsigned int maxNonPlayerDataChunk;

// peer list
struct NetConnectedPeer {
  int socket;
  int player;

  NetHandler*                    netHandler;
  bz_NonPlayerConnectionHandler* apiHandler;

  std::list<std::string> sendChunks;

  BzTime startTime;
  bool   sent;
  bool   deleteMe;
};

extern std::map<int, NetConnectedPeer> netConnectedPeers;

extern void sendBufferedNetDataForPeer(NetConnectedPeer& peer);

#endif /* __BZFS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
