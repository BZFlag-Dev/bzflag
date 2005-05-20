/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif

// class-interface header
#include "WorldWeapons.h"

// system headers
#include <vector>

// common-interface headers
#include "TimeKeeper.h"
#include "ShotUpdate.h"
#include "Protocol.h"
#include "Address.h"
#include "StateDatabase.h"

char *getDirectMessageBuffer();
void broadcastMessage(uint16_t code, int len, const void *msg);


int fireWorldWep ( FlagType* type, float lifetime, PlayerId player, float *pos, float tilt, float direction, int shotID, float dt )
{
	void *buf, *bufStart = getDirectMessageBuffer();

	FiringInfo firingInfo;
	firingInfo.flagType = type;
	firingInfo.lifetime = lifetime;
	firingInfo.shot.player = player;
	memmove(firingInfo.shot.pos, pos, 3 * sizeof(float));
	float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
	const float tiltFactor = cosf(tilt);
	firingInfo.shot.vel[0] = shotSpeed * tiltFactor * cosf(direction);
	firingInfo.shot.vel[1] = shotSpeed * tiltFactor * sinf(direction);
	firingInfo.shot.vel[2] = shotSpeed * sinf(tilt);
	firingInfo.shot.id = shotID;
	firingInfo.shot.dt = dt;

	buf = firingInfo.pack(bufStart);

	if (BZDB.isTrue(StateDatabase::BZDB_WEAPONS)) {
		broadcastMessage(MsgShotBegin, (char *)buf - (char *)bufStart, bufStart);
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


			fireWorldWep( (FlagType*)w->type,BZDB.eval(StateDatabase::BZDB_RELOADTIME),
																	ServerPlayer,w->origin,w->tilt,w->direction,
																	worldShotId++,0);
      if (worldShotId > 30) // Maximum of 30 world shots
				worldShotId = 0;

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
                       float direction, float tilt,
		       float initdelay, const std::vector<float> &delay,
		       TimeKeeper &sync)
{
  Weapon *w = new Weapon();
  w->type = type;
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

//----------WorldWeaponGlobalEventHandler---------------------
// where we do the world weapon handling for event based shots since they are not realy done by the "world"
WorldWeaponGlobalEventHandler::WorldWeaponGlobalEventHandler(FlagType *_type, const float *_origin, float _direction, float _tilt)
{
	type = _type;
	if ( _origin)
		memcpy(origin,_origin,sizeof(float)*3);
	else
		origin[0] = origin[1] = origin[2] = 0.0f;

	direction = _direction;
	tilt = _tilt;
}

WorldWeaponGlobalEventHandler::~WorldWeaponGlobalEventHandler()
{
}

void WorldWeaponGlobalEventHandler::process (BaseEventData *eventData)
{
	if (eventData->eventType != eCaptureEvent)
		return;

	fireWorldWep( type,BZDB.eval(StateDatabase::BZDB_RELOADTIME),
	ServerPlayer,origin,tilt,direction,0,0);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
