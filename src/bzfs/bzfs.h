/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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

static const char copyright[] = "Copyright (c) 1993 - 2004 Tim Riker";

#ifdef _MSC_VER
#pragma warning( 4 : 4786 )
#endif

// to dump score info to stdout
#define PRINTSCORE to include code to dump score info to stdout

#define SERVERLOGINMSG true


// must be before network.h because that defines a close() macro which
// messes up fstreams.	luckily, we don't need to call the close() method
// on any fstream.
#include <fstream>

// must be before windows.h
#include "network.h"
#include <iomanip>

#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#define sleep(_x) Sleep(1000 * (_x))
#endif /* defined(_WIN32) */

#include <stdio.h>
#if !defined(_WIN32)
#include <fcntl.h>
#endif

// system headers
#include <assert.h>
#include <string>
#include <map>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "bzsignal.h"
#include <time.h>

// common-interface headers
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
#include "ConfigFileManager.h"
#include "CommandManager.h"
#include "BZDBCache.h"
#include "TextUtils.h"
#include "common.h"

/* bzfs class-specific headers */
#include "version.h"
#include "commands.h"
#include "TextChunkManager.h"
#include "AccessControlList.h"
#include "CmdLineOptions.h"
#include "WorldInfo.h"
#include "Permissions.h"
#include "WorldWeapons.h"
#include "FlagInfo.h"
#include "VotingArbiter.h"
#include "PlayerInfo.h"
#include "PackVars.h"
#include "TeamBases.h"
#include "ListServerConnection.h"
#include "BZWReader.h"

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

