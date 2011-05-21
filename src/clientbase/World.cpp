/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "World.h"

// system headers
#include <fstream>
#include <time.h>
#include <assert.h>

// common headers
#include "bzfgl.h"
#include "BZDBCache.h"
#include "TextureManager.h"
#include "FileManager.h"
#include "CollisionManager.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "PhysicsDriver.h"
#include "WorldText.h"
#include "FlagSceneNode.h"
#include "ObstacleMgr.h"
#include "LinkManager.h"
#include "MeshDrawInfo.h"
#include "MeshDrawMgr.h"
#include "DirectoryNames.h"
#include "GameTime.h"
#include "WallObstacle.h"
#include "MeshObstacle.h"
#include "LocalPlayer.h"
#include "ClientIntangibilityManager.h"

// local headers
#include "DynamicWorldText.h"
#include "Roster.h"
#include "WorldPlayer.h"


//
// World
//

World*			World::playingField = NULL;
BundleMgr*		World::bundleMgr;
std::string		World::locale("");
int			World::flagTexture(-1);


World::World() :
  gameType(TeamFFA),
  gameOptions(0),
  maxPlayers(0),
  curMaxPlayers(0),
  maxShots(1),
  maxFlags(0),
  botsPerIP(1),
  players(NULL),
  flags(NULL),
  flagNodes(NULL),
  flagWarpNodes(NULL)
{
  worldWeapons = new WorldPlayer();
  waterLevel = -1.0f;
  waterMaterial = NULL;
  drawInfoCount = 0;
  drawInfoArray = NULL;
}


World::~World()
{
  int i;
  freeFlags();
  freeInsideNodes();
  freeMeshDrawMgrs();
  for (i = 0; i < curMaxPlayers; i++) {
    delete players[i];
  }
  delete[] players;
  delete worldWeapons;
  for (i = 0; i < NumTeams; i++) {
    bases[i].clear();
  }

  DYNAMICWORLDTEXT.clear();

  // clear the managers
  DYNCOLORMGR.clear();
  TEXMATRIXMGR.clear();
  MATERIALMGR.clear();
  PHYDRVMGR.clear();
  TRANSFORMMGR.clear();
  OBSTACLEMGR.clear();
  COLLISIONMGR.clear();
  linkManager.clear();

  return;
}


void World::init()
{
  TextureManager &tm = TextureManager::instance();
  flagTexture = tm.getTextureID( "flag" );
}


void World::done()
{
  flagTexture = 0;
}


void World::loadCollisionManager()
{
  COLLISIONMGR.load();
  return;
}


void World::checkCollisionManager()
{
  if (COLLISIONMGR.needReload()) {
    // reload the collision grid
    COLLISIONMGR.load();
  }
  return;
}


void World::setFlagTexture(FlagSceneNode* flag)
{
  flag->setTexture(flagTexture);
}


void World::setWorld(World* _playingField)
{
  playingField = _playingField;
}


TeamColor World::whoseBase(const fvec3& testPos) const
{
  if (gameType != ClassicCTF) {
    return NoTeam;
  }

  const float baseFudge  = 0.1f;
  const float baseFudge2 = baseFudge * 2.0f;

  const fvec3 rayPos(testPos.x, testPos.y, testPos.z + baseFudge);
  const Ray ray(rayPos, fvec3(0.0f, 0.0f, -1.0f));

  const ObsList* oList = COLLISIONMGR.rayTest(&ray, MAXFLOAT);
  const int count = oList->count;

  for (int i = 0; i < count; i++) {
    const Obstacle* obs = oList->list[i];
    const int baseTeam = obs->getBaseTeam();
    if (baseTeam >= 0) {
      const float t = obs->intersect(ray);
      if ((t > 0.0f) && (t < baseFudge2)) {
        return (TeamColor)baseTeam;
      }
    }
  }

  return NoTeam;
}


const Obstacle* World::inBuilding(const fvec3& pos,
                                  float radius, float height) const
{
  // check everything but walls
  const ObsList* oList = COLLISIONMGR.cylinderTest(pos, radius, height);
  for (int i = 0; i < oList->count; i++) {
    const Obstacle* obs = oList->list[i];
    if (obs->inCylinder(pos, radius, height)) {
      return obs;
    }
  }

  return NULL;
}


const Obstacle* World::inBuilding(const fvec3& pos, float angle,
                                  float dx, float dy, float dz) const
{
  // check everything but the walls
  const ObsList* oList = COLLISIONMGR.boxTest(pos, angle, dx, dy, dz);

  for (int i = 0; i < oList->count; i++) {
    const Obstacle* obs = oList->list[i];
    if (obs->inBox(pos, angle, dx, dy, dz)) {
      return obs;
    }
  }

  return NULL;
}


