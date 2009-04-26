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

#ifdef _MSC_VER
#pragma warning(4:4786)
#endif

#include "common.h"

// interface header
#include "TeamBases.h"

// system headers
#include <string.h>

// common headers
#include "Protocol.h"
#include "Pack.h"
#include "BZDBCache.h"
#include "Obstacle.h"
#include "MeshFace.h"
#include "Ray.h"
#include "vectors.h"


//============================================================================//

TeamBases::TeamBases()
{
  color = RedTeam;
}


void TeamBases::addBase(const Obstacle* obs)
{
  teamBases.push_back(TeamBase(obs));
}


int TeamBases::size() const
{
  return teamBases.size();
}


TeamColor TeamBases::getTeam() const
{
  return color;
}


const fvec3& TeamBases::getBasePosition(int base) const
{
  if ((base < 0) || (base >= (int)teamBases.size())) {
    base = 0;
  }
  return teamBases[base].getObstacle()->getPosition();
}


float TeamBases::findBaseZ(const fvec3& testPos) const
{
  TeamBaseList::const_iterator it;
  for (it = teamBases.begin(); it != teamBases.end(); ++it) {
    const Obstacle* obs = it->getObstacle();
    if (obs->getTypeID() == faceType) {
      const MeshFace* face = (const MeshFace*) obs;
      const Ray ray(testPos, fvec3(0.0f, 0.0f, -1.0f));
      const float t = face->intersect(ray);
      if (t >= 0.0f) {
        return face->getPosition().z;
      }
    }
    else {
      const fvec3& pos = obs->getPosition();
      const fvec3& sz  = obs->getSize();
      const float  rot = obs->getRotation();
      float nx = testPos.x - pos.x;
      float ny = testPos.y - pos.y;
      if (nx == 0.0f) {
        nx = 1.0f;
      }
      const float len = sqrt((nx * nx) + (ny * ny));
      float rx = (float)(cosf(atanf(ny / nx) - rot) * len);
      float ry = (float)(sinf(atanf(ny / nx) - rot) * len);

      if ((fabsf(rx) < sz.x) && (fabsf(ry) < sz.y) && (pos.z <= testPos.z)) {
        return pos.z;
      }
    }
  }

  return -1.0f;
}


//============================================================================//

const TeamBase& TeamBases::getRandomBase()
{
  const int index = (int)(bzfrand() * (double)teamBases.size());
  return teamBases[index % teamBases.size()];
}


void TeamBase::getTopCenter(fvec3& topPos) const
{
  if (obstacle->getTypeID() == faceType) {
    const MeshFace* face = (const MeshFace*) obstacle;
    topPos = face->calcCenter();
  }
  else {
    const fvec3& pos  = obstacle->getPosition();
    const fvec3& size = obstacle->getSize();
    topPos = pos;
    topPos.z += size.z;
  }
}


void TeamBase::getRandomPosition(fvec3& randPos) const
{
  if (obstacle->getTypeID() == faceType) {
    const MeshFace* face = (const MeshFace*) obstacle;
    const fvec3 rPos = face->getRandomPoint();
    const fvec3 center = face->calcCenter();
    const fvec3 diff = (rPos - center);
    randPos = center + (0.9f * diff);
  }
  else {
    const fvec3& pos  = obstacle->getPosition();
    const fvec3& size = obstacle->getSize();
    const float  rot  = obstacle->getRotation();

    const float c = cosf(rot);
    const float s = sinf(rot);
    const float tankRad2 = 2.0f * BZDBCache::tankRadius;
    const float deltaX = (size.x - tankRad2) * ((float)bzfrand() - 0.5f);
    const float deltaY = (size.y - tankRad2) * ((float)bzfrand() - 0.5f);
    randPos = pos;
    randPos.x += (deltaX * c) - (deltaY * s);
    randPos.y += (deltaX * s) + (deltaY * c);
    randPos.z += size.z; // the top
  }
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
