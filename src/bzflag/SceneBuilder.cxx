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

#include <stdio.h>
#include "common.h"
#include "playing.h"
#include "SceneBuilder.h"
#include "SceneRenderer.h"
#include "SceneDatabase.h"
#include "ZSceneDatabase.h"
#include "BSPSceneDatabase.h"
#include "World.h"
#include "WallSceneNode.h"
#include "TankSceneNode.h"
#include "StateDatabase.h"
#include "TextureManager.h"
#include "texture.h"

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
  OpenGLTexture *wallTexture = tm.getTexture( "wall" );
  wallTexWidth = wallTexHeight = 10.0f;
  if (wallTexture->isValid())
    wallTexWidth = wallTexture->getAspectRatio() * wallTexHeight;


  // make box styles
  OpenGLTexture *boxTexture = tm.getTexture( "boxwall" );
  boxTexWidth = boxTexHeight = 0.2f * BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
  if (boxTexture->isValid())
    boxTexWidth = boxTexture->getAspectRatio() * boxTexHeight;


  // lower maximum tank lod if lowdetail is true
  if (renderer->useQuality() == 0) TankSceneNode::setMaxLOD(2);
}

SceneDatabaseBuilder::~SceneDatabaseBuilder()
{
  // do nothing
}

SceneDatabase*		SceneDatabaseBuilder::make(const World* world)
{
  // set LOD flags
  wallLOD = BZDB.isTrue("lighting") && BZDB.isTrue("zbuffer");
  baseLOD = BZDB.isTrue("lighting") && BZDB.isTrue("zbuffer");
  boxLOD = BZDB.isTrue("lighting") && BZDB.isTrue("zbuffer");
  pyramidLOD = BZDB.isTrue("lighting") && BZDB.isTrue("zbuffer");
  teleporterLOD = BZDB.isTrue("lighting") && BZDB.isTrue("zbuffer");

  // pick type of database
  SceneDatabase* db;
  if (BZDB.isTrue("zbuffer"))
    db = new ZSceneDatabase;
  else
    db = new BSPSceneDatabase;
  // FIXME -- when making BSP tree, try several shuffles for best tree

  // add nodes to database
  const std::vector<WallObstacle> &walls = world->getWalls();
  std::vector<WallObstacle>::const_iterator wallScan = walls.begin();
  while (wallScan != walls.end()) {
    addWall(db, *wallScan);
    ++wallScan;
  }
  const std::vector<BoxBuilding> &boxes = world->getBoxes();
  std::vector<BoxBuilding>::const_iterator boxScan = boxes.begin();
  while (boxScan != boxes.end()) {
    addBox(db, *boxScan);
    ++boxScan;
  }
  const std::vector<Teleporter> &teleporters = world->getTeleporters();
  std::vector<Teleporter>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    addTeleporter(db, *teleporterScan);
    ++teleporterScan;
  }
  const std::vector<PyramidBuilding> &pyramids = world->getPyramids();
  std::vector<PyramidBuilding>::const_iterator pyramidScan = pyramids.begin();
  while (pyramidScan != pyramids.end()) {
    addPyramid(db, *pyramidScan);
    ++pyramidScan;
  }
  const std::vector<BaseBuilding> &baseBuildings = world->getBases();
  std::vector<BaseBuilding>::const_iterator baseScan = baseBuildings.begin();
  while (baseScan != baseBuildings.end()) {
    addBase(db, *baseScan);
    ++baseScan;
  }

  return db;
}

void			SceneDatabaseBuilder::addWall(SceneDatabase* db,
						const WallObstacle& o)
{
  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = o.newSceneNodeGenerator();

  TextureManager &tm = TextureManager::instance();
  OpenGLTexture *wallTexture = NULL;// = tm.getTexture( "wall" );

  bool  useColorTexture = false;

  // try object, standard, then default
  if (o.userTextures[0].size())
    wallTexture = tm.getTexture(o.userTextures[0].c_str(),false);
  if (!wallTexture || !wallTexture->isValid())
    wallTexture = tm.getTexture("std_wall",false);
  if (!wallTexture || !wallTexture->isValid())
    wallTexture = tm.getTexture( "wall" );
  else
    useColorTexture = wallTexture && wallTexture->isValid();

  while ((node = nodeGen->getNextNode(o.getBreadth() / wallTexWidth,
				o.getHeight() / wallTexHeight, wallLOD))) {
    node->setColor(wallColors[part]);
    node->setModulateColor(wallModulateColors[part]);
    node->setLightedColor(wallLightedColors[0]);
    node->setLightedModulateColor(wallLightedModulateColors[0]);
    node->setMaterial(wallMaterial);
    node->setTexture(*wallTexture);
    node->setUseColorTexture(useColorTexture);

    db->addStaticNode(node);
    part = (part + 1) % 5;
  }
  delete nodeGen;
}

