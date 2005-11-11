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

#ifndef __CMDLINEOPTIONS_H__
#define __CMDLINEOPTIONS_H__

/* bzflag special common - 1st one */
#include "common.h"

/* system headers */
#include <string>
#include <map>
#include <vector>

/* bzflag common headers */
#include "Protocol.h"
#include "Flag.h"
#include "WordFilter.h"

/* bzfs-specific headers */
#include "AccessControlList.h"
#include "TextChunkManager.h"

// avoid dependencies
class EntryZones;

/* constants provided for general consumption */
const int MaxPlayers = 200;
const int MaxShots = 20;

// rabbit selection algorithms
enum RabbitSelection {
  ScoreRabbitSelection,		// default method based on score
  KillerRabbitSelection,	// anoint whoever manages to kill the rabbit
  RandomRabbitSelection		// pick the new rabbit out of a hat
};

typedef std::map<FlagType*, int> FlagNumberMap;
typedef std::map<FlagType*,bool> FlagOptionMap;

#define _DEFAULT_LIN_ACCEL 0.0f
#define _DEFAULT_ANGLE_ACCELL 0.0f

/** CmdLineOptions is a container for any of the bzfs options that may
 * be provided via the command line.
 */
struct CmdLineOptions
{
  CmdLineOptions()
  : wksPort(ServerPort), gameStyle(PlainGameStyle),
    rabbitSelection(ScoreRabbitSelection), msgTimer(0), spamWarnMax(5),
    servermsg(""), advertisemsg(""), worldFile(""),
    pingInterface(""), password(""),
    listServerOverridden(false), publicizedTitle(""), publicizedAddress(""),
    advertiseGroups("EVERYONE"),
    suppressMasterBanList(false), masterBanListOverridden(false),
    maxShots(1), maxTeamScore(0), maxPlayerScore(0),
    numExtraFlags(0), teamKillerKickRatio(0),
    numAllowedFlags(0), shakeWins(0), shakeTimeout(0),
    teamFlagTimeout(30), maxlagwarn(10000), lagwarnthresh(-1.0),
    idlekickthresh(-1.0), timeLimit(0.0f), timeElapsed(0.0f),
    linearAcceleration(_DEFAULT_LIN_ACCEL), angularAcceleration(_DEFAULT_ANGLE_ACCELL), useGivenPort(false),
    useFallbackPort(false), requireUDP(false), randomBoxes(false),
    randomCTF(false), flagsOnBuildings(false), respawnOnBuildings(false),
    oneGameOnly(false), timeManualStart(false), randomHeights(false),
    useTeleporters(false), teamKillerDies(true), printScore(false),
    publicizeServer(false), replayServer(false), startRecording(false),
    timestampLog(false), timestampMicros(false),
    filterFilename(""), filterCallsigns(false), filterChat(false), filterSimple(false),
    banTime(300), voteTime(60), vetoTime(2), votesRequired(2),
    votePercentage(50.1f), voteRepeatTime(300),
    autoTeam(false), citySize(5), cacheURL(""), cacheOut("")
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

    listServerURL.push_back(DefaultListServerURL);
    masterBanListURL.push_back(DefaultMasterBanURL);
  }

  int			wksPort;
  int			gameStyle;
  int			rabbitSelection;
  int			msgTimer;
  int			spamWarnMax;

  std::string   servermsg;
  std::string   advertisemsg;
  std::string   worldFile;
  std::string   pingInterface;
  std::string   password;

  bool listServerOverridden;
  std::vector<std::string>   listServerURL;

  std::string	publicizedTitle;
  std::string	publicizedAddress;
  std::string	advertiseGroups;

  bool			suppressMasterBanList;
  bool			masterBanListOverridden;
  std::vector<std::string>		masterBanListURL;

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
  bool			requireUDP;
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
  bool			replayServer;
  bool			startRecording;
  bool			timestampLog;
  bool			timestampMicros;
	bool			countdownPaused;

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
  unsigned short int banTime;
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

  std::string	   cacheURL;
  std::string	   cacheOut;

  // plugins
  typedef struct
  {
	  std::string plugin;
	  std::string command;
  }pluginDef;

  std::vector<pluginDef>	pluginList;
};


void parse(int argc, char **argv, CmdLineOptions &options, bool fromWorldFile = false);
void finalizeParsing(int argc, char **argv, CmdLineOptions &options, EntryZones& ez);
bool checkCommaList (const char *list, int maxlen);

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
