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
#include "Intersect.h"


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

  const float pos[3] = {x, y, z};
  WallObstacle wall (pos, r, w, h);
  walls.push_back (wall);
}

void WorldInfo::addBox(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  const float pos[3] = {x, y, z};
  BoxBuilding box (pos, r, w, d, h, drive, shoot, false);
  boxes.push_back (box);
}

void WorldInfo::addPyramid(float x, float y, float z, float r, float w, float d, float h, bool drive, bool shoot, bool flipZ)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  const float pos[3] = {x, y, z};
  PyramidBuilding pyr (pos, r, w, d, h, drive, shoot);
  if (flipZ) {
    pyr.setZFlip();
  }
  pyramids.push_back (pyr);
}

void WorldInfo::addTeleporter(float x, float y, float z, float r, float w, float d, float h, float b, bool drive, bool shoot)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  const float pos[3] = {x, y, z};
  Teleporter tele (pos, r, w, d, h, b, drive, shoot);
  teleporters.push_back (tele);

  // default to passthru linkage
  int index = teleporters.size() - 1;
  setTeleporterTarget ((index * 2) + 1, (index * 2));
  setTeleporterTarget ((index * 2), (index * 2) + 1);
}

void WorldInfo::addBase(float x, float y, float z, float r,
                        float w, float d, float h,
                        bool /* drive */, bool /* shoot */)
{
  if ((z + h) > maxHeight)
    maxHeight = z+h;

  const float pos[3] = {x, y, z};
  const float size[3] = {w, d, h};
  BaseBuilding base (pos, r, size, 0 /* fake the team */);
  bases.push_back (base);
}

void WorldInfo::addLink(int from, int to)
{
  // discard links to/from teleporters that don't exist
  // note that -1 "to" means the client picks one at random
  if ((unsigned(from) > teleporters.size() * 2 + 1) ||
     ((unsigned(to) > teleporters.size() * 2 + 1) && (to != -1))) {
    DEBUG1("Warning: bad teleporter link dropped from=%d to=%d\n", from, to);
  } else {
    setTeleporterTarget (from, to);
  }
}

void WorldInfo::addZone(const CustomZone *zone)
{
  entryZones.addZone( zone );
}

void WorldInfo::addWeapon(const FlagType *type, const float *origin, float direction,
                          float initdelay, const std::vector<float> &delay, TimeKeeper &sync)
{
  worldWeapons.add(type, origin, direction, initdelay, delay, sync);
}                          

float WorldInfo::getMaxWorldHeight()
{
  return maxHeight;
}

void WorldInfo::setTeleporterTarget(int src, int tgt)
{
  if ((int)teleportTargets.size() < src+1)
    teleportTargets.resize(((src/2)+1)*2);

  // record target in source entry
  teleportTargets[src] = tgt;
}

WorldWeapons& WorldInfo::getWorldWeapons()
{
  return worldWeapons;
}

void                    WorldInfo::loadCollisionManager()
{
  collisionManager.load(boxes, pyramids, teleporters, bases);
  return;
}