void			SceneDatabaseBuilder::addBox(SceneDatabase* db,
						const BoxBuilding& o)
{
  // this assumes boxes have six parts:  four sides, a roof, and a bottom.
  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = o.newSceneNodeGenerator();
  TextureManager &tm = TextureManager::instance();
  OpenGLTexture *boxTexture = NULL;
  bool  useColorTexture[2] = {false,false};

  // try object, standard, then default
  if (o.userTextures[0].size())
    boxTexture = tm.getTexture(o.userTextures[0].c_str(),false);
  if (!boxTexture || !boxTexture->isValid())
    boxTexture = tm.getTexture(BZDB.get("boxWallTexture").c_str(),true);
   
  useColorTexture[0] = boxTexture && boxTexture->isValid();

  OpenGLTexture *boxTopTexture = NULL;

  if (o.userTextures[1].size())
    boxTopTexture = tm.getTexture(o.userTextures[1].c_str(),false);
  if (!boxTopTexture || !boxTopTexture->isValid())
    boxTopTexture = tm.getTexture(BZDB.get("boxTopTexture").c_str(),true);

  useColorTexture[1] = boxTopTexture && boxTopTexture->isValid();

  float texutureFactor = BZDB.eval("boxWallTexRepeat");
  if (BZDB.eval("useQuality")>=3)
    texutureFactor = BZDB.eval("boxWallHighResTexRepeat");

  while ((node = ((part != 5) ? nodeGen->getNextNode(
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
      node->setTexture(*boxTexture);
      node->setUseColorTexture(useColorTexture[0]);
    }
    else{
      node->setTexture(*boxTopTexture);
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
  ObstacleSceneNodeGenerator* nodeGen = o.newSceneNodeGenerator();

  TextureManager &tm = TextureManager::instance();
  OpenGLTexture *pyramidTexture = NULL;//tm.getTexture( "pyrwall" );

  bool useColorTexture = false;
  // try object, standard, then default
  if (o.userTextures[0].size())
    pyramidTexture = tm.getTexture(o.userTextures[0].c_str(),false);
  if (!pyramidTexture || !pyramidTexture->isValid())
    pyramidTexture = tm.getTexture(BZDB.get("pyrWallTexture").c_str(),false);

  useColorTexture = pyramidTexture && pyramidTexture->isValid();

  // Using boxTexHeight since it's (currently) the same and it's already available
  float texutureFactor = BZDB.eval("pryWallTexRepeat");
  if (BZDB.eval("useQuality")>=3)
    texutureFactor = BZDB.eval("pryWallHighResTexRepeat");

  while ((node = nodeGen->getNextNode(-texutureFactor * boxTexHeight,
				-texutureFactor * boxTexHeight,
				pyramidLOD))) {
    node->setColor(pyramidColors[part]);
    node->setModulateColor(pyramidModulateColors[part]);
    node->setLightedColor(pyramidLightedColors[part]);
    node->setLightedModulateColor(pyramidLightedModulateColors[part]);
    node->setMaterial(pyramidMaterial);
    node->setTexture(*pyramidTexture);
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
  ObstacleSceneNodeGenerator *nodeGen = o.newSceneNodeGenerator();

  TextureManager &tm = TextureManager::instance();
  OpenGLTexture *boxTexture = NULL;//tm.getTexture( "boxwall" );

  bool  useColorTexture[2] = {false,false};

  // try object, standard, then default
  if (o.userTextures[0].size())
    boxTexture = tm.getTexture(o.userTextures[0].c_str(),false);
  if (!boxTexture || !boxTexture->isValid())
  {
    std::string teamBase = Team::getImagePrefix((TeamColor)o.getTeam());
    teamBase+=BZDB.get("baseWallTexture");
    boxTexture = tm.getTexture(teamBase.c_str(),false);
  }
  if (!boxTexture || !boxTexture->isValid())
    boxTexture = tm.getTexture( BZDB.get("boxWallTexture").c_str() );

  useColorTexture[0] = boxTexture && boxTexture->isValid();

  OpenGLTexture *baseTopTexture = NULL;

  if (o.userTextures[1].size())
    baseTopTexture = tm.getTexture(o.userTextures[1].c_str(),false);
  if (!baseTopTexture || !baseTopTexture->isValid()){
    std::string teamBase = Team::getImagePrefix((TeamColor)o.getTeam());
    teamBase+=BZDB.get("baseTopTexture").c_str();
    baseTopTexture = tm.getTexture(teamBase.c_str(),false);
  }
  if (!baseTopTexture || !baseTopTexture->isValid())
    baseTopTexture = NULL;
  else
    useColorTexture[1] = baseTopTexture && baseTopTexture->isValid();

  // this assumes bases have 6 parts - if they don't, it still works
  int part = 0;
  // repeat the textue once for the top and bottom, else use the old messed up way
  while ((node = ((part < 2) ? nodeGen->getNextNode(
                                1,
                                1, boxLOD) :
                                nodeGen->getNextNode(
                                o.getBreadth(),
                                o.getHeight(), boxLOD)))) {
    if(part >= 2) {
      node->setColor(boxColors[part - 2]);
      node->setModulateColor(boxModulateColors[part - 2]);
      node->setLightedColor(boxLightedColors[part - 2]);
      node->setLightedModulateColor(boxLightedModulateColors[part - 2]);
      node->setMaterial(boxMaterial);
      node->setTexture(*boxTexture);
      node->setUseColorTexture(useColorTexture[0]);
    }
    else{
      if (useColorTexture[1]){  // only set the texture if we have one and are using it
        node->setTexture(*baseTopTexture);
        node->setUseColorTexture(useColorTexture[1]);
      }
    }
    part++;
    db->addStaticNode(node);
  }
  delete nodeGen;
}

void			SceneDatabaseBuilder::addTeleporter(SceneDatabase* db,
						const Teleporter& o)
{
  // this assumes teleporters have fourteen parts:  12 border sides, 2 faces
  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = o.newSceneNodeGenerator();

  TextureManager &tm = TextureManager::instance();
  OpenGLTexture *teleporterTexture = NULL;//tm.getTexture( "caution" );

  bool  useColorTexture = false;

  // try object, standard, then default
  if (o.userTextures[0].size())
    teleporterTexture = tm.getTexture(o.userTextures[0].c_str(),false);
  if (!teleporterTexture || !teleporterTexture->isValid())
    teleporterTexture = tm.getTexture(BZDB.get("cautionTexture").c_str(),true);
   
  useColorTexture = BZDB.isTrue("texture") && teleporterTexture && teleporterTexture->isValid();

  while ((node = nodeGen->getNextNode(1.0, o.getHeight() / o.getBreadth(),
							teleporterLOD))) {
    if (part >= 0 && part <= 1) {
      node->setColor(teleporterColors[0]);
      node->setModulateColor(teleporterModulateColors[0]);
      node->setLightedColor(teleporterLightedColors[0]);
      node->setLightedModulateColor(teleporterLightedModulateColors[0]);
      node->setMaterial(teleporterMaterial);
      node->setTexture(*teleporterTexture);
      node->setUseColorTexture(useColorTexture);
   }
    else if (part >= 2 && part <= 11) {
      node->setColor(teleporterColors[1]);
      node->setModulateColor(teleporterModulateColors[1]);
      node->setLightedColor(teleporterLightedColors[1]);
      node->setLightedModulateColor(teleporterLightedModulateColors[1]);
      node->setMaterial(teleporterMaterial);
      node->setTexture(*teleporterTexture);
      node->setUseColorTexture(useColorTexture);
   }
    else {
      node->setColor(teleporterColors[2]);
      node->setLightedColor(teleporterLightedColors[2]);
    }

    db->addStaticNode(node);
    part = (part + 1) % 14;
  }
  delete nodeGen;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

