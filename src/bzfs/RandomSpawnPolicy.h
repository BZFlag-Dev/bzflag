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

#ifndef __RANDOMSPAWNPOLICY_H__
#define __RANDOMSPAWNPOLICY_H__

#include "common.h"

/* system interfacer headers */
#include <string>

/* common interface headers */
#include "SpawnPolicy.h"


/** a RandomSpawnPolicy is a SpawnPolicy that just generates a purely
 *  random (yet hopefully still valid) position and drops you there.
 */
class RandomSpawnPolicy : public SpawnPolicy 
{
public:
  RandomSpawnPolicy();
  virtual ~RandomSpawnPolicy();

  virtual const char *Name() const {
    static char *name = "Random";
    return name;
  }

  virtual void getPosition(float pos[3], int playerId, bool onGroundOnly, bool notNearEdges);
  virtual void getAzimuth(float &azimuth);
};

#endif  /*__RANDOMSPAWNPOLICY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
