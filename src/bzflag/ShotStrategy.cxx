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
  {
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
  }

  // check teleporter borders
  {
    const std::vector<Teleporter> &teleporters = World::getWorld()->getTeleporters();
    std::vector<Teleporter>::const_iterator it = teleporters.begin();
    while (it != teleporters.end()) {
      const Teleporter& teleporter = *it;
      if (!teleporter.isShootThrough()) {
	const float telet = teleporter.intersect(ray);
	int face;
	if (telet > min && telet < t && teleporter.isTeleported(ray, face) < 0.0f) {
	  t = telet;
	  closestObstacle = &teleporter;
	}
      }
      it++;
    }
  }

  // check boxes
  {
    const std::vector<BoxBuilding> &boxes = World::getWorld()->getBoxes();
    std::vector<BoxBuilding>::const_iterator it = boxes.begin();
    while (it != boxes.end()) {
      const BoxBuilding& box = *it;
      if (!box.isShootThrough()) {
	const float boxt = box.intersect(ray);
	if (boxt > min && boxt < t) {
	  t = boxt;
	  closestObstacle = &box;
	}
      }
      it++;
    }
  }

  // check bases
  {
    const std::vector<BaseBuilding> &bases = World::getWorld()->getBases();
    std::vector<BaseBuilding>::const_iterator it = bases.begin();
    while (it != bases.end()) {
      const BaseBuilding& base = *it;
      if (!base.isShootThrough()) {
	const float baset = base.intersect(ray);
	if (baset > min && baset < t) {
	  t = baset;
	  closestObstacle = &base;
	}
      }
      it++;
    }
  }

  // check pyramids
  {
    const std::vector<PyramidBuilding> &pyramids = World::getWorld()->getPyramids();
    std::vector<PyramidBuilding>::const_iterator it = pyramids.begin();
    while (it != pyramids.end()) {
      const PyramidBuilding& pyramid = *it;
      if (!pyramid.isShootThrough()) {
	const float pyramidt = pyramid.intersect(ray);
	if (pyramidt > min && pyramidt < t) {
	  t = pyramidt;
	  closestObstacle = &pyramid;
	}
      }
      it++;
    }
  }

  // check tetrahedrons
  {
    const std::vector<TetraBuilding> &tetras = World::getWorld()->getTetras();
    std::vector<TetraBuilding>::const_iterator it = tetras.begin();
    while (it != tetras.end()) {
      const TetraBuilding& tetra = *it;
      if (!tetra.isShootThrough()) {
        const float tetrat = tetra.intersect(ray);
        if (tetrat > min && tetrat < t) {
          t = tetrat;
          closestObstacle = &tetra;
        }
      }
      it++;
    }
  }

  return closestObstacle;
}

void ShotStrategy::reflect(float* v, const float* n) // const
{
  // normal is assumed to be normalized, v needn't be
  const float d = -2.0f * (n[0] * v[0] + n[1] * v[1] + n[2] * v[2]);
  v[0] += d * n[0];
  v[1] += d * n[1];
  v[2] += d * n[2];
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