const Obstacle* World::hitBuilding(const fvec3& pos, float angle,
                                   float dx, float dy, float dz) const
{
  // check walls
  const ObstacleList& walls = OBSTACLEMGR.getWalls();
  for (unsigned int w = 0; w < walls.size(); w++) {
    const WallObstacle* wall = (const WallObstacle*) walls[w];
    if (wall->inBox(pos, angle, dx, dy, dz)) {
      return wall;
    }
  }

  // check everything else
  const ObsList* oList = COLLISIONMGR.boxTest(pos, angle, dx, dy, dz);

  ClientIntangibilityManager& CIMgr = ClientIntangibilityManager::instance();

  for (int i = 0; i < oList->count; i++) {
    const Obstacle* obs = oList->list[i];
    if ((CIMgr.getWorldObjectTangibility(obs) == 0) &&
        obs->inBox(pos, angle, dx, dy, dz)) {
      return obs;
    }
  }

  return NULL;
}


const Obstacle* World::hitBuilding(const fvec3& oldPos, float oldAngle,
				   const fvec3& pos, float angle,
				   float dx, float dy, float dz,
				   bool directional) const
{
  ClientIntangibilityManager& CIMgr = ClientIntangibilityManager::instance();

  // check walls
  const ObstacleList& walls = OBSTACLEMGR.getWalls();
  for (unsigned int w = 0; w < walls.size(); w++) {
    const WallObstacle* wall = (const WallObstacle*) walls[w];
    if (wall->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz))
      return wall;
  }

  // get the list of potential hits from the collision manager
  const ObsList* oList = COLLISIONMGR.movingBoxTest(oldPos, oldAngle, pos, angle, dx, dy, dz);

  // sort the list by type and height
  qsort(oList->list, oList->count, sizeof(Obstacle*), compareObstacles);

  int i;

  // check non-mesh obstacles
  for (i = 0; i < oList->count; i++) {
    const Obstacle* obs = oList->list[i];
    const ObstacleType type = obs->getTypeID();
    if ((type == faceType) || (type == meshType)) {
      break;
    }

    const bool driveThru = CIMgr.getWorldObjectTangibility(obs) != 0;

    if (!driveThru && obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz)) {
      return obs;
    }
  }

  if (i == oList->count) {
    return NULL; // no more obstacles, we are done
  }

  // do some prep work for mesh faces
  int hitCount = 0;

  const fvec3 vel = pos - oldPos;
  bool goingDown = (vel.z <= 0.0f);

  // check mesh faces
  for (/* do nothing */; i < oList->count; i++) {
    const Obstacle* obs = oList->list[i];
    if (obs->getTypeID() == meshType) {
      break;
    }

    const MeshFace* face = (const MeshFace*) obs;

    // first check the face
    // if the face is drive thru, then we don't care about the tangibility of the mesh
    bool driveThru = obs->isDriveThrough() != 0;
    if (!driveThru) {
      driveThru = CIMgr.getWorldObjectTangibility(obs) != 0;
    }

    if (!driveThru && obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz)) {
      const float facePos2 = face->getPosition()[2];
      if (face->isUpPlane() && (!goingDown || (oldPos[2] < (facePos2 - 1.0e-3f)))) {
	continue;
      }
      else if (face->isDownPlane() && ((oldPos[2] >= facePos2) || goingDown)) {
	continue;
      }
      else {
	// add the face to the hitlist
	oList->list[hitCount] = (Obstacle*) obs;
	hitCount++;
	// compute its dot product and stick it in the scratchPad
	const fvec4& plane = face->getPlane();
	const float dot = fvec3::dot(vel, plane.xyz());
	face->scratchPad = dot;
      }
    }
  }
  // sort the list by dot product
  qsort(oList->list, hitCount, sizeof(Obstacle*), compareHitNormal);

  // see if there as a valid meshface hit
  if (hitCount > 0) {
    const MeshFace* face = (const MeshFace*) oList->list[0];
    if (face->isUpPlane() || (face->scratchPad < 0.0f) || !directional) {
      return face;
    }
  }

  // check mesh obstacles ('i' is at the beginning of the MeshObstacles)
  for (/*no-op*/; i < oList->count; i++) {
    const Obstacle* obs = oList->list[i];
    bool driveThru = CIMgr.getWorldObjectTangibility(obs) != 0;

    if (!driveThru && obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz)) {
      return obs;
    }
  }

  return NULL; // no more obstacles, we are done
}


