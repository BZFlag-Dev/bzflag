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


#include "common.h"

// interface header
#include "DropGeometry.h"

// system headers
#include <stdlib.h>

// common implementation headers
#include "BZDBCache.h"
#include "Obstacle.h"
#include "CollisionManager.h"
#include "Intersect.h"
#include "PhysicsDriver.h"
#include "WorldInfo.h"
#include "ObstacleGroup.h"


// prototypes
static int sortByTopHeight(const void* a, const void* b);



bool DropGeometry::dropPlayer (float pos[3], float minZ, float maxZ,
                               const WorldInfo& world)
{
  minZ= maxZ;
  const float radius = BZDBCache::tankRadius;
  const float maxHeight = world.getMaxWorldHeight();

  // use a downwards ray to hit the onFlatTop() buildings
  const float dir[3] = {0.0f, 0.0f, -1.0f};
  const float org[3] = {pos[0], pos[1], maxHeight + 0.1f};
  Ray ray(org, dir);

  // list of all possible colliders
  const ObsList* olist = COLLISIONMGR.cylinderTest(pos, radius, maxHeight);
  
  // sort by top height
  qsort(olist->list, olist->count, sizeof(Obstacle*), sortByTopHeight);

  for (int i = 0; i < olist->count; i++) {
    
    const Obstacle* obs = olist->list[i];
    obs = obs;
  }

  return true;
}


static int sortByTopHeight(const void* a, const void* b)
{
  const Obstacle* obsA = *((const Obstacle**)a);
  const Obstacle* obsB = *((const Obstacle**)b);
  // this is wasteful, obstacles should have their extents pre-computed.
  // the current getExtents() obstacle function calculates the extents
  // on the fly... As well, extents show be a float[6] array, or perhaps
  //  a class or struct.
  const float topA = obsA->getPosition()[2] + obsA->getHeight();;
  const float topB = obsB->getPosition()[2] + obsB->getHeight();
  if (topA < topB) {
    return -1;
  } else if (topA > topB) {
    return +1;
  } else {
    return 0;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
