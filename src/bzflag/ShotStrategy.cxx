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

/* interface header */
#include "ShotStrategy.h"

/* system implementation headers */
#include <vector>

/* common implementation headers */
#include "StateDatabase.h"
#include "CollisionManager.h"
#include "Intersect.h"
#include "Obstacle.h"
#include "WallObstacle.h"
#include "BoxBuilding.h"

/* local implementation headers */
#include "World.h"


/* local implementation headers */


ShotStrategy::ShotStrategy(ShotPath* _path) :
  path(_path)
{
  // do nothing
}

ShotStrategy::~ShotStrategy()
{
  // do nothing
}

bool ShotStrategy::isStoppedByHit() const
{
  return true;
}

void ShotStrategy::sendUpdate(const FiringInfo&) const
{
  // do nothing by default -- normal shots don't need updates
}

void ShotStrategy::readUpdate(uint16_t, void*)
{
  // do nothing by default -- normal shots don't need updates
}

void ShotStrategy::expire()
{
  // do nothing by default
}

void ShotStrategy::setReloadTime(float t) const
{
  path->setReloadTime(t);
}

void ShotStrategy::setPosition(const float* p) const
{
  path->setPosition(p);
}

void ShotStrategy::setVelocity(const float* v) const
{
  path->setVelocity(v);
}

void ShotStrategy::setExpiring() const
{
  path->setExpiring();
}

void ShotStrategy::setExpired() const
{
  path->setExpired();
}

FiringInfo& ShotStrategy::getFiringInfo(ShotPath* p) const
{
  return p->getFiringInfo();
}

const Obstacle* ShotStrategy::getFirstBuilding(const Ray& ray,
					       float min, float& t)
{
  const Obstacle* closestObstacle = NULL;

  // check walls
  const std::vector<WallObstacle> &walls = World::getWorld()->getWalls();
  std::vector<WallObstacle>::const_iterator it = walls.begin();
  while (it != walls.end()) {
    const WallObstacle& wall = *it;
    if (!wall.isShootThrough()) {
      const float wallt = wall.intersect(ray);
      if (wallt > min && wallt < t) {
        t = wallt;
        closestObstacle = &wall;
      }
    }
    it++;
  }

  //check everything else
  const CollisionManager* colMgr = World::getCollisionManager();
  if (colMgr == NULL) {
    return closestObstacle;
  }

  const ObsList* olist = colMgr->rayTest (&ray);

  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (!obs->isShootThrough()) {
      const float timet = obs->intersect(ray);
      if (obs->getType() == Teleporter::getClassName()) {
        const Teleporter* tele = (const Teleporter*) obs;
        int face;
	if ((timet > min) && (timet < t) &&
            (tele->isTeleported(ray, face) < 0.0f)) {
	  t = timet;
	  closestObstacle = obs;
        }
      }
      else {
	if ((timet > min) && (timet < t)) {
	  t = timet;
	  closestObstacle = obs;
        }
      }
    }
  }

  return closestObstacle;
}

void ShotStrategy::reflect(float* v, const float* n) // const
{
  // normal is assumed to be normalized, v needn't be
  float d = -2.0f * ((n[0] * v[0]) + (n[1] * v[1]) + (n[2] * v[2]));
  
  if (d >= 0.0f) {
    // normal reflection
    v[0] += d * n[0];
    v[1] += d * n[1];
    v[2] += d * n[2];
  } else {
    // refraction due to inverted normal (still using the 2X factor)
    float oldSpeed = sqrtf((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
    d = -2.0f * d; // now using 4X refraction factor
    v[0] += d * n[0];
    v[1] += d * n[1];
    v[2] += d * n[2];
    // keep the same speed as the incoming vector
    float newSpeed = sqrtf((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
    const float scale = (oldSpeed / newSpeed);
    v[0] *= scale;
    v[1] *= scale;
    v[2] *= scale;
  }
  
  return;
}

const Teleporter* ShotStrategy::getFirstTeleporter(const Ray& ray,
						   float min, float& t, int& f) const
{
  const Teleporter* closestTeleporter = NULL;
  int face;

  {
    const std::vector<Teleporter> &teleporters = World::getWorld()->getTeleporters();
    std::vector<Teleporter>::const_iterator it = teleporters.begin();
    while (it != teleporters.end()) {
      const Teleporter& teleporter = *it;
      const float telet = teleporter.isTeleported(ray, face);
      if (telet > min && telet < t) {
	t = telet;
	f = face;
	closestTeleporter = &teleporter;
      }
      it++;
    }
  }

  return closestTeleporter;
}

bool ShotStrategy::getGround(const Ray& r, float min, float &t) const
{
  if (r.getDirection()[2] >= 0.0f)
    return false;

  float groundT = r.getOrigin()[2] / -r.getDirection()[2];
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
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
