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
#include "SceneBuilder.h"

// local implemenation headers
#include "ZSceneDatabase.h"
#include "BSPSceneDatabase.h"
#include "World.h"

// scene node implemenation headers
#include "BaseSceneNodeGenerator.h"
#include "BoxSceneNodeGenerator.h"
#include "EighthDBaseSceneNode.h"
#include "EighthDBoxSceneNode.h"
#include "EighthDimShellNode.h"
#include "EighthDPyrSceneNode.h"
#include "MeshPolySceneNode.h"
#include "MeshSceneNodeGenerator.h"
#include "ObstacleSceneNodeGenerator.h"
#include "PyramidSceneNodeGenerator.h"
#include "TankSceneNode.h"
#include "TextSceneNode.h"
#include "WallSceneNodeGenerator.h"

// common implementation headers
#include "StateDatabase.h"
#include "SceneRenderer.h"
#include "BZDBCache.h"
#include "TextureManager.h"
#include "ObstacleList.h"
#include "ObstacleMgr.h"
#include "WorldText.h"

// local implementation headers
#include "DynamicWorldText.h"

// uncomment for cheaper eighth dimension scene nodes
//#define SHELL_INSIDE_NODES


//
// SceneDatabaseBuilder
//

const fvec4 SceneDatabaseBuilder::wallColors[4] = {
  fvec4(0.5f, 0.5f, 0.5f, 1.0f),
  fvec4(0.4f, 0.4f, 0.4f, 1.0f),
  fvec4(0.5f, 0.5f, 0.5f, 1.0f),
  fvec4(0.6f, 0.6f, 0.6f, 1.0f)
};
const fvec4 SceneDatabaseBuilder::wallModulateColors[4] = {
  fvec4(0.5f, 0.5f, 0.5f, 1.0f),
  fvec4(0.4f, 0.4f, 0.4f, 1.0f),
  fvec4(0.5f, 0.5f, 0.5f, 1.0f),
  fvec4(0.6f, 0.6f, 0.6f, 1.0f)
};
const fvec4 SceneDatabaseBuilder::wallLightedColors[1] = {
  fvec4(0.5f, 0.5f, 0.5f, 1.0f)
};
const fvec4 SceneDatabaseBuilder::wallLightedModulateColors[1] = {
  fvec4(0.5f, 0.5f, 0.5f, 1.0f)
};

const fvec4 SceneDatabaseBuilder::boxColors[6] = {
  fvec4(0.75f,  0.25f,  0.25f,  1.0f),
  fvec4(0.63f,  0.25f,  0.25f,  1.0f),
  fvec4(0.75f,  0.25f,  0.25f,  1.0f),
  fvec4(0.75f,  0.375f, 0.375f, 1.0f),
  fvec4(0.875f, 0.5f,   0.5f,   1.0f),
  fvec4(0.275f, 0.2f,   0.2f,   1.0f)
};
const fvec4 SceneDatabaseBuilder::boxModulateColors[6] = {
  fvec4(0.75f,  0.75f,  0.75f,  1.0f),
  fvec4(0.63f,  0.63f,  0.63f,  1.0f),
  fvec4(0.75f,  0.75f,  0.75f,  1.0f),
  fvec4(0.69f,  0.69f,  0.69f,  1.0f),
  fvec4(0.875f, 0.875f, 0.875f, 1.0f),
  fvec4(0.375f, 0.375f, 0.375f, 1.0f)
};
const fvec4 SceneDatabaseBuilder::boxLightedColors[6] = {
  fvec4(0.75f,  0.25f, 0.25f, 1.0f),
  fvec4(0.75f,  0.25f, 0.25f, 1.0f),
  fvec4(0.75f,  0.25f, 0.25f, 1.0f),
  fvec4(0.75f,  0.25f, 0.25f, 1.0f),
  fvec4(0.875f, 0.5f,  0.5f,  1.0f),
  fvec4(0.875f, 0.5f,  0.5f,  1.0f)
};
const fvec4 SceneDatabaseBuilder::boxLightedModulateColors[6] = {
  fvec4(0.75f,  0.75f,  0.75f,  1.0f),
  fvec4(0.75f,  0.75f,  0.75f,  1.0f),
  fvec4(0.75f,  0.75f,  0.75f,  1.0f),
  fvec4(0.75f,  0.75f,  0.75f,  1.0f),
  fvec4(0.875f, 0.875f, 0.875f, 1.0f),
  fvec4(0.875f, 0.875f, 0.875f, 1.0f)
};