const MeshFace* World::crossingTeleporter(const fvec3& pos, float angle,
                                          float dx, float dy, float dz) const
{
  const ObsList* oList = COLLISIONMGR.boxTest(pos, angle, dx, dy, dz);

  const int count = oList->count;
  for (int i = 0; i < count; i++) {
    const Obstacle* obs = oList->list[i];
    if (obs->getTypeID() == faceType) {
      const MeshFace* face = (const MeshFace*)obs;
      if (face->isLinkFace() &&
          face->isCrossing(pos, angle, dx, dy, dz, NULL)) {
        return face;
      }
    }
  }
  return NULL;
}


const MeshFace* World::crossesTeleporter(const fvec3& oldPos,
                                         const fvec3& newPos) const
{
  // check linkSrcs
  Extents exts;
  exts.expandToPoint(oldPos);
  exts.expandToPoint(newPos);
  const ObsList* oList = COLLISIONMGR.axisBoxTest(exts);
  const int count = oList->count;

  for (int i = 0; i < count; i++) {
    const Obstacle* obs = oList->list[i];
    if (obs->getTypeID() == faceType) {
      const MeshFace* face = (const MeshFace*)obs;
      if (face->isLinkSrc() &&
          face->isDriveThrough() &&
          face->hasCrossed(oldPos, newPos)) {
        return face;
      }
    }
  }

  // didn't cross
  return NULL;
}


float World::getProximity(const fvec3& p, float radius) const
{
  const float r = (radius * 1.2f);

  // get maximum over all teleporters
  float bestProximity = 0.0;

  const Extents exts(fvec3(p.x - r, p.y - r, p.z - r),
                     fvec3(p.x + r, p.y + r, p.z + r));
  const ObsList* oList = COLLISIONMGR.axisBoxTest(exts);
  const int count = oList->count;

  for (int i = 0; i < count; i++) {
    const Obstacle* obs = oList->list[i];
    if (obs->getTypeID() == faceType) {
      const MeshFace* face = (const MeshFace*)obs;
      if (face->isLinkSrc() && !face->linkSrcNoGlow() &&
          (face->isDriveThrough() || face->linkSrcTouch())) {
        const float proximity = face->getProximity(p, r);
        if (proximity > bestProximity) {
          bestProximity = proximity;
        }
      }
    }
  }

  return bestProximity;
}


void World::freeFlags()
{
  int i;
  if (flagNodes) {
    for (i = 0; i < maxFlags; i++) {
      delete flagNodes[i];
    }
  }
  if (flagWarpNodes) {
    for (i = 0; i < maxFlags; i++) {
      delete flagWarpNodes[i];
    }
  }
  delete[] flags;
  delete[] flagNodes;
  delete[] flagWarpNodes;
  flags = NULL;
  flagNodes = NULL;
  flagWarpNodes = NULL;
}


void World::makeMeshDrawMgrs()
{
  // make the display list managers for source meshes
  std::vector<MeshObstacle*> sourceMeshes;
  OBSTACLEMGR.getSourceMeshes(sourceMeshes);
  const size_t count = sourceMeshes.size();
  drawInfoArray = new MeshDrawInfo*[count];
  drawInfoCount = 0;
  for (size_t i = 0; i < count; ++i) {
    MeshDrawInfo* di = (MeshDrawInfo*) sourceMeshes[i]->getDrawInfo();
    if ((di != NULL) && !di->isCopy()) {
      MeshDrawMgr* dm = new MeshDrawMgr(di);
      di->setDrawMgr(dm);
      drawInfoArray[drawInfoCount] = di;
      drawInfoCount++;
    }
  }
  return;
}


void World::freeMeshDrawMgrs()
{
  for (int i = 0; i < drawInfoCount; i++) {
    MeshDrawInfo* di = drawInfoArray[i];
    MeshDrawMgr* dm = di->getDrawMgr();
    delete dm;
    di->setDrawMgr(NULL);
  }
  drawInfoCount = 0;
  delete[] drawInfoArray;
  drawInfoArray = NULL;
  return;
}


void World::updateAnimations(float /*dt*/)
{
  const double gameTime = GameTime::getStepTime();
  for (int i = 0; i < drawInfoCount; i++) {
    MeshDrawInfo* di = drawInfoArray[i];
    di->updateAnimation(gameTime);
  }
  return;
}


