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
#include "BzTime.h"
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
  if (eA.maxs.z > eB.maxs.z) { return +1; }
  if (eB.maxs.z > eA.maxs.z) { return -1; }
  return 0;
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
  staticList[staticCount] = object;
  staticCount++;
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
  dynamicList[dynamicCount] = object;
  dynamicCount++;
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
  static BZDB_int currentDepth(BZDBNAMES.CULLDEPTH);
  static BZDB_int currentElements(BZDBNAMES.CULLELEMENTS);

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

  BzTime startTime = BzTime::getCurrent();

  // sorted from lowest to highest
  qsort(staticList, staticCount, sizeof(SceneNode*), compareZExtents);

  // make the tree
  octree->addNodes (staticList, staticCount, cullDepth, cullElements);

  float elapsed = (float)(BzTime::getCurrent() - startTime);
  logDebugMessage(2,"SceneNode Octree processed in %.3f seconds.\n", elapsed);

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


void ZSceneDatabase::addShadowNodes(SceneRenderer& renderer,
                                    bool staticNodes,
                                    bool dynamicNodes)

{
  int i;

  if (renderer.getSunDirection() == NULL) {
    return;
  }

  // setup the shadow clipping planes
  const fvec4* planes = NULL;
  const int planeCount = renderer.getShadowPlanes(&planes);

  if (staticNodes) {
    // see if we need an octree, or if it needs to be rebuilt
    setupCullList();

    // cull if we're supposed to
    if (octree) {
      culledCount = octree->getShadowList(culledList, staticCount,
                                          planeCount, planes);
    }

    // add the static nodes
    for (i = 0; i < culledCount; i++) {
      SceneNode* node = culledList[i];
      node->addShadowNodes(renderer);

      // clear the state
      node->octreeState = SceneNode::OctreeCulled;
    }
  }

  // add the dynamic nodes
  if (dynamicNodes) {
    if (planeCount <= 0) {
      for (i = 0; i < dynamicCount; i++) {
        SceneNode* node = dynamicList[i];
        node->addShadowNodes(renderer);
      }
    }
    else {
      for (i = 0; i < dynamicCount; i++) {
        SceneNode* node = dynamicList[i];
        if (!node->cullShadow(planeCount, planes)) {
          node->addShadowNodes(renderer);
        }
      }
    }
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
    if (BZDBCache::radarStyle == SceneRenderer::FastSortedRadar) {
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


void ZSceneDatabase::addRenderNodes(SceneRenderer& renderer,
                                    bool staticNodes,
                                    bool dynamicNodes)
{
  int i;
  const ViewFrustum& frustum = renderer.getViewFrustum();
  const fvec3& eye = frustum.getEye();

  if (staticNodes) {
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

      const fvec4* plane = node->getPlane();

      if (plane != NULL) {
        // see if our eye is behind or on the plane
        if (plane->planeDist(eye) <= 0.0f) {
          node->octreeState = SceneNode::OctreeCulled;
          continue;
        }
      }

      // if the Visibility culler tells us that we're
      // fully visible, then skip the extents test
      if (node->octreeState != SceneNode::OctreeVisible) {
        const Extents& exts = node->getExtents();
        if (Intersect::testAxisBoxInFrustum(exts, frustumPtr) == Intersect::Outside) {
          node->octreeState = SceneNode::OctreeCulled;
          continue;
        }
      }

      // add the node
      node->addRenderNodes(renderer);

      // clear the state
      node->octreeState = SceneNode::OctreeCulled;
    }
  }

  // add the dynamic nodes
  if (dynamicNodes) {
    for (i = 0; i < dynamicCount; i++) {
      SceneNode* node = dynamicList[i];
      if (!node->cull(frustum)) {
        node->addRenderNodes(renderer);
      }
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
