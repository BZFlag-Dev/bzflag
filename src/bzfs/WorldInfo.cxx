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
#include "WorldInfo.h"

/* common implementation headers */
#include "global.h"
#include "Pack.h"
#include "Protocol.h"


WorldInfo::WorldInfo() :
  maxHeight(0),
  database(NULL)
{
  size[0] = 400.0f;
  size[1] = 400.0f;
  gravity = -9.81f;
}

WorldInfo::~WorldInfo()
{
  delete[] database;
}

void WorldInfo::addWall(float x, float y, float z, float r, float w, float h)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  int i = walls.size();
  walls.resize(i+1);
  walls[i].pos[0] = x;
  walls[i].pos[1] = y;
  walls[i].pos[2] = z;
  walls[i].rotation = r;
  walls[i].size[0] = w;
  // no depth to walls
  walls[i].size[1] = 0.0f;
  walls[i].size[2] = h;
}

void WorldInfo::addBox(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  int i = boxes.size();
  boxes.resize(i+1);
  boxes[i].pos[0] = x;
  boxes[i].pos[1] = y;
  boxes[i].pos[2] = z;
  boxes[i].rotation = r;
  boxes[i].size[0] = w;
  boxes[i].size[1] = d;
  boxes[i].size[2] = h;
  boxes[i].driveThrough = drive;
  boxes[i].shootThrough = shoot;
}

void WorldInfo::addPyramid(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot, bool flipZ)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  int i = pyramids.size();
  pyramids.resize(i+1);
  pyramids[i].pos[0] = x;
  pyramids[i].pos[1] = y;
  pyramids[i].pos[2] = z;
  pyramids[i].rotation = r;
  pyramids[i].size[0] = w;
  pyramids[i].size[1] = d;
  pyramids[i].size[2] = h;
  pyramids[i].driveThrough = drive;
  pyramids[i].shootThrough = shoot;
  pyramids[i].flipZ = flipZ;
}

void WorldInfo::addTeleporter(float x, float y, float z, float r, float w, float d, float h, float b, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  int i = teleporters.size();
  teleporters.resize(i+1);
  teleporters[i].pos[0] = x;
  teleporters[i].pos[1] = y;
  teleporters[i].pos[2] = z;
  teleporters[i].rotation = r;
  teleporters[i].size[0] = w;
  teleporters[i].size[1] = d;
  teleporters[i].size[2] = h;
  teleporters[i].driveThrough = drive;
  teleporters[i].shootThrough = shoot;
  teleporters[i].border = b;
  // default link through
  teleporters[i].to[0] = i * 2 + 1;
  teleporters[i].to[1] = i * 2;
}

void WorldInfo::addBase(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  int i = bases.size();
  bases.resize(i+1);
  bases[i].pos[0] = x;
  bases[i].pos[1] = y;
  bases[i].pos[2] = z;
  bases[i].rotation = r;
  bases[i].size[0] = w;
  bases[i].size[1] = d;
  bases[i].size[2] = h;
  bases[i].driveThrough = drive;
  bases[i].shootThrough = shoot;
}

void WorldInfo::addLink(int from, int to)
{
  // silently discard links from teleporters that don't exist
  if (unsigned(from) <= teleporters.size() * 2 + 1) {
    teleporters[from / 2].to[from % 2] = to;
  }
}

void WorldInfo::addZone(const CustomZone *zone)
{
  entryZones.addZone( zone );
}

float WorldInfo::getMaxWorldHeight()
{
  return maxHeight;
}

bool WorldInfo::rectHitCirc(float dx, float dy, const float *p, float r) const
{
  // Algorithm from Graphics Gems, pp51-53.
  const float rr = r * r, rx = -p[0], ry = -p[1];
  if (rx + dx < 0.0f) // west of rect
    if (ry + dy < 0.0f) //  sw corner
      return (rx + dx) * (rx + dx) + (ry + dy) * (ry + dy) < rr;
    else if (ry - dy > 0.0f) //  nw corner
      return (rx + dx) * (rx + dx) + (ry - dy) * (ry - dy) < rr;
    else //  due west
      return rx + dx > -r;

  else if (rx - dx > 0.0f) // east of rect
    if (ry + dy < 0.0f) //  se corner
      return (rx - dx) * (rx - dx) + (ry + dy) * (ry + dy) < rr;
    else if (ry - dy > 0.0f) //  ne corner
      return (rx - dx) * (rx - dx) + (ry - dy) * (ry - dy) < rr;
    else //  due east
      return rx - dx < r;

  else if (ry + dy < 0.0f) // due south
    return ry + dy > -r;

  else if (ry - dy > 0.0f) // due north
    return ry - dy < r;

  // circle origin in rect
  return true;
}

bool WorldInfo::inRect(const float *p1, float angle, const float *size, float x, float y, float r) const
{
  // translate origin
  float pa[2];
  pa[0] = x - p1[0];
  pa[1] = y - p1[1];

  // rotate
  float pb[2];
  const float c = cosf(-angle), s = sinf(-angle);
  pb[0] = c * pa[0] - s * pa[1];
  pb[1] = c * pa[1] + s * pa[0];

  // do test
  return rectHitCirc(size[0], size[1], pb, r);
}

