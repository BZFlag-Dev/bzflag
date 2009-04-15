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

/* system headers */
#include <string.h>

#include "Protocol.h"
#include "TeamBases.h"
#include "Pack.h"
#include "BZDBCache.h"

TeamBases::TeamBases()
{
    color = RedTeam;
}

TeamBases::TeamBases(TeamColor team, bool initDefault)
{
    color = team;

    if (!initDefault)
      return;

    float worldSize = BZDBCache::worldSize;
    float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);

    teamBases.resize(1);
    TeamBase &teamBase = teamBases[0];
    switch (team) {
      case RedTeam:
	teamBase.position.x = (-worldSize + baseSize) / 2.0f;
	teamBase.position.y = 0.0f;
	break;

      case GreenTeam:
	teamBase.position.x = (worldSize - baseSize) / 2.0f;
	teamBase.position.y = 0.0f;
	break;

      case BlueTeam:
	teamBase.position.x = 0.0f;
	teamBase.position.y = (-worldSize + baseSize) / 2.0f;
	break;

      case PurpleTeam:
	teamBase.position.x = 0.0f;
	teamBase.position.y = (worldSize - baseSize) / 2.0f;
	break;

      default:
	// no valid team, should throw here if we could
	break;
    }

    teamBase.position.z = 0.0f;
    teamBase.rotation = 0.0f;
    teamBase.size.x = baseSize / 2.0f;
    teamBase.size.y = baseSize / 2.0f;
    teamBase.size.z = 0.0f;
}

void TeamBases::addBase(const fvec3& _pos, const fvec3& _size, float _rotation)
{
  TeamBase base(_pos, _size, _rotation);
  teamBases.push_back(base);
}

int TeamBases::size() const
{
  return teamBases.size();
}

TeamColor TeamBases::getTeam() const
{
  return color;
}

const fvec3& TeamBases::getBasePosition( int base ) const
{
  if ((base < 0) || (base >= (int)teamBases.size()))
    base = 0;

  return teamBases[base].position;
}

float TeamBases::findBaseZ( float x, float y, float z ) const
{
  for (TeamBaseList::const_iterator it = teamBases.begin(); it != teamBases.end(); ++it) {
    const fvec3& pos   = it->position;
    const fvec3& _size = it->size;
    float rotation = it->rotation;
    float nx = x - pos.x;
    float ny = y - pos.y;
    if (nx == 0.0f)
      nx = 1.0f;
    float rx = (float)(cosf(atanf(ny/nx)-rotation) * sqrt((ny * ny) + (nx * nx)));
    float ry = (float)(sinf(atanf(ny/nx)-rotation) * sqrt((ny * ny) + (nx * nx)));


    if (fabsf(rx) < _size.x &&
	fabsf(ry) < _size.y &&
	pos.z <= z)
      return pos.z;
  }

  return -1.0f;
}

const TeamBase &TeamBases::getRandomBase( int id )
{
  return teamBases[id % teamBases.size()];
}

TeamBase::TeamBase(const fvec3& pos, const fvec3& siz, float rot)
: position(pos)
, size(siz)
, rotation(rot)
{
}

void TeamBase::getRandomPosition( float &x, float &y, float &z ) const
{
  float deltaX = (size.x - 2.0f * BZDBCache::tankRadius) * ((float)bzfrand() - 0.5f);
  float deltaY = (size.y - 2.0f * BZDBCache::tankRadius) * ((float)bzfrand() - 0.5f);
  x = position.x + deltaX * cosf(rotation) - deltaY * sinf(rotation);
  y = position.y + deltaX * sinf(rotation) + deltaY * cosf(rotation);
  z = position.z + size.z;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