const fvec4 SceneDatabaseBuilder::pyramidColors[5] = {
  fvec4(0.25f,  0.25f,  0.63f, 1.0f),
  fvec4(0.13f,  0.13f,  0.51f, 1.0f),
  fvec4(0.25f,  0.25f,  0.63f, 1.0f),
  fvec4(0.375f, 0.375f, 0.75f, 1.0f),
  fvec4(0.175f, 0.175f, 0.35f, 1.0f)
};

const fvec4 SceneDatabaseBuilder::pyramidModulateColors[5] = {
  fvec4(0.25f,  0.25f,  0.63f, 1.0f),
  fvec4(0.13f,  0.13f,  0.51f, 1.0f),
  fvec4(0.25f,  0.25f,  0.63f, 1.0f),
  fvec4(0.375f, 0.375f, 0.75f, 1.0f),
  fvec4(0.175f, 0.175f, 0.35f, 1.0f)
};
const fvec4 SceneDatabaseBuilder::pyramidLightedColors[5] = {
  fvec4(0.25f, 0.25f, 0.63f, 1.0f),
  fvec4(0.25f, 0.25f, 0.63f, 1.0f),
  fvec4(0.25f, 0.25f, 0.63f, 1.0f),
  fvec4(0.25f, 0.25f, 0.63f, 1.0f),
  fvec4(0.25f, 0.25f, 0.63f, 1.0f)
};
const fvec4 SceneDatabaseBuilder::pyramidLightedModulateColors[5] = {
  fvec4(0.25f, 0.25f, 0.63f, 1.0f),
  fvec4(0.25f, 0.25f, 0.63f, 1.0f),
  fvec4(0.25f, 0.25f, 0.63f, 1.0f),
  fvec4(0.25f, 0.25f, 0.63f, 1.0f),
  fvec4(0.25f, 0.25f, 0.63f, 1.0f)
};

const fvec4 SceneDatabaseBuilder::teleporterColors[3] = {
  fvec4(1.0f, 0.875f, 0.0f, 1.0f),
  fvec4(0.9f, 0.8f,   0.0f, 1.0f),
  fvec4(0.0f, 0.0f,   0.0f, 0.5f)
};
const fvec4 SceneDatabaseBuilder::teleporterModulateColors[3] = {
  fvec4(1.0f, 1.0f, 1.0f, 1.0f),
  fvec4(0.9f, 0.9f, 0.9f, 1.0f),
  fvec4(0.0f, 0.0f, 0.0f, 0.5f)
};
const fvec4 SceneDatabaseBuilder::teleporterLightedColors[3] = {
  fvec4(1.0f, 0.875f, 0.0f, 1.0f),
  fvec4(1.0f, 0.875f, 0.0f, 1.0f),
  fvec4(0.0f, 0.0f,   0.0f, 0.5f)
};
const fvec4 SceneDatabaseBuilder::teleporterLightedModulateColors[3] = {
  fvec4(1.0f, 1.0f, 1.0f, 1.0f),
  fvec4(1.0f, 1.0f, 1.0f, 1.0f),
  fvec4(0.0f, 0.0f, 0.0f, 0.5f)
};


static const fvec4 black(0.0f, 0.0f, 0.0f, 0.0f);


