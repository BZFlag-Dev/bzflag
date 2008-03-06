/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "TimeKeeper.h"
#include "ShotUpdate.h"
#include "Protocol.h"
#include "Address.h"
#include "StateDatabase.h"
#include "bzfsAPI.h"

// bzfs specific headers
#include "bzfs.h"

char *getDirectMessageBuffer();
void broadcastMessage(uint16_t code, int len, const void *msg);


static int fireWorldWepReal(FlagType* type, float lifetime, PlayerId player,
			    TeamColor teamColor, float *pos, float tilt, float dir,
			    int shotID, float dt)
{
  void *buf, *bufStart = getDirectMessageBuffer();

  FiringInfo firingInfo;
  firingInfo.timeSent = (float)TimeKeeper::getCurrent().getSeconds();
  firingInfo.flagType = type;
  firingInfo.lifetime = lifetime;
  firingInfo.shot.player = player;
  memmove(firingInfo.shot.pos, pos, 3 * sizeof(float));
  const float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  const float tiltFactor = cosf(tilt);
  firingInfo.shot.vel[0] = shotSpeed * tiltFactor * cosf(dir);
  firingInfo.shot.vel[1] = shotSpeed * tiltFactor * sinf(dir);
  firingInfo.shot.vel[2] = shotSpeed * sinf(tilt);
  firingInfo.shot.id = shotID;
  firingInfo.shot.dt = dt;

  firingInfo.shot.team = teamColor;

  buf = firingInfo.pack(bufStart);

  if (BZDB.isTrue(StateDatabase::BZDB_WEAPONS)) {
    broadcastMessage(MsgShotBegin, (char *)buf - (char *)bufStart, bufStart);
  }
  return shotID;
}


static int fireWorldGMReal ( FlagType* type, PlayerId targetPlayerID, float
    lifetime, PlayerId player, float *pos, float tilt, float dir, int shotID,
    float dt)
{

  void *buf, *bufStart = getDirectMessageBuffer();

  FiringInfo firingInfo;
  firingInfo.timeSent = (float)TimeKeeper::getCurrent().getSeconds();
  firingInfo.flagType = type;
  firingInfo.lifetime = lifetime;
  firingInfo.shot.player = player;
  memmove(firingInfo.shot.pos, pos, 3 * sizeof(float));
  const float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  const float tiltFactor = cosf(tilt);
  firingInfo.shot.vel[0] = shotSpeed * tiltFactor * cosf(dir);
  firingInfo.shot.vel[1] = shotSpeed * tiltFactor * sinf(dir);
  firingInfo.shot.vel[2] = shotSpeed * sinf(tilt);
  firingInfo.shot.id = shotID;
  firingInfo.shot.dt = dt;

  firingInfo.shot.team = RogueTeam;

  buf = firingInfo.pack(bufStart);

  if (BZDB.isTrue(StateDatabase::BZDB_WEAPONS)) {
    broadcastMessage(MsgShotBegin, (char *)buf - (char *)bufStart,
                     bufStart);
  }

    // Target the gm.
    // construct and send packet

  char packet[ShotUpdatePLen + PlayerIdPLen];
  buf = (void*)packet;
  buf = firingInfo.shot.pack(buf);
  buf = nboPackUByte(buf, targetPlayerID);
  if (BZDB.isTrue(StateDatabase::BZDB_WEAPONS)) {
    broadcastMessage(MsgGMUpdate, sizeof(packet), packet);
  }

  return shotID;
}

WorldWeapons::WorldWeapons()
: worldShotId(0)
{
}


WorldWeapons::~WorldWeapons()
{
  clear();
}


void WorldWeapons::clear(void)
{
  for (std::vector<Weapon*>::iterator it = weapons.begin();
       it != weapons.end(); ++it) {
    Weapon *w = *it;
    delete w;
  }
  weapons.clear();
}


float WorldWeapons::nextTime ()
{
  TimeKeeper nextShot = TimeKeeper::getSunExplodeTime();
  for (std::vector<Weapon*>::iterator it = weapons.begin();
       it != weapons.end(); ++it) {
    Weapon *w = *it;
    if (w->nextTime <= nextShot) {
      nextShot = w->nextTime;
    }
  }
  return (float)(nextShot - TimeKeeper::getCurrent());
}


