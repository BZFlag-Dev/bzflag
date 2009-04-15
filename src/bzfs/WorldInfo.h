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

#ifndef __WORLDINFO_H__
#define __WORLDINFO_H__

#include "common.h"

// system headers
#include <vector>
#include <string>

// common headers
#include "MapInfo.h"
#include "vectors.h"

// bzfs headers
#include "EntryZones.h"
#include "WorldWeapons.h"
#include "TeamBases.h"
#include "LinkManager.h"

class BzMaterial;
class Obstacle;
class BoxBuilding;
class PyramidBuilding;
class BaseBuilding;
class Teleporter;
class WallObstacle;
class MeshObstacle;
class ArcObstacle;
class ConeObstacle;
class SphereObstacle;

class WorldFileLocation;
class CustomZone;

class FlagType;
class FlagInfo;
class PlayerInfo;

typedef enum {
  NOT_IN_BUILDING,
  IN_BASE,
  IN_BOX_NOTDRIVETHROUGH,
  IN_BOX_DRIVETHROUGH,
  IN_MESH,
  IN_MESHFACE,
  IN_PYRAMID,
  IN_TELEPORTER
} InBuildingType;


class WorldInfo {

public:

  WorldInfo();
  ~WorldInfo();

  void setSize(float x, float y );
  void getSize(float& x, float& y ) { x = size.x; y = size.y; }

  void setGravity ( float g );

  void addWall(float x, float y, float z, float r, float w, float h);
  void addLink(int from, int to);
  void addLink(const std::string& from, const std::string& to);

  void addZone(const CustomZone *zone);
  void addWeapon(const FlagType *type, const fvec3& origin,
                 float direction, float tilt, TeamColor teamColor,
                 float initdelay, const std::vector<float> &delay,
                 TimeKeeper &sync);
  void addWaterLevel (float level, const BzMaterial* matref);

  // for WorldGenerators
  void addBox(float x, float y, float z, float r,
              float w, float d, float h,
              bool drive = false, bool shoot = false, bool rico = false);
  void addPyramid(float x, float y, float z, float r,
                  float w, float d, float h, bool flipZ = false,
                  bool drive = false, bool shoot = false, bool rico = false);
  void addTeleporter(float x, float y, float z, float r,
	             float w, float d, float h, float b, bool horizontal = false,
                     bool drive = false, bool shoot = false, bool rico = false);
  void addBase(const fvec3& pos, float r, const fvec3& size, int color,
               bool drive = false, bool shoot = false, bool rico = false);

  void setMapInfo(const std::vector<std::string>& lines);

  float getWaterLevel() const;
  float getMaxWorldHeight() const;

  bool getFlagDropPoint(const FlagInfo* fi, const fvec3& pos, fvec3& pt) const;
  bool getFlagSpawnPoint(const FlagInfo* fi, fvec3& pt) const;
  bool getPlayerSpawnPoint(const PlayerInfo* pi, fvec3& pt) const;

  void *getDatabase() const;
  int getDatabaseSize() const;
  int getUncompressedSize() const;

  WorldWeapons& getWorldWeapons();
  EntryZones& getEntryZones();

  void finishWorld();
  int packDatabase();

  bool isFinished() { return finished;}

  const Obstacle* hitBuilding(const fvec3& oldPos, float oldAngle,
                              const fvec3& pos, float angle,
                              float tankWidth, float tankBreadth,
                              float tankHeight, bool directional,
                              bool checkwalls = true) const;
private:

  bool finished;

  void loadCollisionManager();
  InBuildingType classifyHit (const Obstacle* obstacle) const;
  void makeWaterMaterial();

public:

  /** check collision between world object and a cylinder.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   * Checking is quite raw. Does not use the CollisionManager and
   * can therefore be used before it has been setup.
   */
  InBuildingType inCylinderNoOctree(Obstacle **location,
				    float x, float y, float z,
				    float r, float height) const;

  /** check collision between world object and a cylinder.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   * Checking is quite raw
   */
  InBuildingType cylinderInBuilding(const Obstacle **obstacle,
				    float x, float y, float z,
				    float radius, float height = 0.0f) const;

  /** check collision between world object and a cylinder.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   * Checking is quite raw
   */
  InBuildingType cylinderInBuilding(const Obstacle **obstacle,
				    const fvec3& pos,
				    float radius, float height = 0.0f) const;

  /** check collision between world object and a Z-axis aligned box.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   */
  InBuildingType boxInBuilding(const Obstacle **obstacle,
			       const fvec3& pos, float angle,
			       float width, float breadth, float height) const;


  /** see if the CollisionManager's view of the world size
   * matches that of BZDB. if not, reload the CollisionManager
   */
  void checkCollisionManager();

  const MapInfo&     getMapInfo()     const { return mapInfo; }
  const LinkManager& getLinkManager() const { return links; }

private:

  fvec2 size;
  float gravity;
  float maxHeight;
  float waterLevel;
  const BzMaterial* waterMatRef;

  EntryZones entryZones;
  LinkManager links;
  WorldWeapons worldWeapons;

  char *database;
  int databaseSize;
  int uncompressedSize;

  MapInfo mapInfo;
};


#endif /* __WORLDINFO_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
