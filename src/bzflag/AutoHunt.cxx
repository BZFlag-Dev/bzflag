/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// Sets autohunt status for players based on the string
// contained in the BZDB "autohunt" variable. The format
// is as follows:
//
//   set autohunt G:CL?L:ST?GM:RATIO:LEADER
//
//   G:       always hunt a player with the Genocide flag
//   CL?L:    if the local player has Cloak, hunt players with Laser 
//   RATIO:   if an enemy player has more kills then deaths on you, hunt them
//   LEADER:  hunt the enemy with the best score

#include "common.h"

// interface header
#include "AutoHunt.h"

// system headers
#include <map>
#include <vector>
#include <string>
#include <stdlib.h>

// common headers
#include "global.h"
#include "Flag.h"
#include "AnsiCodes.h"
#include "TextUtils.h"
#include "StateDatabase.h"

// local headers
#include "playing.h"
#include "World.h"
#include "Player.h"
#include "LocalPlayer.h"
#include "ControlPanel.h"


// typedefs / local classes
class HuntInfo {
  public:
    HuntInfo() {}
    bool active() const { return (coords.size() > 0); }
    std::map<int, float> coords;  // level, proximity
};
typedef std::map<const FlagType*, HuntInfo> FlagHuntInfoMap;
typedef std::map<const FlagType*, FlagHuntInfoMap> FlagPairMap;
class HuntRange {
  public:
    HuntRange() { reset(); }
    void reset()
    {
      minLevel = 1;
      maxLevel = 0;
      minProximity = MAXFLOAT;
      maxProximity = MAXFLOAT;
    }
  public:
    int minLevel;
    int maxLevel;
    float minProximity;
    float maxProximity;
};


// static data
static FlagPairMap FlagPairs;
static HuntInfo TkHuntInfo;
static HuntInfo RatioHuntInfo;
static HuntInfo LeaderHuntInfo;
static int MaxLevel = 0;
static int RadarLevel = 1;

// static functions
static std::string getExpandedString(const std::string& varname);
static void parse(const std::string& msg);
static void huntPlayers();
static void clearPlayers();
static void setupHuntInfoRange(HuntInfo&, const HuntRange&);
static int getHuntInfoLevel(const HuntInfo&, float dist);
static float distBetweenPlayers(const Player*, const Player*);
static bool isEnemy(const Player*);
static bool isBeatingMe(const Player*);
static bool isTeamKiller(const Player* p, float ratio);
static Player* findEnemyLeader();
static void printMaps();


/******************************************************************************/