void WorldWeapons::fire()
{
  TimeKeeper nowTime = TimeKeeper::getCurrent();

  for (std::vector<Weapon*>::iterator it = weapons.begin();
       it != weapons.end(); ++it) {
    Weapon *w = *it;
    if (w->nextTime <= nowTime) {

      fireWorldWepReal((FlagType*)w->type, BZDB.eval(StateDatabase::BZDB_RELOADTIME),
		       ServerPlayer, w->teamColor, w->origin, w->tilt, w->direction,
		       getNewWorldShotID(), 0);

      //Set up timer for next shot, and eat any shots that have been missed
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


void WorldWeapons::add(const FlagType *type, const float *origin,
		       float direction, float tilt, TeamColor teamColor,
		       float initdelay, const std::vector<float> &delay,
		       TimeKeeper &sync)
{
  Weapon *w = new Weapon();
  w->type = type;
  w->teamColor = teamColor;
  memmove(&w->origin, origin, 3*sizeof(float));
  w->direction = direction;
  w->tilt = tilt;
  w->nextTime = sync;
  w->nextTime += initdelay;
  w->initDelay = initdelay;
  w->nextDelay = 0;
  w->delay = delay;

  weapons.push_back(w);
}


unsigned int WorldWeapons::count(void)
{
  return weapons.size();
}


void * WorldWeapons::pack(void *buf) const
{
  buf = nboPackUInt(buf, weapons.size());

  for (unsigned int i=0 ; i < weapons.size(); i++) {
    const Weapon *w = (const Weapon *) weapons[i];
    buf = w->type->pack (buf);
    buf = nboPackVector(buf, w->origin);
    buf = nboPackFloat(buf, w->direction);
    buf = nboPackFloat(buf, w->initDelay);
    buf = nboPackUShort(buf, w->delay.size());
    for (unsigned int j = 0; j < w->delay.size(); j++) {
      buf = nboPackFloat(buf, w->delay[j]);
    }
  }
  return buf;
}


int WorldWeapons::packSize(void) const
{
  int fullSize = 0;

  fullSize += sizeof(uint32_t);

  for (unsigned int i=0 ; i < weapons.size(); i++) {
    const Weapon *w = (const Weapon *) weapons[i];
    fullSize += FlagType::packSize; // flag type
    fullSize += sizeof(float[3]); // pos
    fullSize += sizeof(float);    // direction
    fullSize += sizeof(float);    // init delay
    fullSize += sizeof(uint16_t); // delay count
    for (unsigned int j = 0; j < w->delay.size(); j++) {
      fullSize += sizeof(float);
    }
  }

  return fullSize;
}


int WorldWeapons::getNewWorldShotID(void)
{
  if (worldShotId > _MAX_WORLD_SHOTS) {
    worldShotId = 0;
  }
  return worldShotId++;
}


//----------WorldWeaponGlobalEventHandler---------------------
// where we do the world weapon handling for event based shots since they are not really done by the "world"

WorldWeaponGlobalEventHandler::WorldWeaponGlobalEventHandler(FlagType *_type,
							     const float *_origin,
							     float _direction,
							     float _tilt,
							     TeamColor teamColor )
{
  type = _type;
  if (_origin)
    memcpy(origin,_origin,sizeof(float)*3);
  else
    origin[0] = origin[1] = origin[2] = 0.0f;

  direction = _direction;
  tilt = _tilt;
  team = convertTeam(teamColor);
}

WorldWeaponGlobalEventHandler::~WorldWeaponGlobalEventHandler()
{
}

void WorldWeaponGlobalEventHandler::process (bz_EventData *eventData)
{
  if (!eventData || eventData->eventType != bz_eCaptureEvent)
    return;

  bz_CTFCaptureEventData *capEvent = (bz_CTFCaptureEventData*)eventData;

  if ( capEvent->teamCapped != team )
	  return;

  fireWorldWepReal(type, BZDB.eval(StateDatabase::BZDB_RELOADTIME),
		   ServerPlayer, RogueTeam, origin, tilt, direction,
		   world->getWorldWeapons().getNewWorldShotID(),0);
}


// for bzfsAPI: it needs to be global
int fireWorldWep(FlagType* type, float lifetime, PlayerId player,
			float *pos, float tilt, float direction,
			int shotID, float dt)
{
  return fireWorldWepReal(type, lifetime, player, RogueTeam,
			  pos, tilt, direction, shotID, dt);
}


int fireWorldGM(FlagType* type, PlayerId targetPlayerID, float lifetime,
                PlayerId player, float *pos, float tilt, float direction,
                int shotID, float dt)
{
  return fireWorldGMReal(type, targetPlayerID, lifetime, player, pos, tilt,
                         direction, shotID, dt);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
