/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "SceneBuilder.h"
#include "SceneRenderer.h"
#include "SceneDatabase.h"
#include "ZSceneDatabase.h"
#include "BSPSceneDatabase.h"
#include "World.h"
#include "WallSceneNode.h"
#include "TankSceneNode.h"
#include "playing.h"
#include "texture.h"

static const char*	wallFilename = "wall";
static const char*	boxwallFilename = "boxwall";
static const char*	boxtopFilename = "roof";
static const char*	pyramidFilename = "pyrwall";
static const char*	teleporterFilename = "caution";

static OpenGLTexture	getImage(const BzfString& file,
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
  wallLOD = renderer->useLighting() && renderer->useZBuffer();
  boxLOD = renderer->useLighting() && renderer->useZBuffer();
  pyramidLOD = renderer->useLighting() && renderer->useZBuffer();
  teleporterLOD = renderer->useLighting() && renderer->useZBuffer();

  // pick type of database
  SceneDatabase* db;
  if (renderer->useZBuffer())
    db = new ZSceneDatabase;
  else
    db = new BSPSceneDatabase;
  // FIXME -- when making BSP tree, try several shuffles for best tree

  // add nodes to database
  WallObstaclesCIteratorPtr wallScan(world->getWalls().newCIterator());
  while (!wallScan->isDone()) {
    addWall(db, wallScan->getItem());
    wallScan->next();
  }
  BoxBuildingsCIteratorPtr boxScan(world->getBoxes().newCIterator());
  while (!boxScan->isDone()) {
    addBox(db, boxScan->getItem());
    boxScan->next();
  }
  TeleportersCIteratorPtr teleporterScan(world->getTeleporters().newCIterator());
  while (!teleporterScan->isDone()) {
    addTeleporter(db, teleporterScan->getItem());
    teleporterScan->next();
  }
  PyramidBuildingsCIteratorPtr pyramidScan(world->getPyramids().newCIterator());
  while (!pyramidScan->isDone()) {
    addPyramid(db, pyramidScan->getItem());
    pyramidScan->next();
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
				o.getBreadth() / boxTexWidth,
				o.getHeight() / boxTexHeight, boxLOD) :
				nodeGen->getNextNode(
				0.25f * o.getBreadth(),
				0.25f * o.getHeight(), boxLOD)))) {
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

  while ((node = nodeGen->getNextNode(3.0f,
				3.0f * o.getHeight() / o.getBreadth(),
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
