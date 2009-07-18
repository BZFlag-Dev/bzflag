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

// interface header
#include "DropGeometry.h"

// system headers
#include <stdlib.h>

// common headers
#include "vectors.h"
#include "Intersect.h"
#include "Obstacle.h"
#include "MeshFace.h"
#include "BaseBuilding.h"
#include "ObstacleMgr.h"
#include "CollisionManager.h"
#include "PhysicsDriver.h"
#include "WorldInfo.h"
#include "BZDBCache.h"
#include "ServerIntangibilityManager.h"

//
// Datatype Definitions
//
class HoldingList {
public:
  HoldingList();
  ~HoldingList();
  void copy(const ObsList* list);
public:
  int size;
  int count;
  Obstacle** list;
};

static HoldingList rayList; // ray intersection list

//
// Function Prototypes
//
static int compareAscending(const void* a, const void* b);
static int compareDescending(const void* a, const void* b);
static bool isDeathLanding(const Obstacle* landing);
static bool isOpposingTeam(const Obstacle* obs, int team);
static bool isValidLanding(const Obstacle* obs);
static bool isValidClearance(const fvec3& pos, float radius,
			     float height, int team);
static bool dropIt(fvec3& pos, float minZ, float maxZ,
		   float radius, float height, int team);


//============================================================================//

bool DropGeometry::dropPlayer(fvec3& pos, float minZ, float maxZ)
{
  // fudge-it to avoid spawn stickiness on obstacles
  const float fudge = 0.001f;
  const float tankHeight = BZDBCache::tankHeight + fudge;
  bool value = dropIt(pos, minZ, maxZ, BZDBCache::tankRadius, tankHeight, -1);
  pos.z += fudge;
  return value;
}


bool DropGeometry::dropFlag(fvec3& pos, float minZ, float maxZ)
{
  const float flagHeight = BZDB.eval(BZDBNAMES.FLAGHEIGHT);
  return dropIt(pos, minZ, maxZ, BZDBCache::tankRadius, flagHeight, -1);
}


bool DropGeometry::dropTeamFlag(fvec3& pos, float minZ, float maxZ,
				int team)
{
  // team flags do not get real clearance checks (radius = 0)
  // if you want to put some smarts in to check for wedging
  // (flag is stuck amongst obstacles), then add the code into
  // isValidClearance().
  const float flagHeight = BZDB.eval(BZDBNAMES.FLAGHEIGHT);
  return dropIt(pos, minZ, maxZ, 0.0f, flagHeight, team);
}


//============================================================================//

static inline bool isDeathLanding(const Obstacle* obs)
{
  if (obs->getTypeID() == faceType) {
    const MeshFace* face = (const MeshFace*) obs;
    int driver = face->getPhysicsDriver();
    const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driver);
    if ((phydrv != NULL) && (phydrv->getIsDeath())) {
      return true;
    }
  }
  return false;
}


static inline bool isOpposingTeam(const Obstacle* obs, int team)
{
  if (team < 0) {
    return false;
  }

  const int baseTeam = obs->getBaseTeam();
  if (baseTeam < 0) {
    return false;
  }

  return (baseTeam != team);
}


static inline bool isValidLanding(const Obstacle* obs)
{
  // must be a flattop buildings
  if (!obs->isFlatTop()) {
    return false;
  }
  // drivethrough buildings are not potential landings

  if (ServerIntangibilityManager::instance().getWorldObjectTangibility(obs) != 0) {
    return false;
  }

  // death buildings are not potential landings
  if (isDeathLanding(obs)) {
    return false;
  }

  return true;
}


static bool isValidClearance(const fvec3& pos, float radius,
			     float height, int team)
{
  const ObsList* olist = COLLISIONMGR.cylinderTest(pos, radius, height);

  // invalid if it touches a building
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    const float zTop = obs->getExtents().maxs.z;
    if (zTop > pos.z) {
      if (obs->inCylinder(pos, radius, height)) {
	return false;
      }
    }
    else {
      // do not check coplanars unless they are fatal
      if (isDeathLanding(obs) || isOpposingTeam(obs, team)) {
	const float fudge = 0.001f; // dig in a little to make sure
	const fvec3 testPos(pos.x, pos.y, pos.z - fudge);
	if (obs->inCylinder(testPos, radius, height + fudge)) {
	  return false;
	}
      }
    }
  }

  return true;
}


