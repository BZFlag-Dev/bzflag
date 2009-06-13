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

#ifdef _MSC_VER
#  pragma warning(4:4786)
#endif

/* this should be the only header necessary except for headers specific
 * to the class implementation (such as version.h)
 */
#include "CmdLineOptions.h"

/* system implementation headers */
#include <iostream>
#include <vector>
#include <set>

// for -pidfile option
#include <sys/types.h>
#ifdef HAVE_PROCESS_H
#  include <process.h>
#  include "time.h"
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

/* common implementation headers */
#include "version.h"
#include "Team.h"
#include "TextUtils.h"
#include "BZDBCache.h"
#include "BzMaterial.h"

/* local implementation headers */
#include "bzfs.h"
#include "bzfsPlugins.h"
#include "RecordReplay.h"
#include "BZWError.h"
#include "Permissions.h"
#include "EntryZones.h"
#include "SpawnPolicyFactory.h"


static const char *usageString =
  "[-admsg <text>] "
  "[-advertise <group,group...>] "
  "[-autoTeam] "
  "[-b] "
  "[-badwords <filename>] "
  "[-ban ip{,ip}*] "
  "[-banfile <filename>] "
  "[-botsPerIP <num>] "
  "[-c] "
  "[-cache <url prefix>] "
  "[-cacheout <filename>] "
  "[-conf <filename>] "
  "[-cr] "
  "[-d] "
  "[-density <num>] "
  "[-disableBots] "
  "[+f {good|<id>}] "
  "[-f {bad|<id>}] "
  "[-fb] "
  "[-filterAnnounce] "
  "[-filterCallsigns] "
  "[-filterChat] "
  "[-filterSimple] "
  "[-freezeTag] "
  "[-g] "
  "[-gndtex <texture name>] "
  "[-groupdb <group file>] "
  "[-h] "
  "[-handicap] "
  "[-helpdir <dir>] "
  "[-helpmsg <file> <name>] "
  "[-i interface] "
  "[-j] "
  "[-jitterdrop <num>] "
  "[-jitterwarn <time/ms>] "
  "[-lagdrop <num>] "
  "[-lagwarn <time/ms>] "
  "[-loadplugin <pluginname,commandline>] "
  "[-masterBanURL <URL>] "
  "[-maxidle <time/s>] "
  "[-mp {<count>|[<count>][,<count>][,<count>][,<count>][,<count>][,<count>]}] "
  "[-mps <score>] "
  "[-ms <shots>] "
  "[-mts <score>] "
  "[-noMasterBanlist] "
  "[-noradar] "
  "[-offa] "
  "[-p <port>] "
  "[-packetlossdrop <num>] "
  "[-packetlosswarn <%>] "
  "[-passwd <password>] "
  "[-pidfile <filename>] "
  "[-poll <variable>=<value>] "
  "[-printscore] "
  "[-processor <processor-id>] "
  "[-public <server-description>] "
  "[-publicaddr <server-hostname>[:<server-port>]] "
  "[-publiclist <list-server-url>] "
  "[-q] "
  "[+r] "
  "[-rabbit [score|killer|random]] "
  "[-recbuf <Mbytes>] "
  "[-recbufonly] "
  "[-recdir <dirname>] "
  "[-replay] "
  "[-reportfile <filename>] "
  "[-reportpipe <filename>] "
  "[+s <flag-count>] "
  "[-s <flag-count>] "
  "[-sa] "
  "[-sb] "
  "[-set <name> <value>] "
  "[-setforced <name> <value>] "
  "[-sl <id> <num>] "
  "[-spamtime <time>] "
  "[-spamwarn <warnAmt>] "
  "[-spawnPolicy <policy>] "
  "[-speedtol <tolerance>] "
  "[-srvmsg <text>] "
  "[-st <time>] "
  "[-sw <num>] "
  "[-synctime] "
  "[-synclocation] "
  "[-t] "
  "[-tftimeout <seconds>] "
  "[-time {<seconds>|endTime}] "
  "[-timemanual] "
  "[-tk] "
  "[-tkannounce] "
  "[-tkkr <percent>] "
  "[-ts [micros]] "
  "[-userdb <user permissions file>] "
  "[-vars <filename>] "
  "[-version] "
  "[-world <filename>] "
  "[-worldsize <world size>] ";

