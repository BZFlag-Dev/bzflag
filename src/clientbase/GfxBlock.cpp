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
#include <map>

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


//============================================================================//
//============================================================================//

static std::map<std::string, int> name2idMap;
static std::map<int, std::string> id2nameMap;


static void pushEntry(int id, const std::string& name) {
  name2idMap[name] = id;
  id2nameMap[id] = name;
}


//============================================================================//
//============================================================================//


static bool isBetterClient(EventClient* a, EventClient* b, int orderType) {
  const int aOrder = a->GetOrder(orderType);
  const int bOrder = b->GetOrder(orderType);
  if (aOrder < bOrder) { return true;  }
  if (aOrder > bOrder) { return false; }

  const std::string& aName = a->GetName();
  const std::string& bName = b->GetName();
  if (aName < bName) { return true;  }
  if (aName > bName) { return false; }

  return (a < b);
}


//============================================================================//
//============================================================================//
//
//  GfxBlock
//

GfxBlock::GfxBlock()
  : type(-1)
  , id(-1)
  , world(false) {
}


GfxBlock::GfxBlock(int _type, int _id, bool _world)
  : type(_type)
  , id(_id)
  , world(_world) {
}


GfxBlock::GfxBlock(int _id, const char* name, bool _world)
  : type(Global)
  , id(_id)
  , world(_world) {
  pushEntry(id, name);
  GfxBlockMgr::blocks[id] = this;
}


GfxBlock::~GfxBlock() {
  clear();
}


void GfxBlock::init(int _type, int _id, bool _world) {
  id    = _id;
  type  = _type;
  world = _world;
}


//============================================================================//

void GfxBlock::clear() {
  if (!clients.empty()) {
    clients[0]->LostGfxBlock(type, id); // notify the old owner
  }
  clients.clear();
}


bool GfxBlock::set(EventClient* ec, bool queue) {
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
    const int orderType = world ? EventClient::DrawWorldOrder
                          : EventClient::DrawScreenOrder;
    if (isBetterClient(ec, clients[i], orderType)) {
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


bool GfxBlock::remove(EventClient* ec) {
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


//============================================================================//

const char* GfxBlock::getTypeString(int type) {
  switch ((BlockType)type) {
    case Global:    { return "global";      }
    case Tank:      { return "tank";        }
    case Shot:      { return "shot";        }
    case Flag:      { return "flag";        }
    case TankRadar: { return "tankradar";   }
    case ShotRadar: { return "shotradar";   }
    case FlagRadar: { return "flagradar";   }
    default: {
      return "unknown";
    }
  }
}


int GfxBlock::getStringType(const char* name) {
  const std::string key = name;
  if (key == "global")    { return Global;    }
  else if (key == "tank")      { return Tank;      }
  else if (key == "shot")      { return Shot;      }
  else if (key == "flag")      { return Flag;      }
  else if (key == "tankradar") { return TankRadar; }
  else if (key == "shotradar") { return ShotRadar; }
  else if (key == "flagradar") { return FlagRadar; }
  return -1;
}


//============================================================================//
//============================================================================//
//
//  GfxBlockMgr
//

GfxBlock* GfxBlockMgr::blocks[BlockIDCount];


#undef  GLOBAL_GFX_BLOCK
#define GLOBAL_GFX_BLOCK(Enum, Name, World) \
  GfxBlock GfxBlockMgr::Name (Enum, #Name, World)

// world items
GLOBAL_GFX_BLOCK(Obstacles,    obstacles,  true);
GLOBAL_GFX_BLOCK(Sky,          sky,        true);
GLOBAL_GFX_BLOCK(Stars,        stars,      true);
GLOBAL_GFX_BLOCK(Clouds,       clouds,     true);
GLOBAL_GFX_BLOCK(Ground,       ground,     true);
GLOBAL_GFX_BLOCK(Lights,       lights,     true);
GLOBAL_GFX_BLOCK(Mirror,       mirror,     true);
GLOBAL_GFX_BLOCK(Shadows,      shadows,    true);
GLOBAL_GFX_BLOCK(Mountains,    mountains,  true);
GLOBAL_GFX_BLOCK(Explosions,   explosions, true);
GLOBAL_GFX_BLOCK(Insides,      insides,    true);
GLOBAL_GFX_BLOCK(Halos,        halos,      true);
GLOBAL_GFX_BLOCK(TrackMarks,   trackMarks, true);
GLOBAL_GFX_BLOCK(Weather,      weather,    true);
GLOBAL_GFX_BLOCK(Effects,      effects,    true);

// Screen items
GLOBAL_GFX_BLOCK(Cursor,       cursor,       false);
GLOBAL_GFX_BLOCK(Console,      console,      false);
GLOBAL_GFX_BLOCK(Radar,        radar,        false);
GLOBAL_GFX_BLOCK(TeamScores,   teamScores,   false);
GLOBAL_GFX_BLOCK(PlayerScores, playerScores, false);
GLOBAL_GFX_BLOCK(Menu,         menu,         false);
GLOBAL_GFX_BLOCK(Compose,      compose,      false);
GLOBAL_GFX_BLOCK(TargetBox,    targetBox,    false);
GLOBAL_GFX_BLOCK(ShotStatus,   shotStatus,   false);
GLOBAL_GFX_BLOCK(Markers,      markers,      false);
GLOBAL_GFX_BLOCK(Times,        times,        false);
GLOBAL_GFX_BLOCK(Labels,       labels,       false);
GLOBAL_GFX_BLOCK(Cracks,       cracks,       false);
GLOBAL_GFX_BLOCK(Status,       status,       false);
GLOBAL_GFX_BLOCK(Clock,        clock,        false);
GLOBAL_GFX_BLOCK(FlagHelp,     flagHelp,     false);
GLOBAL_GFX_BLOCK(Alerts,       alerts,       false);

#undef GLOBAL_GFX_BLOCK


//============================================================================//

static bool clearPlayerBlocks(Player* player, void* data) {
  EventClient* ec = (EventClient*)data;

  // clear the player block
  player->getGfxBlock().remove(ec);
  player->getRadarGfxBlock().remove(ec);

  // clear the player shot blocks
  const int maxShots = player->getMaxShots();
  for (int s = 0; s < maxShots; s++) {
    ShotPath* shot = player->getShot(s);
    if (shot) {
      shot->getGfxBlock().remove(ec);
      shot->getRadarGfxBlock().remove(ec);
    }
  }

  return false; // do not break the loop
}


void GfxBlockMgr::removeClient(EventClient* ec) {
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


//============================================================================//

const char* GfxBlockMgr::getIDString(int id) {
  std::map<int, std::string>::const_iterator it = id2nameMap.find(id);
  if (it == id2nameMap.end()) {
    return "unknown";
  }
  return it->second.c_str();
}


int GfxBlockMgr::getStringID(const char* name) {
  std::map<std::string, int>::const_iterator it = name2idMap.find(name);
  if (it == name2idMap.end()) {
    return -1;
  }
  return it->second;
}


//============================================================================//

void GfxBlockMgr::check() {
  assert(name2idMap.size() == BlockIDCount);
  assert(id2nameMap.size() == BlockIDCount);
}


struct InitCheck { InitCheck() { GfxBlockMgr::check(); } } initCheck;


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
