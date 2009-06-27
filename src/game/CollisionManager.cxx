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
#include "CollisionManager.h"

/* system implementation headers */
#include <vector>
#include <math.h>
#include <stdlib.h>

/* common implementation headers */
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "Intersect.h"
#include "Ray.h"
#include "ObstacleMgr.h"
#include "Obstacle.h"
#include "MeshObstacle.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "BaseBuilding.h"
#include "TimeKeeper.h"


/* static variables */

CollisionManager COLLISIONMGR; // the big dog

static const float testFudge = 0.1f;

static int maxDepth = 0;
static int minElements = 0;

static int leafNodes = 0;
static int totalNodes = 0;
static int totalElements = 0;

static ObsList		FullList;  // the complete list of obstacles
static SplitObsList	SplitList; // the complete split list of obstacles
static ObsList		FullPad;   // for returning a full list of obstacles
static SplitObsList	SplitPad;  // for returning a split list of obstacles

static ColDetNodeList	RayList;   // for returning a list a ray hit nodes

static ObsList		EmptyList = { 0, NULL };
static ColDetNodeList	EmptyNodeList = { 0, NULL };


/* static functions */

inline static void addToFullList(Obstacle* obs)
{
  obs->collisionState = false;
  FullList.list[FullList.count] = obs;
  FullList.count++;
  return;
}

inline static void addToFullPadList(Obstacle* obs)
{
  FullPad.list[FullPad.count] = obs;
  FullPad.count++;
  return;
}

inline static void addToRayList(ColDetNode* node)
{
  RayList.list[RayList.count] = node;
  RayList.count++;
  return;
}

static void squeezeChildren(ColDetNode** children)
{
  for (int dst = 0; dst < 8; dst++) {
    if (children[dst] == NULL) {
      // replace with the next non-NULL
      for (int src = (dst + 1); src < 8; src++) {
	if (children[src] != NULL) {
	  children[dst] = children[src];
	  children[src] = NULL;
	  break;
	}
      }
    }
  }
  return;
}

//////////////////////////////////////////////////////////////////////////////
//
// CollisionManager
//

CollisionManager::CollisionManager()
{
  root = NULL;
  FullPad.list = NULL;
  FullList.list = NULL;
  RayList.list = NULL;
  clear();
}


CollisionManager::~CollisionManager()
{
  clear();
}


void CollisionManager::clear()
{
  delete root;
  root = NULL;
  loaded = false;

  worldSize = 0.0f;

  leafNodes = 0;
  totalNodes = 0;
  totalElements = 0;

  delete[] FullPad.list;
  delete[] FullList.list;
  delete[] RayList.list;
  FullPad.list = NULL;
  FullPad.count = 0;
  FullList.list = NULL;
  FullList.count = 0;
  RayList.list = NULL;
  RayList.count = 0;

  for (int i = 0; i < 5; i++) {
    SplitPad.array[i].list = NULL;
    SplitPad.array[i].count = 0;
    SplitList.array[i].list = NULL;
    SplitList.array[i].count = 0;
  }

  return;
}


bool CollisionManager::needReload() const
{
  int newDepth = BZDB.evalInt(StateDatabase::BZDB_COLDETDEPTH);
  int newElements = BZDB.evalInt(StateDatabase::BZDB_COLDETELEMENTS);
  float newWorldSize = BZDBCache::worldSize;
  if ((newDepth != maxDepth) || (newElements != minElements) ||
      (newWorldSize != worldSize)) {
    return true;
  } else {
    return false;
  }
}


const ObsList* CollisionManager::axisBoxTest(const Extents& exts)
{
  if (root == NULL) {
    return &EmptyList;
  }

  FullPad.count = 0;

  // get the list
  root->axisBoxTest(exts);

  // clear the collisionState on the obstacles
  for (int i = 0; i < FullPad.count; i++) {
    FullPad.list[i]->collisionState = false;
  }

  return &FullPad;
}