static const char *extraUsageString =
  "\n"
  "BZFS Option Descriptions\n"
  "\n"
  "\t-admsg: specify a <msg> which will be broadcast every 15 minutes\n"
  "\t-advertise: specify which groups to advertise to (list server)\n"
  "\t-autoTeam: automatically assign players to teams when they join\n"
  "\t-b: randomly oriented buildings\n"
  "\t-badwords: bad-word file\n"
  "\t-ban ip{,ip}*: ban players based on ip address\n"
  "\t-banfile: specify a file to load and store the banlist in\n"
  "\t-botsPerIP: specify how many client-side bots are allowed per IP address\n"
  "\t-c: classic capture-the-flag style game,\n"
  "\t-cache: url to get binary formatted world\n"
  "\t-cacheout: generate a binary cache file\n"
  "\t-conf: configuration file\n"
  "\t-cr: capture-the-flag style game with random world\n"
  "\t-d: increase debugging level\n"
  "\t-density: specify building density for random worlds (default=5)\n"
  "\t-disableBots: disallow clients from using autopilot or robots\n"
  "\t+f: always have flag <id> available\n"
  "\t-f: never randomly generate flag <id>\n"
  "\t-fb: allow flags on box buildings\n"
  "\t-filterAnnounce: announces raw messages on the admin channel for filtered chat\n"
  "\t-filterCallsigns: filter callsigns to disallow inappropriate user names\n"
  "\t-filterChat: filter chat messages\n"
  "\t-filterSimple: perform simple exact matches with the bad word list\n"
  "\t-freezeTag: collisions freeze the player who is farther from home base\n"
  "\t-g: serve one game and then exit\n"
  "\t-gndtex: specify ground texture\n"
  "\t-groupdb: file to read for group permissions\n"
  "\t-h: use random building heights\n"
  "\t-handicap: give advantage based on relative playing ability\n"
  "\t-helpdir: add a helpmsg for every file in <dir>\n"
  "\t-helpmsg: show the lines in <file> on command /help <name>\n"
  "\t-i: listen on <interface>\n"
  "\t-j: allow jumping\n"
  "\t-lagdrop: drop player after this many lag warnings\n"
  "\t-lagwarn: lag warning threshhold time [ms]\n"
  "\t-jitterdrop: drop player after this many jitter warnings\n"
  "\t-jitterwarn: jitter warning threshhold time [ms]\n"
  "\t-loadplugin: load the specified plugin with the specified commandline\n"
  "\t-masterBanURL: URL to atempt to get the master ban list from <URL>\n"
  "\t-maxidle: idle kick threshhold [s]\n"
  "\t-mp: maximum players total or per team\n"
  "\t-mps: set player score limit on each game\n"
  "\t-ms: maximum simultaneous shots per player\n"
  "\t-mts: set team score limit on each game\n"
  "\t-noMasterBanlist: has public servers ignore the master ban list\n"
  "\t-noradar: disallow the use of radar\n"
  "\t-offa: teamless free-for-all game stye\n"
  "\t-p: use alternative port (default=5154)\n"
  "\t-packetlossdrop: drop player after this many packetloss warnings\n"
  "\t-packetlosswarn: packetloss warning threshold [%]\n"
  "\t-passwd: specify a <password> for operator commands\n"
  "\t-pidfile: write the process id into <filename> on startup\n"
  "\t-poll: configure several aspects of the in-game polling system\n"
  "\t-printscore: write score to stdout whenever it changes\n"
  "\t-processor: set cpu affinity to a particular processor\n"
  "\t-public <server-description>\n"
  "\t-publicaddr <effective-server-hostname>[:<effective-server-port>]\n"
  "\t-publiclist <list-server-url>\n"
  "\t-q: don't listen for or respond to pings\n"
  "\t+r: all shots ricochet\n"
  "\t-rabbit [score|killer|random]: rabbit chase style\n"
  "\t-recbuf <Mbytes>: start with a recording buffer of specified megabytes\n"
  "\t-recbufonly: disable recording directly to files\n"
  "\t-recdir <dirname>: specify the directory for recorded file\n"
  "\t-replay: setup the server to replay a previously saved game\n"
  "\t-reportfile <filename>: the file to store reports in\n"
  "\t-reportpipe <filename>: the program to pipe reports through\n"
  "\t+s: always have <num> super flags (default=16)\n"
  "\t-s: allow up to <num> super flags (default=16)\n"
  "\t-sa: insert antidote superflags\n"
  "\t-sb: allow tanks to respawn on buildings\n"
  "\t-set <name> <value>: set a BZDB variable's value\n"
  "\t-setforced <name> <value>: set a BZDB variable's value (whether it\n"
  "\t\texists or not)\n"
  "\t-sl: limit flag <id> to <num> shots\n"
  "\t-spamtime <time>: make <time> be the required time in seconds between\n"
  "\t\tmessages sent that are alike\n"
  "\t-spamwarn <warnAmt>: warn a spammer that sends messages before\n"
  "\t\tspamtime times out <warnAmt> many times\n"
  "\t-spawnPolicy specifies <policy> to use for spawning players\n"
  "\t-speedtol: multiplier of normal speed for auto kick (default=1.25)\n"
  "\t\tshould not be less than 1.0\n"
  "\t-srvmsg: specify a <msg> to print upon client login\n"
  "\t-st: shake bad flags in <time> seconds\n"
  "\t-sw: shake bad flags after <num> wins\n"
  "\t-synctime: synchronize time of day on all clients\n"
  "\t-synclocation: synchronize latitude and longitude on all clients\n"
  "\t-t: allow teleporters\n"
  "\t-tftimeout: set timeout for team flag zapping (default=30)\n"
  "\t-time: set time limit on each game in format of either seconds or ending time in x[x]:[xx:[xx]] format\n"
  "\t-timemanual: countdown for timed games is started with /countdown\n"
  "\t-tk: player does not die when killing a teammate\n"
  "\t-tkannounce: announces team kills to the admin channel\n"
  "\t-tkkr: team-kills-to-wins percentage (1-100) for kicking tk-ing players\n"
  "\t-ts [micros]: timestamp all console output, [micros] to include\n"
  "\t\tmicroseconds\n"
  "\t-userdb: file to read for user access permissions\n"
  "\t-vars: file to read for worlds configuration variables\n"
  "\t-version: print version and exit\n"
  "\t-world: world file to load\n"
  "\t-worldsize: numeric value for the size of the world (default=400)\n"
  "\n"
  "Poll Variables:  (see -poll)\n"
  "\n"
  "\tbanTime: number of minutes player should be banned (default=300)\n"
  "\tvetoTime: max seconds authorized user has to abort poll (default=20)\n"
  "\tvotePercentage: percentage of players required to affirm a poll\n"
  "\t\t(default=50.1%)\n"
  "\tvoteRepeatTime: minimum seconds required before a player may request\n"
  "\t\tanother vote (default=300)\n"
  "\tvotesRequired: minimum number of additional votes required to make a\n"
  "\t\tvote valid (default=2)\n"
  "\tvoteTime: maximum amount of time player has to vote, in seconds\n"
  "\t\t(default=60)\n"
  "\n";


// temp variables for carrying flag options from parse to finalize
static std::vector<std::string> storedFlagDisallows;
static std::vector<std::string> storedFlagCounts;
static std::map<std::string, int> storedFlagLimits;

// with +s all flags are required to exist all the time
static bool allFlagsOut = false;


//============================================================================//
/* private */

static void printVersion()
{
  std::cout << "BZFlag server " << getAppVersion()
            << " (protocol " << getProtocolVersion()
            << ") http://BZFlag.org/" << std::endl;
  std::cout << bzfcopyright << std::endl;
  std::cout.flush();
}


static void usage(const std::string& pname)
{
  printVersion();
  std::cerr << std::endl << "Usage: " << pname << ' ' << usageString << std::endl;
  exit(1);
}


static void extraUsage(const std::string& pname)
{
  char buffer[64];
  printVersion();
  std::cout << std::endl << "Usage: " << pname << ' ' << usageString << std::endl;
  std::cout << std::endl << extraUsageString << std::endl << "Flag codes:" << std::endl;
  const FlagTypeMap& flagMap = FlagType::getFlagMap();
  FlagTypeMap::const_iterator it;
  for (it = flagMap.begin(); it != flagMap.end(); ++it) {
    snprintf(buffer, 64, "\t%2.2s %s\n",
             (*it->second).flagAbbv.c_str(), (*it->second).flagName.c_str());
    std::cout << buffer;
  }
  exit(0);
}


static void checkFromWorldFile(const std::string& option, bool fromWorldFile)
{
  if (fromWorldFile) {
    std::cerr << "ERROR: option [" << option
              << "] cannot be set within a world file" << std::endl;
    usage("bzfs");
  }
}


static int parseIntArg(int& i, const std::vector<std::string>& tokens,
                       int offset = 0)
{
  if ((i + 1) >= (int)tokens.size()) {
    std::cerr << "ERROR: integer argument expected for "
              << tokens[i - offset] << std::endl;
    usage("bzfs");
  }
  i++; // skip the option argument string
  return atoi(tokens[i].c_str());
}


static float parseFloatArg(int& i, const std::vector<std::string>& tokens,
                           int offset = 0)
{
  if ((i + 1) >= (int)tokens.size()) {
    std::cerr << "ERROR: float argument expected for "
              << tokens[i - offset] << std::endl;
    usage("bzfs");
  }
  i++; // skip the option argument string
  return (float)atof(tokens[i].c_str());
}


static std::string parseStringArg(int& i, const std::vector<std::string>& tokens,
                                  int offset = 0)
{
  if ((i + 1) >= (int)tokens.size()) {
    std::cerr << "ERROR: string argument expected for "
              << tokens[i - offset] << std::endl;
    usage("bzfs");
  }
  i++; // skip the option argument string
  return tokens[i];
}


//============================================================================//