SceneDatabaseBuilder::SceneDatabaseBuilder()
: wallMaterial(black, black, 0.0f)
, boxMaterial(black, black, 0.0f)
, pyramidMaterial(black, black, 0.0f)
, teleporterMaterial(black, black, 0.0f)
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
  boxTexWidth = boxTexHeight = 0.2f * BZDB.eval(BZDBNAMES.BOXHEIGHT);
  if (boxTexture>=0)
    boxTexWidth = tm.GetAspectRatio(boxTexture) * boxTexHeight;


  // lower maximum tank lod if lowdetail is true
  if (RENDERER.useQuality() == _LOW_QUALITY)
    TankSceneNode::setMaxLOD(2);
}


SceneDatabaseBuilder::~SceneDatabaseBuilder()
{
  // do nothing
}


SceneDatabase* SceneDatabaseBuilder::make(const World* world)
{
  // set LOD flags
  const bool doLODs = BZDBCache::lighting && BZDBCache::zbuffer;
  wallLOD = baseLOD = boxLOD = pyramidLOD = teleporterLOD = doLODs;

  // pick type of database
  SceneDatabase* db;
  if (BZDBCache::zbuffer) {
    db = new ZSceneDatabase;
  } else {
    db = new BSPSceneDatabase;
  }
  // FIXME -- when making BSP tree, try several shuffles for best tree

  if (!world) {
    return db;
  }

  // free any prior inside nodes
  world->freeInsideNodes();

  // add nodes to database
  unsigned int i;

  const ObstacleList& walls = OBSTACLEMGR.getWalls();
  for (i = 0; i < walls.size(); i++) {
    addWall(db, *((WallObstacle*) walls[i]));
  }

  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  for (i = 0; i < boxes.size(); i++) {
    addBox(db, *((BoxBuilding*) boxes[i]));
  }

  const ObstacleList& bases = OBSTACLEMGR.getBases();
  for (i = 0; i < bases.size(); i++) {
    addBase(db, *((BaseBuilding*) bases[i]));
  }

  const ObstacleList& pyramids = OBSTACLEMGR.getPyrs();
  for (i = 0; i < pyramids.size(); i++) {
    addPyramid(db, *((PyramidBuilding*) pyramids[i]));
  }

  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  for (i = 0; i < meshes.size(); i++) {
    addMesh(db, (MeshObstacle*) meshes[i]);
  }

  // add the world text
  addWorldTexts(db);

  // add the water level node
  addWaterLevel(db, world);

  db->finalizeStatics();

  return db;
}


void SceneDatabaseBuilder::addWorldTexts(SceneDatabase* db)
{
  const std::vector<WorldText*>& texts = OBSTACLEMGR.getTexts();
  for (size_t i = 0; i < texts.size(); i++) {
    const WorldText* text = texts[i];
    if (text->useBZDB) {
      DYNAMICWORLDTEXT.addText(text);
    } else {
      TextSceneNode* node = new TextSceneNode(text);
      db->addStaticNode(node, false);
    }
  }
}


void SceneDatabaseBuilder::addWaterLevel(SceneDatabase* db,
					 const World* world)
{
  fvec4 plane(0.0f, 0.0f, 1.0f, 0.0f);
  const float level = world->getWaterLevel();
  plane.w = -level;

  // don't draw it if it isn't active
  if (level < 0.0f) {
    return;
  }

  // setup the vertex and texture coordinates
  fvec3Array v(4);
  fvec3Array n(0);
  fvec2Array t(4);
  const float hs = 0.5f * BZDBCache::worldSize;
  v[0] = fvec3(-hs, -hs, level);  t[0] = fvec2(0.0f, 0.0f);
  v[1] = fvec3(+hs, -hs, level);  t[1] = fvec2(2.0f, 0.0f);
  v[2] = fvec3(+hs, +hs, level);  t[2] = fvec2(2.0f, 2.0f);
  v[3] = fvec3(-hs, +hs, level);  t[3] = fvec2(0.0f, 2.0f);

  // get the material
  const BzMaterial* mat = world->getWaterMaterial();
  const bool noRadar = mat->getNoRadar();
  const bool noShadow = mat->getNoShadowCast(); // FIXME

  MeshPolySceneNode* node =
    new MeshPolySceneNode(plane, noRadar, noShadow, v, n, t);

  // setup the material
  MeshSceneNodeGenerator::setupNodeMaterial(node, mat);

  db->addStaticNode(node, false);

  return;
}


