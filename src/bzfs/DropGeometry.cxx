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
#include "ObstacleMgr.h"
#include "MeshFace.h"


// cheezy holding list
class HoldingList {
  public:
    HoldingList();
    ~HoldingList();
    void copy(const ObsList* list);
  public:
    int size;
    ObsList olist;
};
static HoldingList ilist; // ray intersection list


// prototypes
static int compareByTopHeight(const void* a, const void* b);
bool regularDrop(float pos[3], float minZ, float maxZ,
                 float height, const WorldInfo* world);


static inline bool isGroundValid(const float pos[3],
                                 float radius, float height)
{
  const ObsList* olist = COLLISIONMGR.cylinderTest(pos, radius, height);

  // invalid if it touches a building
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->inCylinder(pos, radius, height)) {
      return false;
    }
  }
  return true;
}


static inline bool isValidLanding(const Obstacle* landing, const float pos[3],
                                  float radius, float height)
{
  const ObsList* olist = COLLISIONMGR.cylinderTest(pos, radius, height);

  const float landZ = landing->getExtents().maxs[2];

  // invalid if it touches a building
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->getExtents().maxs[2] <= landZ) {
      continue; // do not check the landing
    }
    if (obs->inCylinder(pos, radius, height)) {
      return false;
    }
  }

  return true;
}


bool DropGeometry::dropPlayer(float pos[3], float minZ, float maxZ,
                              const WorldInfo* world)
{
  const float fudge = 0.001f;
  const float tankHeight = BZDBCache::tankHeight + fudge;
  bool value = regularDrop(pos, minZ, maxZ, tankHeight, world);
  pos[2] += fudge;
  return value;
}

                 
bool DropGeometry::dropFlag(float pos[3], float minZ, float maxZ,
                              const WorldInfo* world)
{
  const float flagHeight = BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT);
  return regularDrop(pos, minZ, maxZ, flagHeight, world);
}

                 
bool regularDrop(float pos[3], float minZ, float maxZ,
                 float height, const WorldInfo* world)
{
  // adjust the minimum level
  const float waterLevel = world->getWaterLevel();
  if ((waterLevel > 0.0f) && (waterLevel > minZ)) {
    minZ = waterLevel;
  }

  // use a downwards ray to hit the onFlatTop() buildings
  const float maxHeight = COLLISIONMGR.getWorldExtents().maxs[2];
  const float dir[3] = {0.0f, 0.0f, -1.0f};
  const float org[3] = {pos[0], pos[1], maxHeight + 1.0f};
  Ray ray(org, dir);

  const float radius = BZDBCache::tankRadius;

  // is a simple ground landing good enough?  
  if (minZ <= 0.0f) {
    pos[2] = 0.0f;
    if (isGroundValid(pos, radius, height)) {
      return true;
    }
  }

  // list of all possible landings
  const ObsList* olist = COLLISIONMGR.rayTest(&ray, MAXFLOAT);
  ilist.copy(olist); // copy the list, so that COLLISIONMGR can be re-used
  qsort(ilist.olist.list, ilist.olist.count, sizeof(Obstacle*),
        compareByTopHeight); // sort by top height (lowest first)

  // check for a raised landing  
  for (int i = 0; i < ilist.olist.count; i++) {
    const Obstacle* obs = ilist.olist.list[i];

    // make sure that it's within the limits    
    const float zTop = obs->getExtents().maxs[2];
    if (zTop > maxZ) {
      return false; // no more obstacles to check
    }
    if (zTop < minZ) {
      continue;
    }
    pos[2] = zTop;
    
    // must be a flattop buildings
    if (!obs->isFlatTop()) {
      continue;
    }
    // drivethrough buildings are not potential landings
    if (obs->isDriveThrough()) {
      continue;
    }

    // death buildings are not potential landings
    if (obs->getType() == MeshFace::getClassName()) {
      const MeshFace* face = (const MeshFace*) obs;
      int driver = face->getPhysicsDriver();
      const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driver);
      if ((phydrv != NULL) && (phydrv->getIsDeath())) {
        return false;
      }
    }
  
    // make sure that it intersects
    if (obs->intersect(ray) < 0.0f) {
      continue;
    }
    
    if (isValidLanding(obs, pos, radius, height)) {
      return true;
    }
  }

  return false;
}


static int compareByTopHeight(const void* a, const void* b)
{
  const Obstacle* obsA = *((const Obstacle**)a);
  const Obstacle* obsB = *((const Obstacle**)b);
  const float topA = obsA->getExtents().maxs[2];
  const float topB = obsB->getExtents().maxs[2];
  if (topA < topB) {
    return -1;
  } else if (topA > topB) {
    return +1;
  } else {
    return 0;
  }
}


HoldingList::HoldingList()
{
  size = 0;
  olist.count = size;
  olist.list = NULL;
  return;
}

HoldingList::~HoldingList()
{
  delete olist.list;
  return;
}
  
void HoldingList::copy(const ObsList* list)
{
  if (list->count > size) {
    delete olist.list;
    size = list->count;
    olist.list = new Obstacle*[size];
  }
  olist.count = list->count;
  memcpy(olist.list, list->list, olist.count * sizeof(Obstacle*));
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
