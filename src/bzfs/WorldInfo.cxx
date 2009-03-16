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

/* interface header */
#include "WorldInfo.h"

/* system headers */
#include <ctype.h>

/* common implementation headers */
#include "global.h"
#include "Pack.h"
#include "Protocol.h"
#include "Extents.h"
#include "Intersect.h"
#include "CollisionManager.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "BzMaterial.h"
#include "PhysicsDriver.h"
#include "WorldText.h"
#include "MeshTransform.h"
#include "MeshDrawInfo.h"
#include "TimeKeeper.h"
#include "BzDocket.h"

/* obstacle implementation headers */
#include "ObstacleMgr.h"
#include "Obstacle.h"
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
#include "ServerIntangibilityManager.h"
#include "CollisionManager.h"

/* local implementation headers */
#include "bzfs.h"
#include "FlagInfo.h"
#include "PlayerInfo.h"
#include "CustomZone.h"

/* compression library header */
#include "zlib.h"


WorldInfo::WorldInfo() :
  maxHeight(0.0f),
  database(NULL)
{
  size[0] = 400.0f;
  size[1] = 400.0f;
  gravity = -9.81f;
  waterLevel = -1.0f;
  waterMatRef = NULL;
	finished = false;
}

WorldInfo::~WorldInfo()
{
  delete[] database;
  database = NULL;
  databaseSize = 0;
  uncompressedSize = 0;
  links.clear();
  OBSTACLEMGR.clear();
	finished = false;
}


void WorldInfo::addWall(float x, float y, float z,
                        float r, float w, float h)
{
  const float pos[3] = {x, y, z};
  WallObstacle* wall = new WallObstacle(pos, r, w, h, false);
  OBSTACLEMGR.addWorldObstacle(wall);
}


void WorldInfo::addLink(int src, int dst)
{
  links.addLink(src, dst);
  return;
}

void WorldInfo::addLink(const std::string& src, const std::string& dst)
{
  links.addLink(src, dst);
  return;
}


void WorldInfo::addZone(const CustomZone *zone)
{
  entryZones.addZone( zone );
}

void WorldInfo::addWeapon(const FlagType *type, const float *origin,
			  float direction, float tilt, TeamColor teamColor,
			  float initdelay, const std::vector<float> &delay,
			  TimeKeeper &sync)
{
  worldWeapons.add(type, origin, direction, tilt,
		   teamColor, initdelay, delay, sync);
}

void WorldInfo::addWaterLevel (float level, const BzMaterial* matref)
{
  waterLevel = level;
  waterMatRef = matref;
}

void WorldInfo::addBox(float x, float y, float z, float r,
		       float w, float d, float h,
		       bool drive, bool shoot, bool rico)
{
  const float pos[3] = {x, y, z};
  BoxBuilding* box = new BoxBuilding(pos, r, w, d, h,
                                     drive, shoot, rico, false);
  OBSTACLEMGR.addWorldObstacle(box);
}

void WorldInfo::addPyramid(float x, float y, float z, float r,
			   float w, float d, float h, bool flipZ,
			   bool drive, bool shoot, bool rico)
{
  const float pos[3] = {x, y, z};
  PyramidBuilding* pyr = new PyramidBuilding(pos, r, w, d, h,
                                             drive, shoot, rico);
  if (flipZ) {
    pyr->setZFlip();
  }
  OBSTACLEMGR.addWorldObstacle(pyr);
}

void WorldInfo::addTeleporter(float x, float y, float z, float r,
			      float w, float d, float h, float b,
			      bool horizontal,
			      bool drive, bool shoot, bool rico)
{
  const float pos[3] = {x, y, z};
  Teleporter* tele = new Teleporter(pos, r, w, d, h, b,
                                    horizontal, drive, shoot, rico);
  OBSTACLEMGR.addWorldObstacle(tele);
}

void WorldInfo::addBase(const float pos[3], float r,
			const float _size[3], int color,
			bool /* drive */, bool /* shoot */, bool rico)
{
  BaseBuilding* base = new BaseBuilding(pos, r, _size, color, rico);
  OBSTACLEMGR.addWorldObstacle(base);
}


void WorldInfo::setMapInfo(const std::vector<std::string>& lines)
{
  mapInfo.setLines(lines);
}


