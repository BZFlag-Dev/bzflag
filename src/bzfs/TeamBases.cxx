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

#ifdef _MSC_VER
#pragma warning(4:4786)
#endif

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
    float pyrBase = BZDB.eval(StateDatabase::BZDB_PYRBASE);
    float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);

    teamBases.resize(1);
    TeamBase &teamBase = teamBases[0];
    switch (team)
    {
      case RedTeam:
	teamBase.position[0] = (-worldSize + baseSize) / 2.0f;
	teamBase.position[1] = 0.0f;
	teamBase.safetyZone[0] = teamBase.position[0] + 0.5f * baseSize + pyrBase;
	teamBase.safetyZone[1] = teamBase.position[1] + 0.5f * baseSize + pyrBase;
      break;

      case GreenTeam:
	teamBase.position[0] = (worldSize - baseSize) / 2.0f;
	teamBase.position[1] = 0.0f;
	teamBase.safetyZone[0] = teamBase.position[0] - 0.5f * baseSize - pyrBase;
	teamBase.safetyZone[1] = teamBase.position[1] - 0.5f * baseSize - pyrBase;
      break;

      case BlueTeam:
	teamBase.position[0] = 0.0f;
	teamBase.position[1] = (-worldSize + baseSize) / 2.0f;
	teamBase.safetyZone[0] = teamBase.position[0] - 0.5f * baseSize - pyrBase;
	teamBase.safetyZone[1] = teamBase.position[1] + 0.5f * baseSize + pyrBase;
      break;

      case PurpleTeam:
	teamBase.position[0] = 0.0f;
	teamBase.position[1] = (worldSize - baseSize) / 2.0f;
	teamBase.safetyZone[0] = teamBase.position[0] + 0.5f * baseSize + pyrBase;
	teamBase.safetyZone[1] = teamBase.position[1] - 0.5f * baseSize - pyrBase;
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
    teamBase.safetyZone[2] = teamBase.position[2];
}

void TeamBases::addBase( const float *position, const float *size, float rotation, const float *safetyZone )
{
  TeamBase base(position, size, rotation, safetyZone);
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

void *TeamBases::pack( void *buf ) const
{
  for (TeamBaseList::const_iterator it = teamBases.begin(); it != teamBases.end(); ++it) {
    buf = nboPackUShort(buf, WorldCodeBaseSize);
    buf = nboPackUShort(buf, WorldCodeBase);
    buf = nboPackUShort(buf, uint16_t(color));
    buf = nboPackVector(buf, it->position);
    buf = nboPackFloat(buf, it->rotation);
    buf = nboPackVector(buf, it->size);
    buf = nboPackUByte(buf, 0); // state bits
  }

  return buf;
}

float TeamBases::findBaseZ( float x, float y, float z ) const
{
  for (TeamBaseList::const_iterator it = teamBases.begin(); it != teamBases.end(); ++it) {
    const float *pos  = it->position;
    const float *size = it->size;
    float rotation = it->rotation;
    float nx = x - pos[0];
    float ny = y - pos[1];
    if (nx == 0.0f)
      nx = 1.0f;
    float rx = (float)(cosf(atanf(ny/nx)-rotation) * sqrt((ny * ny) + (nx * nx)));
    float ry = (float)(sinf(atanf(ny/nx)-rotation) * sqrt((ny * ny) + (nx * nx)));


    if (fabsf(rx) < size[0] &&
	fabsf(ry) < size[1] &&
	pos[2] <= z)
      return pos[2];
  }

  return -1.0f;
}

void TeamBases::getSafetyZone( float &x, float &y, float &z ) const
{
  int baseIndex = (int) (teamBases.size() * bzfrand());
  const TeamBase &base = teamBases[baseIndex];

  x = base.safetyZone[0];
  y = base.safetyZone[1];
  z = base.safetyZone[2];
}

const TeamBase &TeamBases::getRandomBase( int id )
{
  return teamBases[id % teamBases.size()];
}

TeamBase::TeamBase(const float *pos, const float *siz, float rot, const float *safety)
{
  memcpy(&position, pos, sizeof position);
  memcpy(&size, siz, sizeof size);
  memcpy(&safetyZone, safety, sizeof safetyZone);
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