void World::freeInsideNodes() const
{
  unsigned int i;
  int j;
  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  for (i = 0; i < boxes.size(); i++) {
    Obstacle* obs = boxes[i];
    for (j = 0; j < obs->getInsideSceneNodeCount(); j++) {
      delete obs->getInsideSceneNodeList()[j];
    }
    obs->freeInsideSceneNodeList();
  }
  const ObstacleList& pyramids = OBSTACLEMGR.getPyrs();
  for (i = 0; i < pyramids.size(); i++) {
    Obstacle* obs = pyramids[i];
    for (j = 0; j < obs->getInsideSceneNodeCount(); j++) {
      delete obs->getInsideSceneNodeList()[j];
    }
    obs->freeInsideSceneNodeList();
  }
  const ObstacleList& basesR = OBSTACLEMGR.getBases();
  for (i = 0; i < basesR.size(); i++) {
    Obstacle* obs = basesR[i];
    for (j = 0; j < obs->getInsideSceneNodeCount(); j++) {
      delete obs->getInsideSceneNodeList()[j];
    }
    obs->freeInsideSceneNodeList();
  }
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  for (i = 0; i < meshes.size(); i++) {
    Obstacle* obs = meshes[i];
    for (j = 0; j < obs->getInsideSceneNodeCount(); j++) {
      delete obs->getInsideSceneNodeList()[j];
    }
    obs->freeInsideSceneNodeList();
  }
  return;
}


void World::initFlag(int index)
{
  // set the color
  const fvec4& color = flags[index].type->getColor();
  flagNodes[index]->setColor(color[0], color[1], color[2], 1.0f);

  // set the texture
  int texID = -1;
  const TeamColor flagTeam = flags[index].type->flagTeam;
  if (flagTeam != NoTeam) {
    TextureManager &tm = TextureManager::instance();
    const std::string flagTextureName =
      Team::getImagePrefix(flagTeam) + "flag";
    texID = tm.getTextureID(flagTextureName);
    if (texID >= 0) {
      flagNodes[index]->setTexture(texID);
      flagNodes[index]->setUseColor(false);
    }
  }
  if (texID < 0) {
    flagNodes[index]->setTexture(World::flagTexture);
    flagNodes[index]->setUseColor(true);
  }

  // if coming or going then position warp
  Flag& flag = flags[index];
  if (flag.status == FlagComing || flag.status == FlagGoing) {
    fvec3 pos;
    pos[0] = flag.position[0];
    pos[1] = flag.position[1];
    pos[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	0.25f * BZDBCache::gravity * flag.flightEnd) + flag.position[2];
    flagWarpNodes[index]->move(pos);
    flagWarpNodes[index]->setSizeFraction(0.0f);
  }
}


void World::updateWind(float /*dt*/)
{
  static BZDB_float maxWindSpeed("_maxWindSpeed");
  static BZDB_float minWindSpeed("_minWindSpeed");

  // pretty cheezy, should be fields and such
  const double gt = GameTime::getStepTime();

  const double oneMinuteFactor = (1.0 / (60.0 * (M_PI * 2.0)));
  const float wsf = (float)(0.5 + (0.5 * cos(gt * 15.0f * oneMinuteFactor)));
  const float windSpeed = ((1.0f - wsf) * minWindSpeed) +
			  (wsf * maxWindSpeed);

  const float windAngle = (float)((M_PI * 2.0) *
				  (cos(gt * 3.0f * oneMinuteFactor) +
				   cos(gt * 10.0f * oneMinuteFactor)));

  wind[0] = windSpeed * cosf(windAngle);
  wind[1] = windSpeed * sinf(windAngle);
  wind[2] = 0.0f;
}


