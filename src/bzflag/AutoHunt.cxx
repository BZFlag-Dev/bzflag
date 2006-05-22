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
#include <set>
#include <vector>
#include <string>

// common headers
#include "global.h"
#include "Flag.h"
#include "TextUtils.h"
#include "StateDatabase.h"

// local headers
#include "World.h"
#include "Player.h"
#include "LocalPlayer.h"


// typedefs
typedef std::multimap<const FlagType*, const FlagType*> FlagPairMap;


// static data
static std::set<const FlagType*> SoloFlags;
static FlagPairMap FlagPairs;
static bool HuntRatio = false;
static bool HuntLeader = false;
static std::set<const FlagType*> ActiveTargetFlags;

// static functions
static bool parse(const std::string& msg);
static void huntPlayers();
static void clearPlayers();
static void setupActiveFlags();
static bool isEnemy(const Player*);
static bool isBeatingMe(const Player*);
static Player* findEnemyLeader();


/******************************************************************************/

static bool parse(const std::string& cmd)
{
  bool error = false;

  HuntRatio = false;
  HuntLeader = false;
  FlagPairs.clear();
  SoloFlags.clear();

  DEBUG3("AutoHunt\n");
  std::vector<std::string> tokens = TextUtils::tokenize(cmd, ":");

  for (int i = 0; i < (int)tokens.size(); i++) {
    std::string token = TextUtils::toupper(tokens[i]);

    DEBUG3("  Token: %-10s", token.c_str());

    if (token == "RATIO") {
      HuntRatio = true;
      DEBUG3("(ratio)\n");
    }
    else if (token == "LEADER") {
      HuntLeader = true;
      DEBUG3("(leader)\n");
    }
    else {
      std::vector<std::string> flags = TextUtils::tokenize(token, "?");
      if (flags.size() == 1) {
        FlagType* ft = Flag::getDescFromAbbreviation(flags[0].c_str());
        if (ft != Flags::Null) {
          SoloFlags.insert(ft);
          DEBUG3("(soloFlag)\n");
        } else {
          error = true;
          DEBUG3("ERROR\n");
        }
      }
      else if (flags.size() == 2) {
        FlagType* ft0 = Flag::getDescFromAbbreviation(flags[0].c_str());
        FlagType* ft1 = Flag::getDescFromAbbreviation(flags[1].c_str());
        if ((ft0 != Flags::Null) && (ft1 != Flags::Null)) {
          FlagPairs.insert(FlagPairMap::value_type(ft0, ft1));
          DEBUG3("(flagPair)\n");
        } else {
          error = true;
          DEBUG3("ERROR\n");
        }
      }
      else {
        error = true;
        DEBUG3("ERROR\n");
      }
    }
  }
  
  return error;
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
  // clear the autohunt status
  for (int i = 0; i < maxPlayers; i++) {
    Player* p = players[i];
    if ((p != NULL) && isEnemy(p)) {
      const int pScore = p->getWins();
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
      p->setAutoHunted(false);
    }
  }
  
  return;
}


/******************************************************************************/


static void huntPlayers()
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
  
  // set the autohunt status based on active flag types and ratio
  for (int i = 0; i < maxPlayers; i++) {
    Player* p = players[i];
    if ((p != NULL) && isEnemy(p)) {
      // flag type
      FlagType* ft = p->getFlag();
      if (ft != Flags::Null) {
        if (ActiveTargetFlags.find(ft) != ActiveTargetFlags.end()) {
          p->setAutoHunted(true);
        }
      }
      // player ratio
      if (HuntRatio && isBeatingMe(p)) {
        p->setAutoHunted(true);
      }
    }
  }

  // set the autohunt status based on score leadership
  if (HuntLeader) {
    Player* leader = findEnemyLeader();
    if (leader != NULL) {
      leader->setAutoHunted(true);
    }
  }
  
  return;
}


/******************************************************************************/

static void setupActiveFlags()
{
  // copy the solo flags
  ActiveTargetFlags = SoloFlags;

  // activate any flags associated to with the local player's flag
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank) {
    FlagType* ft = myTank->getFlag();
    if (ft != Flags::Null) {
      FlagPairMap::const_iterator it;
      FlagPairMap::const_iterator first = FlagPairs.lower_bound(ft);
      FlagPairMap::const_iterator last = FlagPairs.upper_bound(ft);
      for (it = first; it != last; it++) {
        ActiveTargetFlags.insert(it->second);
      }
    }
  }
  return;
}


/******************************************************************************/

void AutoHunt::update()
{
  static std::string active = "";

  std::string current = BZDB.get("autohunt");
  if ((current.size() == 0) && (active.size() == 0)) {
    return;
  }

  clearPlayers();

  if (active != current) {
    active = current;
    parse(active);
  }

  if (active.size() > 0) {
    setupActiveFlags();
    huntPlayers();
  }
  
  return;
}


/******************************************************************************/

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