bool CmdLineOptions::parsePlayerCount(const std::string& arg)
{
  const char* argv = arg.c_str();

  /* either a single number or 5 or 6 optional numbers separated by
   * 4 or 5 (mandatory) commas.
   */
  const char *scan = argv;
  while (*scan && *scan != ',') scan++;

  if (*scan == ',') {
    // okay, it's the comma separated list.  count commas
    int commaCount = 1;
    while (*++scan) {
      if (*scan == ',')
	commaCount++;
    }
    if (commaCount != 5) {
      std::cout << "improper player count list" << std::endl;
      return false;
    }

    // no team size limit by default for real teams, set it to max players
    int i;
    for (i = 0; i < CtfTeams ; i++) {
      maxTeam[i] = maxRealPlayers;
    }
    // allow 5 Observers by default
    maxTeam[ObserverTeam] = 5;

    // now get the new counts

    // number of counts given
    int countCount = 0;
    scan = argv;
    // ctf teams plus observers
    for (i = 0; i < CtfTeams + 1; i++) {
      char *tail;
      long count = strtol(scan, &tail, 10);
      if (tail != scan) {
	// got a number
	countCount++;
	if (count < 0) {
	  maxTeam[i] = 0;
	} else {
	  if (count > maxRealPlayers) {
	    if (i == ObserverTeam && count > MaxPlayers)
	      maxTeam[i] = MaxPlayers;
	    else
	      maxTeam[i] = maxRealPlayers;
	  } else {
	    maxTeam[i] = uint8_t(count);
	  }
	}
      } // end if tail != scan
      while (*tail && *tail != ',') tail++;
      scan = tail + 1;
    } // end iteration over teams

    // if no/zero players were specified, allow a bunch of rogues
    bool allZero = true;
    for (i = 0; i < CtfTeams; i++) {
      if (maxTeam[i] != 0) {
	allZero = false;
      }
    }
    if (allZero) {
      maxTeam[RogueTeam] = MaxPlayers;
    }

    // if all counts explicitly listed then add 'em up and set maxRealPlayers
    // (unless max total players was explicitly set)
    if (countCount >= CtfTeams && maxRealPlayers == MaxPlayers) {
      maxRealPlayers = 0;
      for (i = 0; i < CtfTeams ; i++) {
	maxRealPlayers += maxTeam[i];
      }
    }

  } else {
    /* single number was provided instead of comma-separated */
    char *tail;
    long count = strtol(argv, &tail, 10);
    if (argv == tail) {
      std::cout << "improper player count" << std::endl;
      return false;
    }
    if (count < 1) {
      maxRealPlayers = 1;
    } else {
      if (count > MaxPlayers) {
	maxRealPlayers = MaxPlayers;
      } else {
	maxRealPlayers = uint8_t(count);
      }
    }
    // limit max team size to max players
    for (int i = 0; i < CtfTeams ; i++) {
      if (maxTeam[i] > maxRealPlayers)
	maxTeam[i] = maxRealPlayers;
    }
  } // end check if comm-separated list

  maxPlayers = maxRealPlayers + maxTeam[ObserverTeam];
  if (maxPlayers > MaxPlayers) {
    maxPlayers = MaxPlayers;
  }

  return true;
}


void CmdLineOptions::tokenizeLines(const std::vector<std::string>& lines,
                                         std::vector<std::string>& tokens)
{
  for (size_t i = 0; i < lines.size(); i++) {
    const std::string& line = lines[i];

    int startPos = line.find_first_not_of(" \t\r\n");

    while ((startPos >= 0) && (line.at(startPos) != '#')) {
      int endPos;
      if (line.at(startPos) == '"') {
        startPos++;
        endPos = line.find_first_of('"', startPos);
      } else {
        endPos = line.find_first_of("\t \r\n", startPos+1);
      }
      if (endPos < 0) {
        endPos = line.length();
      }
      tokens.push_back(line.substr(startPos,endPos-startPos));
      startPos = line.find_first_not_of("\t \r\n", endPos+1);
    }
  }
}


void CmdLineOptions::parseArgOptions(int argc, char** argv)
{
  // FIXME -- hack to clear static vars
  storedFlagDisallows.clear();
  storedFlagCounts.clear();
  storedFlagLimits.clear();
  allFlagsOut = false;

  std::vector<std::string> tokens;
  for (int i = 1; i < argc; i++) {
    tokens.push_back(argv[i]);
    // clear the password memory
    if ((strcmp(argv[i - 1], "-passwd") == 0) ||
        (strcmp(argv[i - 1], "-password") == 0)) {
      memset(argv[i], 0, strlen(argv[i]));
    }
  }
  parse(tokens, false);
}


void CmdLineOptions::parseFileOptions(const std::string& filePath)
{
  char delim = '\n';

  BZWError errorHandler(filePath);

  std::ifstream confStrm(filePath.c_str());
  if (!confStrm.is_open()) {
    errorHandler.fatalError(std::string("could not find bzflag configuration file"), 0);
    exit(1);
  }


  char buffer[4096];

  confStrm.getline(buffer, sizeof(buffer), delim);
  if (!confStrm.good()) {
    // maybe it's a Mac file and delim is \r
    delim = '\r';
    confStrm.seekg(0, std::ifstream::beg);
    confStrm.clear();
    confStrm.getline(buffer, sizeof(buffer), delim);
    if (!confStrm.good()) {
      errorHandler.fatalError(
        std::string("could not find bzflag configuration file on open"), 0);
      exit(1);
    }
  }
  confStrm.seekg(0, std::ifstream::beg);

  std::vector<std::string> lines;


  while (confStrm.good()) {
    confStrm.getline(buffer, sizeof(buffer), delim);
    lines.push_back(buffer);
  }

  std::vector<std::string> tokens;
  tokenizeLines(lines, tokens);

  parse(tokens, false);
}


void CmdLineOptions::parseWorldOptions(const std::vector<std::string>& lines)
{
  std::vector<std::string> tokens;
  tokenizeLines(lines, tokens);
  parse(tokens, true);
}


//============================================================================//