const ObsList* CollisionManager::cylinderTest(const fvec3& pos,
                                              float radius, float height) const
{
  if (root == NULL) {
    return &EmptyList;
  }

  fvec3 tmpMins, tmpMaxs;
  tmpMins.x = pos.x - radius;
  tmpMins.y = pos.y - radius;
  tmpMins.z = pos.z;
  tmpMaxs.x = pos.x + radius;
  tmpMaxs.y = pos.y + radius;
  tmpMaxs.z = pos.z + height;

  FullPad.count = 0;

  // get the list
  Extents exts;
  exts.set(tmpMins, tmpMaxs);
  root->axisBoxTest(exts);

  // clear the collisionState on the obstacles
  for (int i = 0; i < FullPad.count; i++) {
    FullPad.list[i]->collisionState = false;
  }

  return &FullPad;
}


const ObsList* CollisionManager::boxTest(const fvec3& pos, float /*angle*/,
                                         float dx, float dy, float dz) const
{
  float radius = sqrtf(dx*dx + dy*dy);
  return cylinderTest(pos, radius, dz);
}


const ObsList* CollisionManager::movingBoxTest(
				  const fvec3& oldPos, float /*oldAngle*/,
				  const fvec3& pos, float /*angle*/,
				  float dx, float dy, float dz) const
{
  fvec3 newpos = pos;

  // adjust the Z parameters for the motion
  if (oldPos.z < pos.z) {
    newpos.z = oldPos.z;
    dz = dz + (pos.z - oldPos.z);
  } else {
    dz = dz + (oldPos.z - pos.z);
  }

  float radius = sqrtf(dx*dx + dy*dy);
  return cylinderTest(newpos, radius, dz);
}


const ObsList* CollisionManager::rayTest(const Ray* ray, float timeLeft) const
{
  if (root == NULL) {
    return &EmptyList;
  }

  FullPad.count = 0;
  RayList.count = 0;

  // get the list
  root->rayTest(ray, timeLeft + 0.1f);

  // clear the collisionState on the obstacles
  for (int i = 0; i < FullPad.count; i++) {
    FullPad.list[i]->collisionState = false;
  }

  return &FullPad;
}


static int compareRayNodes(const void *a, const void *b)
{
  const ColDetNode* nodeA = *((ColDetNode**)a);
  const ColDetNode* nodeB = *((ColDetNode**)b);
  return (nodeA->getInTime() > nodeB->getInTime());
}


const ColDetNodeList* CollisionManager::rayTestNodes(const Ray* ray,
                                                     float timeLeft) const
{
  if (root == NULL) {
    return &EmptyNodeList;
  }

  RayList.count = 0;

  // get the list
  root->rayTestNodes(ray, timeLeft + 0.1f);

  // sort the list of node
  qsort(RayList.list, RayList.count, sizeof(ColDetNode*), compareRayNodes);

  return &RayList;
}


