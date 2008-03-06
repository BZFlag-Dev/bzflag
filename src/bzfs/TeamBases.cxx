/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
    switch (team)
    {
      case RedTeam:
	teamBase.position[0] = (-worldSize + baseSize) / 2.0f;
	teamBase.position[1] = 0.0f;
      break;

      case GreenTeam:
	teamBase.position[0] = (worldSize - baseSize) / 2.0f;
	teamBase.position[1] = 0.0f;
      break;

      case BlueTeam:
	teamBase.position[0] = 0.0f;
	teamBase.position[1] = (-worldSize + baseSize) / 2.0f;
      break;

      case PurpleTeam:
	teamBase.position[0] = 0.0f;
	teamBase.position[1] = (worldSize - baseSize) / 2.0f;
      break;

      default:
	// no valid team, should throw here if we could
	break;
    }

    teamBase.position[2] = 0.0f;
    teamBase.rotation = 0.0f;
    teamBase.size[0] = baseSize / 2.0f;
    teamBase.size[1] = baseSize / 2.0f;
    teamBase.size[2] = 0.0f;
}

void TeamBases::addBase(const float *position, const float *_size,
			float rotation )
{
  TeamBase base(position, _size, rotation);
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

const float *TeamBases::getBasePosition( int base ) const
{
  if ((base < 0) || (base >= (int)teamBases.size()))
    base = 0;

  return teamBases[base].position;
}

float TeamBases::findBaseZ( float x, float y, float z ) const
{
  for (TeamBaseList::const_iterator it = teamBases.begin(); it != teamBases.end(); ++it) {
    const float *pos  = it->position;
    const float *_size = it->size;
    float rotation = it->rotation;
    float nx = x - pos[0];
    float ny = y - pos[1];
    if (nx == 0.0f)
      nx = 1.0f;
    float rx = (float)(cosf(atanf(ny/nx)-rotation) * sqrt((ny * ny) + (nx * nx)));
    float ry = (float)(sinf(atanf(ny/nx)-rotation) * sqrt((ny * ny) + (nx * nx)));


    if (fabsf(rx) < _size[0] &&
	fabsf(ry) < _size[1] &&
	pos[2] <= z)
      return pos[2];
  }

  return -1.0f;
}

const TeamBase &TeamBases::getRandomBase( int id )
{
  return teamBases[id % teamBases.size()];
}

TeamBase::TeamBase(const float *pos, const float *siz, float rot)
{
  memcpy(&position, pos, sizeof position);
  memcpy(&size, siz, sizeof size);
  rotation = rot;
}

void TeamBase::getRandomPosition( float &x, float &y, float &z ) const
{
  float deltaX = (size[0] - 2.0f * BZDBCache::tankRadius) * ((float)bzfrand() - 0.5f);
  float deltaY = (size[1] - 2.0f * BZDBCache::tankRadius) * ((float)bzfrand() - 0.5f);
  x = position[0] + deltaX * cosf(rotation) - deltaY * sinf(rotation);
  y = position[1] + deltaX * sinf(rotation) + deltaY * cosf(rotation);
  z = position[2] + size[2];
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