void CmdLineOptions::parse(const std::vector<std::string>& tokens, bool fromWorldFile)
{
  // InertiaGameStyle maintained just for compatibility
  // Same effect is achieved setting linear/angular Acceleration
  gameOptions |= int(InertiaGameStyle);

  // parse command line
  int playerCountArg  = 0;
  int playerCountArg2 = 0;

  for (int i = 0; i < (int)tokens.size(); i++) {

    const std::string& token = tokens[i];

    if (token == "-admsg") {
      const std::string msg = parseStringArg(i, tokens);
      if ((advertisemsg != "") || msg.empty()) {
	advertisemsg += "\\n";
      }
      advertisemsg += msg;
    }
    else if (token == "-advertise") {
      const std::string adList = parseStringArg(i, tokens);
      if (checkCommaList(adList, 2048)) {
	std::cerr << "Invalid group list for -advertise" << std::endl;
      } else {
	advertiseGroups = adList;
      }
    }
    else if (token == "-autoTeam") {
      autoTeam = true;
    }
    else if (token == "-b") {
      // random rotation to boxes in capture-the-flag game
      randomBoxes = true;
    }
    else if (token == "-badwords") {
      filterFilename = parseStringArg(i, tokens);
    }
    else if (token == "-ban") {
      acl.ban(parseStringArg(i, tokens).c_str());
    }
    else if (token == "-banfile") {
      const std::string banFile = parseStringArg(i, tokens);
      acl.setBanFile(banFile);
      if (!acl.load()) {
	std::cerr << "ERROR: could not load banfile ["
	          << banFile << "]" << std::endl;
	usage(execName);
      }
    }
    else if (token == "-botsPerIP") {
      botsPerIP = parseIntArg(i, tokens);
    }
    else if (token == "-c") {
      // capture the flag style
      if (gameType == RabbitChase) {
	std::cerr << "Capture the flag incompatible with Rabbit Chase" << std::endl;
	std::cerr << "Capture the flag assumed" << std::endl;
      }
      gameType = ClassicCTF;
    }
    else if (token == "-cache") {
      cacheURL = parseStringArg(i, tokens);
    }
    else if (token == "-cacheout") {
      cacheOut = parseStringArg(i, tokens);
    }
    else if (token == "-conf") {
      checkFromWorldFile(token, fromWorldFile);
      parseFileOptions(parseStringArg(i, tokens));
      numAllowedFlags = 0; // FIXME - Huh, does a reset?
    }
    else if (token == "-cr")  {
      // CTF with random world
      randomCTF = true;
      // capture the flag style
      if (gameType == RabbitChase)  {
	std::cerr << "Capture the flag incompatible with Rabbit Chase" << std::endl;
	std::cerr << "Capture the flag assumed" << std::endl;
      }
      gameType = ClassicCTF;
    }
    else if (token == "-density") {
      citySize = parseIntArg(i, tokens);
    }
    else if (token == "-disableBots") {
      // disallow clients from using autopilot or bots
      BZDB.set(StateDatabase::BZDB_DISABLEBOTS, "true");
    }
    else if (strncmp(token.c_str(), "-d", 2) == 0) {
      // increase debug level - this must be the last
      // option beginning with -d so that -dd, -ddd, etc. work
      int count = 0;
      const char *scan;
      for (scan = token.c_str() + 1; *scan == 'd'; scan++) {
        count++;
      }
      if (*scan != '\0') {
	std::cerr << "ERROR: bad argument [" << token << "]" << std::endl;
	usage(execName);
      }
      debugLevel += count;
      // std::cout << "Debug level is now " << debugLevel << "" << std::endl;
    }
    else if (token == "-f") {
      // disallow given flag
      storedFlagDisallows.push_back(parseStringArg(i, tokens));
    }
    else if (token == "+f") {
      // add required flag
      storedFlagCounts.push_back(parseStringArg(i, tokens));
    }
    else if (token == "-fb") {
      // flags on buildings
      flagsOnBuildings = true;
    }
    else if (token == "-filterAnnounce") {
      filterAnnounce = true;
    }
    else if (token == "-filterCallsigns") {
      filterCallsigns = true;
    }
    else if (token == "-filterChat") {
      filterChat = true;
    }
    else if (token == "-filterSimple") {
      filterSimple = true;
    }
    else if (token == "-freezeTag") {
      gameOptions |= int(FreezeTagGameStyle);
    }
    else if (token == "-g") {
      oneGameOnly = true;
    }
    else if (token == "-gndtex") {
      const std::string texName = parseStringArg(i, tokens);
      BzMaterial material;
      material.setName("GroundMaterial");
      material.setTexture(texName);
      MATERIALMGR.addMaterial(&material);
    }
    else if (token == "-groupdb") {
      groupsFile = parseStringArg(i, tokens);
      std::cerr << "using group file \"" << groupsFile << "\"" << std::endl;
    }
    else if (token == "-h") {
      randomHeights = true;
    }
    else if (token == "-help") {
      extraUsage(execName);
    }
    else if (token == "-helpdir") {
      const std::string dirName = parseStringArg(i, tokens);
      OSDir d(dirName);
      OSFile f;
      helpDirs.push_back(d);
      bool first;
      for(first = true; d.getNextFile(f, "*.txt", false); first = false) {
	std::string path = f.getFullOSPath(), name = f.getFileName();
	if (!textChunker.parseFile(path, name, 50, MessageLen))
	  std::cerr << "WARNING: couldn't read helpmsg file [" << path << "]" << std::endl;
	else
	  logDebugMessage(3, "Loaded help message: %s", name.c_str());
      }
      if (first)
	std::cerr << "WARNING: empty or inaccessible helpdir [" << dirName << "]" << std::endl;
    }
    else if (token == "-helpmsg") {
      const std::string fileName = parseStringArg(i, tokens);
      const std::string chunkName = parseStringArg(i, tokens, 1);
      if (!textChunker.parseFile(fileName, chunkName, 50, MessageLen)) {
	std::cerr << "ERROR: couldn't read helpmsg file ["
	          << fileName << "]" << std::endl;
	usage(execName);
      } else {
	logDebugMessage(3, ("Loaded help message: %s\n"), chunkName.c_str());
      }
    }
    else if (token == "-i") {
      // use a different interface
      pingInterface = parseStringArg(i, tokens);
    }
    else if (token == "-j") {
      // allow jumping
      gameOptions |= int(JumpingGameStyle);
    }
    else if (token == "-handicap") {
      // allow handicap advantage
      gameOptions |= int(HandicapGameStyle);
    }
    else if (token == "-jitterdrop") {
      maxjitterwarn = parseIntArg(i, tokens);
    }
    else if (token == "-jitterwarn") {
      jitterwarnthresh = (float)parseIntArg(i, tokens) / 1000.0f;
    }
    else if (token == "-lagdrop") {
      maxlagwarn = parseIntArg(i, tokens);
    }
    else if (token == "-lagwarn") {
      lagwarnthresh = (float)parseIntArg(i, tokens) / 1000.0f;
    }
    else if (token == "-loadplugin") {
      const std::string pluginPath = parseStringArg(i, tokens);
      std::vector<std::string> a =
        TextUtils::tokenize(pluginPath, std::string(","), 2);
      CmdLineOptions::pluginDef	pDef;
      if (a.size() >= 1)
	pDef.plugin = a[0];
      if (a.size() >= 2)
	pDef.command = a[1];
      if (pDef.plugin.size()) {
        if (!fromWorldFile) {
	  pluginList.push_back(pDef);
        }
#ifdef BZ_PLUGINS
        else {
          if (!loadPlugin(pDef.plugin, pDef.command)) {
            std::string text = "WARNING: unable to load the plugin; ";
            text += pDef.plugin + "\n";
            logDebugMessage(0, text.c_str());
          }
        }
#endif // BZ_PLUGINS
      }
    }
    else if (token == "-maxidle") {
      idlekickthresh = (float)parseIntArg(i, tokens);
    }
    else if (token == "-mp") {
      // set maximum number of players
      const int mpArgIndex = i + 1;
      if (mpArgIndex >= (int)tokens.size()) {
	std::cerr << "ERROR: missing -mp argument" << std::endl;
	usage(execName);
      }
      // FIXME: lame var hacking
      if (playerCountArg == 0) {
	playerCountArg = mpArgIndex;
      } else {
	playerCountArg2 = mpArgIndex;
      }
      i++;
    }
    else if (token == "-mps") {
      // set maximum player score
      maxPlayerScore = parseIntArg(i, tokens);
      if (maxPlayerScore < 1) {
	std::cerr << "disabling player score limit" << std::endl;
	maxPlayerScore = 0;
      }
    }
    else if (token == "-ms") {
      // set maximum number of shots
      const int newMaxShots = parseIntArg(i, tokens);
      if (newMaxShots == 0) {
	std::cerr << "WARNING: tanks will not be able to shoot" << std::endl;
	maxShots = 0;
      } else if (newMaxShots < 1) {
	std::cerr << "using minimum number of shots of 1" << std::endl;
	maxShots = 1;
      } else if (newMaxShots > MaxShots) {
	std::cerr << "using maximum number of shots of " << MaxShots << std::endl;
	maxShots = uint16_t(MaxShots);
      }
      else maxShots = uint16_t(newMaxShots);
    }
    else if (token == "-mts") {
      // set maximum team score
      maxTeamScore = parseIntArg(i, tokens);
      if (maxTeamScore < 1) {
	std::cerr << "disabling team score limit" << std::endl;
	maxTeamScore = 0;
      }
    }
    else if (token == "-noMasterBanlist") {
      suppressMasterBanList = true;
    }
    else if (token == "-noradar") {
      BZDB.set(StateDatabase::BZDB_RADARLIMIT, "-1.0");
    }
    else if (token == "-masterBanURL") {
      // if this is the first master ban url, override the default
      // list.  otherwise just keep adding urls.
      if (!masterBanListOverridden) {
	masterBanListURL.clear();
	masterBanListOverridden = true;
      }
      masterBanListURL.push_back(parseStringArg(i, tokens));
    }
    else if (token == "-noTeamKills") {
      // forbid team kills
      gameOptions |= int(NoTeamKills);
    }
    else if (token == "-p") {
      // use a different port
      checkFromWorldFile(token, fromWorldFile);
      wksPort = parseIntArg(i, tokens);
      if (wksPort < 1 || wksPort > 65535) {
	wksPort = ServerPort;
      } else {
	useGivenPort = true;
      }
    }
    else if (token == "-packetlossdrop") {
      maxpacketlosswarn = parseIntArg(i, tokens);
    }
    else if (token == "-packetlosswarn") {
      packetlosswarnthresh = (float)parseIntArg(i, tokens) / 1000.0f;
    }  else if (token == "-passwd" || token == "-password") {
      checkFromWorldFile(token, fromWorldFile);
      password = parseStringArg(i, tokens);
    }
    else if (token == "-pidfile") {
      unsigned int pid = 0;
      const std::string pidFile = parseStringArg(i, tokens);
      FILE *fp = fopen(pidFile.c_str(), "wt");
#ifndef HAVE_PROCESS_H
      pid = getpid();
#else
      pid = _getpid();
#endif
      if (fp) {
	fprintf(fp, "%d", pid);
	fclose(fp);
      }
    }  else if (token == "-pf") {
      // try wksPort first and if we can't open that port then
      // let system assign a port for us.
      useFallbackPort = true;
    }
    else if (token == "-poll") {
      // parse the variety of poll system variables
      std::vector<std::string> args =
        TextUtils::tokenize(parseStringArg(i, tokens), "=", 2, true);
      if (args.size() != 2) {
	std::cerr << "ERROR: expected -poll variable=value" << std::endl;
	usage(execName);
      }

      const std::string lower = TextUtils::tolower(args[0]);
      const unsigned short int uint16Value = atoi(args[1].c_str());
      if (lower == "bantime") {
	banTime = uint16Value;
      } else if (lower == "vetotime") {
	vetoTime = uint16Value;
      } else if (lower == "voterepeattime") {
	voteRepeatTime = uint16Value;
      } else if (lower == "votesrequired") {
	votesRequired = uint16Value;
      } else if (lower == "votetime") {
	voteTime = uint16Value;
      } else if (lower == "votepercentage") {
	votePercentage = (float)atof(args[1].c_str());
      } else {
	std::cerr << "ERROR: unknown variable for -poll, skipping";
      }
    }
    else if (token == "-printscore") {
      // dump score whenever it changes
      printScore = true;
    }
    else if (token == "-processor") {
      processorAffinity = parseIntArg(i, tokens);
    }
    else if (token == "-public") {
      publicizeServer = true;
      publicizedTitle = parseStringArg(i, tokens);
      if (publicizedTitle.length() > 127) {
	publicizedTitle.resize(127);
	std::cerr << "description too long... truncated" << std::endl;
      }
    }
    else if (token == "-publicaddr") {
      checkFromWorldFile(token, fromWorldFile);
      publicizedAddress = parseStringArg(i, tokens);
      publicizeServer = true;
    }
    else if (token == "-publiclist") {
      // if this is the first -publiclist, override the default list
      // server.  otherwise just keep adding urls.
      if (!listServerOverridden) {
	listServerURL.clear();
	listServerOverridden = true;
      }
      checkFromWorldFile(token, fromWorldFile);
      listServerURL.push_back(parseStringArg(i, tokens));
    }
    else if (token == "-q") {
      // don't handle pings
      checkFromWorldFile(token, fromWorldFile);
      handlePings = false;
    }
    else if (token == "+r") {
      // all shots ricochet style
      gameOptions |= int(RicochetGameStyle);
    }
    else if (token == "-rabbit")	{
      // rabbit chase style
      if (gameType == ClassicCTF) {
	std::cerr << "Rabbit Chase incompatible with Capture the flag" << std::endl;
	std::cerr << "Rabbit Chase assumed" << std::endl;
      }
      gameType = RabbitChase;

      // default selection style
      rabbitSelection = ScoreRabbitSelection;

      // if there are any arguments following, see if they are a
      // rabbit selection styles.
      if ((i + 1) < (int)tokens.size()) {
        const std::string& style = tokens[i + 1];
	if (style == "score") {
	  rabbitSelection = ScoreRabbitSelection;
	  i++;
	} else if (style == "killer") {
	  rabbitSelection = KillerRabbitSelection;
	  i++;
	} else if (style == "random") {
	  rabbitSelection = RandomRabbitSelection;
	  i++;
	}
      }
    }
    else if (token == "-recbuf") {
      Record::setSize(ServerPlayer, parseIntArg(i, tokens));
      startRecording = true;
    }
    else if (token == "-recbufonly") {
      Record::setAllowFileRecs(false);
    }
    else if (token == "-recdir") {
      Record::setDirectory(parseStringArg(i, tokens).c_str());
    }
    else if (token == "-replay") {
      replayServer = true;
    }
    else if (token == "-reportfile") {
      reportFile = parseStringArg(i, tokens);
    }
    else if (token == "-reportpipe") {
      reportPipe = parseStringArg(i, tokens);
    }
    else if (token == "-tkannounce") {
      tkAnnounce = true;
    }
    else if (token == "+s" || token == "-s") {
      // with +s all flags are required to exist all the time
      allFlagsOut = token[0] == '+' ? true : false;
      // set number of random flags
      if (((i + 1) < (int)tokens.size()) && isdigit(tokens[i + 1][0])) {
        numExtraFlags = parseIntArg(i, tokens);
	if (numExtraFlags  == 0) {
	  numExtraFlags = 16;
        }
      } else {
	numExtraFlags = 16;
      }
    }
    else if (token == "-sa") {
      // insert antidote flags
      gameOptions |= int(AntidoteGameStyle);
    }
    else if (token == "-sb") {
      // respawns on buildings
      respawnOnBuildings = true;
    }
    else if (token == "-set") {
      const std::string name = parseStringArg(i, tokens);
      const std::string value = parseStringArg(i, tokens, 1);
      if (!BZDB.isSet(name)) {
	std::cerr << "ERROR: unknown BZDB variable specified [" << name << "]" << std::endl;
	exit(1);
      }
      BZDB.set(name, value);
      logDebugMessage(1, "set variable: %s = %s\n",
                      name.c_str(), BZDB.get(name).c_str());
    }
    else if (token == "-setforced") {
      const std::string name = parseStringArg(i, tokens);
      const std::string value = parseStringArg(i, tokens, 1);
      const bool exists = BZDB.isSet(name);
      if (exists) {
	std::cerr << "-setforced: " << name << " already exists" << std::endl;
      } else {
	addBzfsCallback(name, NULL);
      }
      BZDB.set(name, value);
      logDebugMessage(1, "set variable: %s = %s\n",
                      name.c_str(), BZDB.get(name).c_str());
    }
    else if (token == "-sl") {
      // shot limits
      const std::string flagType = parseStringArg(i, tokens);
      const std::string limitStr = parseStringArg(i, tokens, 1);
      int x = 1;
      if (isdigit(limitStr[0])) {
	x = atoi(limitStr.c_str());
	if (x < 1) {
	  std::cerr << "can only limit to 1 or more shots, changing to 1" << std::endl;
	  x = 1;
	}
      } else {
	std::cerr << "ERROR: invalid shot limit [" << limitStr << "]" << std::endl;
	usage(execName);
      }
      storedFlagLimits[flagType] = x;
    }
    else if (token == "-spamtime") {
      msgTimer = parseIntArg(i, tokens);
      std::cerr << "using spam time of " << msgTimer << std::endl;
    }
    else if (token == "-spamwarn") {
      spamWarnMax = parseIntArg(i, tokens);
      std::cerr << "using spam warn amount of " << spamWarnMax << std::endl;
    }
    else if (token == "-spawnPolicy") {
      const std::string policy = parseStringArg(i, tokens);
      bool validPolicy = SPAWNPOLICY.IsRegistered(policy);
      if (validPolicy) {
	SPAWNPOLICY.setDefault(policy);
	std::cerr << "using " << policy << " spawn policy" << std::endl;
      } else {
	std::cerr << "ERROR: unknown spawn policy specified [" << policy << "]" << std::endl;
	std::cerr << std::endl << "Available Policies" << std::endl << "------------------" << std::endl;
	SPAWNPOLICY.Print(std::cerr);
	std::cerr << std::endl;
	usage(execName);
      }
    }
    else if (token == "-speedtol") {
      speedTolerance = parseFloatArg(i, tokens);
      std::cerr << "using speed autokick tolerance of \"" << speedTolerance << "\"" << std::endl;
    }
    else if (token == "-srvmsg") {
      const std::string msg = parseStringArg(i, tokens);
      if ((servermsg != "") || msg.empty()) {
	servermsg += "\\n";
      }
      servermsg += msg;
    }
    else if (token == "-st") {
      // set shake timeout
      const float timeout = parseFloatArg(i, tokens);
      if (timeout < 0.1f) {
	shakeTimeout = 1;
	std::cerr << "using minimum shake timeout of " << 0.1f * (float)shakeTimeout << std::endl;
      } else if (timeout > 300.0f) {
	shakeTimeout = 3000;
	std::cerr << "using maximum shake timeout of " << 0.1f * (float)shakeTimeout << std::endl;
      } else {
	shakeTimeout = uint16_t(timeout * 10.0f + 0.5f);
      }
      gameOptions |= int(ShakableGameStyle);
    }
    else if (token == "-sw") {
      // set shake win count
      const int count = parseIntArg(i, tokens);
      if (count < 1) {
	shakeWins = 1;
	std::cerr << "using minimum shake win count of " << shakeWins << std::endl;
      } else if (count > 20) {
	shakeWins = 20;
	std::cerr << "using maximum ttl of " << shakeWins << std::endl;
      } else {
	shakeWins = uint16_t(count);
      }
      gameOptions |= int(ShakableGameStyle);
    }
    else if (token == "-synctime") {
      // client clocks should be synchronized to server clock
      BZDB.set(StateDatabase::BZDB_SYNCTIME, "1.0"); // any positive number
    }
    else if (token == "-synclocation") {
      // client coordinates should be set to server coordinates
      BZDB.set(StateDatabase::BZDB_SYNCLOCATION, "true");
    }
    else if (token == "-t") {
      // allow teleporters
      useTeleporters = true;
      if (worldFile != "")
	std::cerr << "-t is meaningless when using a custom world, ignoring" << std::endl;
    }
    else if (token == "-offa") {
      // teamless ffa style
      if (gameType == RabbitChase || gameType == ClassicCTF) {
	std::cerr << "Open (Teamless) Free-for-all incompatible with other modes" << std::endl;
	std::cerr << "Open Free-for-all assumed" << std::endl;
      }
      gameType = OpenFFA;
    }
    else if (token == "-tftimeout") {
      // use team flag timeout
      teamFlagTimeout = parseIntArg(i, tokens);
      if (teamFlagTimeout < 0) {
	teamFlagTimeout = 0;
      }
      std::cerr << "using team flag timeout of " << teamFlagTimeout << " seconds" << std::endl;
    }
    else if (token == "-time") {
      const std::string timeStr = parseStringArg(i, tokens);
      if (timeStr.find(':') == std::string::npos) {
	timeLimit = (float)atof(timeStr.c_str());
      }
      else {
	std::vector<std::string> endTime = TextUtils::tokenize(timeStr, std::string(":"));
	{
	  unsigned int sizer = endTime.size();
	  while (sizer != 3) {
	    endTime.push_back("00");
	    ++sizer;
	  }
	  if (sizer > 3) {
	    std::cerr << "ERROR: too many arguments to -time" << std::endl;
	    usage(execName);
	  }
	}
	time_t tnow = time(0);
	struct tm *now = localtime(&tnow);
	unsigned int hour = now->tm_hour, min = now->tm_min, sec = now->tm_sec,
	  cmdHour = atoi(endTime[0].c_str()),
	  cmdMin = atoi(endTime[1].c_str()),
	  cmdSec = atoi(endTime[2].c_str());
	unsigned long secsToday = (hour * 3600) + (min * 60) + sec,
	  secsTill = (cmdHour * 3600) + (cmdMin * 60) + cmdSec;
	if (secsToday > secsTill) { //if the requested time has already past
	  timeLimit = (float)((86400 - secsToday) + secsTill); //secs left today + till req. time
	} else {
	  timeLimit = (float)(secsTill - secsToday);
        }
      }
      if (timeLimit <= 0.0f) {
	// league matches are 30 min
	timeLimit = 1800.0f;
      }
      std::cerr << "using time limit of " << (int)timeLimit << " seconds" << std::endl;
      timeElapsed = timeLimit;
    }
    else if (token == "-timemanual") {
      timeManualStart = true;
    }
    else if (token == "-tk") {
      // team killer does not die
      teamKillerDies = false;
    }
    else if (token == "-tkkr") {
      teamKillerKickRatio = parseIntArg(i, tokens);
      if (teamKillerKickRatio < 0) {
	teamKillerKickRatio = 0;
	std::cerr << "disabling team killer kick ratio";
      }
    }
    else if (token == "-ts") {
      // timestamp output
      timestampLog = true;
      // if there is an argument following, see if it is 'micros'
      if ((i + 1) < (int)tokens.size()) {
	if (TextUtils::compare_nocase(tokens[i + 1], "micros") == 0) {
	  timestampMicros = true;
	  i++;
	}
      }
    }
    else if (token == "-userdb") {
      userDatabaseFile = parseStringArg(i, tokens);
      std::cerr << "using userDB file \"" << userDatabaseFile << "\"" << std::endl;
    }
    else if (token == "-v" || token == "-version") {
      printVersion();
      exit(0);
    }
    else if (token == "-vars") {
      bzdbVars = parseStringArg(i, tokens);
    }
    else if (token == "-world") {
      checkFromWorldFile(token, fromWorldFile);
      worldFile = parseStringArg(i, tokens);

      numAllowedFlags = 0; // FIXME - Huh, does a reset?

      if (useTeleporters) {
	std::cerr << "-t is meaningless when using a custom world, ignoring" << std::endl;
      }
    }
    else if (token == "-worldsize") {
      BZDB.set(StateDatabase::BZDB_WORLDSIZE,
               TextUtils::format("%d", parseIntArg(i, tokens) * 2));
      std::cerr << "using world size of [" << BZDBCache::worldSize << "]" << std::endl;
    } else {
      std::cerr << "ERROR: bad argument [" << token << "]" << std::endl;
      usage(execName);
    }
  }

  // get player counts.  done after other arguments because we need
  // to ignore counts for rogues if rogues aren't allowed.
  if ((playerCountArg > 0) && !parsePlayerCount(tokens[playerCountArg])) {
    std::cerr << "ERROR: unable to parse the player count (check -mp option)" << std::endl;
    usage(execName);
  }
  if ((playerCountArg2 > 0) && !parsePlayerCount(tokens[playerCountArg2])) {
    std::cerr << "ERROR: unable to parse the player count (check -mp option)" << std::endl;
    usage(execName);
  }

  return;
}