void WorldInfo::makeWaterMaterial()
{
  // the texture matrix
  TextureMatrix* texmat = new TextureMatrix;
  texmat->setName("WaterMaterial");
  texmat->setDynamicShift(0.05f, 0.0f);
  texmat->finalize();
  int texmatIndex = TEXMATRIXMGR.addMatrix(texmat);

  // the material
  BzMaterial material;
  const float diffuse[4] = {0.65f, 1.0f, 0.5f, 0.9f};
  material.reset();
  material.setName("WaterMaterial");
  material.setTexture("water");
  material.setTextureMatrix(texmatIndex); // generate a default later
  material.setDiffuse(diffuse);
  material.setUseTextureAlpha(true); // make sure that alpha is enabled
  material.setUseColorOnTexture(false); // only use the color as a backup
  material.setUseSphereMap(false);
  material.setNoRadar(true);
  material.setNoShadow(true);
  waterMatRef = MATERIALMGR.addMaterial(&material);

  return;
}

float WorldInfo::getWaterLevel() const
{
  return waterLevel;
}

float WorldInfo::getMaxWorldHeight() const
{
  return maxHeight;
}

WorldWeapons& WorldInfo::getWorldWeapons()
{
  return worldWeapons;
}

EntryZones& WorldInfo::getEntryZones()
{
  return entryZones;
}


void		    WorldInfo::loadCollisionManager()
{
  COLLISIONMGR.load();
  return;
}

void		    WorldInfo::checkCollisionManager()
{
  if (COLLISIONMGR.needReload()) {
    // reload the collision grid
    COLLISIONMGR.load();
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

bool WorldInfo::inRect(const float *p1, float angle, const float *_size,
		       float x, float y, float r) const
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
  return rectHitCirc(_size[0], _size[1], pb, r);
}


InBuildingType WorldInfo::inCylinderNoOctree(Obstacle **location,
					     float x, float y, float z,
					     float radius, float height) const
{
  if (height < Epsilon) {
    height = Epsilon;
  }

  float pos[3] = {x, y, z};

  for (int type = 0; type < ObstacleTypeCount; type++) {
    const ObstacleList& list = OBSTACLEMGR.getWorld()->getList(type);
    for (unsigned int i = 0; i < list.size(); i++) {
      Obstacle* obs = list[i];
      if (obs->inCylinder(pos, radius, height)) {
	if (location != NULL) {
	  *location = obs;
	}
	return classifyHit(obs);
      }
    }
  }

  if (location != NULL) {
    *location = (Obstacle *)NULL;
  }

  return NOT_IN_BUILDING;
}


InBuildingType WorldInfo::cylinderInBuilding(const Obstacle **location,
					     const float* pos, float radius,
					     float height) const
{
  if (height < Epsilon) {
    height = Epsilon;
  }

  *location = NULL;

  // check everything but walls
  const ObsList* olist = COLLISIONMGR.cylinderTest (pos, radius, height);
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->inCylinder(pos, radius, height)) {
      *location = obs;
      break;
    }
  }

  return classifyHit (*location);
}


InBuildingType WorldInfo::cylinderInBuilding(const Obstacle **location,
					     float x, float y, float z, float radius,
					     float height) const
{
  const float pos[3] = {x, y, z};
  return cylinderInBuilding (location, pos, radius, height);
}


InBuildingType WorldInfo::boxInBuilding(const Obstacle **location,
					const float* pos, float angle,
					float width, float breadth, float height) const
{
  if (height < Epsilon) {
    height = Epsilon;
  }

  *location = NULL;

  // check everything but walls
  const ObsList* olist =
    COLLISIONMGR.boxTest (pos, angle, width, breadth, height);
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->inBox(pos, angle, width, breadth, height)) {
      *location = obs;
      break;
    }
  }

  return classifyHit (*location);
}


InBuildingType WorldInfo::classifyHit (const Obstacle* obstacle) const
{
  if (obstacle == NULL) {
    return NOT_IN_BUILDING;
  } else if (obstacle->getType() == BoxBuilding::getClassName()) {
    if (ServerIntangibilityManager::instance().getWorldObjectTangibility(obstacle->getGUID()) != 0) {
      return IN_BOX_DRIVETHROUGH;
    } else {
      return IN_BOX_NOTDRIVETHROUGH;
    }
  } else if (obstacle->getType() == PyramidBuilding::getClassName()) {
    return IN_PYRAMID;
  } else if (obstacle->getType() == TetraBuilding::getClassName()) {
    return IN_TETRA;
  } else if (obstacle->getType() == MeshObstacle::getClassName()) {
    return IN_MESH;
  } else if (obstacle->getType() == MeshFace::getClassName()) {
    return IN_MESHFACE;
  } else if (obstacle->getType() == BaseBuilding::getClassName()) {
    return IN_BASE;
  } else if (obstacle->getType() == Teleporter::getClassName()) {
    return IN_TELEPORTER;
  } else {
    // FIXME - choke here?
    printf ("*** Unknown obstacle type in WorldInfo::classifyHit()\n");
    return IN_BASE;
  }
}


