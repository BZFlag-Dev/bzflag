/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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

#include <vector>
#include <string>
#include <algorithm>
#include <math.h>

#include "global.h"
#include "Address.h"
#include "Protocol.h"

typedef enum { NOT_IN_BUILDING, IN_BASE, IN_BOX, IN_PYRAMID, IN_TELEPORTER } InBuildingType;

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
  float getMaxWorldHeight();
  int packDatabase();
  void *getDatabase() const;
  int getDatabaseSize() const;

 private:

  bool rectHitCirc(float dx, float dy, const float *p, float r) const;

 public:

  struct ObstacleLocation {
    public:
    float pos[3];
    float rotation;
    float size[3];
    bool shootThrough;
    bool driveThrough;
    bool flipZ;
    ObstacleLocation &operator=(const ObstacleLocation &ol)
    {
      memcpy(pos, ol.pos, sizeof(float) * 3);
      rotation = ol.rotation;
      memcpy(size, ol.size, sizeof(float) * 3);
      shootThrough = ol.shootThrough;
      driveThrough = ol.driveThrough;
      flipZ = ol.flipZ;
      return *this;
    }
  };

  struct Teleporter : public ObstacleLocation {
    public:
    float border;
    int to[2];
  };

  InBuildingType inBuilding(ObstacleLocation **location, float x, float y, float z, float radius) const;
  bool inRect(const float *p1, float angle, const float *size, float x, float y, float radius) const;

 private:

  float size[2];
  float gravity;
  int numWalls;
  int numBases;
  int numBoxes;
  int numPyramids;
  int numTeleporters;
  int sizeWalls;
  int sizeBoxes;
  int sizePyramids;
  int sizeTeleporters;
  int sizeBases;
  float maxHeight;
  ObstacleLocation *walls;
  ObstacleLocation *boxes;
  ObstacleLocation *bases;
  ObstacleLocation *pyramids;
  Teleporter *teleporters;
  char *database;
  int databaseSize;
};


#else
class WorldInfo;
#endif /* __WORLDINFO_H__ */
// ex: shiftwidth=2 tabstop=8
