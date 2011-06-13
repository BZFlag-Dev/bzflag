/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#pragma warning( 4: 4786 )
#endif

// class-interface header
#include "WorldWeapons.h"

#include "WorldInfo.h"
// system headers
#include <vector>

// common-interface headers
#include "Protocol.h"
#include "bzfsMessages.h"
#include "bzfs/bzfsAPI.h"
#include "common/BzTime.h"
#include "common/StateDatabase.h"
#include "game/NetMessage.h"
#include "game/ShotUpdate.h"
#include "net/Address.h"

// bzfs specific headers
#include "bzfs.h"

extern bz_eTeamType convertTeam(TeamColor team);
extern TeamColor convertTeam(bz_eTeamType team);


static int fireWorldWepReal(FlagType* type, float lifetime, PlayerId player,
                            TeamColor teamColor, const fvec3& pos, float tilt,
                            float dir, int shotID, float dt) {
  FiringInfo firingInfo;
  firingInfo.timeSent = BzTime::getCurrent().getSeconds();
  firingInfo.flagType = type;
  firingInfo.lifetime = lifetime;
  firingInfo.shotType = type->flagShot;
  firingInfo.shot.player = player;
  firingInfo.shot.pos = pos;
  const float shotSpeed = BZDB.eval(BZDBNAMES.SHOTSPEED);
  const float tiltFactor = cosf(tilt);
  firingInfo.shot.vel.x = shotSpeed * tiltFactor * cosf(dir);
  firingInfo.shot.vel.y = shotSpeed * tiltFactor * sinf(dir);
  firingInfo.shot.vel.z = shotSpeed * sinf(tilt);
  firingInfo.shot.id = shotID;
  firingInfo.shot.dt = dt;

  firingInfo.shot.team = teamColor;

  NetMessage netMsg;
  firingInfo.pack(netMsg);

  if (BZDB.isTrue(BZDBNAMES.WEAPONS)) {
    netMsg.broadcast(MsgWShotBegin);
  }

  return shotID;
}

static int fireWorldGMReal(FlagType* type, PlayerId targetPlayerID, float
                           lifetime, PlayerId player, const fvec3& pos,
                           float tilt, float dir, int shotID, float dt) {
  FiringInfo firingInfo;
  firingInfo.timeSent = BzTime::getCurrent().getSeconds();
  firingInfo.flagType = type;
  firingInfo.lifetime = lifetime;
  firingInfo.shot.player = player;
  firingInfo.shot.pos = pos;
  const float shotSpeed = BZDB.eval(BZDBNAMES.SHOTSPEED);
  const float tiltFactor = cosf(tilt);
  firingInfo.shot.vel.x = shotSpeed * tiltFactor * cosf(dir);
  firingInfo.shot.vel.y = shotSpeed * tiltFactor * sinf(dir);
  firingInfo.shot.vel.z = shotSpeed * sinf(tilt);
  firingInfo.shot.id = shotID;
  firingInfo.shot.dt = dt;

  firingInfo.shot.team = RogueTeam;

  // Target the GM, construct and send packet
  if (BZDB.isTrue(BZDBNAMES.WEAPONS)) {
    NetMessage netMsg;
    firingInfo.pack(netMsg);
    netMsg.broadcast(MsgShotBegin);
    netMsg.packUInt8(targetPlayerID);
    netMsg.broadcast(MsgGMUpdate);
  }

  return shotID;
}


WorldWeapons::WorldWeapons()
  : worldShotId(0) {
}


WorldWeapons::~WorldWeapons() {
  clear();
}


void WorldWeapons::clear(void) {
  for (std::vector<Weapon*>::iterator it = weapons.begin();
       it != weapons.end(); ++it) {
    Weapon* w = *it;
    delete w;
  }
  weapons.clear();
}


float WorldWeapons::nextTime() {
  BzTime nextShot = BzTime::getSunExplodeTime();
  for (std::vector<Weapon*>::iterator it = weapons.begin();
       it != weapons.end(); ++it) {
    Weapon* w = *it;
    if (w->nextTime <= nextShot) {
      nextShot = w->nextTime;
    }
  }
  return (float)(nextShot - BzTime::getCurrent());
}


void WorldWeapons::fire() {
  BzTime nowTime = BzTime::getCurrent();

  for (std::vector<Weapon*>::iterator it = weapons.begin();
       it != weapons.end(); ++it) {
    Weapon* w = *it;
    if (w->nextTime <= nowTime) {

      const float reloadTime = BZDB.eval(BZDBNAMES.RELOADTIME);
      fireWorldWepReal((FlagType*)w->type, reloadTime, ServerPlayer,
                       w->teamColor, w->origin, w->tilt, w->direction,
                       getNewWorldShotID(), 0);

      // Set up timer for next shot, and eat any shots that have been missed
      while (w->nextTime <= nowTime) {
        w->nextTime += w->delay[w->nextDelay];
        w->nextDelay++;
        if (w->nextDelay == (int)w->delay.size()) {
          w->nextDelay = 0;
        }
      }
    }
  }
}


