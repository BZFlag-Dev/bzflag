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

#include <stdio.h>
#include "common.h"
#include "SceneBuilder.h"
#include "SceneRenderer.h"
#include "SceneDatabase.h"
#include "ZSceneDatabase.h"
#include "BSPSceneDatabase.h"
#include "World.h"
#include "WallSceneNode.h"
#include "TankSceneNode.h"
#include "StateDatabase.h"
#include "playing.h"
#include "texture.h"

static const char*	wallFilename = "wall";
static const char*	boxwallFilename = "boxwall";
static const char*	boxtopFilename = "roof";
static const char*	pyramidFilename = "pyrwall";
static const char*	teleporterFilename = "caution";

static OpenGLTexture	getImage(const std::string& file,
				float* aspectRatio = NULL,
				OpenGLTexture::Filter f =
					OpenGLTexture::LinearMipmapLinear)
{
  int width, height;
  OpenGLTexture tex = getTexture(file, &width, &height, f);
  if (aspectRatio) *aspectRatio = float(height) / float(width);
  return tex;
}

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

  // make styles -- first the outer wall
  wallTexWidth = wallTexHeight = 10.0f;
  wallTexture = getImage(wallFilename, &wallTexWidth);
  if (wallTexture.isValid()) wallTexWidth *= wallTexHeight;

  // make box styles
  boxTexWidth = boxTexHeight = 0.2f * BoxHeight;
  boxTexture = getImage(boxwallFilename, &boxTexWidth);
  if (boxTexture.isValid()) boxTexWidth *= boxTexHeight;
  boxTopTexture = getImage(boxtopFilename);

  // make pyramid styles
  pyramidTexture = getImage(pyramidFilename);

  // make teleporter styles
  teleporterTexture = getImage(teleporterFilename);

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
  wallLOD = BZDB->isTrue("lighting") && BZDB->isTrue("zbuffer");
  baseLOD = BZDB->isTrue("lighting") && BZDB->isTrue("zbuffer");
  boxLOD = BZDB->isTrue("lighting") && BZDB->isTrue("zbuffer");
  pyramidLOD = BZDB->isTrue("lighting") && BZDB->isTrue("zbuffer");
  teleporterLOD = BZDB->isTrue("lighting") && BZDB->isTrue("zbuffer");

  // pick type of database
  SceneDatabase* db;
  if (BZDB->isTrue("zbuffer"))
    db = new ZSceneDatabase;
  else
    db = new BSPSceneDatabase;
  // FIXME -- when making BSP tree, try several shuffles for best tree

  // add nodes to database
  std::vector<WallObstacle> walls = world->getWalls();
  std::vector<WallObstacle>::iterator wallScan = walls.begin();
  while (wallScan != walls.end()) {
    addWall(db, *wallScan);
    ++wallScan;
  }
  std::vector<BoxBuilding> boxes = world->getBoxes();
  std::vector<BoxBuilding>::iterator boxScan = boxes.begin();
  while (boxScan != boxes.end()) {
    addBox(db, *boxScan);
    ++boxScan;
  }
  std::vector<Teleporter> teleporters = world->getTeleporters();
  std::vector<Teleporter>::iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    addTeleporter(db, *teleporterScan);
    ++teleporterScan;
  }
  std::vector<PyramidBuilding> pyramids = world->getPyramids();
  std::vector<PyramidBuilding>::iterator pyramidScan = pyramids.begin();
  while (pyramidScan != pyramids.end()) {
    addPyramid(db, *pyramidScan);
    ++pyramidScan;
  }
  std::vector<BaseBuilding> baseBuildings = world->getBases();
  std::vector<BaseBuilding>::iterator baseScan = baseBuildings.begin();
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
  while ((node = nodeGen->getNextNode(o.getBreadth() / wallTexWidth,
				o.getHeight() / wallTexHeight, wallLOD))) {
    node->setColor(wallColors[part]);
    node->setModulateColor(wallModulateColors[part]);
    node->setLightedColor(wallLightedColors[0]);
    node->setLightedModulateColor(wallLightedModulateColors[0]);
    node->setMaterial(wallMaterial);
    node->setTexture(wallTexture);

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

  while ((node = ((part != 5) ? nodeGen->getNextNode(
				-1.5f*boxTexWidth,
				-1.5f*boxTexWidth, boxLOD) :
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
    if (part < 4) node->setTexture(boxTexture);
    else node->setTexture(boxTopTexture);

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
  
  // Using boxTexHeight since it's (currently) the same and it's already available
  while ((node = nodeGen->getNextNode(-3.0f * boxTexHeight,
				-3.0f * boxTexHeight,
				pyramidLOD))) {
    node->setColor(pyramidColors[part]);
    node->setModulateColor(pyramidModulateColors[part]);
    node->setLightedColor(pyramidLightedColors[part]);
    node->setLightedModulateColor(pyramidLightedModulateColors[part]);
    node->setMaterial(pyramidMaterial);
    node->setTexture(pyramidTexture);

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

  // this assumes bases have 6 parts - if they don't, it still works
  int part = 0;
  while ((node = ((part < 2) ? nodeGen->getNextNode(
				o.getBreadth(), o.getHeight(),
				baseLOD) : nodeGen->getNextNode(
				-1.5f*boxTexWidth,
				-1.5f*boxTexWidth, boxLOD)))) {
    if(part >= 2) {
      node->setColor(boxColors[part - 2]);
      node->setModulateColor(boxModulateColors[part - 2]);
      node->setLightedColor(boxLightedColors[part - 2]);
      node->setLightedModulateColor(boxLightedModulateColors[part - 2]);
      node->setMaterial(boxMaterial);
      node->setTexture(boxTexture);
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

  while ((node = nodeGen->getNextNode(1.0, o.getHeight() / o.getBreadth(),
							teleporterLOD))) {
    if (part >= 0 && part <= 1) {
      node->setColor(teleporterColors[0]);
      node->setModulateColor(teleporterModulateColors[0]);
      node->setLightedColor(teleporterLightedColors[0]);
      node->setLightedModulateColor(teleporterLightedModulateColors[0]);
      node->setMaterial(teleporterMaterial);
      node->setTexture(teleporterTexture);
    }
    else if (part >= 2 && part <= 11) {
      node->setColor(teleporterColors[1]);
      node->setModulateColor(teleporterModulateColors[1]);
      node->setLightedColor(teleporterLightedColors[1]);
      node->setLightedModulateColor(teleporterLightedModulateColors[1]);
      node->setMaterial(teleporterMaterial);
      node->setTexture(teleporterTexture);
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
// ex: shiftwidth=2 tabstop=8