void World::updateFlag(int index, float dt)
{
  if (!flagNodes) {
    return;
  }

  Flag& flag = flags[index];
  FlagSceneNode* flagNode = flagNodes[index];
  const fvec4* color = flagNode->getColor();
  float alpha = color->a;

  switch (flag.status) {
    default: {
      // do nothing (don't move cos either it's not moving or we
      // don't know the position to move it to)
      break;
    }
    case FlagInAir: {
      flag.flightTime += dt;
      if (flag.flightTime >= flag.flightEnd) {
	// touchdown
	flag.status = FlagOnGround;
	flag.position[0] = flag.landingPosition[0];
	flag.position[1] = flag.landingPosition[1];
	flag.position[2] = flag.landingPosition[2];
      }
      else {
	// still flying
	float t = flag.flightTime / flag.flightEnd;
	flag.position = ((1.0f - t) * flag.launchPosition) +
	                        (t  * flag.landingPosition);
        flag.position.z += flag.flightTime *
          (flag.initialVelocity + 0.5f * BZDBCache::gravity * flag.flightTime);
      }
      break;
    }
    case FlagComing: {
      flag.flightTime += dt;
      if (flag.flightTime >= flag.flightEnd) {
	// touchdown
	flag.status = FlagOnGround;
	flag.position.z = 0.0f;
	alpha = 1.0f;
      }
      else if (flag.flightTime >= (0.5f * flag.flightEnd)) {
	// falling
	flag.position.z = flag.flightTime * (flag.initialVelocity +
	    0.5f * BZDBCache::gravity * flag.flightTime) + flag.landingPosition.z;
	alpha = 1.0f;
      }
      else {
	// hovering
	flag.position[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	    0.25f * BZDBCache::gravity * flag.flightEnd) + flag.landingPosition[2];

	// flag is fades in during first half of hovering period
	// and is opaque during the second half.  flag warp grows
	// to full size during first half, and shrinks to nothing
	// during second half.
	if (flag.flightTime >= 0.25f * flag.flightEnd) {
	  // second half
	  float t = (flag.flightTime - 0.25f * flag.flightEnd) /
						(0.25f * flag.flightEnd);
	  alpha = 1.0f;
	  flagWarpNodes[index]->setSizeFraction(1.0f - t);
	} else {
	  // first half
	  float t = flag.flightTime / (0.25f * flag.flightEnd);
	  alpha = t;
	  flagWarpNodes[index]->setSizeFraction(t);
	}
      }
      break;
    }
    case FlagGoing: {
      flag.flightTime += dt;
      if (flag.flightTime >= flag.flightEnd) {
	// all gone
	flag.status = FlagNoExist;
      }
      else if (flag.flightTime < (0.5f * flag.flightEnd)) {
	// rising
	flag.position.z = flag.flightTime * (flag.initialVelocity +
	    0.5f * BZDBCache::gravity * flag.flightTime) + flag.landingPosition[2];
	alpha = 1.0f;
      }
      else {
	// hovering
	flag.position.z = 0.5f * flag.flightEnd * (flag.initialVelocity +
	    0.25f * BZDBCache::gravity * flag.flightEnd) + flag.landingPosition[2];

	// flag is opaque during first half of hovering period
	// and fades out during the second half.  flag warp grows
	// to full size during first half, and shrinks to nothing
	// during second half.
	if (flag.flightTime < 0.75f * flag.flightEnd) {
	  // first half
	  float t = (0.75f * flag.flightEnd - flag.flightTime) /
						(0.25f * flag.flightEnd);
	  alpha = 1.0f;
	  flagWarpNodes[index]->setSizeFraction(1.0f - t);
	} else {
	  // second half
	  float t = (flag.flightEnd - flag.flightTime) /
						(0.25f * flag.flightEnd);
	  alpha = t;
	  flagWarpNodes[index]->setSizeFraction(t);
	}
      }
      break;
    }
  }

  // update alpha if changed
  if (alpha != color->a) {
    flagNode->setAlpha(alpha);
  }

  // move flag scene node
  flagNode->move(flags[index].position);

  // setup the flag angle
  if (flag.status != FlagOnTank) {
    flagNode->setWind(wind, dt);
    flagNode->setFlat(false);
  }
  else {
    const Player* flagPlayer = lookupPlayer(flag.owner);
    if (flagPlayer == NULL) {
      flagNode->setWind(wind, dt);
      flagNode->setFlat(false);
    }
    else {
      if (flag.type == Flags::Narrow) {
	flagNode->setAngle(flagPlayer->getAngle());
	flagNode->setFlat(true);
      }
      else {
	fvec3 myWind;
	getWind(myWind, flagPlayer->getPosition());
	const fvec3& vel = flagPlayer->getVelocity();
	myWind.xy() -= vel.xy();
	if (flagPlayer->isFalling()) {
	  myWind.z -= vel.z;
	}
	flagNode->setWind(myWind, dt);
	flagNode->setFlat(false);
      }
    }
  }
}


void World::addFlags(SceneDatabase* scene, bool seerView)
{
  if (!flagNodes) return;
  for (int i = 0; i < maxFlags; i++) {
    // if not showing flags, only allow FlagOnTank through
    if ((flags[i].status != FlagOnTank) && !BZDBCache::displayMainFlags) {
      continue;
    }
    if (flags[i].gfxBlock.blocked()) {
      continue;
    }

    if (flags[i].status == FlagNoExist) continue;
    // skip flag on a tank that isn't alive. also skip
    // Cloaking flag on a tank if we don't have a Seer flag.
    if (flags[i].status == FlagOnTank) {
      if ((flags[i].type == Flags::Cloaking) && !seerView) {
	continue;
      }
      int j;
      for (j = 0; j < curMaxPlayers; j++) {
	if (players[j] && players[j]->getId() == flags[i].owner) {
	  break;
        }
      }

      if ((j < curMaxPlayers) &&
          !(players[j]->getStatus() & PlayerState::Alive)) {
	continue;
      }
    }

    scene->addDynamicNode(flagNodes[i]);

    // add warp if coming/going and hovering
    if ((flags[i].status == FlagComing &&
	flags[i].flightTime < 0.5 * flags[i].flightEnd) ||
	(flags[i].status == FlagGoing &&
	flags[i].flightTime >= 0.5 * flags[i].flightEnd))
      scene->addDynamicNode(flagWarpNodes[i]);
  }
}