void SceneDatabaseBuilder::addWall(SceneDatabase* db, const WallObstacle& o)
{
  if (o.getHeight() <= 0.0f) {
    return;
  }

  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = new WallSceneNodeGenerator (&o);

  TextureManager &tm = TextureManager::instance();
  int wallTexture = -1;

  bool useColorTexture = false;

  // try object, standard, then default
  if (o.userTextures[0].size())
    wallTexture = tm.getTextureID(o.userTextures[0],false);
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

    db->addStaticNode(node, false);
    part = (part + 1) % 5;
  }
  delete nodeGen;
}


void SceneDatabaseBuilder::addMesh(SceneDatabase* db, MeshObstacle* mesh)
{
  WallSceneNode* node;
  MeshSceneNodeGenerator* nodeGen = new MeshSceneNodeGenerator (mesh);

  while ((node = nodeGen->getNextNode(wallLOD))) {
    // make the inside node
    const bool ownTheNode = db->addStaticNode(node, true);
    // The BSP can split a MeshPolySceneNode and then delete it, which is
    // not good for the EighthDimShellNode's referencing scheme. If the
    // BSP would have split and then deleted this node, ownTheNode will
    // return true, and the EighthDimShellNode will then own the node.
    EighthDimShellNode* inode = new EighthDimShellNode(node, ownTheNode);
    mesh->addInsideSceneNode(inode);
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
  int boxTexture = -1;
  bool useColorTexture[2] = { false, false };

  // try object, standard, then default
  if (o.userTextures[0].size())
    boxTexture = tm.getTextureID(o.userTextures[0],false);
  if (boxTexture < 0)
    boxTexture = tm.getTextureID(BZDB.get("boxWallTexture"),true);

  useColorTexture[0] = boxTexture >= 0;

  int boxTopTexture = -1;

  if (o.userTextures[1].size())
    boxTopTexture = tm.getTextureID(o.userTextures[1],false);
  if (boxTopTexture < 0)
    boxTopTexture = tm.getTextureID(BZDB.get("boxTopTexture"),true);

  useColorTexture[1] = boxTopTexture >= 0;

  float textureFactor = BZDB.eval("boxWallTexRepeat");
  if (RENDERER.useQuality() >= _HIGH_QUALITY)
    textureFactor = BZDB.eval("boxWallHighResTexRepeat");

  while ((node = ((part < 4) ?
		  nodeGen->getNextNode(-textureFactor*boxTexWidth,
				       -textureFactor*boxTexWidth, boxLOD) :
		  // I'm using boxTexHeight for roof since textures are same
		  // size and this number is available
		  nodeGen->getNextNode(-boxTexHeight,
				       -boxTexHeight, boxLOD)))) {
    node->setColor(boxColors[part]);
    node->setModulateColor(boxModulateColors[part]);
    node->setLightedColor(boxLightedColors[part]);
    node->setLightedModulateColor(boxLightedModulateColors[part]);
    node->setMaterial(boxMaterial);
    if (part < 4) {
      node->setTexture(boxTexture);
      node->setUseColorTexture(useColorTexture[0]);
    } else {
      node->setTexture(boxTopTexture);
      node->setUseColorTexture(useColorTexture[1]);
    }

#ifdef SHELL_INSIDE_NODES
    const bool ownTheNode = db->addStaticNode(node, true);
    EighthDimShellNode* inode = new EighthDimShellNode(node, ownTheNode);
    o.addInsideSceneNode(inode);
#else
    db->addStaticNode(node, false);
#endif // SHELL_INSIDE_NODES

    part = (part + 1) % 6;
  }

#ifndef SHELL_INSIDE_NODES
  // add the inside node
  SceneNode* inode =
    new EighthDBoxSceneNode(o.getPosition(), o.getSize(), o.getRotation());
  o.addInsideSceneNode(inode);
#endif // SHELL_INSIDE_NODES

  delete nodeGen;
}


void SceneDatabaseBuilder::addPyramid(SceneDatabase* db, PyramidBuilding& o)
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
    pyramidTexture = tm.getTextureID(o.userTextures[0],false);
  if (pyramidTexture < 0)
    pyramidTexture = tm.getTextureID(BZDB.get("pyrWallTexture"),false);

  useColorTexture = pyramidTexture >= 0;

  // Using boxTexHeight since it's (currently) the same and it's already available
  float textureFactor = BZDB.eval("pyrWallTexRepeat");
  if (RENDERER.useQuality() >= _HIGH_QUALITY)
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

#ifdef SHELL_INSIDE_NODES
    const bool ownTheNode = db->addStaticNode(node, true);
    EighthDimShellNode* inode = new EighthDimShellNode(node, ownTheNode);
    o.addInsideSceneNode(inode);
#else
    db->addStaticNode(node, false);
#endif // SHELL_INSIDE_NODES

    part = (part + 1) % 5;
  }

