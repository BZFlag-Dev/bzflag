/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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
  return (nextShot - TimeKeeper::getCurrent());
}


void WorldWeapons::fire()
{
  TimeKeeper nowTime = TimeKeeper::getCurrent();

  for (std::vector<Weapon*>::iterator it = weapons.begin();
       it != weapons.end(); ++it) {
    Weapon *w = *it;
    if (w->nextTime <= nowTime) {

      void *buf, *bufStart = getDirectMessageBuffer();

      FiringInfo firingInfo;
      firingInfo.flagType = (FlagType*)w->type;
      firingInfo.lifetime = BZDB.eval(StateDatabase::BZDB_RELOADTIME);
      firingInfo.shot.player = ServerPlayer;
      memmove(firingInfo.shot.pos, w->origin, 3 * sizeof(float));
      float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
      firingInfo.shot.vel[0] = shotSpeed * cosf(w->direction);
      firingInfo.shot.vel[1] = shotSpeed * sinf(w->direction);
      firingInfo.shot.vel[2] = 0.0f;
      firingInfo.shot.id = worldShotId++;
      if (worldShotId > 30) { // Maximum of 30 world shots
	    worldShotId = 0;
      }
      firingInfo.shot.dt = 0;
      buf = firingInfo.pack(bufStart);

      if (BZDB.isTrue(StateDatabase::BZDB_WEAPONS)) {
        broadcastMessage(MsgShotBegin, (char *)buf - (char *)bufStart, bufStart);
      }

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


void WorldWeapons::add(const FlagType *type, const float *origin, float direction,
                       float initdelay, const std::vector<float> &delay,
                       TimeKeeper &sync)
{
  Weapon *w = new Weapon();
  w->type = type;
  memmove(&w->origin, origin, 3*sizeof(float));
  w->direction = direction;
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

int WorldWeapons::packSize(void) const
{
  int size = 0;
  for (unsigned int i=0 ; i < weapons.size(); i++) {
    size += 2 + 2 + WorldCodeWeaponSize;
    size += weapons[i]->delay.size() * sizeof (float);
  }
  return size;
}

void * WorldWeapons::pack(void *buf) const
{
  for (unsigned int i=0 ; i < weapons.size(); i++) {
    const Weapon *w = (const Weapon *) weapons[i];
    void *bufStart = buf;
    buf = nboPackUShort(buf, 0); // place-holder
    buf = nboPackUShort(buf, WorldCodeWeapon);
    buf = w->type->pack (buf);                  //  2
    buf = nboPackVector(buf, w->origin);        // 14
    buf = nboPackFloat(buf, w->direction);      // 18
    buf = nboPackFloat(buf, w->initDelay);      // 22
    buf = nboPackUShort(buf, w->delay.size());  // 24
    for (unsigned int j = 0; j < w->delay.size(); j++) {
      buf = nboPackFloat(buf, w->delay[j]);
    }
    uint16_t len = (char*)buf - (char*)bufStart;
    nboPackUShort (bufStart, len - (2 * sizeof(uint16_t)));
  }
  return buf;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