static int getZoneTeamFlagCount(FlagType* flagType, EntryZones& entryZones,
				const std::set<FlagType*>& forbidden)
{
  if (forbidden.find(flagType) != forbidden.end()) {
    return 0;
  }
  // tally zone team flags
  int count = 0;
  const ZoneList& zl = entryZones.getZoneList();
  for (int z = 0; z < (int)zl.size(); z++) {
    const ZoneFlagMap& zfm = zl[z].getZoneFlagMap();
    ZoneFlagMap::const_iterator zfmIt;
    for (zfmIt = zfm.begin(); zfmIt != zfm.end(); zfmIt++) {
      if (zfmIt->first == flagType) {
	count += zfmIt->second;
      }
    }
  }
  return count;
}


static int addZoneTeamFlags(int startIndex,
			    FlagType* flagType, EntryZones& entryZones,
			    const std::set<FlagType*>& forbidden)
{
  if (forbidden.find(flagType) != forbidden.end()) {
    return startIndex;
  }
  // add zone team flags
  const ZoneList& zl = entryZones.getZoneList();
  for (int z = 0; z < (int)zl.size(); z++) {
    const ZoneFlagMap& zfm = zl[z].getZoneFlagMap();
    ZoneFlagMap::const_iterator zfmIt;
    for (zfmIt = zfm.begin(); zfmIt != zfm.end(); zfmIt++) {
      if (zfmIt->first == flagType) {
	const int count = zfmIt->second;
	for (int c = 0; c < count; c++) {
	  entryZones.addZoneFlag(z, startIndex);
	  FlagInfo::get(startIndex++)->setRequiredFlag(flagType);
	}
      }
    }
  }
  return startIndex;
}


