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

#include <string.h>
#include "common.h"
#include "SceneNode.h"
#include "WallSceneNode.h"
#include "ZSceneDatabase.h"
#include "SphereSceneNode.h"
#include "Octree.h"
#include "StateDatabase.h"
#include "SceneRenderer.h"
#include "TimeKeeper.h"
#include "Intersect.h"


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


void ZSceneDatabase::updateNodeStyles()
{
  for (int i = 0; i < staticCount; i++) {
    staticList[i]->notifyStyleChange();
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

  octree->addNodes (staticList, staticCount, cullDepth, cullElements);

  float elapsed = TimeKeeper::getCurrent() - startTime;
  DEBUG2 ("Octree processed in %.3f seconds.\n", elapsed);

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
    node->octreeState = SceneNode::OctreeCulled;
  }

  // add the dynamic nodes
  for (i = 0; i < dynamicCount; i++) {
    dynamicList[i]->addShadowNodes(renderer);
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
    culledCount = octree->getFrustumList (
		    culledList, staticCount, (const Frustum *) &frustum);
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
      float mins[3], maxs[3];
      node->getExtents(mins, maxs);
      if (testAxisBoxInFrustum(mins, maxs, frustumPtr) == Outside) {
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
