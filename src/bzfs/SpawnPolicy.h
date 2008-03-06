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

#ifndef __SPAWNPOLICY_H__
#define __SPAWNPOLICY_H__

#include "common.h"

/* common interface headers */
#include "global.h"  /* for TeamColor */


/** a SpawnPolicy is used to determine a new SpawnPosition.  Policies
 *  are defined to describe various spawning behaviors such as purely
 *  random, aggressive, or classical behavior.  Factors that can be
 *  taken into account are the proximity to other players, bullets,
 *  flags, etc.
 */
class SpawnPolicy
{

public:
  SpawnPolicy();
  virtual ~SpawnPolicy();

  virtual void getPosition(float pos[3], int playerId, bool onGroundOnly, bool notNearEdges);
  virtual void getAzimuth(float &azimuth);
  
protected:
  virtual bool isImminentlyDangerous() const;

private:  
  float enemyProximityCheck(float &enemyAngle) const;
  float distanceFrom(const float *farPos) const;
  bool  isFacing(const float *enemyPos, const float enemyAzimuth, const float deviation) const;

  /* temp, internal use */
  TeamColor   team;
  float       testPos[3];
  
  float safeSWRadius;
  float safeSRRadius;
  float safeDistance;
};

#endif  /*__SPAWNPOLICY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