static std::string indent = "";


static void writeDefaultOBJMaterials(std::ostream& out)
{
  typedef struct {
    const char* name;
    const char* texture;
    fvec4 color;
  } MatProps;
  const MatProps defaultMats[] = {
    {"ground",         "ground.png",                fvec4(0.5f, 0.5f, 0.5f, 1.0f)},
    {"boxtop",         "roof.png",                  fvec4(1.0f, 1.0f, 0.9f, 1.0f)},
    {"boxwall",        "boxwall.png",               fvec4(1.0f, 0.9f, 0.8f, 1.0f)},
    {"pyrwall",        "pyrwall.png",               fvec4(0.8f, 0.8f, 1.0f, 1.0f)},
    {"telefront",      "telelink.png",              fvec4(1.0f, 0.0f, 0.0f, 0.5f)},
    {"teleback",       "telelink.png",              fvec4(0.0f, 1.0f, 0.0f, 0.5f)},
    {"telerim",        "caution.png",               fvec4(1.0f, 1.0f, 0.0f, 0.5f)},
    {"basetop_team1",  "skins/red/basetop.png",     fvec4(1.0f, 0.8f, 0.8f, 1.0f)},
    {"basewall_team1", "skins/red/basewall.png",    fvec4(1.0f, 0.8f, 0.8f, 1.0f)},
    {"basetop_team2",  "skins/green/basetop.png",   fvec4(0.8f, 1.0f, 0.8f, 1.0f)},
    {"basewall_team2", "skins/green/basewall.png",  fvec4(0.8f, 1.0f, 0.8f, 1.0f)},
    {"basetop_team3",  "skins/blue/basetop.png",    fvec4(0.8f, 0.8f, 1.0f, 1.0f)},
    {"basewall_team3", "skins/blue/basewall.png",   fvec4(0.8f, 0.8f, 1.0f, 1.0f)},
    {"basetop_team4",  "skins/purple/basetop.png",  fvec4(1.0f, 0.8f, 1.0f, 1.0f)},
    {"basewall_team4", "skins/purple/basewall.png", fvec4(1.0f, 0.8f, 1.0f, 1.0f)}
  };
  const int count = sizeof(defaultMats) / sizeof(defaultMats[0]);
  BzMaterial mat;
  for (int i = 0; i < count; i++) {
    const MatProps& mp = defaultMats[i];
    mat.setName(mp.name);
    mat.setTexture(mp.texture);
    mat.setDiffuse(mp.color);
    mat.printMTL(out, "");
  }
  return;
}


static void writeOBJGround(std::ostream& out)
{
  const float ws = BZDBCache::worldSize / 2.0f;
  const float ts = BZDBCache::worldSize / 100.0f;
  out << "o bzground" << std::endl;
  out << "v " << -ws << " " << -ws << " 0" << std::endl;
  out << "v " << +ws << " " << -ws << " 0" << std::endl;
  out << "v " << +ws << " " << +ws << " 0" << std::endl;
  out << "v " << -ws << " " << +ws << " 0" << std::endl;
  out << "vt " << -ts << " " << -ts << std::endl;
  out << "vt " << +ts << " " << -ts << std::endl;
  out << "vt " << +ts << " " << +ts << std::endl;
  out << "vt " << -ts << " " << +ts << std::endl;
  out << "vn 0 0 1" << std::endl;
  out << "usemtl ground" << std::endl;
  out << "f -4/-4/-1 -3/-3/-1 -2/-2/-1 -1/-1/-1" << std::endl;
  out << std::endl;
  return;
}


static void writeBZDBvar(const std::string& name, void *userData)
{
  std::ofstream& out = *((std::ofstream*)userData);
  if ((BZDB.getPermission(name) == StateDatabase::Server)
      && (BZDB.get(name) != BZDB.getDefault(name))) {
    std::string qmark = "";
    if (BZDB.get(name).find(' ') != std::string::npos) {
      qmark = '"';
    }
    std::string set = "  -set ";
    if (BZDB.getDefault(name) == "") {
      set = "  -setforced ";
    }
    out << indent << set << name << " "
		  << qmark << BZDB.get(name) << qmark << std::endl;
  }
  return;
}