static int compareAscending(const void* a, const void* b)
{
  const Obstacle* obsA = *((const Obstacle**)a);
  const Obstacle* obsB = *((const Obstacle**)b);
  const float topA = obsA->getExtents().maxs.z;
  const float topB = obsB->getExtents().maxs.z;
  if (topA < topB) {
    return -1;
  } else if (topA > topB) {
    return +1;
  } else {
    return 0;
  }
}


static int compareDescending(const void* a, const void* b)
{
  const Obstacle* obsA = *((const Obstacle**)a);
  const Obstacle* obsB = *((const Obstacle**)b);
  const float topA = obsA->getExtents().maxs.z;
  const float topB = obsB->getExtents().maxs.z;
  if (topA < topB) {
    return +1;
  } else if (topA > topB) {
    return -1;
  } else {
    return 0;
  }
}


//============================================================================//

static bool dropIt(fvec3& pos, float minZ, float maxZ,
		   float radius, float height, int team)
{
  int i;

  // special case, just check the ground
  if (maxZ <= 0.0f) {
    pos.z = 0.0f;
    if (isValidClearance(pos, radius, height, team)) {
      return true;
    } else {
      return false;
    }
  }

  // adjust the position to the minimum level
  if (pos.z < minZ) {
    pos.z = minZ;
  }

  // use a downwards ray to hit the onFlatTop() buildings
  const float maxHeight = COLLISIONMGR.getWorldExtents().maxs.z;
  const fvec3 dir(0.0f, 0.0f, -1.0f);
  const fvec3 org(pos.x, pos.y, maxHeight + 1.0f);
  Ray ray(org, dir);

  // list of  possible landings
  const ObsList* olist = COLLISIONMGR.rayTest(&ray, MAXFLOAT);
  rayList.copy(olist); // copy the list, so that COLLISIONMGR can be re-used

  const float startZ = pos.z;

  // are we in the clear?
  if (isValidClearance(pos, radius, height, team)) {
    // sort from highest to lowest
    qsort(rayList.list, rayList.count, sizeof(Obstacle*), compareDescending);
    // no interference, try dropping
    for (i = 0; i < rayList.count; i++) {
      const Obstacle* obs = rayList.list[i];
      const float zTop = obs->getExtents().maxs.z;
      // make sure that it's within the limits
      if ((zTop > startZ) || (zTop > maxZ)) {
	continue;
      }
      if (zTop < minZ) {
	break;
      }
      pos.z = zTop;

      if (obs->intersect(ray) >= 0.0f) {
	if (isValidLanding(obs) &&
	    isValidClearance(pos, radius, height, team)) {
	  return true;
	} else {
	  // a potential hit surface was tested and failed, unless
	  // we want to pass through it, we have to return false.
	  return false;
	}
      }
    }
    // check the ground
    if (minZ <= 0.0f) {
      pos.z = 0.0f;
      if (isValidClearance(pos, radius, height, team)) {
	return true;
      }
    }
  }
  else {
    // sort from lowest to highest
    qsort(rayList.list, rayList.count, sizeof(Obstacle*), compareAscending);
    // we're blocked, try climbing
    for (i = 0; i < rayList.count; i++) {
      const Obstacle* obs = rayList.list[i];
      const float zTop = obs->getExtents().maxs.z;
      // make sure that it's within the limits
      if ((zTop < startZ) || (zTop < minZ)) {
	continue;
      }
      if (zTop > maxZ) {
	return false;
      }
      pos.z = zTop;

      if (isValidLanding(obs) &&
	  (obs->intersect(ray) >= 0.0f) &&
	  isValidClearance(pos, radius, height, team)) {
	return true;
      }
    }
  }

  return false;
}


//============================================================================//

HoldingList::HoldingList()
{
  size = count = 0;
  list = NULL;
  return;
}

HoldingList::~HoldingList()
{
  delete[] list;
  return;
}

void HoldingList::copy(const ObsList* olist)
{
  if (olist->count > size) {
    // increase the list size
    delete[] list;
    size = olist->count;
    list = new Obstacle*[size];
  }
  count = olist->count;
  memcpy(list, olist->list, count * sizeof(Obstacle*));
  return;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
