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

#ifndef __DEFAULTSPAWNPOLICY_H__
#define __DEFAULTSPAWNPOLICY_H__

// interface header
#include "SpawnPolicy.h"

// common headers
#include "global.h"  /* for TeamColor */
#include "vectors.h"


/** a DefaultSpawnPolicy is used to determine a new SpawnPosition.  Policies
 *  are defined to describe various spawning behaviors such as purely
 *  random, aggressive, or classical behavior.  Factors that can be
 *  taken into account are the proximity to other players, bullets,
 *  flags, etc.
 */
class DefaultSpawnPolicy : public SpawnPolicy {
  public:
    DefaultSpawnPolicy();
    virtual ~DefaultSpawnPolicy();

    virtual const char* Name() const {
      static const char* name = "Default";
      return name;
    }

    virtual void getPosition(fvec3& pos, int playerId, bool onGroundOnly, bool notNearEdges);
    virtual void getAzimuth(float& azimuth);

  protected:
    float enemyProximityCheck(float& enemyAngle) const;

  private:
    /* temp, internal use */
    TeamColor   team;
    fvec3       testPos;
};

#endif  /*__DEFAULTSPAWNPOLICY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
