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

#include "WorldInfo.h"

WorldInfo::WorldInfo() :
  numWalls(0),
  numBases(0),
  numBoxes(0),
  numPyramids(0),
  numTeleporters(0),
  sizeWalls(0),
  sizeBoxes(0),
  sizePyramids(0),
  sizeTeleporters(0),
  sizeBases(0),
  maxHeight(0),
  walls(NULL),
  boxes(NULL),
  bases(NULL),
  pyramids(NULL),
  teleporters(NULL),
  database(NULL)
{
  size[0] = 400.0f;
  size[1] = 400.0f;
  gravity = -9.81f;
}

WorldInfo::~WorldInfo()
{
  free(walls);
  free(boxes);
  free(pyramids);
  free(teleporters);
  if(bases != NULL)
    free(bases);
  delete[] database;
}

void WorldInfo::addWall(float x, float y, float z, float r, float w, float h)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  if (numWalls >= sizeWalls) {
    sizeWalls = (sizeWalls == 0) ? 16 : 2 * sizeWalls;
    walls = (ObstacleLocation *)realloc(walls, sizeof(ObstacleLocation) * sizeWalls);
  }
  walls[numWalls].pos[0] = x;
  walls[numWalls].pos[1] = y;
  walls[numWalls].pos[2] = z;
  walls[numWalls].rotation = r;
  walls[numWalls].size[0] = w;
  // no depth to walls
  walls[numWalls].size[1] = 0.0f;
  walls[numWalls].size[2] = h;
  numWalls++;
}

void WorldInfo::addBox(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  if (numBoxes >= sizeBoxes) {
    sizeBoxes = (sizeBoxes == 0) ? 16 : 2 * sizeBoxes;
    boxes = (ObstacleLocation *)realloc(boxes, sizeof(ObstacleLocation) * sizeBoxes);
  }
  boxes[numBoxes].pos[0] = x;
  boxes[numBoxes].pos[1] = y;
  boxes[numBoxes].pos[2] = z;
  boxes[numBoxes].rotation = r;
  boxes[numBoxes].size[0] = w;
  boxes[numBoxes].size[1] = d;
  boxes[numBoxes].size[2] = h;
  boxes[numBoxes].driveThrough = drive;
  boxes[numBoxes].shootThrough = shoot;
  numBoxes++;
}

void WorldInfo::addPyramid(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot, bool flipZ)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  if (numPyramids >= sizePyramids) {
    sizePyramids = (sizePyramids == 0) ? 16 : 2 * sizePyramids;
    pyramids = (ObstacleLocation *)realloc(pyramids, sizeof(ObstacleLocation) * sizePyramids);
  }
  pyramids[numPyramids].pos[0] = x;
  pyramids[numPyramids].pos[1] = y;
  pyramids[numPyramids].pos[2] = z;
  pyramids[numPyramids].rotation = r;
  pyramids[numPyramids].size[0] = w;
  pyramids[numPyramids].size[1] = d;
  pyramids[numPyramids].size[2] = h;
  pyramids[numPyramids].driveThrough = drive;
  pyramids[numPyramids].shootThrough = shoot;
  pyramids[numPyramids].flipZ = flipZ;
numPyramids++;
}

void WorldInfo::addTeleporter(float x, float y, float z, float r, float w, float d, float h, float b, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  if (numTeleporters >= sizeTeleporters) {
    sizeTeleporters = (sizeTeleporters == 0) ? 16 : 2 * sizeTeleporters;
    teleporters = (Teleporter *)realloc(teleporters, sizeof(Teleporter) * sizeTeleporters);
  }
  teleporters[numTeleporters].pos[0] = x;
  teleporters[numTeleporters].pos[1] = y;
  teleporters[numTeleporters].pos[2] = z;
  teleporters[numTeleporters].rotation = r;
  teleporters[numTeleporters].size[0] = w;
  teleporters[numTeleporters].size[1] = d;
  teleporters[numTeleporters].size[2] = h;
  teleporters[numTeleporters].driveThrough = drive;
  teleporters[numTeleporters].shootThrough = shoot;
  teleporters[numTeleporters].border = b;
  // default link through
  teleporters[numTeleporters].to[0] = numTeleporters * 2 + 1;
  teleporters[numTeleporters].to[1] = numTeleporters * 2;
  numTeleporters++;
}

void WorldInfo::addBase(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  if(numBases >= sizeBases) {
    sizeBases = (sizeBases == 0) ? 16 : 2 * sizeBases;
    bases = (ObstacleLocation *) realloc(bases, sizeof(ObstacleLocation) * sizeBases);
  }
  bases[numBases].pos[0] = x;
  bases[numBases].pos[1] = y;
  bases[numBases].pos[2] = z;
  bases[numBases].rotation = r;
  bases[numBases].size[0] = w;
  bases[numBases].size[1] = d;
  bases[numBases].size[2] = h;
  bases[numBases].driveThrough = drive;
  bases[numBases].shootThrough = shoot;
  numBases++;
}

