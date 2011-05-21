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

/* interface header */
#include "ShotStrategy.h"

/* common implementation headers */
#include "CollisionManager.h"
#include "Obstacle.h"
#include "ObstacleList.h"
#include "WallObstacle.h"
#include "MeshFace.h"
#include "ObstacleMgr.h"


ShotStrategy::ShotStrategy(ShotPath* _path) :
  path(_path) {
  // do nothing
}


ShotStrategy::~ShotStrategy() {
  // do nothing
}


bool ShotStrategy::isStoppedByHit() const {
  return true;
}


void ShotStrategy::sendUpdate(const FiringInfo&) const {
  // do nothing by default -- normal shots don't need updates
}


void ShotStrategy::readUpdate(void*) {
  // do nothing by default -- normal shots don't need updates
}


void ShotStrategy::expire() {
  // do nothing by default
}


void ShotStrategy::setReloadTime(float t) const {
  path->setReloadTime(t);
}


void ShotStrategy::setPosition(const fvec3& pos) const {
  path->setPosition(pos);
}


void ShotStrategy::setVelocity(const fvec3& vel) const {
  path->setVelocity(vel);
}


void ShotStrategy::setExpiring() const {
  path->setExpiring();
}


void ShotStrategy::setExpired() const {
  path->setExpired();
}


FiringInfo& ShotStrategy::getFiringInfo(ShotPath* p) const {
  return p->getFiringInfo();
}


const Obstacle* ShotStrategy::getFirstBuilding(const Ray& ray,
                                               float min, float& t) {
  const Obstacle* closestObstacle = NULL;
  unsigned int i = 0;

  // check walls
  const ObstacleList& walls = OBSTACLEMGR.getWalls();
  for (i = 0; i < walls.size(); i++) {
    const WallObstacle* wall = (const WallObstacle*) walls[i];
    if (!wall->isShootThrough()) {
      const float wallt = wall->intersect(ray);
      if ((wallt > min) && (wallt < t)) {
        t = wallt;
        closestObstacle = wall;
      }
    }
  }

  // check everything else
  const ObsList* olist = COLLISIONMGR.rayTest(&ray, t);

  for (i = 0; i < (unsigned int)olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->isShootThrough()) {
      if ((obs->getTypeID() != faceType) ||
          !((const MeshFace*)obs)->isLinkSrc()) {
        continue; // shootThrough, and not a linkSrc; skip it
      }
    }
    const float timet = obs->intersect(ray);
    if ((timet > min) && (timet < t)) {
      t = timet;
      closestObstacle = obs;
    }
  }

  return closestObstacle;
}


const Obstacle* ShotStrategy::getFirstLinkSrc(const Ray& ray,
                                              float min, float& t) {
  const Obstacle* closestObstacle = NULL;

  // check everything else
  const ObsList* olist = COLLISIONMGR.rayTest(&ray, t);

  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (!obs->isShootThrough() || (obs->getTypeID() != faceType)) {
      continue;
    }
    const MeshFace* face = (const MeshFace*)obs;
    if (!face->isLinkSrc()) {
      continue;
    }
    const float timet = obs->intersect(ray);
    if ((timet > min) && (timet < t)) {
      t = timet;
      closestObstacle = obs;
    }
  }

  return closestObstacle;
}


void ShotStrategy::reflect(fvec3& v, const fvec3& n) { // const
  // normal is assumed to be normalized, v needn't be
  float d = -2.0f * fvec3::dot(v, n);

  if (d >= 0.0f) {
    // normal reflection
    v += d * n;
  }
  else {
    // refraction due to inverted normal (still using the 2X factor)
    float oldSpeed = v.length();
    d = -2.0f * d; // now using 4X refraction factor
    v += d * n;
    // keep the same speed as the incoming vector
    float newSpeed = v.length();
    v *= (oldSpeed / newSpeed);
  }

  return;
}


bool ShotStrategy::getGround(const Ray& r, float min, float& t) {
  if (r.getDirection().z >= 0.0f) {
    return false;
  }

  float groundT = r.getOrigin().z / -r.getDirection().z;
  if ((groundT > min) && (groundT < t)) {
    t = groundT;
    return true;
  }
  return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