bool WorldInfo::getFlagDropPoint(const FlagInfo* fi, const float* pos,
				float* pt) const
{
  FlagType* flagType = fi->flag.type;
  const int flagTeam = (int)flagType->flagTeam;
  const bool isTeamFlag = (flagTeam != NoTeam);

  if (isTeamFlag) {
    const std::string& safetyQual =
      CustomZone::getFlagSafetyQualifier(flagTeam);
    if (entryZones.getClosePoint(safetyQual, pos, pt)) {
      return true;
    }
  } else {
    const std::string& idQual =
      CustomZone::getFlagIdQualifier(fi->getIndex());
    if (entryZones.getClosePoint(idQual, pos, pt)) {
      return true;
    }
    const std::string& typeQual =
      CustomZone::getFlagTypeQualifier(flagType);
    if (entryZones.getClosePoint(typeQual, pos, pt)) {
      return true;
    }
  }
  return false;
}


bool WorldInfo::getFlagSpawnPoint(const FlagInfo* fi, float* pt) const
{
  FlagType* flagType = fi->flag.type;
  const int flagTeam = (int)flagType->flagTeam;
  const bool isTeamFlag = (flagTeam != NoTeam);

  const std::string& idQual =
    CustomZone::getFlagIdQualifier(fi->getIndex());
  if (entryZones.getRandomPoint(idQual, pt)) {
    return true;
  }

  if (!isTeamFlag) {
    const std::string& typeQual =
      CustomZone::getFlagTypeQualifier(flagType);
    if (entryZones.getRandomPoint(typeQual, pt)) {
      return true;
    }
  }
  return false;
}


bool WorldInfo::getPlayerSpawnPoint(const PlayerInfo* pi, float* pt) const
{
  const std::string& teamQual =
    CustomZone::getPlayerTeamQualifier((int)pi->getTeam());
  if (entryZones.getRandomPoint(teamQual, pt)) {
    return true;
  }
  return false;
}


void WorldInfo::finishWorld()
{
  entryZones.calculateQualifierLists();

  loadCollisionManager();

  links.doLinking();

  maxHeight = COLLISIONMGR.getWorldExtents().maxs[2];
  const float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
  if (maxHeight < wallHeight) {
    maxHeight = wallHeight;
  }
  if (maxHeight < 0.0f) {
    maxHeight = 0.0f;
  }

  finished = true;

  return;
}


