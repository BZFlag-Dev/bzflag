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

#include "common.h"

// implemention header
#include "GfxBlock.h"

// system headers
#include <assert.h>
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;

// common headers
#include "EventClient.h"

// bzflag headers
#include "World.h"
#include "ClientFlag.h"
#include "ShotPath.h"
#include "Player.h"
#include "LocalPlayer.h"
#include "WorldPlayer.h"
#include "Roster.h"


/******************************************************************************/
/******************************************************************************/

static map<string, int> name2idMap;
static map<int, string> id2nameMap;


static void pushEntry(int id, const string& name)
{
  name2idMap[name] = id;
  id2nameMap[id] = name;
}


/******************************************************************************/
/******************************************************************************/


static bool isBetterClient(EventClient* a, EventClient* b)
{
  if (a->GetOrder() < b->GetOrder()) { return true; }
  if (a->GetOrder() > b->GetOrder()) { return false; }
  return (a->GetName() < b->GetName());
}


/******************************************************************************/
/******************************************************************************/
//
//  GfxBlock
//

GfxBlock::GfxBlock()
: type(-1)
, id(-1)
{
}


GfxBlock::GfxBlock(int _type, int _id)
: type(_type)
, id(_id)
{
}


GfxBlock::GfxBlock(int _id, const char* name)
: type(Global)
, id(_id)
{
  pushEntry(id, name);
  GfxBlockMgr::blocks[id] = this;
}


GfxBlock::~GfxBlock()
{
  clear();
}


/******************************************************************************/

void GfxBlock::clear()
{
  if (!clients.empty()) {
    clients[0]->LostGfxBlock(type, id); // notify the old owner
  }
  clients.clear();
}


bool GfxBlock::set(EventClient* ec, bool queue)
{
  if (ec == NULL) {
    return false;
  }
  if (clients.empty()) {
    clients.push_back(ec);
    return true; // proud new owner
  }
  if (clients[0] == ec) {
    return true; // already own it
  }
  for (size_t i = 0; i < clients.size(); i++) {
    if (clients[i] == ec) {
      return false;
    }
    if (isBetterClient(ec, clients[i])) {
      clients.insert(clients.begin() + i, ec);
      if (i == 0) {
        clients[1]->LostGfxBlock(type, id); // notify the old owner
        return true;
      }
      return false;
    }
  }
  if (queue) {
    clients.push_back(ec);
  }
  return false;
}


bool GfxBlock::remove(EventClient* ec)
{
  if (clients.empty()) {
    return false;
  }
  EventClient* top = clients[0];
  bool success = false;
  for (size_t i = 0; i < clients.size(); i++) {
    if (clients[i] == ec) {
      clients.erase(clients.begin() + i);
      success = true;
      i--;
    }
  }
  if (!clients.empty() && (clients[0] != top)) {
    clients[0]->GotGfxBlock(type, id); // notify the new owner
  }
  return success;
}


/******************************************************************************/

const char* GfxBlock::getTypeString(int type)
{
  switch ((BlockType)type) {
    case Global: { return "global"; }    
    case Tank:   { return "tank";   }    
    case Shot:   { return "shot";   }    
    case Flag:   { return "flag";   }    
    default: {
      return "unknown";
    }
  }
}


int GfxBlock::getStringType(const char* name)
{
  const string key = name;
       if (key == "global") { return Global; }
  else if (key == "tank")   { return Tank;   }
  else if (key == "shot")   { return Shot;   }
  else if (key == "flag")   { return Flag;   }
  return -1;
}
        

/******************************************************************************/
/******************************************************************************/
//
//  GfxBlockMgr
//

GfxBlock* GfxBlockMgr::blocks[BlockIDCount];


#undef  GLOBAL_GFX_BLOCK
#define GLOBAL_GFX_BLOCK(Enum, Name) GfxBlock GfxBlockMgr::Name (Enum, #Name)