void CmdLineOptions::finalizeParsing(EntryZones& entryZones)
{
  if (flagsOnBuildings && !(gameOptions & JumpingGameStyle)) {
    std::cout << "WARNING: flags on boxes without jumping is potentially unsafe" << std::endl;
  }

  if (gameType == RabbitChase) {
    for (int j = RedTeam; j <= PurpleTeam; j++) {
      if (maxTeam[j] > 0
	  && maxTeam[RogueTeam] != maxRealPlayers)
	std::cout << "only rogues are allowed in Rabbit Chase; zeroing out "
		  << Team::getName((TeamColor) j) << std::endl;
      maxTeam[j] = 0;
    }
  }

  // disallowed flags
  std::vector<std::string>::iterator vsitr;
  for (vsitr = storedFlagDisallows.begin(); vsitr != storedFlagDisallows.end(); ++vsitr) {
    if (strcmp(vsitr->c_str(), "bad") == 0) {
      FlagSet badFlags = Flag::getBadFlags();
      for (FlagSet::iterator it = badFlags.begin(); it != badFlags.end(); it++)
	flagDisallowed[*it] = true;
    } else if (strcmp(vsitr->c_str(), "good") == 0) {
      FlagSet goodFlags = Flag::getGoodFlags();
      for (FlagSet::iterator it = goodFlags.begin(); it != goodFlags.end(); it++)
	flagDisallowed[*it] = true;
    } else {
      FlagType* fDesc = Flag::getDescFromAbbreviation(vsitr->c_str());
      if (fDesc == Flags::Null) {
	std::cerr << "ERROR: invalid flag [" << (*vsitr) << "]" << std::endl;
	usage(execName);
      }
      flagDisallowed[fDesc] = true;
    }
  }
  storedFlagDisallows.clear();

  // explicitly added flags
  for (vsitr = storedFlagCounts.begin(); vsitr != storedFlagCounts.end(); ++vsitr) {
    size_t rptBgn = vsitr->find_first_of('{');
    int rptCnt = 1;
    if (rptBgn > 0) {
      rptCnt = atoi(vsitr->substr(rptBgn+1, vsitr->length() - rptBgn).c_str());
      if (rptCnt <= 0)
	rptCnt = 1;
      (*vsitr) = vsitr->substr(0, rptBgn);
    }
    if (strcmp(vsitr->c_str(), "good") == 0) {
      FlagSet goodFlags = Flag::getGoodFlags();
      for (FlagSet::iterator it = goodFlags.begin(); it != goodFlags.end(); it++)
	flagCount[*it] += rptCnt;
    } else if (strcmp(vsitr->c_str(), "bad") == 0) {
      FlagSet badFlags = Flag::getBadFlags();
      for (FlagSet::iterator it = badFlags.begin(); it != badFlags.end(); it++)
	flagCount[*it] += rptCnt;
    } else if (strcmp(vsitr->c_str(), "team") == 0) {
      for (int t = RedTeam; t <= PurpleTeam; t++)
	numTeamFlags[t] += rptCnt;
    } else {
      FlagType *fDesc = Flag::getDescFromAbbreviation(vsitr->c_str());
      if (fDesc == Flags::Null) {
	std::cerr << "ERROR: invalid flag [" << (*vsitr) << "]" << std::endl;
	usage(execName);
      } else if (fDesc->flagTeam != NoTeam) {
	numTeamFlags[fDesc->flagTeam] += rptCnt;
      } else {
	flagCount[fDesc] += rptCnt;
      }
    }
  }
  storedFlagCounts.clear();

  // do we have any team flags?
  bool hasTeam = false;
  for (int p = RedTeam; p <= PurpleTeam; p++) {
    if (maxTeam[p] > 1) {
      hasTeam = true;
      break;
    }
  }

  std::set<FlagType*> forbidden;
  forbidden.insert(Flags::Null);

  // first disallow flags inconsistent with game style
  if (gameOptions & JumpingGameStyle) {
    forbidden.insert(Flags::Jumping);
  } else {
    forbidden.insert(Flags::NoJumping);
  }
  if (gameOptions & RicochetGameStyle)
    forbidden.insert(Flags::Ricochet);

  if (!useTeleporters && (worldFile == ""))
    forbidden.insert(Flags::PhantomZone);

  if (!hasTeam) {
    forbidden.insert(Flags::Genocide);
    forbidden.insert(Flags::Colorblindness);
    forbidden.insert(Flags::Masquerade);
  }
  if ((gameType == ClassicCTF) == 0) {
    forbidden.insert(Flags::RedTeam);
    forbidden.insert(Flags::GreenTeam);
    forbidden.insert(Flags::BlueTeam);
    forbidden.insert(Flags::PurpleTeam);
  }
  if (maxTeam[RedTeam] <= 0)
    forbidden.insert(Flags::RedTeam);

  if (maxTeam[GreenTeam] <= 0)
    forbidden.insert(Flags::GreenTeam);

  if (maxTeam[BlueTeam] <= 0)
    forbidden.insert(Flags::BlueTeam);

  if (maxTeam[PurpleTeam] <= 0)
    forbidden.insert(Flags::PurpleTeam);

  // void the forbidden flags
  std::set<FlagType*>::const_iterator sit;
  for (sit = forbidden.begin(); sit != forbidden.end(); sit++) {
    FlagType* ft = *sit;
    flagCount[ft] = 0;
    flagDisallowed[ft] = true;
  }

  // zone team flag counts
  const int zoneTeamFlagCounts[5] = {
    0, // rogue
    getZoneTeamFlagCount(Flags::RedTeam, entryZones, forbidden),
    getZoneTeamFlagCount(Flags::GreenTeam, entryZones, forbidden),
    getZoneTeamFlagCount(Flags::BlueTeam, entryZones, forbidden),
    getZoneTeamFlagCount(Flags::PurpleTeam, entryZones, forbidden)
  };

  // make sure there is at least one team flag for each active team
  if (gameType == ClassicCTF) {
    for (int col = RedTeam; col <= PurpleTeam; col++) {
      if ((maxTeam[col] > 0) &&
	  (numTeamFlags[col] <= 0) &&
	  (zoneTeamFlagCounts[col] <= 0)) {
	numTeamFlags[col] = 1;
      }
    }
  }

  // make table of allowed extra flags
  if (numExtraFlags > 0) {
    // now count how many aren't disallowed
    for (FlagTypeMap::iterator it = FlagType::getFlagMap().begin();
	 it != FlagType::getFlagMap().end(); ++it) {
      if (!flagDisallowed[it->second]) {
	numAllowedFlags++;
      }
    }
    // if none allowed then no extra flags either
    if (numAllowedFlags == 0) {
      numExtraFlags = 0;
    } else {
      // types of extra flags allowed
      std::vector<FlagType*> allowedFlags;
      allowedFlags.clear();
      for (FlagTypeMap::iterator it = FlagType::getFlagMap().begin();
	   it != FlagType::getFlagMap().end(); ++it) {
	FlagType *fDesc = it->second;
	if ((fDesc == Flags::Null) || (fDesc->flagTeam != ::NoTeam))
	  continue;
	if (!flagDisallowed[it->second])
	  allowedFlags.push_back(it->second);
      }
      // Loading allowedFlags vector
      FlagInfo::setAllowed(allowedFlags);
    }
  }

  const ZoneList& zl = entryZones.getZoneList();

  // allocate space for extra flags
  numFlags = numExtraFlags;
  // allocate space for team flags
  if (gameType & ClassicCTF) {
    for (int col = RedTeam; col <= PurpleTeam; col++) {
      if (maxTeam[col] > 0) {
	numFlags += numTeamFlags[col];
      }
    }
  }
  // allocate space for normal flags
  for (FlagTypeMap::iterator it = FlagType::getFlagMap().begin();
       it != FlagType::getFlagMap().end(); ++it) {
    numFlags += flagCount[it->second];
  }
  // allocate space for zone flags (including teams flags)
  for (int z = 0; z < (int)zl.size(); z++) {
    const CustomZone& zone = zl[z];
    const ZoneFlagMap& zfm = zone.getZoneFlagMap();
    ZoneFlagMap::const_iterator zfmIt;
    for (zfmIt = zfm.begin(); zfmIt != zfm.end(); zfmIt++) {
      if (forbidden.find(zfmIt->first) == forbidden.end()) {
	numFlags += zfmIt->second;
      }
    }
  }

  FlagInfo::setSize(numFlags);

  // add team flags (ordered)
  int f = 0;
  if (gameType == ClassicCTF) {
    if (maxTeam[RedTeam] > 0) {
      f = addZoneTeamFlags(f, Flags::RedTeam, entryZones, forbidden);
      for (int n = 0; n < numTeamFlags[RedTeam]; n++)
	FlagInfo::get(f++)->setRequiredFlag(Flags::RedTeam);
    }
    if (maxTeam[GreenTeam] > 0) {
      f = addZoneTeamFlags(f, Flags::GreenTeam, entryZones, forbidden);
      for (int n = 0; n < numTeamFlags[GreenTeam]; n++)
	FlagInfo::get(f++)->setRequiredFlag(Flags::GreenTeam);
    }
    if (maxTeam[BlueTeam] > 0) {
      f = addZoneTeamFlags(f, Flags::BlueTeam, entryZones, forbidden);
      for (int n = 0; n < numTeamFlags[BlueTeam]; n++)
	FlagInfo::get(f++)->setRequiredFlag(Flags::BlueTeam);
    }
    if (maxTeam[PurpleTeam] > 0) {
      f = addZoneTeamFlags(f, Flags::PurpleTeam, entryZones, forbidden);
      for (int n = 0; n < numTeamFlags[PurpleTeam]; n++)
	FlagInfo::get(f++)->setRequiredFlag(Flags::PurpleTeam);
    }
  }

  // super flags?
  if (f < numFlags)
    gameOptions |= int(SuperFlagGameStyle);

  // shot limits
  std::map<std::string, int>::iterator msiitr;
  for (msiitr = storedFlagLimits.begin(); msiitr != storedFlagLimits.end(); ++msiitr) {
    FlagType *fDesc = Flag::getDescFromAbbreviation(msiitr->first.c_str());
    if (fDesc == Flags::Null) {
      std::cerr << "ERROR: invalid flag [" << msiitr->first << "]" << std::endl;
      usage(execName);
    } else {
      // this parameter has already been validated
      flagLimit[fDesc] = msiitr->second;
    }
  }
  storedFlagLimits.clear();

  // add normal flags
  for (FlagTypeMap::iterator it2 = FlagType::getFlagMap().begin();
       it2 != FlagType::getFlagMap().end(); ++it2) {
    FlagType *fDesc = it2->second;

    if ((fDesc != Flags::Null) && (fDesc->flagTeam == NoTeam)) {
      for (int j = 0; j < flagCount[fDesc]; j++)
	FlagInfo::get(f++)->setRequiredFlag(fDesc);
    }
  }
  // add zone flags
  for (int z = 0; z < (int)zl.size(); z++) {
    const CustomZone& zone = zl[z];
    const ZoneFlagMap& zfm = zone.getZoneFlagMap();
    ZoneFlagMap::const_iterator zfmIt;
    for (zfmIt = zfm.begin(); zfmIt != zfm.end(); zfmIt++) {
      FlagType* ft = zfmIt->first;
      if ((ft->flagTeam == ::NoTeam) && // no team flags here
	  (forbidden.find(ft) == forbidden.end())) {
	const int count = zfmIt->second;
	for (int c = 0; c < count; c++) {
	  entryZones.addZoneFlag(z, f);
	  FlagInfo::get(f++)->setRequiredFlag(ft);
	}
      }
    }
  }
  // add extra flags
  for (; f < numFlags; f++)
    FlagInfo::get(f)->required = allFlagsOut;

  // sum the sources of team flags
  if (gameType & ClassicCTF) {
    for (int col = RedTeam; col <= PurpleTeam; col++)
      numTeamFlags[col] += zoneTeamFlagCounts[col];
  }

  // processor affinity
  if (processorAffinity >= 0) {
    logDebugMessage(2, "Setting process affinity to processor %d\n", processorAffinity);
    TimeKeeper::setProcessorAffinity(processorAffinity);
  } else {
    logDebugMessage(2, "Processor affinity is floating (managed by OS)\n");
  }


  // debugging
  logDebugMessage(1,"type: %d\n", gameType);
  if (gameType == ClassicCTF)
    logDebugMessage(1,"  capture the flag\n");
  if (gameType == RabbitChase)
    logDebugMessage(1,"  rabbit chase\n");

  logDebugMessage(1,"options: %X\n", gameOptions);
  if (gameOptions & int(SuperFlagGameStyle))
    logDebugMessage(1,"  super flags allowed\n");
  if (gameOptions & int(JumpingGameStyle))
    logDebugMessage(1,"  jumping allowed\n");
  if (gameOptions & int(RicochetGameStyle))
    logDebugMessage(1,"  all shots ricochet\n");
  if (gameOptions & int(ShakableGameStyle))
    logDebugMessage(1,"  shakable bad flags: timeout=%f, wins=%i\n",
		    0.1f * float(shakeTimeout), shakeWins);
  if (gameOptions & int(HandicapGameStyle))
    logDebugMessage(1,"  handicap mode on\n");
  if (gameOptions & int(AntidoteGameStyle))
    logDebugMessage(1,"  antidote flags\n");

  return;
}


// simple syntax check of comma-seperated list of group names (returns true if error)
bool CmdLineOptions::checkCommaList(const std::string& list, int maxLen)
{
  if ((int)list.size() > maxLen) {
    return true;
  }
  if (list.empty()) {
    return false;
  }
  if ((list[0] == ',') || (list[list.size() - 1] == ',')) {
    return true;
  }
  for (size_t i = 0; i < list.size(); i++) {
    const unsigned char c = list[i];
    if ((c < ' ') || (c > 'z') || (c == '\'') || (c == '"')) {
      return true;
    }
  }
  return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