void WorldWeapons::add(const FlagType* type, const fvec3& origin,
                       float direction, float tilt, TeamColor teamColor,
                       float initdelay, const std::vector<float> &delay,
                       BzTime& sync, bool fromMesh) {
  Weapon* w = new Weapon();
  w->type = type;
  w->teamColor = teamColor;
  memmove(&w->origin, origin, 3 * sizeof(float));
  w->direction = direction;
  w->tilt = tilt;
  w->nextTime = sync;
  w->nextTime += initdelay;
  w->initDelay = initdelay;
  w->nextDelay = 0;
  w->delay = delay;
  w->fromMesh = fromMesh;

  weapons.push_back(w);
}


unsigned int WorldWeapons::count(void) {
  return weapons.size();
}


void* WorldWeapons::pack(void* buf) const {
  uint32_t realCount = 0;
  for (unsigned int i = 0 ; i < weapons.size(); i++) {
    const Weapon* w = (const Weapon*) weapons[i];
    if (!w->fromMesh) {
      realCount++;
    }
  }

  buf = nboPackUInt32(buf, realCount);

  for (unsigned int i = 0 ; i < weapons.size(); i++) {
    const Weapon* w = (const Weapon*) weapons[i];
    if (w->fromMesh) {
      continue;
    }
    buf = w->type->pack(buf);
    buf = nboPackFVec3(buf, w->origin);
    buf = nboPackFloat(buf, w->direction);
    buf = nboPackFloat(buf, w->initDelay);
    buf = nboPackUInt16(buf, w->delay.size());
    for (unsigned int j = 0; j < w->delay.size(); j++) {
      buf = nboPackFloat(buf, w->delay[j]);
    }
  }
  return buf;
}


int WorldWeapons::packSize(void) const {
  int fullSize = 0;

  fullSize += sizeof(uint32_t);

  for (unsigned int i = 0 ; i < weapons.size(); i++) {
    const Weapon* w = (const Weapon*) weapons[i];
    if (w->fromMesh) {
      continue;
    }
    fullSize += FlagType::packSize; // flag type
    fullSize += sizeof(fvec3); // pos
    fullSize += sizeof(float);    // direction
    fullSize += sizeof(float);    // init delay
    fullSize += sizeof(uint16_t); // delay count
    for (unsigned int j = 0; j < w->delay.size(); j++) {
      fullSize += sizeof(float);
    }
  }

  return fullSize;
}


int WorldWeapons::getNewWorldShotID(void) {
  if (worldShotId > _MAX_WORLD_SHOTS) {
    worldShotId = 0;
  }
  return worldShotId++;
}


//----------WorldWeaponGlobalEventHandler---------------------
// where we do the world weapon handling for event based shots since they are not really done by the "world"

WorldWeaponGlobalEventHandler::WorldWeaponGlobalEventHandler(FlagType* _type,
    const fvec3& _origin,
    float _direction,
    float _tilt,
    TeamColor teamColor) {
  type = _type;
  origin = _origin;
  direction = _direction;
  tilt = _tilt;
  team = convertTeam(teamColor);
}


WorldWeaponGlobalEventHandler::~WorldWeaponGlobalEventHandler() {
}


void WorldWeaponGlobalEventHandler::process(bz_EventData* eventData) {
  if (!eventData) {
    return;
  }

  switch (eventData->eventType) {
    case bz_eCaptureEvent: {
      bz_CTFCaptureEventData_V1* capEvent = (bz_CTFCaptureEventData_V1*)eventData;
      if (capEvent->teamCapped != team) {
        return;
      }
      break;
    }
    case bz_ePlayerDieEvent:
    case bz_ePlayerSpawnEvent: {
      break;
    }
    default: {
      return;
    }
  }

  fireWorldWepReal(type, BZDB.eval(BZDBNAMES.RELOADTIME),
                   ServerPlayer, convertTeam(team), origin, tilt, direction,
                   world->getWorldWeapons().getNewWorldShotID(), 0);
}


// for bzfsAPI: it needs to be global
int fireWorldWep(FlagType* type, float lifetime, PlayerId player,
                 const fvec3& pos, float tilt, float direction,
                 int shotID, float dt) {
  return fireWorldWepReal(type, lifetime, player, RogueTeam,
                          pos, tilt, direction, shotID, dt);
}


int fireWorldGM(FlagType* type, PlayerId targetPlayerID, float lifetime,
                PlayerId player, const fvec3& pos, float tilt, float direction,
                int shotID, float dt) {
  return fireWorldGMReal(type, targetPlayerID, lifetime, player, pos, tilt,
                         direction, shotID, dt);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