int WorldInfo::packDatabase()
{
  // deallocate any prior database
  if (database) {
    delete[] database;
    databaseSize = 0;
    uncompressedSize = 0;
  }

  BzDocket luaWorld("LuaWorld");
  luaWorld.addDir(clOptions->luaWorldDir, "");

  // make default water material. we wait to make the default material
  // to avoid messing up any user indexing. this has to be done before
  // the texture matrices and materials are packed.
  if ((waterLevel >= 0.0f) && (waterMatRef == NULL)) {
    makeWaterMaterial();
  }

  // compute the database size
  databaseSize =
    mapInfo.packSize() +
    DYNCOLORMGR.packSize() + TEXMATRIXMGR.packSize() +
    MATERIALMGR.packSize() + PHYDRVMGR.packSize() +
    TRANSFORMMGR.packSize() + OBSTACLEMGR.packSize() + links.packSize() +
    WORLDTEXTMGR.packSize() + worldWeapons.packSize() + entryZones.packSize() +
    luaWorld.packSize();

  // add water level size
  databaseSize += sizeof(float);
  if (waterLevel >= 0.0f) {
    databaseSize += sizeof(int32_t);
  }

  // allocate the buffer
  database = new char[databaseSize];
  void *databasePtr = database;

  // pack map information
  databasePtr = mapInfo.pack(databasePtr);

  // pack dynamic colors
  databasePtr = DYNCOLORMGR.pack(databasePtr);

  // pack texture matrices
  databasePtr = TEXMATRIXMGR.pack(databasePtr);

  // pack materials
  databasePtr = MATERIALMGR.pack(databasePtr);

  // pack physics drivers
  databasePtr = PHYDRVMGR.pack(databasePtr);

  // pack obstacle transforms
  databasePtr = TRANSFORMMGR.pack(databasePtr);

  // pack obstacles
  databasePtr = OBSTACLEMGR.pack(databasePtr);

  // pack teleporter links
  databasePtr = links.pack(databasePtr);

  // pack world text
  databasePtr = WORLDTEXTMGR.pack(databasePtr);

  // pack water level
  databasePtr = nboPackFloat(databasePtr, waterLevel);
  if (waterLevel >= 0.0f) {
    int matindex = MATERIALMGR.getIndex(waterMatRef);
    databasePtr = nboPackInt(databasePtr, matindex);
  }

  // pack weapons
  databasePtr = worldWeapons.pack(databasePtr);

  // pack entry zones
  databasePtr = entryZones.pack(databasePtr);

  // pack the LuaWorld docket
  databasePtr = luaWorld.pack(databasePtr);


  // compress the map database
  TimeKeeper startTime = TimeKeeper::getCurrent();
  uLong gzDBlen = compressBound(databaseSize);

  char* gzDB = new char[gzDBlen];
  int code = compress2 ((Bytef*)gzDB, &gzDBlen, (Bytef*)database, databaseSize, 9);
  if (code != Z_OK) {
    printf ("Could not create compressed world database: %i\n", code);
    delete[] gzDB;
    delete[] database;
    exit (1);
  }
  TimeKeeper endTime = TimeKeeper::getCurrent();

  // switch to the compressed map database
  uncompressedSize = databaseSize;
  databaseSize = gzDBlen;
  delete[] database;
  database = gzDB;

  logDebugMessage(1,"Map size: uncompressed = %i, compressed = %i\n",
	   uncompressedSize, databaseSize);

  logDebugMessage(3,"Compression: %.3f seconds\n", endTime - startTime);

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

int WorldInfo::getUncompressedSize() const
{
  return uncompressedSize;
}

const Obstacle* WorldInfo::hitBuilding(const float* oldPos, float oldAngle,
				   const float* pos, float angle,
				   float dx, float dy, float dz,
				   bool directional, bool checkWalls) const
{
  // check walls
  if(checkWalls)
  {
    const ObstacleList& walls = OBSTACLEMGR.getWalls();
    for (unsigned int w = 0; w < walls.size(); w++) {
      const WallObstacle* wall = (const WallObstacle*) walls[w];
      if (wall->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz))
	return wall;
    }
  }

  // get the list of potential hits from the collision manager
  const ObsList* olist = COLLISIONMGR.movingBoxTest (oldPos, oldAngle, pos, angle, dx, dy, dz);

  // sort the list by type and height
  qsort (olist->list, olist->count, sizeof(Obstacle*), compareObstacles);

  int i;

  // check non-mesh obstacles
  for (i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    const char* type = obs->getType();
    if ((type == MeshFace::getClassName()) || (type == MeshObstacle::getClassName()))
      break;

    bool driveThru = ServerIntangibilityManager::instance().getWorldObjectTangibility(obs)!=0;

    if ( !driveThru && obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz))
      return obs;
  }

  if (i == olist->count)
    return NULL; // no more obstacles, we are done

  // do some prep work for mesh faces
  int hitCount = 0;
  float vel[3];
  vel[0] = pos[0] - oldPos[0];
  vel[1] = pos[1] - oldPos[1];
  vel[2] = pos[2] - oldPos[2];
  bool goingDown = (vel[2] <= 0.0f);

  // check mesh faces
  for (/* do nothing */; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    const char* type = obs->getType();
    if (type == MeshObstacle::getClassName())
      break;

    const MeshFace* face = (const MeshFace*) obs;

    // first check the face
    // if the face is drive thru, then we don't care about the tangibility of the mesh
    bool driveThru = obs->isDriveThrough() != 0;
    if (!driveThru)
      driveThru = ServerIntangibilityManager::instance().getWorldObjectTangibility(obs)!=0;

    if ( !driveThru && obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz)) {
      const float facePos2 = face->getPosition()[2];
      if (face->isUpPlane() && (!goingDown || (oldPos[2] < (facePos2 - 1.0e-3f))))
	continue;
      else if (face->isDownPlane() && ((oldPos[2] >= facePos2) || goingDown))
	continue;
      else {
	// add the face to the hitlist
	olist->list[hitCount] = (Obstacle*) obs;
	hitCount++;
	// compute its dot product and stick it in the scratchPad
	const float* p = face->getPlane();
	const float dot = (vel[0] * p[0]) + (vel[1] * p[1]) + (vel[2] * p[2]);
	face->scratchPad = dot;
      }
    }
  }
  // sort the list by dot product (this sort will be replaced with a running tab
  qsort (olist->list, hitCount, sizeof(Obstacle*), compareHitNormal);

  // see if there as a valid meshface hit
  if (hitCount > 0) {
    const MeshFace* face = (const MeshFace*) olist->list[0];
    if (face->isUpPlane() || (face->scratchPad < 0.0f) || !directional)
      return face;
  }
  if (i == olist->count)
    return NULL; // no more obstacles, we are done

  // JeffM, I have NO clue why we do this again, we just got done checking all the faces in the thing
  // all this seems to do is screw us up by testing the same thing again with worse paramaters

  // check mesh obstacles
  for (; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    bool driveThru = ServerIntangibilityManager::instance().getWorldObjectTangibility(obs)!=0;

    if (!driveThru && obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz))
      return obs;
  }

  return NULL; // no more obstacles, we are done
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
