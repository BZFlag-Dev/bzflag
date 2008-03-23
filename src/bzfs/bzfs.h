/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
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
#include <list>

// must be before windows.h
#include "network.h"

// common interface headers
#include "Address.h"
#include "Flag.h"
#include "Ping.h"

// bzfs specific headers
#include "TeamBases.h"
#include "CmdLineOptions.h"
#include "GameKeeper.h"
#include "FlagInfo.h"
#include "WorldInfo.h"
#include "RejoinList.h"
#include "VotingArbiter.h"

#include "bzfsAPI.h"


class PendingChatMessages
{
public:
  int to;
  int from;
  std::string text;

  PendingChatMessages ( int t, int f, const std::string &m )
  {
    to = t;
    from = f;
    text = m;
  }

  PendingChatMessages ( int t, int f, const char* m )
  {
    to = t;
    from = f;
    if (m)
      text = m;
  }

  PendingChatMessages ( const PendingChatMessages &m )
  {
    to = m.to;
    from = m.from;
    text = m.text;
  }
};

extern std::list<PendingChatMessages> pendingChatMessages;

// to dump score info to stdout
#define SERVERLOGINMSG true


extern void sendMessage(int	 playerIndex,
			PlayerId    dstPlayer,
			const char *message);
extern void removePlayer(int	 playerIndex,
			 const char *reason,
			 bool	notify = true);
extern void sendPlayerMessage(GameKeeper::Player *playerData,
			      PlayerId dstPlayer,
			      const char *message);
extern char *getDirectMessageBuffer();
extern void  pausePlayer(int playerIndex, bool paused);
extern void  sendFlagUpdate(FlagInfo &flag);
extern void  dropFlag(FlagInfo &flag);
extern void  sendIPUpdate(int targetPlayer = -1, int playerIndex = -1);
extern void  sendPlayerInfo(void);
extern int   directMessage( NetHandler *handler, uint16_t code, int len, const void *msg );

NetHandler *getPlayerNetHandler ( int playerIndex );

extern int   getCurMaxPlayers();
extern bool  areFoes(TeamColor team1, TeamColor team2);
extern PingPacket getTeamCounts();
extern void  zapFlagByPlayer(int playerIndex);
extern void  resetFlag(FlagInfo &flag);
extern void  dropFlag(FlagInfo& flag, const float dropPos[3]);
extern void  publicize();
extern TeamColor  whoseBase(float x, float y, float z);
extern void checkGameOn ( void );
extern PlayerId getNewPlayerID ( void );
PlayerId getNewPlayer(NetHandler *netHandler);
extern bool validPlayerCallsign ( int playerIndex );
extern void addPlayer(int playerIndex, GameKeeper::Player *playerData);
bool updatePlayerState ( GameKeeper::Player *playerData, PlayerState &state, float timeStamp, bool shortState );
extern void rejectPlayer(int playerIndex, uint16_t code, const char *reason);
extern bool worldStateChanging ( void );
extern void searchFlag(GameKeeper::Player &playerData);
extern void relayPlayerPacket(int index, uint16_t len, const void *rawbuf, uint16_t code);
extern void playerAlive(int playerIndex);
extern int  lookupPlayer(const PlayerId& id);
extern void playerKilled(int victimIndex, int killerIndex, BlowedUpReason reason, int16_t shotIndex, const FlagType *flagType, int phydrv, bool respawnOnBase = false);
extern void doSpawns();
extern void dropPlayerFlag(GameKeeper::Player &playerData, const float dropPos[3]);
extern void captureFlag(int playerIndex, TeamColor teamCaptured);
extern bool invalidPlayerAction(PlayerInfo &p, int t, const char *action);
extern bool allowTeams ( void );
extern void addBzfsCallback(const std::string& name, void* data);
extern void pauseCountdown ( const char *pausedBy );
extern void resumeCountdown ( const char *resumedBy );
extern void resetTeamScores ( void );
extern void startCountdown ( int delay, float limit, const char *buyWho );
bool defineWorld ( void );
bool saveWorldCache ( const char* fileName = NULL );
void rescanForBans ( bool isOperator = true, const char* callsign = NULL, int playerID = -1 );
void zapFlag(FlagInfo &flag);
void anointNewRabbit( int killerId = NoPlayer);

extern int bz_pwrite(NetHandler *handler, const void *b, int l);
extern void pwriteBroadcast(const void *b, int l, int mask);


void lagKick(int playerIndex);
void jitterKick(int playerIndex);
void packetLossKick(int playerIndex);

void processCollision ( GameKeeper::Player *player, GameKeeper::Player *otherPlayer, float pos[3] );

typedef struct _CheatProtectionOptions
{
  bool doHeightChecks;
  bool doSpeedChecks;
  _CheatProtectionOptions()
  {
    doHeightChecks = true;
    doSpeedChecks = true;
  }
} CheatProtectionOptions;
extern CheatProtectionOptions	cheatProtectionOptions;

// initialize permission groups
extern void initGroups();

extern BasesList	bases;
extern CmdLineOptions	*clOptions;
extern uint16_t		curMaxPlayers;
extern bool		done;
extern bool		gameOver;
extern TeamInfo		team[NumTeams];
extern int		numFlags;
extern bool		countdownActive;
extern int		countdownDelay;
extern TimeKeeper	countdownPauseStart;
extern int		countdownResumeTime;
extern char		hexDigest[50];
extern WorldInfo	*world;
extern char		*worldDatabase;
extern uint32_t		worldDatabaseSize;
extern char		*worldSettings;
extern uint8_t		rabbitIndex;
extern float		speedTolerance;
extern bool		handlePings;
extern uint16_t		maxPlayers;
extern uint16_t		maxRealPlayers;
extern float		pluginWorldSize;
extern float		pluginWorldHeight;
extern std::map<std::string,float> APIWaitTimes;
extern bool		isIdentifyFlagIn;
extern bool		worldWasSentToAPlayer;
extern RejoinList	rejoinList;
extern bool		publiclyDisconnected;
extern VotingArbiter	*votingArbiter;
extern PingPacket	pingReply;
extern Address		serverAddress;

// FIXME - this should not be hangin out here
extern bool dontWait;
extern float maxWorldHeight;

extern unsigned int maxNonPlayerDataChunk;

class NonPlayerDataChunk
{
  public:
  NonPlayerDataChunk()
  {
    data = NULL;
    size = 0;
  }

  NonPlayerDataChunk( const char* d, unsigned int s )
  {
    if (s == 0)
      data = NULL;
   data = (char*)malloc(s);
    memcpy(data,d,s);
    size = s;
  }

  NonPlayerDataChunk( const NonPlayerDataChunk &t )
  {
    if (t.size == 0)
      data = NULL;
    else
      data = (char*)malloc(t.size);
    memcpy(data,t.data,t.size);
    size = t.size;
  }

  ~NonPlayerDataChunk()
  {
    if (data && size>0)
      free (data);

    data = NULL;
    size = 0;
  }

  char *data;
  unsigned int size;
};

// peer list
typedef struct
{
  int socket;
  int player;
  NetHandler *handler;
  std::vector<bz_NonPlayerConnectionHandler*> notifyList;

  std::vector<NonPlayerDataChunk> pendingSendChunks;

  double    startTime;
  bool	    sent;
  bool	    deleteMe;
}NetConnectedPeer;

extern std::map<int,NetConnectedPeer> netConnectedPeers;

void sendBufferedNetDataForPeer (NetConnectedPeer &peer );

#endif /* __BZFS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