InBuildingType WorldInfo::inBuilding(ObstacleLocation **location,
				     float x, float y, float z, float r,
				     float height)
{
  ObstacleLocationList::iterator it;
  PyramidList::iterator pyrit;
  TeleporterList::iterator telit;

  for (it = bases.begin(); it != bases.end(); ++it) {
    ObstacleLocation &base = *it;
    if ((base.pos[2] < (z + height))
	&& ((base.pos[2] + base.size[2]) > z)
	&& (inRect(base.pos, base.rotation, base.size, x, y, r))) {
      if (location != NULL)
	*location = &base;
      return IN_BASE;
    }
  }
  for (it = boxes.begin(); it != boxes.end(); ++it) {
    ObstacleLocation &box = *it;
    if ((box.pos[2] < (z + height))
	&& ((box.pos[2] + box.size[2]) > z)
	&& (inRect(box.pos, box.rotation, box.size, x, y, r))) {
      if (location != NULL)
	*location = &box;
      if (box.driveThrough) return IN_BOX_DRIVETHROUGH;
      else return IN_BOX_NOTDRIVETHROUGH;
    }
  }
  for (pyrit = pyramids.begin(); pyrit != pyramids.end(); ++pyrit) {
    ObstacleLocation &pyr = *pyrit;
    if ((pyr.pos[2] < (z + height))
	&& ((pyr.pos[2] + pyr.size[2]) > z)
	&& (inRect(pyr.pos, pyr.rotation, pyr.size,x,y,r))) {
      if (location != NULL)
	*location = &pyr;
      return IN_PYRAMID;
    }
  }
  for (telit = teleporters.begin(); telit != teleporters.end(); ++telit) {
    Teleporter &tele = *telit;
    if ((tele.pos[2] < (z + height))
	&& ((tele.pos[2] + tele.size[2]) > z)
	&& (inRect(tele.pos, tele.rotation, tele.size, x, y, r))) {
      static ObstacleLocation __teleporter;
      __teleporter = tele;
      if (location != NULL)
	*location = &__teleporter;
      return IN_TELEPORTER;
    }
  }

  if (location != NULL)
    *location = (ObstacleLocation *)NULL;
  return NOT_IN_BUILDING;
}

bool WorldInfo::getZonePoint(const std::string &qualifier, float *pt)
{
  return entryZones.getZonePoint(qualifier, pt);
}

void WorldInfo::finishWorld()
{
  entryZones.calculateQualifierLists();
}

int WorldInfo::packDatabase()
{
  databaseSize =
    (2 + 2 + WorldCodeWallSize) * walls.size() +
    (2 + 2 + WorldCodeBoxSize) * boxes.size() +
    (2 + 2 + WorldCodePyramidSize) * pyramids.size() +
    (2 + 2 + WorldCodeTeleporterSize) * teleporters.size() +
    (2 + 2 + WorldCodeLinkSize) * 2 * teleporters.size();
  database = new char[databaseSize];
  void *databasePtr = database;

  // define i out here so we avoid the loop variable scope debates
  ObstacleLocationList::iterator it;
  unsigned char	bitMask;

  // add walls  
  for (it = walls.begin(); it != walls.end(); ++it) {
    ObstacleLocation &wall = *it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeWallSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeWall);
    databasePtr = nboPackVector(databasePtr, wall.pos);
    databasePtr = nboPackFloat(databasePtr, wall.rotation);
    databasePtr = nboPackFloat(databasePtr, wall.size[0]);
    // walls have no depth
    databasePtr = nboPackFloat(databasePtr, wall.size[2]);
  }

  // add boxes
  for (it = boxes.begin(); it != boxes.end(); ++it) {
    ObstacleLocation &box = *it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeBoxSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeBox);
    databasePtr = nboPackVector(databasePtr, box.pos);
    databasePtr = nboPackFloat(databasePtr, box.rotation);
    databasePtr = nboPackVector(databasePtr, box.size);
    bitMask = 0;
    if (box.driveThrough)
      bitMask |= _DRIVE_THRU;
    if (box.shootThrough)
      bitMask |= _SHOOT_THRU;
    databasePtr = nboPackUByte(databasePtr, bitMask);
  }

  // add pyramids
  for (PyramidList::iterator pyr_it = pyramids.begin(); pyr_it != pyramids.end(); ++pyr_it) {
    Pyramid &pyr = *pyr_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodePyramidSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodePyramid);
    databasePtr = nboPackVector(databasePtr, pyr.pos);
    databasePtr = nboPackFloat(databasePtr, pyr.rotation);
    databasePtr = nboPackVector(databasePtr, pyr.size);
    bitMask = 0;
    if (pyr.driveThrough)
      bitMask |= _DRIVE_THRU;
    if (pyr.shootThrough)
      bitMask |= _SHOOT_THRU;
    if (pyr.flipZ)
      bitMask |= _FLIP_Z;
    databasePtr = nboPackUByte(databasePtr, bitMask);
  }

  // add teleporters
  int i = 0;
  for (TeleporterList::iterator telit = teleporters.begin(); telit != teleporters.end(); ++telit, i++) {
    Teleporter &tele = *telit;
    databasePtr = nboPackUShort(databasePtr, WorldCodeTeleporterSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeTeleporter);
    databasePtr = nboPackVector(databasePtr, tele.pos);
    databasePtr = nboPackFloat(databasePtr, tele.rotation);
    databasePtr = nboPackVector(databasePtr, tele.size);
    databasePtr = nboPackFloat(databasePtr, tele.border);
    bitMask = 0;
    if (tele.driveThrough)
      bitMask |= _DRIVE_THRU;
    if (tele.shootThrough)
      bitMask |= _SHOOT_THRU;
    databasePtr = nboPackUByte(databasePtr, bitMask);
    // and each link
    databasePtr = nboPackUShort(databasePtr, WorldCodeLinkSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
    databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2));
    databasePtr = nboPackUShort(databasePtr, uint16_t(tele.to[0]));
    databasePtr = nboPackUShort(databasePtr, WorldCodeLinkSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
    databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2 + 1));
    databasePtr = nboPackUShort(databasePtr, uint16_t(tele.to[1]));
  }

  return 1;
}

void *WorldInfo::getDatabase() const
{
  return database;
}

int WorldInfo::getDatabaseSize() const
{
  return databaseSize;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
