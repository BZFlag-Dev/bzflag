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

#ifdef _MSC_VER
#pragma warning( 4 : 4786 )
#endif

// to dump score info to stdout
#define PRINTSCORE to include code to dump score info to stdout

#define SERVERLOGINMSG true

#include "common.h"

// must be before network.h because that defines a close() macro which
// messes up fstreams.	luckily, we don't need to call the close() method
// on any fstream.
#include <fstream>

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
#include "VotingArbiter.h"

#include <list>
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

extern void sendMessage(int	 playerIndex,
			PlayerId    dstPlayer,
			const char *message);
extern void removePlayer(int	 playerIndex,
			 const char *reason,
			 bool	notify = true);
extern void playerKilled(int	     victimIndex,
			 int	     killerIndex,
			 int	     reason,
			 int16_t	 shotIndex,
			 const FlagType *flagType,
			 int	     phydrv,
			 bool	    respawnOnBase = false);
extern void doSpawns();
extern void sendPlayerMessage(GameKeeper::Player *playerData,
			      PlayerId dstPlayer,
			      const char *message);
extern char *getDirectMessageBuffer();
extern void  broadcastMessage(uint16_t code, int len, const void *msg);
extern void  sendTeamUpdate(int playerIndex = -1,
			    int teamIndex1 = -1,
			    int teamIndex2 = -1);
extern void  sendFlagUpdate(FlagInfo &flag);
extern void  sendDrop(FlagInfo &flag);
extern void  sendIPUpdate(int targetPlayer = -1, int playerIndex = -1);
extern void  sendPlayerInfo(void);
extern void  directMessage(int playerIndex, uint16_t code,
			   int len, const void *msg);
extern int   getCurMaxPlayers();
extern bool  areFoes(TeamColor team1, TeamColor team2);
extern PingPacket getTeamCounts();
extern void       zapFlagByPlayer(int playerIndex);
extern void       resetFlag(FlagInfo &flag);
extern void       dropFlag(FlagInfo& flag, const float dropPos[3]);
extern void       publicize();
extern TeamColor  whoseBase(float x, float y, float z);
bool defineWorld ( void );
bool saveWorldCache( const char* file = NULL );

void rescanForBans ( bool isOperator = true, const char* callsign = NULL, int playerID = -1 );

// initialize permission groups
extern void initGroups();

extern BasesList bases;
extern CmdLineOptions *clOptions;
extern uint16_t	curMaxPlayers;
extern bool	    done;
extern bool	    gameOver;
extern TeamInfo	team[NumTeams];
extern int	     numFlags;
extern bool	    countdownActive;
extern int	     countdownDelay;
extern TimeKeeper      countdownPauseStart;
extern int	     countdownResumeDelay;
extern char	    hexDigest[50];
extern WorldInfo      *world;
extern char	   *worldDatabase;
extern uint32_t	worldDatabaseSize;
extern char	    worldSettings[4 + WorldSettingsSize];
extern uint8_t	 rabbitIndex;
extern float	   speedTolerance;
extern bool	    handlePings;
extern uint16_t	maxPlayers;
extern uint16_t	maxRealPlayers;
extern float	   pluginWorldSize;
extern float	   pluginWorldHeight;
extern float	   pluginMaxWait;

extern bool		  publiclyDisconected;

extern VotingArbiter *votingarbiter;

void pauseCountdown ( const char *pausedBy );
void resumeCountdown ( const char *resumedBy );
void resetTeamScores ( void );
void startCountdown ( int delay, float limit, const char *buyWho );

void makeWalls ( void );
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

