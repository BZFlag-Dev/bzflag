/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __BZFS_H__
#define __BZFS_H__

static const char copyright[] = "Copyright (c) 1993 - 2003 Tim Riker";

// to enforce a game time limit
#define TIMELIMIT
// to dump score info to stdout
#define PRINTSCORE to include code to dump score info to stdout

// Like verbose debug messages?
#define DEBUG1 if (clOptions->debug >= 1) printf
#define DEBUG2 if (clOptions->debug >= 2) printf
#define DEBUG3 if (clOptions->debug >= 3) printf
#define DEBUG4 if (clOptions->debug >= 4) printf

#define SERVERLOGINMSG true

#if defined(__sgi)
#define FD_SETSIZE (MaxPlayers + 10)
#endif /* defined(__sgi) */

#ifdef _WIN32
#pragma warning( 4 : 4786 )
#endif

// must be before network.h because that defines a close() macro which
// messes up fstreams.	luckily, we don't need to call the close() method
// on any fstream.
#include <fstream>

// must be before windows.h
#include "network.h"
#include <iomanip>

#if defined(_WIN32)
  #pragma warning(disable: 4786)
#endif


#if defined(_WIN32)
#include <windows.h>
#define popen _popen
#define pclose _pclose
#define sleep(_x) Sleep(1000 * (_x))
#endif /* defined(_WIN32) */

#include <stdio.h>
#if !defined(_WIN32)
#include <fcntl.h>
#endif

#include <string>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "bzsignal.h"
#include <time.h>
#include "common.h"
#include "global.h"
#include "Protocol.h"
#include "Address.h"
#include "Pack.h"
#include "PlayerState.h"
#include "TimeKeeper.h"
#include "Flag.h"
#include "Team.h"
#include "Ping.h"
#include "multicast.h"
#include "TimeBomb.h"
#include "md5.h"
#include "ShotUpdate.h"
#include "WordFilter.h"
#include "ConfigFileManager.h"
#include "CommandManager.h"
#include "VotingBooth.h"

/* bzfs class specific headers */
#include "TextChunkManager.h"
#include "AccessControlList.h"
#include "CmdLineOptions.h"
#include "WorldInfo.h"
#include "Permissions.h"
#include "WorldWeapons.h"
#include "FlagInfo.h"


#endif
// ex: shiftwidth=2 tabstop=8