void CollisionManager::load()
{
  int i;

  TimeKeeper startTime = TimeKeeper::getCurrent();

  // get the lists
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  const ObstacleList& pyrs = OBSTACLEMGR.getPyrs();
  const ObstacleList& bases = OBSTACLEMGR.getBases();
  const int boxCount = (int)boxes.size();
  const int pyrCount = (int)pyrs.size();
  const int baseCount = (int)bases.size();
  const int meshCount = (int)meshes.size();

  // clean out the cell lists
  clear();

  // setup the octree parameters
  worldSize = BZDBCache::worldSize;
  maxDepth = BZDB.evalInt(StateDatabase::BZDB_COLDETDEPTH);
  minElements = BZDB.evalInt(StateDatabase::BZDB_COLDETELEMENTS);

  // determine the total number of obstacles
  int fullCount = 0;
  for (i = 0; i < boxCount; i++) {
    if (!boxes[i]->isPassable()) { fullCount++; }
  }
  for (i = 0; i < pyrCount; i++) {
    if (!pyrs[i]->isPassable()) { fullCount++; }
  }
  for (i = 0; i < baseCount; i++) {
    if (!bases[i]->isPassable()) { fullCount++; }
  }
  for (i = 0; i < meshCount; i++) {
    MeshObstacle* mesh = (MeshObstacle*) meshes[i];
    if (!mesh->isPassable()) {
      for (int f = 0; f < mesh->getFaceCount(); f++) {
	MeshFace* face = (MeshFace*) mesh->getFace(f);
	if (!face->isPassable() || face->isLinkSrc()) {
	  fullCount++;
        }
      }
      fullCount++; // one for the mesh itself
    }
  }

  // get the memory for the full list and the scratch pad
  FullPad.list = new Obstacle*[fullCount];
  FullList.list = new Obstacle*[fullCount];
  FullList.count = 0;

  //
  // add everything to the full list
  //
  // they should have been sorted from top to bottom by the
  // ObstacleMgr, but we'll want the non-mesh obstacles sorted
  // from bottom to top, so insert them in reverse order to
  // speed of the sorting.
  //

  for (i = (boxCount - 1); i >= 0; i--) {
    if (!boxes[i]->isPassable()) { addToFullList(boxes[i]); }
  }
  for (i = (pyrCount - 1); i >= 0; i--) {
    if (!pyrs[i]->isPassable())  { addToFullList(pyrs[i]); }
  }
  for (i = (baseCount - 1); i >= 0; i--) {
    if (!bases[i]->isPassable()) { addToFullList(bases[i]); }
  }
  // add the mesh types last (faces then meshes)
  for (i = (meshCount - 1); i >= 0; i--) {
    MeshObstacle* mesh = (MeshObstacle*) meshes[i];
    if (!mesh->isPassable()) {
      const int meshFaceCount = mesh->getFaceCount();
      for (int f = 0; f < meshFaceCount; f++) {
	MeshFace* face = (MeshFace*) mesh->getFace(f);
	if (!face->isPassable() || face->isLinkSrc()) {
	  addToFullList((Obstacle*) face);
        }
      }
    }
  }
  for (i = (meshCount - 1); i >= 0; i--) {
    if (!meshes[i]->isPassable()) { addToFullList(meshes[i]); }
  }

  // do the type/height sort
  qsort(FullList.list, FullList.count, sizeof(Obstacle*), compareObstacles);

  // generate the octree
  setExtents (&FullList);
  root = new ColDetNode(0, gridExtents, &FullList);

  // tally the stats
  leafNodes = 0;
  totalNodes = 0;
  totalElements = 0;
  root->tallyStats();

  // setup the ray list
  RayList.list = new ColDetNode*[leafNodes];
  RayList.count = 0;

  // print some statistics
  logDebugMessage(2,"ColDet Octree obstacles = %i\n", FullList.count);
  for (i = 0; i < 3; i++) {
    logDebugMessage(2,"  grid extent[%i] = %f, %f\n", i, gridExtents.mins[i],
					       gridExtents.maxs[i]);
  }
  for (i = 0; i < 3; i++) {
    logDebugMessage(2,"  world extent[%i] = %f, %f\n", i, worldExtents.mins[i],
						worldExtents.maxs[i]);
  }
  logDebugMessage(2,"ColDet Octree leaf nodes  = %i\n", leafNodes);
  logDebugMessage(2,"ColDet Octree total nodes = %i\n", totalNodes);
  logDebugMessage(2,"ColDet Octree total elements = %i\n", totalElements);

  // print the timing info
  float elapsed = (float)(TimeKeeper::getCurrent() - startTime);
  logDebugMessage(2,"Collision Octree processed in %.3f seconds.\n", elapsed);


  // setup the split list
  // FIXME: currently unused, untested, and incorrect
  Obstacle** listPtr = FullList.list;
  SplitList.named.boxes.list = listPtr;
  SplitList.named.boxes.count = (int)boxes.size();
  listPtr = listPtr + boxes.size();
  SplitList.named.bases.list = listPtr;
  SplitList.named.bases.count = (int)bases.size();
  listPtr = listPtr + bases.size();
  SplitList.named.pyrs.list = listPtr;
  SplitList.named.pyrs.count = (int)pyrs.size();
  listPtr = listPtr + pyrs.size();

  loaded = true;

  return;
}


void CollisionManager::setExtents(ObsList *list)
{
  int i;
  worldExtents.reset();

  // find the real world extents
  for (i = 0; i < list->count; i++) {
    const Obstacle* obs = list->list[i];
    const Extents& obsExts = obs->getExtents();
    worldExtents.expandToBox(obsExts);
  }

  // find the longest axis
  float width = -MAXFLOAT;
  for (i = 0; i < 3; i++) {
    const float axisWidth = worldExtents.getWidth(i);
    if (axisWidth > width) {
      width = axisWidth;
    }
  }

  gridExtents = worldExtents;

  // make it a cube, with Z on its minimum
  for (i = 0; i < 2; i++) {
    const float axisWidth = worldExtents.getWidth(i);
    if (axisWidth < width) {
      const float adjust = 0.5f * (width - axisWidth);
      gridExtents.mins[i] = gridExtents.mins[i] - adjust;
      gridExtents.maxs[i] = gridExtents.maxs[i] + adjust;
    }
  }
  gridExtents.maxs.z = gridExtents.mins.z + width;

  return;
}


