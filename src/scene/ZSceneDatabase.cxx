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
#include "StateDatabase.h"
#include "SceneRenderer.h"
#include "TimeKeeper.h"
#include "Intersect.h"


ZSceneDatabase::ZSceneDatabase()
{
  staticCount = staticSize = 0;
  dynamicCount = dynamicSize = 0;
  staticList = dynamicList = NULL;
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
}


void ZSceneDatabase::addStaticNode(SceneNode* object)
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

  // add the static nodes
  for (i = 0; i < staticCount; i++) {
    SceneNode* node = staticList[i];
    node->addShadowNodes(renderer);
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

  // add the static nodes
  for (i = 0; i < staticCount; i++) {
    // NOTE: you can run about 5% faster if you assume that
    //       all static nodes are WallSceneNodes. You can
    //       then use getPlaneRaw(), and skip the 'if (plane)'
    //       test.
    WallSceneNode* node = (WallSceneNode*) staticList[i];
    
    const float* plane = node->getPlane();
    if (plane) {
      if (((eye[0] * plane[0]) + (eye[1] * plane[1]) + (eye[2] * plane[2]) +
           plane[3]) <= 0.0f) {
        continue;
      }

      // if the Visibility culler tells us that we're
      // fully visible, then skip the rest of these tests
      const Frustum* f = (const Frustum *) &frustum;
      float mins[3], maxs[3];
      node->getExtents(mins, maxs);
      if (testAxisBoxInFrustum(mins, maxs, f) == Outside) {
        continue;
      }

      node->addRenderNodes(renderer);
    } 
    else {
      if (!node->cull(frustum)) {
        node->addRenderNodes(renderer);
      }
    }
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


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