bool World::writeWorld(const std::string& filename, std::string& fullname)
{
  const bool saveAsOBJ = BZDB.isTrue("saveAsOBJ");
  if (saveAsOBJ) {
    indent = "# ";
  } else {
    indent = "";
  }

  fullname = getWorldDirName();
  fullname += filename;
  if (saveAsOBJ) {
    if (strstr(fullname.c_str(), ".obj") == NULL) {
      fullname += ".obj";
    }
  } else {
    if (strstr(fullname.c_str(), ".bzw") == NULL) {
      fullname += ".bzw";
    }
  }

  std::ostream *stream = FILEMGR.createDataOutStream(fullname.c_str());
  if (stream == NULL) {
    return false;
  }

  // for notational convenience
  std::ostream& out = *stream;

  time_t nowTime = time(NULL);
  out << "# BZFlag client: saved world on " << ctime(&nowTime) << std::endl;

  // Write the Map Information
  mapInfo.print(out, indent);

  // Write the Server Options
  {
    out << indent << "options" << std::endl;

    // FIXME - would be nice to get a few other thing
    //	 -fb, -sb, rabbit style, a real -mp, etc... (also, flags?)

    if (allowTeamFlags()) {
      out << indent << "  -c" << std::endl;
      out << indent << "  -mp 2,";
      for (int t = RedTeam; t <= PurpleTeam; t++) {
        out << ((getBase(t, 0) == NULL) ? "0," : "2,");
      }
      out << "2" << std::endl;
    }
    if (allowRabbit())
      out << indent << "  -rabbit" << std::endl;
    if (allowJumping())
      out << indent << "  -j" << std::endl;
    if (allShotsRicochet())
      out << indent << "  +r" << std::endl;
    if (allowHandicap())
      out << indent << "  -handicap" << std::endl;
    if (allowAntidote()) {
      out << indent << "  -sa" << std::endl;
      out << indent << "  -st " << getFlagShakeTimeout() << std::endl;
      out << indent << "  -sw " << getFlagShakeWins() << std::endl;
    }

    out << indent << "  -ms " << getMaxShots() << std::endl;

    // Write BZDB server variables that aren't defaults
    BZDB.iterate(writeBZDBvar, &out);

    out << indent << "end" << std::endl << std::endl;
  }

  // Write World object
  {
    float worldSize = BZDBCache::worldSize;
    float flagHeight = BZDB.eval(BZDBNAMES.FLAGHEIGHT);
    if ((worldSize != atof(BZDB.getDefault(BZDBNAMES.WORLDSIZE).c_str())) ||
        (flagHeight != atof(BZDB.getDefault(BZDBNAMES.FLAGHEIGHT).c_str()))) {
      out << indent << "world" << std::endl;
      if (worldSize != atof(BZDB.getDefault(BZDBNAMES.WORLDSIZE).c_str()))
	out << indent << "  size " << worldSize / 2.0f << std::endl;
      if (flagHeight != atof(BZDB.getDefault(BZDBNAMES.FLAGHEIGHT).c_str()))
	out << indent << "  flagHeight " << flagHeight << std::endl;

      if (!OBSTACLEMGR.getWalls().size())
	out << indent << "noWalls" << std::endl;

      out << indent << "end" << std::endl << std::endl;
    }
  }

  // Write dynamic colors
  DYNCOLORMGR.print(out, indent);

  // Write texture matrices
  TEXMATRIXMGR.print(out, indent);

  // Write materials
  if (!saveAsOBJ) {
    MATERIALMGR.print(out, indent);
  }
  else {
    const std::string mtlname = filename + ".mtl";
    const std::string mtlfile = getWorldDirName() + mtlname;
    std::ostream* mtlStream = FILEMGR.createDataOutStream(mtlfile.c_str());
    if (mtlStream != NULL) {
      out << "mtllib " << mtlname << std::endl; // index the mtl file
      out << std::endl;
      *mtlStream << "# BZFlag client: saved world on " << ctime(&nowTime);
      *mtlStream << std::endl;
      writeDefaultOBJMaterials(*mtlStream);
      MATERIALMGR.printMTL(*mtlStream, "");
      delete mtlStream;
    }
  }

  // Write physics drivers
  PHYDRVMGR.print(out, indent);

  // Write obstacle transforms
  TRANSFORMMGR.print(out, indent);

  // Write water level
  if (waterLevel >= 0.0f) {
    out << indent << "waterLevel" << std::endl;
    out << indent << "  height " << waterLevel << std::endl;
    out << indent << "  matref ";
    MATERIALMGR.printReference(out, waterMaterial);
    out << std::endl;
    out << indent << "end" << std::endl << std::endl;
  }

  // Write the world obstacles
  if (saveAsOBJ) {
    writeOBJGround(out);
  }
  OBSTACLEMGR.print(out, indent);

  // Write weapons
  for (std::vector<Weapon>::iterator it = weapons.begin();
       it != weapons.end(); ++it) {
    Weapon weapon = *it;
    out << indent << "weapon" << std::endl;
    if (weapon.type != Flags::Null) {
      out << indent << "  type " << weapon.type->flagAbbv << std::endl;
    }
    out << indent << "  position " << weapon.pos[0] << " " << weapon.pos[1] << " "
	<< weapon.pos[2] << std::endl;
    out << indent << "  rotation " << ((weapon.dir * 180.0) / M_PI) << std::endl;
    out << indent << "  initdelay " << weapon.initDelay << std::endl;
    if (weapon.delay.size() > 0) {
      out << indent << "  delay";
      for (std::vector<float>::iterator dit = weapon.delay.begin();
	   dit != weapon.delay.end(); ++dit) {
	out << " " << (float)*dit;
      }
      out << std::endl;
    }
    out << indent << "end" << std::endl << std::endl;
  }

  // Write entry zones
  for (std::vector<EntryZone>::iterator it = entryZones.begin();
       it != entryZones.end(); ++it) {
    EntryZone zone = *it;
    out << indent << "zone" << std::endl;
    out << indent << "  position " << zone.pos[0] << " " << zone.pos[1] << " "
	<< zone.pos[2] << std::endl;
    out << indent << "  size " << zone.size[0] << " " << zone.size[1] << " "
	<< zone.size[2] << std::endl;
    out << indent << "  rotation " << ((zone.rot * 180.0) / M_PI) << std::endl;
    if (zone.flags.size() > 0) {
      out << indent << "  flag";
      std::vector<FlagType*>::iterator fit;
      for (fit = zone.flags.begin(); fit != zone.flags.end(); ++fit) {
	out << " " << (*fit)->flagAbbv;
      }
      out << std::endl;
    }
    if (zone.teams.size() > 0) {
      out << indent << "  team";
      std::vector<TeamColor>::iterator tit;
      for (tit = zone.teams.begin(); tit != zone.teams.end(); ++tit) {
	out << " " << (*tit);
      }
      out << std::endl;
    }
    if (zone.safety.size() > 0) {
      out << indent << "  safety";
      std::vector<TeamColor>::iterator sit;
      for (sit = zone.safety.begin(); sit != zone.safety.end(); ++sit) {
	out << " " << (*sit);
      }
      out << std::endl;
    }
    out << indent << "end" << std::endl << std::endl;
  }

  delete stream;

  return true;
}