void CollisionManager::draw(DrawLinesFunc drawLinesFunc)
{
  if (root != NULL) {
    root->draw(drawLinesFunc);
  }
  return;
}


//////////////////////////////////////////////////////////////////////////////
//
// ColDetNode
//

ColDetNode::ColDetNode(unsigned char _depth,
		       const Extents& exts,
		       ObsList *_list)
{
  int i;

  depth = _depth;

  for (i = 0; i < 8; i++) {
    children[i] = NULL;
  }
  childCount = 0;

  // alloacte enough room for the incoming list
  const int listBytes = _list->count * sizeof(Obstacle*);
  fullList.list = (Obstacle**) malloc(listBytes);

  // copy the extents, and make the testing extents with margin
  extents = exts;
  Extents testExts;
  testExts = exts;
  testExts.addMargin(testFudge);

  // setup some test parameters
  fvec3 pos;
  pos.x = 0.5f * (testExts.maxs.x + testExts.mins.x);
  pos.y = 0.5f * (testExts.maxs.y + testExts.mins.y);
  pos.z = testExts.mins.z;
  fvec3 size;
  size.x = 0.5f * (testExts.maxs.x - testExts.mins.x);
  size.y = 0.5f * (testExts.maxs.y - testExts.mins.y);
  size.z = (testExts.maxs.z - testExts.mins.z);
  fvec3 point;
  point.x = pos.x;
  point.y = pos.y;
  point.z = 0.5f * (testExts.maxs.z + testExts.mins.z);

  // find all of the intersecting nodes
  //
  // the mesh obstacles will be the last in the list, so we
  // record whether or not any of their faces have intersected
  // with cell along the way (in the collisionState variable,
  // which must be cleared before leaving).
  //
  fullList.count = 0;
  for (i = 0; i < _list->count; i++) {
    Obstacle* obs = _list->list[i];
    const ObstacleType obsType = obs->getTypeID();
    if (testExts.touches(obs->getExtents())) {
      if (obsType != meshType) {
	if (obs->inBox(pos, 0.0f, size.x, size.y, size.z)) {
	  // add this obstacle to the list
	  fullList.list[fullList.count] = obs;
	  fullList.count++;
	  if (obsType == faceType) {
	    // record this face's hit in its mesh obstacle
	    MeshFace* face = (MeshFace*) obs;
	    MeshObstacle* mesh = face->getMesh();
	    if (mesh != NULL) {
	      mesh->collisionState = true;
	    }
	  }
	}
      } else {
	MeshObstacle* mesh = (MeshObstacle*) obs;
	if (mesh->collisionState) {
	  fullList.list[fullList.count] = (Obstacle*) mesh;
	  fullList.count++;
	  mesh->collisionState = false;
	}
	else if (mesh->containsPointNoOctree(point)) {
	  fullList.list[fullList.count] = (Obstacle*) mesh;
	  fullList.count++;
	}
      }
    }
  }

  // count will remain as the total numbers of
  // scene nodes that intersect with this cell
  count = fullList.count;

  // resize the list to save space
  fullList.list = (Obstacle**) realloc(fullList.list,
                                       fullList.count * sizeof(Obstacle*));

  // return if this is a leaf node
  if (((int)depth >= maxDepth) || (fullList.count <= minElements)) {
    resizeCell();
    //logDebugMessage(4,"COLDET LEAF NODE: depth = %d, items = %i\n", depth, count);
    return;
  }

  // sow the seeds
  depth++;
  makeChildren();

  // non NULLs first
  squeezeChildren(children);

  // resize this branch cell
  resizeCell();

  // clear the list
  fullList.count = 0;
  free(fullList.list);
  fullList.list = NULL;

  //logDebugMessage(4,"COLDET BRANCH NODE: depth = %d, children = %i\n", depth, childCount);

  return;
}


ColDetNode::~ColDetNode()
{
  for (int i = 0; i < 8; i++) {
    delete children[i];
  }
  free (fullList.list);
  return;
}