static void parse(const std::string& cmd)
{
  std::vector<std::string> errors;

  HuntRange range;
  FlagPairs.clear();
  TkHuntInfo.coords.clear();
  RatioHuntInfo.coords.clear();
  LeaderHuntInfo.coords.clear();
  RadarLevel = 1;


  DEBUG3("AutoHunt: %s\n", cmd.c_str());
  std::vector<std::string> tokens = TextUtils::tokenize(cmd, ":");

  for (int i = 0; i < (int)tokens.size(); i++) {
    std::string token = TextUtils::toupper(tokens[i]);
    DEBUG3("  Token: %-10s", token.c_str());

    bool useRange = true;
    
    // parse the level
    int level = 1;
    const int len = (int)token.size();
    if (len >= 2) {
      if (token[len - 2] == '#') {
        int number = token[len - 1] - '0';
        if ((number >= 0) && (number <= 9)) {
          level = number;
          token.resize(len - 2);
          useRange = false;
        } else {
          errors.push_back(tokens[i]);
          DEBUG3("Error parsing level\n");
          continue;
        }
      }
    }
    if (level <= 0) {
      continue;
    }

    // parse the proximity
    float proximity = MAXFLOAT;
    std::string::size_type lessThen = token.find_first_of('<');
    if (lessThen != std::string::npos) {
      char* end;
      const char* start = token.c_str() + lessThen + 1;
      proximity = (float)strtod(start, &end);
      if (end != start) {
        token.resize(lessThen);
        useRange = false;
      } else {
        errors.push_back(tokens[i]);
        DEBUG3("Error parsing proximity\n");
        continue;
      }
    }
    
    if (token == "RADARLEVEL") {
      RadarLevel = level;
      DEBUG3("(radarLevel/%i/%f)\n", RadarLevel, proximity);
    }
    else if (token == "MIN") {
      range.minLevel = level;
      range.minProximity = proximity;
      DEBUG3("(range min/%i/%f)\n", level, proximity);
    }
    else if (token == "MAX") {
      range.maxLevel = level;
      range.maxProximity = proximity;
      DEBUG3("(range max/%i/%f)\n", level, proximity);
    }
    else if (token == "RESET") {
      range.reset();
      DEBUG3("(range reset)\n");
    }
    else if (token == "TK") {
      if (useRange) {
        setupHuntInfoRange(TkHuntInfo, range);
        DEBUG3("(tk/range)\n");
      } else {
        TkHuntInfo.coords[level] = proximity;
        DEBUG3("(tk/%i/%f)\n", level, proximity);
      }
    }
    else if (token == "RATIO") {
      if (useRange) {
        setupHuntInfoRange(RatioHuntInfo, range);
        DEBUG3("(ratio/range)\n");
      } else {
        RatioHuntInfo.coords[level] = proximity;
        DEBUG3("(ratio/%i/%f)\n", level, proximity);
      }
    }
    else if (token == "LEADER") {
      if (useRange) {
        setupHuntInfoRange(LeaderHuntInfo, range);
        DEBUG3("(leader/range)\n");
      } else {
        LeaderHuntInfo.coords[level] = proximity;
        DEBUG3("(leader/%i/%f)\n", level, proximity);
      }
    }
    else {
      std::vector<std::string> flags = TextUtils::tokenize(token, "?");
      if (flags.size() == 1) {
        // SoloFlags
        FlagType* ft = Flag::getDescFromAbbreviation(flags[0].c_str());
        if (ft != Flags::Null) {
          FlagHuntInfoMap& fhi = FlagPairs[Flags::Null]; // reserved for solos
          HuntInfo& hi = fhi[ft];
          if (useRange) {
            setupHuntInfoRange(hi, range);
            DEBUG3("(soloFlag/range)\n");
          } else {
            hi.coords[level] = proximity;
            DEBUG3("(soloFlag/%i/%f)\n", level, proximity);
          }
        } else {
          errors.push_back(tokens[i]);
          DEBUG3("Error parsing solo flag type\n");
        }
      }
      else if (flags.size() == 2) {
        // FlagPairs
        FlagType* ft0 = Flag::getDescFromAbbreviation(flags[0].c_str());
        FlagType* ft1 = Flag::getDescFromAbbreviation(flags[1].c_str());
        if ((ft0 != Flags::Null) && (ft1 != Flags::Null)) {
          FlagHuntInfoMap& fhi = FlagPairs[ft0];
          HuntInfo& hi = fhi[ft1];
          if (useRange) {
            setupHuntInfoRange(hi, range);
            DEBUG3("(flagPair/range)\n");
          } else {
            hi.coords[level] = proximity;
            DEBUG3("(flagPair/%i/%f)\n", level, proximity);
          }
        } else {
          errors.push_back(tokens[i]);
          DEBUG3("Error parsing flag pair type\n");
        }
      }
      else {
        errors.push_back(tokens[i]);
        DEBUG3("Error parsing token type\n");
      }
    }
  }

  if (errors.size() > 0) {
    std::string parseError = "AutoHunt: ";
    parseError += cmd;
    controlPanel->addMessage(parseError, -1);
    for (int i = 0; i < (int)errors.size(); i++) {
      std::string error = "  Bad Token: ";
      error += errors[i];
      controlPanel->addMessage(error, -1);
    }
  }

  return;
}


/******************************************************************************/

static void setupHuntInfoRange(HuntInfo& hi, const HuntRange& hr)
{
  const int levelCount = (hr.maxLevel - hr.minLevel);
  if (levelCount <= 0) {
    if (hr.minLevel > 0) {
      hi.coords[hr.minLevel] = hr.minProximity;
    }
    return;
  }
  const float range = (hr.minProximity - hr.maxProximity);
  const float delta = range / (float)levelCount;
  int step = 0;
  for (int i = hr.minLevel; i <= hr.maxLevel; i++) {
    if (i > 0) {
      hi.coords[i] = hr.minProximity - ((float)step * delta);
    }
    step++;
  }
  return;
}


/******************************************************************************/

static void printMaps()
{
  if (debugLevel < 3) {
    return;
  }

  std::map<int, float>::const_iterator cit;
  FlagPairMap::const_iterator it;
  for (it = FlagPairs.begin(); it != FlagPairs.end(); it++) {
    DEBUG3("  %s\n", (it->first == Flags::Null) ? "NULL" : it->first->flagAbbv);
    const FlagHuntInfoMap& fhim = it->second;
    FlagHuntInfoMap::const_iterator fhim_it;
    for (fhim_it = fhim.begin(); fhim_it != fhim.end(); fhim_it++) {
      DEBUG3("    %s\n", fhim_it->first->flagAbbv);
      const HuntInfo& hi = fhim_it->second;
      for (cit = hi.coords.begin(); cit != hi.coords.end(); cit++) {
        DEBUG3("      level: %i, dist: %f\n", cit->first, cit->second);
      }
    }
  }

  DEBUG3("Tk:\n");
  const HuntInfo& thi = TkHuntInfo;
  for (cit = thi.coords.begin(); cit != thi.coords.end(); cit++) {
    DEBUG3("  level: %i, dist: %f\n", cit->first, cit->second);
  }

  DEBUG3("Ratio:\n");
  const HuntInfo& rhi = RatioHuntInfo;
  for (cit = rhi.coords.begin(); cit != rhi.coords.end(); cit++) {
    DEBUG3("  level: %i, dist: %f\n", cit->first, cit->second);
  }

  DEBUG3("Leader:\n");
  const HuntInfo& lhi = RatioHuntInfo;
  for (cit = lhi.coords.begin(); cit != lhi.coords.end(); cit++) {
    DEBUG3("  level: %i, dist: %f\n", cit->first, cit->second);
  }

  return;
}


