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

/* bzfs implementation headers */
#include "EntryZones.h"
#include "WorldWeapons.h"
#include "TeamBases.h"

/* common implementation headers */
#include "BzMaterial.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "TetraBuilding.h"
#include "Teleporter.h"
#include "WallObstacle.h"
#include "MeshObstacle.h"
#include "ArcObstacle.h"
#include "ConeObstacle.h"
#include "SphereObstacle.h"
#include "CollisionManager.h"

class WorldFileLocation;
class CustomZone;

typedef std::vector<std::string> QualifierList;

typedef enum {
  NOT_IN_BUILDING,
  IN_BASE,
  IN_BOX_NOTDRIVETHROUGH,
  IN_BOX_DRIVETHROUGH,
  IN_MESHFACE,
  IN_PYRAMID,
  IN_TETRA,
  IN_TELEPORTER
} InBuildingType;


class WorldInfo {

public:

  WorldInfo();
  ~WorldInfo();

  void setSize ( float x, float y );
  void setGravity ( float g );
  void addWall(float x, float y, float z, float r, float w, float h);
  void addBox(float x, float y, float z, float r, float w, float d, float h, bool drive = false, bool shoot = false);
  void addPyramid(float x, float y, float z, float r, float w, float d, float h, bool drive = false, bool shoot = false, bool flipZ = false);
  void addTeleporter(float x, float y, float z, float r, float w, float d, float h, float b, bool horizontal, bool drive = false, bool shoot = false);
  void addTetra(const float vertices[4][3], const bool visible[4],
                const bool useColor[4], const float colors[4][4],
                const bool useNormals[4], const float normals[4][3][3],
                const bool useTexCoords[4], const float texCoords[4][3][2],
                const int textureMatrices[4], const std::string textures[4],
                bool drive = false, bool shoot = false);
  void addBase(float x, float y, float z, float r, float w, float d, float h,
	       int color, bool drive = false, bool shoot = false);
  void addLink(int from, int to);
  void addMesh(MeshObstacle* mesh);
  void addArc(ArcObstacle* arc);
  void addCone(ConeObstacle* cone);
  void addSphere(SphereObstacle* sphere);
  void addZone(const CustomZone *zone);
  void addEntryZone( QualifierList &qualifiers, WorldFileLocation *zone );
  void addWeapon(const FlagType *type, const float *origin, float direction,
                 float initdelay, const std::vector<float> &delay, TimeKeeper &sync);
  void addWaterLevel (float level, const BzMaterial* matref);
  float getWaterLevel() const;
  float getMaxWorldHeight();
  bool getZonePoint(const std::string &qualifier, float *pt);
  bool getSafetyPoint(const std::string &qualifier, const float *pos, float *pt);
  void finishWorld();
  int packDatabase(const BasesList* baseList);
  void *getDatabase() const;
  int getDatabaseSize() const;
  int getUncompressedSize() const;
  WorldWeapons& getWorldWeapons();

private:

  void setTeleporterTarget(int src, int tgt);
  bool rectHitCirc(float dx, float dy, const float *p, float r) const;
  void loadCollisionManager();
  InBuildingType classifyHit (const Obstacle* obstacle);
  void makeWaterMaterial();

public:

  /** check collision between world object and a cylinder.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   * Checking is quite raw. Does not use the CollisionManager and
   * can therefore be used before it has been setup.
   */
  InBuildingType inCylinderNoOctree(Obstacle **location,
                                               float x, float y, float z, float r,
                                               float height);

  /** check collision between world object and a cylinder.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   * Checking is quite raw
   */
  InBuildingType cylinderInBuilding(const Obstacle **obstacle,
			            float x, float y, float z,
			            float radius, float height = 0.0f);

  /** check collision between world object and a cylinder.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   * Checking is quite raw
   */
  InBuildingType cylinderInBuilding(const Obstacle **obstacle,
			            const float* pos,
			            float radius, float height = 0.0f);

  /** check collision between world object and a Z-axis aligned box.
   * return value is kind of collision.
   * location will return a pointer to the world colliding object
   */
  InBuildingType boxInBuilding(const Obstacle **obstacle,
			       const float* pos, float angle,
			       float width, float breadth, float height);


  /** see if the CollisionManager's view of the world size
   * matches that of BZDB. if not, reload the CollisionManager
   */
  void checkCollisionManager();

  bool inRect(const float *p1, float angle, const float *size,
                         float x, float y, float r) const;

private:

  float size[2];
  float gravity;
  float maxHeight;
  float waterLevel;
  const BzMaterial* waterMatRef;

  std::vector<WallObstacle*> 	walls;
  std::vector<MeshObstacle*> 	meshes;
  std::vector<ArcObstacle*> 	arcs;
  std::vector<ConeObstacle*> 	cones;
  std::vector<SphereObstacle*> 	spheres;
  std::vector<TetraBuilding*> 	tetras;
  std::vector<BoxBuilding*>     boxes;
  std::vector<BaseBuilding*>	bases;
  std::vector<PyramidBuilding*> pyramids;
  std::vector<Teleporter*> 	teleporters;

  EntryZones	       entryZones;
  WorldWeapons         worldWeapons;
  std::vector<int> teleportTargets;

  CollisionManager collisionManager;

  char *database;
  int databaseSize;
  int uncompressedSize;
};


#endif /* __WORLDINFO_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
