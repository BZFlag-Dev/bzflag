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

// bzflag common header
#include "common.h"

// system headers
#include <stdio.h>
#include <string>
#include <vector>

// local implemenation headers
#include "SceneBuilder.h"
#include "SceneRenderer.h"
#include "SceneDatabase.h"
#include "ZSceneDatabase.h"
#include "BSPSceneDatabase.h"
#include "World.h"

// scene node implemenation headers
#include "WallSceneNode.h"
#include "MeshPolySceneNode.h"
#include "TankSceneNode.h"
#include "BoxSceneNodeGenerator.h"
#include "WallSceneNodeGenerator.h"
#include "MeshSceneNodeGenerator.h"
#include "BaseSceneNodeGenerator.h"
#include "PyramidSceneNodeGenerator.h"
#include "ObstacleSceneNodeGenerator.h"
#include "TeleporterSceneNodeGenerator.h"

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "TextureManager.h"
#include "BzMaterial.h"
#include "DynamicColor.h"


//
// SceneDatabaseBuilder
//

static const GLfloat	black[3] = { 0.0f, 0.0f, 0.0f };

const GLfloat		SceneDatabaseBuilder::wallColors[4][4] = {
				{ 0.5f, 0.5f, 0.5f, 1.0f },
				{ 0.4f, 0.4f, 0.4f, 1.0f },
				{ 0.5f, 0.5f, 0.5f, 1.0f },
				{ 0.6f, 0.6f, 0.6f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::wallModulateColors[4][4] = {
				{ 0.5f, 0.5f, 0.5f, 1.0f },
				{ 0.4f, 0.4f, 0.4f, 1.0f },
				{ 0.5f, 0.5f, 0.5f, 1.0f },
				{ 0.6f, 0.6f, 0.6f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::wallLightedColors[1][4] = {
				{ 0.5f, 0.5f, 0.5f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::wallLightedModulateColors[1][4] = {
				{ 0.5f, 0.5f, 0.5f, 1.0f }
			};

const GLfloat		SceneDatabaseBuilder::boxColors[6][4] = {
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.63f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.375f, 0.375f, 1.0f },
				{ 0.875f, 0.5f, 0.5f, 1.0f },
				{ 0.275f, 0.2f, 0.2f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::boxModulateColors[6][4] = {
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.63f, 0.63f, 0.63f, 1.0f },
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.69f, 0.69f, 0.69f, 1.0f },
				{ 0.875f, 0.875f, 0.875f, 1.0f },
				{ 0.375f, 0.375f, 0.375f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::boxLightedColors[6][4] = {
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.875f, 0.5f, 0.5f, 1.0f },
				{ 0.875f, 0.5f, 0.5f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::boxLightedModulateColors[6][4] = {
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.875f, 0.875f, 0.875f, 1.0f },
				{ 0.875f, 0.875f, 0.875f, 1.0f }
			};

const GLfloat		SceneDatabaseBuilder::pyramidColors[5][4] = {
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.13f, 0.13f, 0.51f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.375f, 0.375f, 0.75f, 1.0f },
				{ 0.175f, 0.175f, 0.35f, 1.0f }
			};

const GLfloat		SceneDatabaseBuilder::pyramidModulateColors[5][4] = {
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.13f, 0.13f, 0.51f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.375f, 0.375f, 0.75f, 1.0f },
				{ 0.175f, 0.175f, 0.35f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::pyramidLightedColors[5][4] = {
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::pyramidLightedModulateColors[5][4] = {
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f },
				{ 0.25f, 0.25f, 0.63f, 1.0f }
			};

const GLfloat		SceneDatabaseBuilder::teleporterColors[3][4] = {
				{ 1.0f, 0.875f, 0.0f, 1.0f },
				{ 0.9f, 0.8f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f, 0.5f }
			};
const GLfloat		SceneDatabaseBuilder::teleporterModulateColors[3][4] = {
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ 0.9f, 0.9f, 0.9f, 1.0f },
				{ 0.0f, 0.0f, 0.0f, 0.5f }
			};
const GLfloat		SceneDatabaseBuilder::teleporterLightedColors[3][4] = {
				{ 1.0f, 0.875f, 0.0f, 1.0f },
				{ 1.0f, 0.875f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f, 0.5f }
			};
const GLfloat		SceneDatabaseBuilder::teleporterLightedModulateColors[3][4] = {
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f, 0.5f }
			};

SceneDatabaseBuilder::SceneDatabaseBuilder(const SceneRenderer* _renderer) :
				renderer(_renderer),
				wallMaterial(black, black, 0.0f),
				boxMaterial(black, black, 0.0f),
				pyramidMaterial(black, black, 0.0f),
				teleporterMaterial(black, black, 0.0f)
{
  // FIXME -- should get texture heights from resources

  TextureManager &tm = TextureManager::instance();


  // make styles -- first the outer wall
  int wallTexture = tm.getTextureID( "wall" );
  wallTexWidth = wallTexHeight = 10.0f;
  if (wallTexture>=0)
    wallTexWidth = tm.GetAspectRatio(wallTexture) * wallTexHeight;


  // make box styles
  int boxTexture = tm.getTextureID( "boxwall" );
  boxTexWidth = boxTexHeight = 0.2f * BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
  if (boxTexture>=0)
    boxTexWidth = tm.GetAspectRatio(boxTexture) * boxTexHeight;


  // lower maximum tank lod if lowdetail is true
  if (renderer->useQuality() == 0)
    TankSceneNode::setMaxLOD(2);
}


SceneDatabaseBuilder::~SceneDatabaseBuilder()
{
  // do nothing
}


SceneDatabase*		SceneDatabaseBuilder::make(const World* world)
{
  // set LOD flags
  wallLOD = BZDBCache::lighting && BZDBCache::zbuffer;
  baseLOD = BZDBCache::lighting && BZDBCache::zbuffer;
  boxLOD = BZDBCache::lighting && BZDBCache::zbuffer;
  pyramidLOD = BZDBCache::lighting && BZDBCache::zbuffer;
  teleporterLOD = BZDBCache::lighting && BZDBCache::zbuffer;

  // pick type of database
  SceneDatabase* db;
  if (BZDBCache::zbuffer)
    db = new ZSceneDatabase;
  else
    db = new BSPSceneDatabase;
  // FIXME -- when making BSP tree, try several shuffles for best tree

  if (!world) return db;

  // add nodes to database
  const std::vector<WallObstacle*> &walls = world->getWalls();
  std::vector<WallObstacle*>::const_iterator wallScan = walls.begin();
  while (wallScan != walls.end()) {
    addWall(db, **wallScan);
    ++wallScan;
  }
  const std::vector<MeshObstacle*> &meshes = world->getMeshes();
  std::vector<MeshObstacle*>::const_iterator meshScan = meshes.begin();
  while (meshScan != meshes.end()) {
    addMesh(db, *meshScan);
    ++meshScan;
  }
  const std::vector<BoxBuilding*> &boxes = world->getBoxes();
  std::vector<BoxBuilding*>::const_iterator boxScan = boxes.begin();
  while (boxScan != boxes.end()) {
    addBox(db, **boxScan);
    ++boxScan;
  }
  const std::vector<Teleporter*> &teleporters = world->getTeleporters();
  std::vector<Teleporter*>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    addTeleporter(db, *(*teleporterScan), world);
    ++teleporterScan;
  }
  const std::vector<PyramidBuilding*> &pyramids = world->getPyramids();
  std::vector<PyramidBuilding*>::const_iterator pyramidScan = pyramids.begin();
  while (pyramidScan != pyramids.end()) {
    addPyramid(db, **pyramidScan);
    ++pyramidScan;
  }
  const std::vector<BaseBuilding*> &baseBuildings = world->getBases();
  std::vector<BaseBuilding*>::const_iterator baseScan = baseBuildings.begin();
  while (baseScan != baseBuildings.end()) {
    addBase(db, **baseScan);
    ++baseScan;
  }

  // add the water level node
  addWaterLevel(db, world);

  return db;
}


void SceneDatabaseBuilder::addWaterLevel(SceneDatabase* db,
                                         const World* world)
{
  float plane[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
  const float level = world->getWaterLevel();
  plane[3] = -level;

  // don't draw it if it isn't active
  if (level < 0.0f) {
    return;
  }

  // setup the vertex and texture coordinates
  float size = BZDBCache::worldSize;
  GLfloat3Array v(4);
  GLfloat3Array n(0);
  GLfloat2Array t(4);
  v[0][0] = v[0][1] = v[1][1] = v[3][0] = -size/2.0f;
  v[1][0] = v[2][0] = v[2][1] = v[3][1] = +size/2.0f;
  v[0][2] = v[1][2] = v[2][2] = v[3][2] = level;
  t[0][0] = t[0][1] = t[1][1] = t[3][0] = 0.0f;
  t[1][0] = t[2][0] = t[2][1] = t[3][1] = 2.0f;

  MeshPolySceneNode* node =
    new MeshPolySceneNode(plane, v, n, t);

  // setup the material
  const BzMaterial* mat = world->getWaterMaterial();
  MeshSceneNodeGenerator::setupNodeMaterial(node, mat);

  db->addStaticNode(node);

  return;
}


void			SceneDatabaseBuilder::addWall(SceneDatabase* db,
						const WallObstacle& o)
{
  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = new WallSceneNodeGenerator (&o);

  TextureManager &tm = TextureManager::instance();
  int wallTexture = -1;

  bool  useColorTexture = false;

  // try object, standard, then default
  if (o.userTextures[0].size())
    wallTexture = tm.getTextureID(o.userTextures[0].c_str(),false);
  if (wallTexture < 0)
    wallTexture = tm.getTextureID( "wall" );
  else
    useColorTexture = wallTexture >= 0;

  while ((node = nodeGen->getNextNode(o.getBreadth() / wallTexWidth,
				o.getHeight() / wallTexHeight, wallLOD))) {
    node->setColor(wallColors[part]);
    node->setModulateColor(wallModulateColors[part]);
    node->setLightedColor(wallLightedColors[0]);
    node->setLightedModulateColor(wallLightedModulateColors[0]);
    node->setMaterial(wallMaterial);
    node->setTexture(wallTexture);
    node->setUseColorTexture(useColorTexture);

    db->addStaticNode(node);
    part = (part + 1) % 5;
  }
  delete nodeGen;
}


void SceneDatabaseBuilder::addMesh(SceneDatabase* db, MeshObstacle* mesh)
{
  WallSceneNode* node;
  MeshSceneNodeGenerator* nodeGen = new MeshSceneNodeGenerator (mesh);

  while ((node = nodeGen->getNextNode(wallLOD))) {
    db->addStaticNode(node);
  }
  delete nodeGen;
}


void SceneDatabaseBuilder::addBox(SceneDatabase* db, BoxBuilding& o)
{
  // this assumes boxes have six parts:  four sides, a roof, and a bottom.
  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = new BoxSceneNodeGenerator(&o);
  TextureManager &tm = TextureManager::instance();
  int    boxTexture = -1;
  bool  useColorTexture[2] = {false,false};

  // try object, standard, then default
  if (o.userTextures[0].size())
    boxTexture = tm.getTextureID(o.userTextures[0].c_str(),false);
  if (boxTexture < 0)
    boxTexture = tm.getTextureID(BZDB.get("boxWallTexture").c_str(),true);

  useColorTexture[0] = boxTexture >= 0;

  int boxTopTexture = -1;

  if (o.userTextures[1].size())
    boxTopTexture = tm.getTextureID(o.userTextures[1].c_str(),false);
  if (boxTopTexture < 0)
    boxTopTexture = tm.getTextureID(BZDB.get("boxTopTexture").c_str(),true);

  useColorTexture[1] = boxTopTexture >= 0;

  float texutureFactor = BZDB.eval("boxWallTexRepeat");
  if (BZDB.eval("useQuality") >= 3)
    texutureFactor = BZDB.eval("boxWallHighResTexRepeat");

  while ((node = ((part < 4) ? nodeGen->getNextNode(
				-texutureFactor*boxTexWidth,
				-texutureFactor*boxTexWidth, boxLOD) :
    // I'm using boxTexHeight for roof since textures are same
    // size and this number is available
				nodeGen->getNextNode(
				-boxTexHeight,
				-boxTexHeight, boxLOD)))) {
    node->setColor(boxColors[part]);
    node->setModulateColor(boxModulateColors[part]);
    node->setLightedColor(boxLightedColors[part]);
    node->setLightedModulateColor(boxLightedModulateColors[part]);
    node->setMaterial(boxMaterial);
    if (part < 4){
      node->setTexture(boxTexture);
      node->setUseColorTexture(useColorTexture[0]);
    }
    else{
      node->setTexture(boxTopTexture);
      node->setUseColorTexture(useColorTexture[1]);
    }

    db->addStaticNode(node);
    part = (part + 1) % 6;
  }
  delete nodeGen;
}


void			SceneDatabaseBuilder::addPyramid(SceneDatabase* db,
						const PyramidBuilding& o)
{
  // this assumes pyramids have four parts:  four sides
  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = new PyramidSceneNodeGenerator(&o);

  TextureManager &tm = TextureManager::instance();
  int pyramidTexture = -1;

  bool useColorTexture = false;
  // try object, standard, then default
  if (o.userTextures[0].size())
    pyramidTexture = tm.getTextureID(o.userTextures[0].c_str(),false);
  if (pyramidTexture < 0)
    pyramidTexture = tm.getTextureID(BZDB.get("pyrWallTexture").c_str(),false);

  useColorTexture = pyramidTexture >= 0;

  // Using boxTexHeight since it's (currently) the same and it's already available
  float textureFactor = BZDB.eval("pyrWallTexRepeat");
  if (BZDB.eval("useQuality") >= 3)
    textureFactor = BZDB.eval("pyrWallHighResTexRepeat");

  while ((node = nodeGen->getNextNode(-textureFactor * boxTexHeight,
				-textureFactor * boxTexHeight,
				pyramidLOD))) {
    node->setColor(pyramidColors[part]);
    node->setModulateColor(pyramidModulateColors[part]);
    node->setLightedColor(pyramidLightedColors[part]);
    node->setLightedModulateColor(pyramidLightedModulateColors[part]);
    node->setMaterial(pyramidMaterial);
    node->setTexture(pyramidTexture);
    node->setUseColorTexture(useColorTexture);

    db->addStaticNode(node);
    part = (part + 1) % 5;
  }
  delete nodeGen;
}


void			SceneDatabaseBuilder::addBase(SceneDatabase *db,
						const BaseBuilding &o)
{
  WallSceneNode *node;
  ObstacleSceneNodeGenerator* nodeGen = new BaseSceneNodeGenerator(&o);

  TextureManager &tm = TextureManager::instance();
  int   boxTexture = -1;

  bool  useColorTexture[2] = {false,false};

  // try object, standard, then default
  if (o.userTextures[0].size())
    boxTexture = tm.getTextureID(o.userTextures[0].c_str(),false);
  if (boxTexture < 0) {
    std::string teamBase = Team::getImagePrefix((TeamColor)o.getTeam());
    teamBase += BZDB.get("baseWallTexture");
    boxTexture = tm.getTextureID(teamBase.c_str(),false);
  }
  if (boxTexture < 0)
    boxTexture = tm.getTextureID( BZDB.get("boxWallTexture").c_str() );

  useColorTexture[0] = boxTexture >= 0;

  int   baseTopTexture = -1;

  if (o.userTextures[1].size())
    baseTopTexture = tm.getTextureID(o.userTextures[1].c_str(),false);
  if (baseTopTexture < 0) {
    std::string teamBase = Team::getImagePrefix((TeamColor)o.getTeam());
    teamBase += BZDB.get("baseTopTexture").c_str();
    baseTopTexture = tm.getTextureID(teamBase.c_str(),false);
  }
  if (baseTopTexture < 0)
    baseTopTexture = -1;
  else
    useColorTexture[1] = baseTopTexture >= 0;

  // this assumes bases have 6 parts - if they don't, it still works
  int part = 0;
  // repeat the textue once for the top and bottom, else use the old messed up way
  // There are 3 cases for the texture ordering:
  // 1. getNextNode() only returns the top texture
  // 2. getNextNode() returns the top texture(0), and the 4 sides(1-4)
  // 3. getNextNode() returns the top texture(0), and the 4 sides(1-4), and the bottom(5)
  while ((node = ( ((part % 5) == 0) ? nodeGen->getNextNode(1,1, boxLOD) :
                                      nodeGen->getNextNode(o.getBreadth(),
                                                           o.getHeight(),
                                                           boxLOD)))) {
    if ((part % 5) != 0) {
      node->setColor(boxColors[part - 2]);
      node->setModulateColor(boxModulateColors[part - 2]);
      node->setLightedColor(boxLightedColors[part - 2]);
      node->setLightedModulateColor(boxLightedModulateColors[part - 2]);
      node->setMaterial(boxMaterial);
      node->setTexture(boxTexture);
      node->setUseColorTexture(useColorTexture[0]);
    }
    else{
      if (useColorTexture[1]) {  // only set the texture if we have one and are using it
        node->setTexture(baseTopTexture);
        node->setUseColorTexture(useColorTexture[1]);
      }
    }
    part++;
    db->addStaticNode(node);
  }
  delete nodeGen;
}

void			SceneDatabaseBuilder::addTeleporter(SceneDatabase* db,
						const Teleporter& o,
						const World* world)
{
  // this assumes teleporters have fourteen parts:  12 border sides, 2 faces
  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = new TeleporterSceneNodeGenerator(&o);

  TextureManager &tm = TextureManager::instance();
  int             teleporterTexture = -1;

  bool  useColorTexture = false;

  // try object, standard, then default
  if (o.userTextures[0].size())
    teleporterTexture = tm.getTextureID(o.userTextures[0].c_str(),false);
  if (teleporterTexture < 0)
    teleporterTexture = tm.getTextureID(BZDB.get("cautionTexture").c_str(),true);

  useColorTexture = teleporterTexture >= 0;

  int numParts = o.isHorizontal() ? 18 : 14;

  while ((node = nodeGen->getNextNode(1.0, o.getHeight() / o.getBreadth(),
							teleporterLOD))) {
    if (o.isHorizontal ()) {
      if (part >= 0 && part <= 15) {
        node->setColor (teleporterColors[0]);
        node->setModulateColor (teleporterModulateColors[0]);
        node->setLightedColor (teleporterLightedColors[0]);
        node->setLightedModulateColor (teleporterLightedModulateColors[0]);
        node->setMaterial (teleporterMaterial);
        node->setTexture (teleporterTexture);
        node->setUseColorTexture (useColorTexture);
      }
    }
    else {
      if (part >= 0 && part <= 1) {
        node->setColor (teleporterColors[0]);
        node->setModulateColor (teleporterModulateColors[0]);
        node->setLightedColor (teleporterLightedColors[0]);
        node->setLightedModulateColor (teleporterLightedModulateColors[0]);
        node->setMaterial (teleporterMaterial);
        node->setTexture (teleporterTexture);
        node->setUseColorTexture (useColorTexture);
      }
      else if (part >= 2 && part <= 11) {
        node->setColor (teleporterColors[1]);
        node->setModulateColor (teleporterModulateColors[1]);
        node->setLightedColor (teleporterLightedColors[1]);
        node->setLightedModulateColor (teleporterLightedModulateColors[1]);
        node->setMaterial (teleporterMaterial);
        node->setTexture (teleporterTexture);
        node->setUseColorTexture (useColorTexture);
      }
    }

    db->addStaticNode(node);
    part = (part + 1) % numParts;
  }

  MeshPolySceneNode* linkNode;
  const BzMaterial* mat = world->getLinkMaterial();

  linkNode = MeshSceneNodeGenerator::getMeshPolySceneNode(o.getBackLink());
  MeshSceneNodeGenerator::setupNodeMaterial(linkNode, mat);
  db->addStaticNode(linkNode);
  
  linkNode = MeshSceneNodeGenerator::getMeshPolySceneNode(o.getFrontLink());
  MeshSceneNodeGenerator::setupNodeMaterial(linkNode, mat);
  db->addStaticNode(linkNode);
  
  delete nodeGen;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

