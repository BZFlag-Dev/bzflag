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

/*
 *
 */

#include <string.h>
#include "common.h"
#include "SceneNode.h"
#include "ZSceneDatabase.h"
#include "SphereSceneNode.h"
#include "Octree.h"
#include "StateDatabase.h"
#include "TimeKeeper.h"

ZSceneDatabase::ZSceneDatabase() :
				staticCount(0),
				staticSize(0),
				staticList(NULL),
				dynamicCount(0),
				dynamicSize(0),
				dynamicList(NULL)
{
  // do nothing
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

void			ZSceneDatabase::addStaticNode(SceneNode* object)
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

void			ZSceneDatabase::addDynamicNode(SceneNode* object)
{
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

void			ZSceneDatabase::addDynamicSphere(SphereSceneNode* n)
{
  // just add sphere -- don't need to break it up for hidden surfaces
  addDynamicNode(n);
}

void			ZSceneDatabase::addShadowNodes(SceneRenderer& renderer)
{
  int i;
  for (i = 0; i < staticCount; i++) {
    staticList[i]->addShadowNodes(renderer);
  }
  for (i = 0; i < dynamicCount; i++) {
    dynamicList[i]->addShadowNodes(renderer);
  }
}


void			ZSceneDatabase::removeDynamicNodes()
{
  dynamicCount = 0;
}

void			ZSceneDatabase::removeAllNodes()
{
  staticCount = 0;
  dynamicCount = 0;
}

bool			ZSceneDatabase::isOrdered()
{
  return false;
}

SceneIterator*		ZSceneDatabase::getRenderIterator()
{
  return new ZSceneIterator(this);
}


//
// ZSceneIterator
//

ZSceneIterator::ZSceneIterator(const ZSceneDatabase* _db) :
				SceneIterator(), db(_db)
{
  cullDepth = BZDB.evalInt(StateDatabase::BZDB_CULLDEPTH);
  cullElements = BZDB.evalInt(StateDatabase::BZDB_CULLELEMENTS);
  octree = NULL;
  culledList = NULL;
  if (cullDepth > 0) {
    makeCuller();
  }
  else {
    culledList = db->staticList;
    culledCount = db->staticCount;
  }
  reset();
}

ZSceneIterator::~ZSceneIterator()
{
  delete octree;
  if (culledList != db->staticList) {
    delete culledList;
  }
}

void			ZSceneIterator::makeCuller()
{
  delete octree;
  octree = new Octree;

  TimeKeeper startTime = TimeKeeper::getCurrent();

  octree->addNodes (db->staticList, db->staticCount, cullDepth, cullElements);

  float elapsed = TimeKeeper::getCurrent() - startTime;
  DEBUG2 ("Octree processed in %.3f seconds.\n", elapsed);

  if (culledList != db->staticList) {
    delete culledList;
  }
  // make scratch pad for the culler
  culledList = new SceneNode*[db->staticCount];
}

void			ZSceneIterator::resetFrustum(const ViewFrustum* frustum)
{
  const int currentDepth = BZDB.evalInt(StateDatabase::BZDB_CULLDEPTH);
  const int currentElements = BZDB.evalInt(StateDatabase::BZDB_CULLELEMENTS);

  if ((currentDepth != cullDepth) || (currentElements != cullElements)) {

    cullDepth = currentDepth;
    cullElements = currentElements;

    delete octree;
    octree = NULL;

    if (cullDepth > 0) {
      makeCuller();
    }
    else {
      if (culledList != db->staticList) {
        delete culledList;
      }
      culledList = db->staticList;
      culledCount = db->staticCount;
    }
  }

  // cull if we're supposed to
  if (octree) {
    culledCount = octree->getFrustumList (culledList, db->staticCount,
                                          (const Frustum *) frustum);
  }
}

void			ZSceneIterator::reset()
{
  culledIndex = 0;
  culledDone = (culledCount == culledIndex);
  dynamicIndex = 0;
  dynamicDone = (db->dynamicCount == dynamicIndex);
}

SceneNode*		ZSceneIterator::getNext()
{
  if (!culledDone) {
    SceneNode* node = culledList[culledIndex++];
    culledDone = (culledIndex >= culledCount);
    return node;
  }
  if (!dynamicDone) {
    SceneNode* node = db->dynamicList[dynamicIndex++];
    dynamicDone = (db->dynamicCount == dynamicIndex);
    return node;
  }
  return NULL;
}

SceneNode* ZSceneIterator::getNextLight()
{
  // Currently static nodes does not own light
  if (!dynamicDone) {
    SceneNode* node = db->dynamicList[dynamicIndex++];
    dynamicDone     = (db->dynamicCount == dynamicIndex);
    return node;
  }
  return NULL;
}

void     		ZSceneIterator::drawCuller()
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