// world items
GLOBAL_GFX_BLOCK(Obstacles,    obstacles);
GLOBAL_GFX_BLOCK(Sky,          sky);
GLOBAL_GFX_BLOCK(Stars,        stars);
GLOBAL_GFX_BLOCK(Clouds,       clouds);
GLOBAL_GFX_BLOCK(Ground,       ground);
GLOBAL_GFX_BLOCK(Lights,       lights);
GLOBAL_GFX_BLOCK(Mirror,       mirror);
GLOBAL_GFX_BLOCK(Shadows,      shadows);
GLOBAL_GFX_BLOCK(Mountains,    mountains);
GLOBAL_GFX_BLOCK(Explosions,   explosions);
GLOBAL_GFX_BLOCK(Insides,      insides);
GLOBAL_GFX_BLOCK(Halos,        halos);
GLOBAL_GFX_BLOCK(TrackMarks,   trackMarks);
GLOBAL_GFX_BLOCK(Weather,      weather);
GLOBAL_GFX_BLOCK(Effects,      effects);

// UI items
GLOBAL_GFX_BLOCK(Console,      console);
GLOBAL_GFX_BLOCK(Radar,        radar);
GLOBAL_GFX_BLOCK(TeamScores,   teamScores);
GLOBAL_GFX_BLOCK(PlayerScores, playerScores);
GLOBAL_GFX_BLOCK(Menu,         menu);
GLOBAL_GFX_BLOCK(Compose,      compose);
GLOBAL_GFX_BLOCK(TargetBox,    targetBox);
GLOBAL_GFX_BLOCK(ShotStatus,   shotStatus);
GLOBAL_GFX_BLOCK(Markers,      markers);
GLOBAL_GFX_BLOCK(Times,        times);
GLOBAL_GFX_BLOCK(Labels,       labels);
GLOBAL_GFX_BLOCK(Cracks,       cracks);
GLOBAL_GFX_BLOCK(Status,       status);
GLOBAL_GFX_BLOCK(Clock,        clock);
GLOBAL_GFX_BLOCK(FlagHelp,     flagHelp);
GLOBAL_GFX_BLOCK(Alerts,       alerts);

#undef GLOBAL_GFX_BLOCK


/******************************************************************************/

static bool clearPlayerBlocks(Player* player, void* data)
{
  EventClient* ec = (EventClient*)data;

  // clear the player block
  player->getGfxBlock().remove(ec);

  // clear the player shot blocks
  const int maxShots = player->getMaxShots();
  for (int s = 0; s < maxShots; s++) {
    ShotPath* shot = player->getShot(s);
    if (shot) {
      shot->getGfxBlock().remove(ec);
    }
  }

  return false; // do not break the loop
}


void GfxBlockMgr::removeClient(EventClient* ec)
{
  for (int i = 0; i < (int)BlockIDCount; i++) {
    blocks[i]->remove(ec);
  }

  // clear the flag blocks
  const World* world = World::getWorld();
  if (world != NULL) {
    for (int i = 0; i < world->getMaxFlags(); i++) {
      ClientFlag& flag = world->getClientFlag(i);
      flag.gfxBlock.remove(ec);
    }
  }

  // clear the players and their shots
  iteratePlayers(clearPlayerBlocks, ec, true /*includeWeapons*/);
}


/******************************************************************************/

const char* GfxBlockMgr::getIDString(int id)
{
  map<int, string>::const_iterator it = id2nameMap.find(id);
  if (it == id2nameMap.end()) {
    return "unknown";
  }
  return it->second.c_str();
}


int GfxBlockMgr::getStringID(const char* name)
{
  map<string, int>::const_iterator it = name2idMap.find(name);
  if (it == name2idMap.end()) {
    return -1;
  }
  return it->second;
}


/******************************************************************************/

void GfxBlockMgr::check()
{
  assert(name2idMap.size() == BlockIDCount);
  assert(id2nameMap.size() == BlockIDCount);
}


class SillyCheck {
  public:
    SillyCheck() { GfxBlockMgr::check(); }
};
static SillyCheck sillyCheck;


/******************************************************************************/
/******************************************************************************/




// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
