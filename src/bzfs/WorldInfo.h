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
#include "WorldWeapons.h"

#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "Teleporter.h"
#include "WallObstacle.h"
#include "CollisionManager.h"

class WorldFileLocation;
class CustomZone;

typedef std::vector<std::string> QualifierList;

typedef enum { NOT_IN_BUILDING, IN_BASE, IN_BOX_NOTDRIVETHROUGH, IN_BOX_DRIVETHROUGH, IN_PYRAMID, IN_TELEPORTER } InBuildingType;


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
  void addWeapon(const FlagType *type, const float *origin, float direction,
                 float initdelay, const std::vector<float> &delay, TimeKeeper &sync);
  float getMaxWorldHeight();
  bool getZonePoint(const std::string &qualifier, float *pt);
  bool getSafetyPoint(const std::string &qualifier, const float *pos, float *pt);
  void finishWorld();
  int packDatabase();
  void *getDatabase() const;
  int getDatabaseSize() const;
  WorldWeapons& getWorldWeapons();

private:

  void setTeleporterTarget(int src, int tgt);
  bool rectHitCirc(float dx, float dy, const float *p, float r) const;
  void loadCollisionManager();
  InBuildingType classifyHit (const Obstacle* obstacle);

public:

  /** check collision between world object and a cylinder.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   * Checking is quite raw
   */
  InBuildingType cylinderInBuilding(const Obstacle **obstacle,
			            float x, float y, float z,
			            float radius, float height = 0.0f);
  
  /** check collision between world object and a Z-axis aligned box.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   */
  InBuildingType boxInBuilding(const Obstacle **obstacle,
			       float x, float y, float z, float angle,
			       float width, float breadth, float height);
  
  /** see if the CollisionManager's view of the world size
   * matches that of BZDB. if not, reload the CollisionManager
   */
  void checkCollisionManager();

  bool WorldInfo::inRect(const float *p1, float angle, const float *size,
                         float x, float y, float r) const;

private:

  float size[2];
  float gravity;
  float maxHeight;

  std::vector<BoxBuilding> boxes;
  std::vector<BaseBuilding> bases;
  std::vector<PyramidBuilding> pyramids;
  std::vector<WallObstacle> walls;
  std::vector<Teleporter> teleporters;

  EntryZones	       entryZones;
  WorldWeapons         worldWeapons;
  std::vector<int> teleportTargets;
  
  CollisionManager collisionManager;

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
