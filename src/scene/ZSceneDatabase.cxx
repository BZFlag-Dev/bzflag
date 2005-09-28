/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// system headers
#include <string.h>
#include <stdlib.h>

// implementation header
#include "ZSceneDatabase.h"

// common headers
#include "SceneNode.h"
#include "WallSceneNode.h"
#include "SphereSceneNode.h"
#include "SceneRenderer.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "TimeKeeper.h"
#include "Intersect.h"
#include "Extents.h"

// local headers
#include "Octree.h"


// utility function
static int compareZExtents(const void* a, const void* b)
{
  const SceneNode* nodeA = *((const SceneNode**)a);
  const SceneNode* nodeB = *((const SceneNode**)b);
  const Extents& eA = nodeA->getExtents();
  const Extents& eB = nodeB->getExtents();
  if (eA.maxs[2] > eB.maxs[2]) {
    return +1;
  }
  else if (eB.maxs[2] > eA.maxs[2]) {
    return -1;
  }
  else {
    return 0;
  }
}


ZSceneDatabase::ZSceneDatabase()
{
  staticCount = staticSize = 0;
  dynamicCount = dynamicSize = 0;
  staticList = dynamicList = culledList = NULL;
  culledCount = 0;
  cullDepth = cullElements = 0;
  octree = NULL;
  return;
}


ZSceneDatabase::~ZSceneDatabase()
{
  // free static nodes
  for (int i = 0; i < staticCount; i++) {
    delete staticList[i];
  }

  // free lists
  delete[] staticList;
  delete[] dynamicList;
  if (culledList != staticList) {
    delete[] culledList;
  }

  // delete the octree
  delete octree;
}


void ZSceneDatabase::finalizeStatics()
{
  return;
}


bool ZSceneDatabase::addStaticNode(SceneNode* object, bool /*dontFree*/)
{
  if (staticCount == staticSize) {
    if (staticSize == 0) staticSize = 15;
    else staticSize += staticSize + 1;
    SceneNode** newStatic = new SceneNode*[staticSize];
    if (staticList)
      ::memcpy(newStatic, staticList, staticCount * sizeof(SceneNode*));
    delete[] staticList;
    staticList = newStatic;
  }
  staticList[staticCount++] = object;
  return false; // node would not be freed
}


void ZSceneDatabase::addDynamicNode(SceneNode* object)
{
  object->notifyStyleChange();
  if (dynamicCount == dynamicSize) {
    if (dynamicSize == 0) dynamicSize = 15;
    else dynamicSize += dynamicSize + 1;
    SceneNode** newDynamic = new SceneNode*[dynamicSize];
    if (dynamicList)
      ::memcpy(newDynamic, dynamicList,  dynamicCount * sizeof(SceneNode*));
    delete[] dynamicList;
    dynamicList = newDynamic;
  }
  dynamicList[dynamicCount++] = object;
}


void ZSceneDatabase::addDynamicSphere(SphereSceneNode* n)
{
  // just add sphere -- don't need to break it up for hidden surfaces
  addDynamicNode(n);
}


void ZSceneDatabase::removeDynamicNodes()
{
  dynamicCount = 0;
}


void ZSceneDatabase::removeAllNodes()
{
  staticCount = 0;
  dynamicCount = 0;
}


bool ZSceneDatabase::isOrdered()
{
  return false;
}


const Extents* ZSceneDatabase::getVisualExtents() const
{
  if (octree) {
    return octree->getVisualExtents();
  } else {
    return NULL;
  }
}


void ZSceneDatabase::updateNodeStyles()
{
  for (int i = 0; i < staticCount; i++) {
    staticList[i]->notifyStyleChange();
  }
  return;
}


void ZSceneDatabase::setOccluderManager(int occl)
{
  if (octree) {
    octree->setOccluderManager(occl);
  }
  return;
}


void ZSceneDatabase::setupCullList()
{
  const int currentDepth = BZDB.evalInt(StateDatabase::BZDB_CULLDEPTH);
  const int currentElements = BZDB.evalInt(StateDatabase::BZDB_CULLELEMENTS);

  if ((culledList == NULL) ||
      (currentDepth != cullDepth) || (currentElements != cullElements)) {

    cullDepth = currentDepth;
    cullElements = currentElements;

    delete octree;
    octree = NULL;

    if (cullDepth > 0) {
      makeCuller();
    }
    else {
      if (culledList != staticList) {
	delete culledList;
      }
      culledList = staticList;
      culledCount = staticCount;
    }
  }

  return;
}


