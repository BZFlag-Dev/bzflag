/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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

// bzfs specific headers
#include "TeamBases.h"
#include "CmdLineOptions.h"
#include "GameKeeper.h"
#include "FlagInfo.h"

extern void sendMessage(int         playerIndex,
			PlayerId    dstPlayer,
			const char *message);
extern void removePlayer(int         playerIndex, 
			 const char *reason,
			 bool        notify = true);
extern void playerKilled(int             victimIndex,
			 int             killerIndex,
			 int             reason,
			 int16_t         shotIndex,
			 const FlagType *flagType,
			 int             phydrv,
			 bool            respawnOnBase = false);
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

// initialize permission groups
extern void initGroups();

extern BasesList bases;
extern CmdLineOptions *clOptions;
extern uint16_t        curMaxPlayers;
extern bool            done;
extern bool            gameOver;
extern TeamInfo        team[NumTeams];
extern int             numFlags;
extern bool            countdownActive;
extern int             countdownDelay;
extern TimeKeeper      countdownPauseStart;

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