void ColDetNode::makeChildren()
{
  int side[3];    // the axis sides  (0 or 1)
  Extents exts;
  childCount = 0;
  const fvec3 center = 0.5f * (extents.mins + extents.maxs);
  const fvec3* extentSet[3] = { &extents.mins, &center, &extents.maxs };

  for (side[0] = 0; side[0] < 2; side[0]++) {
    for (side[1] = 0; side[1] < 2; side[1]++) {
      for (side[2] = 0; side[2] < 2; side[2]++) {

	for (int a = 0; a < 3; a++) {
	  exts.mins[a] = (*extentSet[side[a] + 0])[a];
	  exts.maxs[a] = (*extentSet[side[a] + 1])[a];
	}

	int kid = side[0] + (2 * side[1]) + (4 * side[2]);

	children[kid] = new ColDetNode(depth, exts, &fullList);

	if (children[kid]->getCount() == 0) {
	  delete children[kid];
	  children[kid] = NULL;
	}
	else {
	  childCount++;
	}
      }
    }
  }

  return;
}


void ColDetNode::resizeCell()
{
  int i;
  Extents absExts;

  if (childCount > 0) {
    // use lower children's extents
    for (i = 0; i < childCount; i++) {
      absExts.expandToBox(children[i]->getExtents());
    }
  } else {
    // check all of the obstacles
    for (i = 0; i < fullList.count; i++) {
      const Obstacle* obs = fullList.list[i];
      absExts.expandToBox(obs->getExtents());
    }
  }

  for (i = 0; i < 3; i++) {
    if (absExts.mins[i] > extents.mins[i]) {
      extents.mins[i] = absExts.mins[i];
    }
    if (absExts.maxs[i] < extents.maxs[i]) {
      extents.maxs[i] = absExts.maxs[i];
    }
  }

  return;
}


void ColDetNode::axisBoxTest(const Extents& exts) const
{
  int i;

  if (!extents.touches(exts)) {
    return;
  }

  if (childCount == 0) {
    for (i = 0; i < fullList.count; i++) {
      Obstacle* obs = fullList.list[i];
      if (obs->collisionState == false) {
	obs->collisionState = true;
	addToFullPadList(obs);
      }
    }
  }
  else {
    for (i = 0; i < childCount; i++) {
      children[i]->axisBoxTest(exts);
    }
  }

  return;
}


void ColDetNode::boxTest(const fvec3& pos, float angle,
                         float dx, float dy, float dz) const
{
  int i;

/* FIXME
  if ((_maxs.x < mins.x) || (_mins.x > maxs.x) ||
      (_maxs.y < mins.y) || (_mins.y > maxs.y) ||
      (_maxs.z < mins.z) || (_mins.z > maxs.z)) {
    return;
  }
*/

  if (childCount == 0) {
    for (i = 0; i < fullList.count; i++) {
      Obstacle* obs = fullList.list[i];
      if (obs->collisionState == false) {
	obs->collisionState = true;
	addToFullPadList(obs);
      }
    }
  }
  else {
    for (i = 0; i < childCount; i++) {
      children[i]->boxTest(pos, angle, dx, dy, dz);
    }
  }

  return;
}


void ColDetNode::rayTest(const Ray* ray, float timeLeft) const
{
  if (!Intersect::testRayHitsAxisBox(ray, extents, &inTime) ||
      (inTime > timeLeft)) {
    return;
  }

  if (childCount == 0) {
    for (int i = 0; i < fullList.count; i++) {
      Obstacle* obs = fullList.list[i];
      if (obs->collisionState == false) {
	obs->collisionState = true;
	addToFullPadList(obs);
      }
    }
  }
  else {
    for (int i = 0; i < childCount; i++) {
      children[i]->rayTest(ray, timeLeft);
    }
  }

  return;
}


void ColDetNode::rayTestNodes(const Ray* ray, float timeLeft) const
{
  if (!Intersect::testRayHitsAxisBox(ray, extents, &inTime, &outTime) ||
      (inTime > timeLeft)) {
    return;
  }

  if (childCount == 0) {
    addToRayList((ColDetNode*)this);
  }
  else {
    for (int i = 0; i < childCount; i++) {
      children[i]->rayTestNodes(ray, timeLeft);
    }
  }

  return;
}