/******************************************************************************/

static Player* findEnemyLeader()
{
  World* world = World::getWorld();
  if (world == NULL) {
    return NULL;
  }
  Player** players = (Player**)world->getPlayers();
  if (players == NULL) {
    return NULL;
  }
  const int maxPlayers = world->getCurMaxPlayers();

  int maxScore = -123456789;
  Player* leader = NULL;
  for (int i = 0; i < maxPlayers; i++) {
    Player* p = players[i];
    if ((p != NULL) && isEnemy(p)) {
      const int pScore = p->getScore();
      if (pScore > maxScore) {
        leader = p;
        maxScore = pScore;
      }
    }
  }

  return leader;
}


/******************************************************************************/

// FIXME: rabbits?

static bool isEnemy(const Player* p)
{
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if ((myTank == NULL) || (p == NULL)) {
    return false;
  }

  const TeamColor myTeam = myTank->getTeam();
  if (myTeam == ObserverTeam) {
    return false;
  }

  const TeamColor playerTeam = p->getTeam();
  if (playerTeam == ObserverTeam) {
    return false;
  }

  if ((myTeam == RogueTeam) || (myTeam != playerTeam)) {
    return true;
  }

  return false;
}


/******************************************************************************/

static bool isBeatingMe(const Player* p)
{
  if (p == NULL) {
    return false;
  }
  if (p->getLocalLosses() > p->getLocalWins()) {
    return true;
  }
  return false;
}


/******************************************************************************/

static bool isTeamKiller(const Player* p, float tkRatio)
{
  if (tkRatio > 0.0) {
    if (((p->getWins() > 0) && (p->getTKRatio() > tkRatio)) ||
        ((p->getWins() == 0) && (p->getTeamKills() >= 3))) {
      return true;
    }
  }
  return false;
}
                            

/******************************************************************************/
                            
static void clearPlayers()
{
  World* world = World::getWorld();
  if (world == NULL) {
    return;
  }
  Player** players = (Player**)world->getPlayers();
  if (players == NULL) {
    return;
  }
  const int maxPlayers = world->getCurMaxPlayers();

  // clear the autohunt status
  for (int i = 0; i < maxPlayers; i++) {
    Player* p = players[i];
    if (p != NULL) {
      p->setAutoHuntLevel(0);
    }
  }

  return;
}


/******************************************************************************/

static float distBetweenPlayers(const Player* p0, const Player* p1)
{
  const float* pos0 = p0->getPosition();
  const float* pos1 = p1->getPosition();
  const float dx = pos0[0] - pos1[0];
  const float dy = pos0[1] - pos1[1];
  const float dz = pos0[2] - pos1[2];
  return sqrtf((dx * dx) + (dy * dy) + (dz * dz));
}


/******************************************************************************/

static int getHuntInfoLevel(const HuntInfo& hi, float dist)
{
  // search backwards, from highest level to lowest level
  std::map<int, float>::const_reverse_iterator rit;
  for (rit = hi.coords.rbegin(); rit != hi.coords.rend(); rit++) {
    if (dist < rit->second) {
      return rit->first;
    }
  }
  return 0;
}


/******************************************************************************/