static void drawLines(int count, const fvec3* vertices, int color)
{
  const fvec4 colors[] = {
    fvec4(0.25f, 0.25f, 0.25f, 0.8f), // gray    (branch node)
    fvec4(0.25f, 0.25f, 0.0f,  0.8f), // yellow  (regular)
    fvec4(0.0f,  0.25f, 0.25f, 0.8f), // cyan    (meshed)
    fvec4(0.25f, 0.0f,  0.25f, 0.8f)  // purple  (meshed + regular)
  };
  const int colorCount = sizeof(colors) / sizeof(colors[0]);

  if (color < 0) {
    color = 0;
  } else if (color >= colorCount) {
    color = colorCount - 1;
  }
  glColor4fv(colors[color]);

  glBegin(GL_LINE_STRIP); {
    for (int i = 0; i < count; i++) {
      glVertex3fv(vertices[i]);
    }
  } glEnd();

  return;
}


void World::drawCollisionGrid() const
{
  GLboolean usingTextures;

  glGetBooleanv(GL_TEXTURE_2D, &usingTextures);
  glDisable(GL_TEXTURE_2D);

  COLLISIONMGR.draw(&drawLines);

  if (usingTextures) {
    glEnable(GL_TEXTURE_2D);
  }

  return;
}


RemotePlayer* World::getCurrentRabbit() const
{
  if (players == NULL) {
    return NULL;
  }
  for (int i = 0; i < curMaxPlayers; i++) {
    RemotePlayer* p = players[i];
    if (p && p->isAlive() && (p->getTeam() == RabbitTeam)) {
      return p;
    }
  }
  return NULL;
}


void World::setPlayersSize(int _playersSize)
{
  playersSize = _playersSize;
  players     = new RemotePlayer*[playersSize];
  for (int i = 0; i < maxPlayers; i++) {
    players[i] = NULL;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
