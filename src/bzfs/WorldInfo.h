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

#ifndef __WORLDINFO_H__
#define __WORLDINFO_H__

#include "common.h"

/* system interface headers */
#include <vector>
#include <string>
#include "EntryZones.h"

class WorldFileLocation;
class CustomZone;

typedef std::vector<std::string> QualifierList;

typedef enum { NOT_IN_BUILDING, IN_BASE, IN_BOX_NOTDRIVETHROUGH, IN_BOX_DRIVETHROUGH, IN_PYRAMID, IN_TELEPORTER } InBuildingType;

struct ObstacleLocation {
public:
  float pos[3];
  float rotation;
  float size[3];
  bool shootThrough;
  bool driveThrough;
  ObstacleLocation &operator=(const ObstacleLocation &ol)
  {
    memcpy(pos, ol.pos, sizeof(float) * 3);
    rotation = ol.rotation;
    memcpy(size, ol.size, sizeof(float) * 3);
    shootThrough = ol.shootThrough;
    driveThrough = ol.driveThrough;
    return *this;
  }
};

typedef std::vector<ObstacleLocation> ObstacleLocationList;

struct Teleporter : public ObstacleLocation {
public:
  float border;
  int to[2];
};

typedef std::vector<Teleporter> TeleporterList;

struct Pyramid : public ObstacleLocation {
public:
  bool flipZ;
  Pyramid &operator=(const Pyramid &p)
  {
    flipZ = p.flipZ;
    return *this;
  }
};

typedef std::vector<Pyramid> PyramidList;


class WorldInfo {

public:

  WorldInfo();
  ~WorldInfo();

  void setSize ( float x, float y );
  void setGravity ( float g );
  void addWall(float x, float y, float z, float r, float w, float h);
  void addBox(float x, float y, float z, float r, float w, float d, float h, bool drive = false, bool shoot = false);
  void addPyramid(float x, float y, float z, float r, float w, float d, float h, bool drive = false, bool shoot = false, bool flipZ = false);
  void addTeleporter(float x, float y, float z, float r, float w, float d, float h, float b, bool drive = false, bool shoot = false);
  void addBase(float x, float y, float z, float r, float w, float d, float h, bool drive = false, bool shoot = false);
  void addLink(int from, int to);
  void addZone(const CustomZone *zone);
  void addEntryZone( QualifierList &qualifiers, WorldFileLocation *zone );
  float getMaxWorldHeight();
  bool getZonePoint(const std::string &qualifier, float *pt);
  bool getSafetyPoint(const std::string &qualifier, const float *pos, float *pt);
  void finishWorld();
  int packDatabase();
  void *getDatabase() const;
  int getDatabaseSize() const;

private:

  bool rectHitCirc(float dx, float dy, const float *p, float r) const;

public:

  /** check collision between world object and a cylinder.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   * Checking is quite raw
   */
  InBuildingType inBuilding(ObstacleLocation **location,
			    float x, float y, float z,
			    float radius, float height = 0.0f);
  /** check collision between a rectangle and a circle
   */
  bool inRect(const float *p1, float angle, const float *size, float x, float y, float radius) const;

private:

  float size[2];
  float gravity;
  float maxHeight;
  ObstacleLocationList walls;
  ObstacleLocationList boxes;
  ObstacleLocationList bases;
  PyramidList	       pyramids;
  TeleporterList       teleporters;
  EntryZones	       entryZones;
  char *database;
  int databaseSize;
};


#endif /* __WORLDINFO_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
