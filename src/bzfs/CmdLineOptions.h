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

#ifndef __CMDLINEOPTIONS_H__
#define __CMDLINEOPTIONS_H__

/* system headers */
#include <string>
#include <map>

/* bzflag common headers */
#include "config.h"
#include "common.h"
#include "Protocol.h"
#include "Flag.h"
#include "WordFilter.h"

/* bzfs-specific headers */
#include "AccessControlList.h"
#include "FlagInfo.h"

/* XXX -- nasty header to include just for a copyright */
#include "bzfs.h"


/* constants provided for general consumption */
const int MaxPlayers = 200;
const int MaxShots = 10;

extern const char *usageString;
extern const char *extraUsageString;

// rabbit selection algorithms
enum RabbitSelection {
  ScoreRabbitSelection,		// default method based on score
  KillerRabbitSelection,  	// anoint whoever manages to kill the rabbit
  RandomRabbitSelection		// pick the new rabbit out of a hat
};

typedef std::map<FlagType*, int> FlagNumberMap;
typedef std::map<FlagType*,bool> FlagOptionMap;

/** CmdLineOptions is a container for any of the bzfs options that may
 * be provided via the command line.
 */
struct CmdLineOptions
{
  CmdLineOptions()
  : wksPort(ServerPort), gameStyle(PlainGameStyle), rabbitSelection(ScoreRabbitSelection), servermsg(NULL),
    advertisemsg(NULL), worldFile(NULL), pingInterface(NULL),
    listServerURL(DefaultListServerURL), password(NULL),
    publicizedTitle(""), publicizedAddress(""),
    maxShots(1), maxTeamScore(0), maxPlayerScore(0),
    numExtraFlags(0), teamKillerKickRatio(0),
    numAllowedFlags(0), shakeWins(0), shakeTimeout(0),
    teamFlagTimeout(30), maxlagwarn(10000), lagwarnthresh(-1.0),
    idlekickthresh(-1.0), timeLimit(0.0f), timeElapsed(0.0f),
    linearAcceleration(0.0f), angularAcceleration(0.0f), useGivenPort(false),
    useFallbackPort(false), requireUDP(false), randomBoxes(false),
    randomCTF(false), flagsOnBuildings(false), respawnOnBuildings(false),
    oneGameOnly(false), timeManualStart(false), randomHeights(false),
    useTeleporters(false), teamKillerDies(true), printScore(false),
    publicizeServer(false), filterFilename(""),
    filterCallsigns(false), filterChat(false), filterSimple(false), voteTime(60),
    vetoTime(20), votesRequired(3), votePercentage(50.1f),
    voteRepeatTime(300), autoTeam(false), citySize(5)
  {
    int i;
    for (FlagTypeMap::iterator it = FlagType::getFlagMap().begin();
	 it != FlagType::getFlagMap().end(); ++it) {
	flagCount[it->second] = 0;
	flagLimit[it->second] = -1;
	flagDisallowed[it->second] = false;
    }

    for (i = 0; i < NumTeams; i++) {
      maxTeam[i] = MaxPlayers;
      numTeamFlags[i] = 0;
    }
  }

  int			wksPort;
  int			gameStyle;
  int			rabbitSelection;

  const char		*servermsg;
  const char		*advertisemsg;
  const char		*worldFile;
  const char		*pingInterface;
  const char		*listServerURL;
  char			*password;

  std::string		publicizedTitle;
  std::string		publicizedAddress;

  uint16_t		maxShots;
  int			maxTeamScore;
  int			maxPlayerScore;
  int			numExtraFlags;
  int			teamKillerKickRatio; // if players tk*100/wins > teamKillerKickRatio -> kicked
  int			numAllowedFlags;
  uint16_t		shakeWins;
  uint16_t		shakeTimeout;
  int			teamFlagTimeout;
  int			maxlagwarn;

  float			lagwarnthresh;
  float			idlekickthresh;
  float			timeLimit;
  float			timeElapsed;
  float			linearAcceleration;
  float			angularAcceleration;

  bool			useGivenPort;
  bool			useFallbackPort;
  bool			requireUDP; // true if only new clients allowed
  bool			randomBoxes;
  bool			randomCTF;
  bool			flagsOnBuildings;
  bool			respawnOnBuildings;
  bool			oneGameOnly;
  bool			timeManualStart;
  bool			randomHeights;
  bool			useTeleporters;
  bool			teamKillerDies;
  bool			printScore;
  bool			publicizeServer;

  uint16_t		maxTeam[NumTeams];
  FlagNumberMap		flagCount;
  FlagNumberMap		flagLimit; // # shots allowed / flag
  FlagOptionMap		flagDisallowed;

  AccessControlList	acl;
  TextChunkManager	textChunker;

  /* inappropriate language filter */
  std::string		filterFilename;
  bool			filterCallsigns;
  bool			filterChat;
  bool			filterSimple;
  WordFilter		filter;

  /* vote poll options */
  unsigned short int voteTime;
  unsigned short int vetoTime;
  unsigned short int votesRequired;
  float votePercentage;
  unsigned short int voteRepeatTime;

  std::string		reportFile;
  std::string		reportPipe;

  std::string		bzdbVars;

  /* team balancing options */
  bool			autoTeam;

  /* city options */
  int			citySize;
  int			numTeamFlags[NumTeams];
};


void printVersion();
void usage(const char *pname);
void extraUsage(const char *pname);

char **parseConfFile( const char *file, int &ac);
void parse(int argc, char **argv, CmdLineOptions &options);


#else
struct CmdLineOptions;
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