static void huntPlayers()
{
  MaxLevel = 0;

  World* world = World::getWorld();
  if (world == NULL) {
    return;
  }
  Player** players = (Player**)world->getPlayers();
  if (players == NULL) {
    return;
  }
  const int maxPlayers = world->getCurMaxPlayers();

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank == NULL) {
    return;
  }
  const bool observer = (myTank->getTeam() == ObserverTeam);
  
  // for detecting team killers
  const float tkWarnRatio = BZDB.eval("tkwarnratio");

  // setup the paired flag map
  const FlagType* myFlagType = myTank->getFlag();
  const FlagHuntInfoMap* pairedMap = NULL;
  if (myFlagType != Flags::Null) {
    FlagPairMap::const_iterator fpm_it = FlagPairs.find(myFlagType);
    if (fpm_it != FlagPairs.end()) {
      pairedMap = &(fpm_it->second);
    }
  }

  // setup the solo flag map
  const FlagHuntInfoMap& soloMap = FlagPairs[Flags::Null];

  // setup hunt level based on player status
  for (int i = 0; i < maxPlayers; i++) {

    Player* p = players[i];
    if (p == NULL) {
      continue;
    }
    const bool myEnemy = isEnemy(p);

    // get the distance
    const float dist = distBetweenPlayers(myTank, p);

    // flag type
    FlagType* ft = p->getFlag();
    if (ft != Flags::Null) {
      // solo flag type
      if (myEnemy || observer) {
        FlagHuntInfoMap::const_iterator sit = soloMap.find(ft);
        if (sit != soloMap.end()) {
          const HuntInfo& hi = sit->second;
          const int level = getHuntInfoLevel(hi, dist);
          if (level > p->getAutoHuntLevel()) {
            p->setAutoHuntLevel(level);
          }
        }
      }
      // paired flag type
      if (myEnemy && (pairedMap != NULL)) {
        FlagHuntInfoMap::const_iterator pit = pairedMap->find(ft);
        if (pit != pairedMap->end()) {
          const HuntInfo& hi = pit->second;
          const int level = getHuntInfoLevel(hi, dist);
          if (level > p->getAutoHuntLevel()) {
            p->setAutoHuntLevel(level);
          }
        }
      }
    }

    // player ratio
    if (myEnemy && RatioHuntInfo.active() && isBeatingMe(p)) {
      const int level = getHuntInfoLevel(RatioHuntInfo, dist);
      if (level > p->getAutoHuntLevel()) {
        p->setAutoHuntLevel(level);
      }
    }

    // teamkill ratio
    if (!myEnemy && TkHuntInfo.active() && isTeamKiller(p, tkWarnRatio)) {
      const int level = getHuntInfoLevel(TkHuntInfo, dist);
      if (level > p->getAutoHuntLevel()) {
        p->setAutoHuntLevel(level);
      }
    }

    // update MaxLevel
    if (p->getAutoHuntLevel() > MaxLevel) {
      MaxLevel = p->getAutoHuntLevel();
    }
  }

  // set the autohunt status based on score leadership
  if (LeaderHuntInfo.active()) {
    Player* leader = findEnemyLeader();
    if (leader != NULL) {
      const float dist = distBetweenPlayers(myTank, leader);
      const int level = getHuntInfoLevel(LeaderHuntInfo, dist);
      if (level > leader->getAutoHuntLevel()) {
        leader->setAutoHuntLevel(level);
        // update MaxLevel
        if (level > MaxLevel) {
          MaxLevel = level;
        }
      }
    }
  }

  return;
}


/******************************************************************************/

static std::string getExpandedString(const std::string& varname)
{
  static int depth = 0;
  if (depth >= 8) { // recursion protection
    return "";
  }

  const std::string init = BZDB.get(varname);
  if (init.size() == 0) {
    return "";
  }

  std::string expanded;
  const std::vector<std::string> tokens = TextUtils::tokenize(init, ":");
  for (int i = 0; i < (int)tokens.size(); i++) {
    std::string token = tokens[i];
    if (token[0] == '*') {
      depth++;
      token = getExpandedString(tokens[i].c_str() + 1);
      depth--;
    }
    expanded += token;
    if (i < ((int)tokens.size() - 1)) {
      expanded += ':';
    }
  }

  return expanded;
}


/******************************************************************************/
/******************************************************************************/

void AutoHunt::update()
{
  static std::string active = "";

  std::string current = getExpandedString("autohunt");
  if ((current.size() == 0) && (active.size() == 0)) {
    return;
  }

  MaxLevel = 0;

  clearPlayers();

  if (active != current) {
    active = current;
    parse(active);
    printMaps(); // for debugging
  }

  huntPlayers();

  return;
}


/******************************************************************************/

const char* AutoHunt::getColorString(int level)
{
  switch (level) {
    case 1: return ANSI_STR_FG_BLACK;
    case 2: return ANSI_STR_FG_MAGENTA;
    case 3: return ANSI_STR_FG_BLUE;
    case 4: return ANSI_STR_FG_GREEN;
    case 5: return ANSI_STR_FG_YELLOW;
    case 6: return ANSI_STR_FG_ORANGE;
    case 7: return ANSI_STR_FG_RED;
    case 8: return ANSI_STR_FG_WHITE;
    case 9: return ANSI_STR_FG_WHITE ANSI_STR_UNDERLINE;
    default: return ANSI_STR_FG_BLACK;
  }
}


/******************************************************************************/

int AutoHunt::getMaxLevel()
{
  return MaxLevel;
}


/******************************************************************************/

int AutoHunt::getRadarLevel()
{
  return RadarLevel;
}


/******************************************************************************/

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