#ifndef SHELL_INSIDE_NODES
  // add the inside node
  SceneNode* inode =
    new EighthDPyrSceneNode(o.getPosition(), o.getSize(), o.getRotation());
  o.addInsideSceneNode(inode);
#endif // SHELL_INSIDE_NODES

  delete nodeGen;
}


void SceneDatabaseBuilder::addBase(SceneDatabase *db, BaseBuilding &o)
{
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = new BaseSceneNodeGenerator(&o);

  TextureManager &tm = TextureManager::instance();
  int boxTexture = -1;

  bool  useColorTexture[2] = {false,false};

  // try object, standard, then default
  if (o.userTextures[0].size())
    boxTexture = tm.getTextureID(o.userTextures[0],false);
  if (boxTexture < 0) {
    std::string teamBase = Team::getImagePrefix((TeamColor)o.getBaseTeam());
    teamBase += BZDB.get("baseWallTexture");
    boxTexture = tm.getTextureID(teamBase,false);
  }
  if (boxTexture < 0)
    boxTexture = tm.getTextureID( BZDB.get("boxWallTexture") );

  useColorTexture[0] = boxTexture >= 0;

  int baseTopTexture = -1;

  if (o.userTextures[1].size())
    baseTopTexture = tm.getTextureID(o.userTextures[1],false);
  if (baseTopTexture < 0) {
    std::string teamBase = Team::getImagePrefix((TeamColor)o.getBaseTeam());
    teamBase += BZDB.get("baseTopTexture");
    baseTopTexture = tm.getTextureID(teamBase,false);
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
    } else {
      if (useColorTexture[1]) {  // only set the texture if we have one and are using it
	node->setTexture(baseTopTexture);
	node->setUseColorTexture(useColorTexture[1]);
      }
    }
    part++;

#ifdef SHELL_INSIDE_NODES
    const bool ownTheNode = db->addStaticNode(node, true);
    EighthDimShellNode* inode = new EighthDimShellNode(node, ownTheNode);
    o.addInsideSceneNode(inode);
#else
    db->addStaticNode(node, false);
#endif // SHELL_INSIDE_NODES
  }

#ifndef SHELL_INSIDE_NODES
  // add the inside node
  SceneNode* inode = new
    EighthDBaseSceneNode(o.getPosition(), o.getSize(), o.getRotation());
  o.addInsideSceneNode(inode);
#endif // SHELL_INSIDE_NODES

  delete nodeGen;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