void WorldInfo::addLink(int from, int to)
{
  // silently discard links from teleporters that don't exist
  if (from <= numTeleporters * 2 + 1) {
    teleporters[from / 2].to[from % 2] = to;
  }
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

InBuildingType WorldInfo::inBuilding(WorldInfo::ObstacleLocation **location,
				     float x, float y, float z, float r,
				     float height) const
{

  if (height < Epsilon)
    height = Epsilon;

  int i;
  for (i = 0; i < numBases; i++) {
    if ((bases[i].pos[2] < (z + height))
	&& ((bases[i].pos[2] + bases[i].size[2]) > z)
	&&	(inRect(bases[i].pos, bases[i].rotation, bases[i].size, x, y, r))) {
      if(location != NULL)
	*location = &bases[i];
      return IN_BASE;
    }
  }
  for (i = 0; i < numBoxes; i++)
    if ((boxes[i].pos[2] < (z + height))
	&& ((boxes[i].pos[2] + boxes[i].size[2]) > z)
	&&	(inRect(boxes[i].pos, boxes[i].rotation, boxes[i].size, x, y, r))) {
      if (location != NULL)
	*location = &boxes[i];
      if (boxes[i].driveThrough) return IN_BOX_DRIVETHROUGH;
      else return IN_BOX_NOTDRIVETHROUGH;
    }
  for (i = 0; i < numPyramids; i++) {
    if ((pyramids[i].pos[2] < (z + height))
	&& ((pyramids[i].pos[2] + pyramids[i].size[2]) > z)
	&&	(inRect(pyramids[i].pos, pyramids[i].rotation, pyramids[i].size,x,y,r))) {
      if (location != NULL)
	*location = &pyramids[i];
      return IN_PYRAMID;
    }
  }
  for (i = 0; i < numTeleporters; i++)
    if ((teleporters[i].pos[2] < (z + height))
	&& ((teleporters[i].pos[2] + teleporters[i].size[2]) > z)
	&&	(inRect(teleporters[i].pos, teleporters[i].rotation, teleporters[i].size, x, y, r))) {
      static ObstacleLocation __teleporter;
      __teleporter = teleporters[i];
      if (location != NULL)
	*location = &__teleporter;
      return IN_TELEPORTER;
    }
  if (location != NULL)
    *location = (ObstacleLocation *)NULL;
  return NOT_IN_BUILDING;
}

int WorldInfo::packDatabase()
{
  databaseSize =
    (2 + 2 + WorldCodeWallSize) * numWalls +
    (2 + 2 + WorldCodeBoxSize) * numBoxes +
    (2 + 2 + WorldCodePyramidSize) * numPyramids +
    (2 + 2 + WorldCodeTeleporterSize) * numTeleporters +
    (2 + 2 + WorldCodeLinkSize) * 2 * numTeleporters;
  database = new char[databaseSize];
  void *databasePtr = database;

  // define i out here so we avoid the loop variable scope debates
  int i;
  unsigned char	bitMask;

  // add walls
  ObstacleLocation *pWall;
  for (i = 0, pWall = walls ; i < numWalls ; i++, pWall++) {
    databasePtr = nboPackUShort(databasePtr, WorldCodeWallSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeWall);
    databasePtr = nboPackVector(databasePtr, pWall->pos);
    databasePtr = nboPackFloat(databasePtr, pWall->rotation);
    databasePtr = nboPackFloat(databasePtr, pWall->size[0]);
    // walls have no depth
    databasePtr = nboPackFloat(databasePtr, pWall->size[2]);
  }

  // add boxes
  ObstacleLocation *pBox;
  for (i = 0, pBox = boxes ; i < numBoxes ; i++, pBox++) {
    databasePtr = nboPackUShort(databasePtr, WorldCodeBoxSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeBox);
    databasePtr = nboPackVector(databasePtr, pBox->pos);
    databasePtr = nboPackFloat(databasePtr, pBox->rotation);
    databasePtr = nboPackVector(databasePtr, pBox->size);
	bitMask = 0;
	if (pBox->driveThrough)
		bitMask |= _DRIVE_THRU;
 	if (pBox->shootThrough)
		bitMask |= _SHOOT_THRU;
	databasePtr = nboPackUByte(databasePtr, bitMask);
 }

  // add pyramids
  ObstacleLocation *pPyramid;
  for (i = 0, pPyramid = pyramids ; i < numPyramids ; i++, pPyramid++) {
    databasePtr = nboPackUShort(databasePtr, WorldCodePyramidSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodePyramid);
    databasePtr = nboPackVector(databasePtr, pPyramid->pos);
    databasePtr = nboPackFloat(databasePtr, pPyramid->rotation);
    databasePtr = nboPackVector(databasePtr, pPyramid->size);
	bitMask = 0;
	if (pPyramid->driveThrough)
		bitMask |= _DRIVE_THRU;
 	if (pPyramid->shootThrough)
		bitMask |= _SHOOT_THRU;
 	if (pPyramid->flipZ)
		bitMask |= _FLIP_Z;
	databasePtr = nboPackUByte(databasePtr, bitMask);
  }

  // add teleporters
  Teleporter *pTeleporter;
  for (i = 0, pTeleporter = teleporters ; i < numTeleporters ; i++, pTeleporter++) {
    databasePtr = nboPackUShort(databasePtr, WorldCodeTeleporterSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeTeleporter);
    databasePtr = nboPackVector(databasePtr, pTeleporter->pos);
    databasePtr = nboPackFloat(databasePtr, pTeleporter->rotation);
    databasePtr = nboPackVector(databasePtr, pTeleporter->size);
    databasePtr = nboPackFloat(databasePtr, pTeleporter->border);
 	bitMask = 0;
	if (pTeleporter->driveThrough)
		bitMask |= _DRIVE_THRU;
 	if (pTeleporter->shootThrough)
		bitMask |= _SHOOT_THRU;
	databasePtr = nboPackUByte(databasePtr, bitMask);
   // and each link
    databasePtr = nboPackUShort(databasePtr, WorldCodeLinkSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
    databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2));
    databasePtr = nboPackUShort(databasePtr, uint16_t(pTeleporter->to[0]));
    databasePtr = nboPackUShort(databasePtr, WorldCodeLinkSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
    databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2 + 1));
    databasePtr = nboPackUShort(databasePtr, uint16_t(pTeleporter->to[1]));
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
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