/*
void ColDetNode::boxTestSplit(const fvec3& pos, float angle,
                              float dx, float dy, float dz) const
{
  pos = pos;
  angle = dx =dy =dz;
  return;
}
*/


void ColDetNode::tallyStats()
{
  totalNodes++;
  totalElements += fullList.count;

  if (childCount > 0) {
    for (int i = 0; i < childCount; i++) {
      children[i]->tallyStats();
    }
  } else {
    leafNodes++;
  }

  return;
}


void ColDetNode::draw(DrawLinesFunc drawLinesFunc)
{
  int x, y, z, c;
  fvec3 points[5];
  const fvec3* exts[2] = { &extents.mins, &extents.maxs };

  // pick a color
  int hasMeshObs = 0;
  int hasNormalObs = 0;
  for (x = 0; x < fullList.count; x++) {
    switch (fullList.list[x]->getTypeID()) {
      case meshType:
      case faceType: {
        hasMeshObs = 1;
        break;
      }
      default: {
        hasNormalObs = 1;
        break;
      }
    }
  }
  int color = hasNormalObs + (2 * hasMeshObs);

  // draw Z-normal squares
  for (z = 0; z < 2; z++) {
    for (c = 0; c < 4; c++) {
      x = ((c + 0) % 4) / 2;
      y = ((c + 1) % 4) / 2;
      points[c].x = exts[x]->x;
      points[c].y = exts[y]->y;
      points[c].z = exts[z]->z;
    }
    points[4] = points[0];
    drawLinesFunc(5, points, color);
  }

  // draw the corner edges
  for (c = 0; c < 4; c++) {
    x = ((c + 0) % 4) / 2;
    y = ((c + 1) % 4) / 2;
    for (z = 0; z < 2; z++) {
      points[z].x = exts[x]->x;
      points[z].y = exts[y]->y;
      points[z].z = exts[z]->z;
    }
    drawLinesFunc(2, points, color);
  }

  // draw the kids
  for (c = 0; c < childCount; c++) {
    children[c]->draw(drawLinesFunc);
  }

  return;
}


inline int compareHeights(const Obstacle*& obsA, const Obstacle* obsB)
{
  const Extents& eA = obsA->getExtents();
  const Extents& eB = obsB->getExtents();
  if (eA.maxs.z > eB.maxs.z) {
    return -1;
  } else {
    return +1;
  }
}

int compareObstacles(const void* a, const void* b)
{
  // - normal object come first (from lowest to highest)
  // - then come the mesh face (highest to lowest)
  // - and finally, the mesh objects (checkpoints really)
  const Obstacle* obsA = *((const Obstacle**)a);
  const Obstacle* obsB = *((const Obstacle**)b);
  const ObstacleType typeA = obsA->getTypeID();
  const ObstacleType typeB = obsB->getTypeID();

  bool isMeshA = (typeA == meshType);
  bool isMeshB = (typeB == meshType);

  if (isMeshA) {
    if (!isMeshB) {
      return +1;
    } else {
      return compareHeights(obsA, obsB);
    }
  }

  if (isMeshB) {
    if (!isMeshA) {
      return -1;
    } else {
      return compareHeights(obsA, obsB);
    }
  }

  bool isFaceA = (typeA == faceType);
  bool isFaceB = (typeB == faceType);

  if (isFaceA) {
    if (!isFaceB) {
      return +1;
    } else {
      return compareHeights(obsA, obsB);
    }
  }

  if (isFaceB) {
    if (!isFaceA) {
      return -1;
    } else {
      return compareHeights(obsA, obsB);
    }
  }

  return compareHeights(obsB, obsA); // reversed
}

int compareHitNormal(const void* a, const void* b)
{
  const MeshFace* faceA = *((const MeshFace**) a);
  const MeshFace* faceB = *((const MeshFace**) b);

  // Up Planes come first
  if (faceA->isUpPlane() && !faceB->isUpPlane()) {
    return -1;
  }
  if (faceB->isUpPlane() && !faceA->isUpPlane()) {
    return +1;
  }

  // highest Up Plane comes first
  if (faceA->isUpPlane() && faceB->isUpPlane()) {
    if (faceA->getPosition().z > faceB->getPosition().z) {
      return -1;
    } else {
      return +1;
    }
  }

  // compare the dot products
  if (faceA->scratchPad < faceB->scratchPad) {
    return -1;
  } else {
    return +1;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