void                    WorldInfo::checkCollisionManager()
{
  float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  if (worldSize != collisionManager.getWorldSize()) {
    // reload the collision grid
    collisionManager.load(boxes, pyramids, teleporters, bases);
  }
  return;
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

InBuildingType WorldInfo::cylinderInBuilding(const Obstacle **location,
				             const float* pos, float radius,
                                             float height)
{
  checkCollisionManager(); // FIXME - this is lame

  if (height < Epsilon) {
    height = Epsilon;
  }
    
  *location = NULL;
  
  // check everything but walls
  ObstacleList olist = collisionManager.getObstacles (pos, radius);
  for (ObstacleList::const_iterator oit = olist.begin();
       oit != olist.end(); oit++) {
    const Obstacle* obs = *oit;
    if (obs->inCylinder(pos, radius, height)) {
      *location = obs;
      break;
    }
  }

  return classifyHit (*location);
} 

InBuildingType WorldInfo::cylinderInBuilding(const Obstacle **location,
				             float x, float y, float z, float radius,
                                             float height)
{
  const float pos[3] = {x, y, z};
  return cylinderInBuilding (location, pos, radius, height);
}

InBuildingType WorldInfo::boxInBuilding(const Obstacle **location,
				        const float* pos, float angle,
				        float width, float breadth, float height)
{
  checkCollisionManager(); // FIXME - this is lame

  if (height < Epsilon) {
    height = Epsilon;
  }
    
  *location = NULL;
  
  // check everything but walls
  ObstacleList olist = collisionManager.getObstacles (pos, angle, width, breadth);
  for (ObstacleList::const_iterator oit = olist.begin();
       oit != olist.end(); oit++) {
    const Obstacle* obs = *oit;
    if (obs->inBox(pos, angle, width, breadth, height)) {
      *location = obs;
      break;
    }
  }
  
  return classifyHit (*location);
}
  
InBuildingType WorldInfo::classifyHit (const Obstacle* obstacle)
{
  if (obstacle == NULL) {
    return NOT_IN_BUILDING;
  }
  else if (obstacle->getType() == BoxBuilding::getClassName()) {
    if (obstacle->isDriveThrough()) {
      return IN_BOX_DRIVETHROUGH;
    }
    else {
      return IN_BOX_NOTDRIVETHROUGH;
    }
  }
  else if (obstacle->getType() == PyramidBuilding::getClassName()) {
    return IN_PYRAMID;
  }
  else if (obstacle->getType() == BaseBuilding::getClassName()) {
    return IN_BASE;
  }
  else if (obstacle->getType() == Teleporter::getClassName()) {
    return IN_TELEPORTER;
  }
  else {
    // FIXME - choke here?
    printf ("*** Unknown obstacle type in WorldInfo::boxInBuilding()\n");
    return IN_BASE;
  }
} 

bool WorldInfo::getZonePoint(const std::string &qualifier, float *pt)
{
  const Obstacle* loc;
  InBuildingType type;

  if (!entryZones.getZonePoint(qualifier, pt))
    return false;

  type = cylinderInBuilding(&loc, pt[0], pt[1], 0.0f, 1.0f, pt[2]);
  if (type == NOT_IN_BUILDING)
    pt[2] = 0.0f;
  else
    pt[2] = loc->getPosition()[2] + loc->getSize()[2];
  return true;
}

bool WorldInfo::getSafetyPoint(const std::string &qualifier, const float *pos, float *pt)
{
  const Obstacle *loc;
  InBuildingType type;

  if (!entryZones.getSafetyPoint(qualifier, pos, pt))
    return false;

  type = cylinderInBuilding(&loc, pt[0], pt[1], 0.0f, 1.0f, pt[2]);
  if (type == NOT_IN_BUILDING)
    pt[2] = 0.0f;
  else
    pt[2] = loc->getPosition()[2] + loc->getSize()[2];
  return true;
}

void WorldInfo::finishWorld()
{
  entryZones.calculateQualifierLists();
  loadCollisionManager ();
}

int WorldInfo::packDatabase()
{
  databaseSize =
    (2 + 2 + WorldCodeWallSize) * walls.size() +
    (2 + 2 + WorldCodeBoxSize) * boxes.size() +
    (2 + 2 + WorldCodePyramidSize) * pyramids.size() +
    (2 + 2 + WorldCodeTeleporterSize) * teleporters.size() +
    (2 + 2 + WorldCodeLinkSize) * 2 * teleporters.size() +
    worldWeapons.packSize() + entryZones.packSize();
  database = new char[databaseSize];
  void *databasePtr = database;

  unsigned char	bitMask;

  // add walls  
  for (std::vector<WallObstacle>::iterator wall_it = walls.begin();
       wall_it != walls.end(); ++wall_it) {
    WallObstacle &wall = *wall_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeWallSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeWall);
    databasePtr = nboPackVector(databasePtr, wall.getPosition());
    databasePtr = nboPackFloat(databasePtr, wall.getRotation());
    databasePtr = nboPackFloat(databasePtr, wall.getSize()[1]);
    // walls have no depth
    databasePtr = nboPackFloat(databasePtr, wall.getSize()[2]);
  }

  // add boxes
  for (std::vector<BoxBuilding>::iterator box_it = boxes.begin();
       box_it != boxes.end(); ++box_it) {
    BoxBuilding &box = *box_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeBoxSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeBox);
    databasePtr = nboPackVector(databasePtr, box.getPosition());
    databasePtr = nboPackFloat(databasePtr, box.getRotation());
    databasePtr = nboPackVector(databasePtr, box.getSize());
    bitMask = 0;
    if (box.isDriveThrough())
      bitMask |= _DRIVE_THRU;
    if (box.isShootThrough())
      bitMask |= _SHOOT_THRU;
    databasePtr = nboPackUByte(databasePtr, bitMask);
  }

  // add pyramids
  for (std::vector<PyramidBuilding>::iterator pyr_it = pyramids.begin();
       pyr_it != pyramids.end(); ++pyr_it) {
    PyramidBuilding &pyr = *pyr_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodePyramidSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodePyramid);
    databasePtr = nboPackVector(databasePtr, pyr.getPosition());
    databasePtr = nboPackFloat(databasePtr, pyr.getRotation());
    databasePtr = nboPackVector(databasePtr, pyr.getSize());
    bitMask = 0;
    if (pyr.isDriveThrough())
      bitMask |= _DRIVE_THRU;
    if (pyr.isShootThrough())
      bitMask |= _SHOOT_THRU;
    if (pyr.getZFlip())
      bitMask |= _FLIP_Z;
    databasePtr = nboPackUByte(databasePtr, bitMask);
  }

  // add teleporters
  int i = 0;
  for (std::vector<Teleporter>::iterator tele_it = teleporters.begin();
       tele_it != teleporters.end(); ++tele_it, i++) {
    Teleporter &tele = *tele_it;
    databasePtr = nboPackUShort(databasePtr, WorldCodeTeleporterSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeTeleporter);
    databasePtr = nboPackVector(databasePtr, tele.getPosition());
    databasePtr = nboPackFloat(databasePtr, tele.getRotation());
    databasePtr = nboPackVector(databasePtr, tele.getSize());
    databasePtr = nboPackFloat(databasePtr, tele.getBorder());
    bitMask = 0;
    if (tele.isDriveThrough())
      bitMask |= _DRIVE_THRU;
    if (tele.isShootThrough())
      bitMask |= _SHOOT_THRU;
    databasePtr = nboPackUByte(databasePtr, bitMask);
    // and each link
    databasePtr = nboPackUShort(databasePtr, WorldCodeLinkSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
    databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2));
    databasePtr = nboPackUShort(databasePtr, uint16_t(teleportTargets[i * 2]));
    databasePtr = nboPackUShort(databasePtr, WorldCodeLinkSize);
    databasePtr = nboPackUShort(databasePtr, WorldCodeLink);
    databasePtr = nboPackUShort(databasePtr, uint16_t(i * 2 + 1));
    databasePtr = nboPackUShort(databasePtr, uint16_t(teleportTargets[i * 2 + 1]));
  }
  
  databasePtr = worldWeapons.pack (databasePtr);
  databasePtr = entryZones.pack (databasePtr);

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