void ZSceneDatabase::makeCuller()
{
  delete octree;
  octree = new Octree;

  TimeKeeper startTime = TimeKeeper::getCurrent();

  // sorted from lowest to highest
  qsort(staticList, staticCount, sizeof(SceneNode*), compareZExtents);

  // make the tree
  octree->addNodes (staticList, staticCount, cullDepth, cullElements);

  float elapsed = (float)(TimeKeeper::getCurrent() - startTime);
  DEBUG2 ("SceneNode Octree processed in %.3f seconds.\n", elapsed);

  if (culledList != staticList) {
    delete culledList;
  }

  // make scratch pad for the culler
  culledList = new SceneNode*[staticCount];
}


void ZSceneDatabase::addLights(SceneRenderer& renderer)
{
  // add the lights from the dynamic nodes
  for (int i = 0; i < dynamicCount; i++) {
    dynamicList[i]->addLight(renderer);
  }

  return;
}


void ZSceneDatabase::addShadowNodes(SceneRenderer& renderer)
{
  int i;
  const float* sunDir = renderer.getSunDirection();

  if (sunDir == NULL) {
    // no sun = no shadows, simple
    return;
  }

  // see if we need an octree, or if it needs to be rebuilt
  setupCullList();

  // cull if we're supposed to
  if (octree) {
    const ViewFrustum& vf = renderer.getViewFrustum();
    const Frustum* frustum = (const Frustum*) &vf;
    culledCount = octree->getShadowList (culledList, staticCount,
					 frustum, sunDir);
  }

  // add the static nodes
  for (i = 0; i < culledCount; i++) {
    SceneNode* node = culledList[i];
    node->addShadowNodes(renderer);

    // clear the state
    node->octreeState = SceneNode::OctreeCulled;
  }

  // add the dynamic nodes
  for (i = 0; i < dynamicCount; i++) {
    dynamicList[i]->addShadowNodes(renderer);
  }

  return;
}


void ZSceneDatabase::renderRadarNodes(const ViewFrustum& vf)
{
  // see if we need an octree, or if it needs to be rebuilt
  setupCullList();

  // cull if we're supposed to
  if (octree) {
    const Frustum* f = (const Frustum *) &vf;
    culledCount = octree->getRadarList (culledList, staticCount, f);

    // sort based on heights
    if (BZDBCache::radarStyle == 2) {
      qsort(culledList, culledCount, sizeof(SceneNode*), compareZExtents);
    }
  }

  // render through the sceneNodes
  for (int i = 0; i < culledCount; i++) {
    SceneNode* snode = culledList[i];
    snode->renderRadar();
    snode->octreeState = SceneNode::OctreeCulled; // clear the state
  }

  return;
}


void ZSceneDatabase::addRenderNodes(SceneRenderer& renderer)
{
  int i;
  const ViewFrustum& frustum = renderer.getViewFrustum();
  const float* eye = frustum.getEye();

  // see if we need an octree, or if it needs to be rebuilt
  setupCullList();

  // cull if we're supposed to
  if (octree) {
    const Frustum* f = (const Frustum *) &frustum;
    culledCount = octree->getFrustumList (culledList, staticCount, f);
  }

  const Frustum* frustumPtr = (const Frustum *) &frustum;

  // add the static nodes
  for (i = 0; i < culledCount; i++) {
    SceneNode* node = culledList[i];

    const float* plane = node->getPlane();

    if (plane != NULL) {
      // see if our eye is behind the plane
      if (((eye[0] * plane[0]) + (eye[1] * plane[1]) + (eye[2] * plane[2]) +
	   plane[3]) <= 0.0f) {
	node->octreeState = SceneNode::OctreeCulled;
	continue;
      }
    }

    // if the Visibility culler tells us that we're
    // fully visible, then skip the extents test
    if (node->octreeState != SceneNode::OctreeVisible) {
      const Extents& exts = node->getExtents();
      if (testAxisBoxInFrustum(exts, frustumPtr) == Outside) {
	node->octreeState = SceneNode::OctreeCulled;
	continue;
      }
    }

    // add the node
    node->addRenderNodes(renderer);

    // clear the state
    node->octreeState = SceneNode::OctreeCulled;
  }

  // add the dynamic nodes
  for (i = 0; i < dynamicCount; i++) {
    SceneNode* node = dynamicList[i];
    if (!node->cull(frustum)) {
      node->addRenderNodes(renderer);
    }
  }

  return;
}


void ZSceneDatabase::drawCuller()
{
  if (octree) {
    octree->draw ();
  }
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
