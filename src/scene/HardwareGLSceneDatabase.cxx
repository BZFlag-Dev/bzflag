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

/*
 *
 */

#include <string.h>
#include "common.h"
#include "SceneNode.h"
#include "HardwareGLSceneDatabase.h"
#include "SphereSceneNode.h"

HardwareGLSceneDatabase::HardwareGLSceneDatabase() :
				staticCount(0),
				staticSize(0),
				staticList(NULL),
				dynamicCount(0),
				dynamicSize(0),
				dynamicList(NULL)
{
  // do nothing
}

HardwareGLSceneDatabase::~HardwareGLSceneDatabase()
{
  // free static nodes
  for (int i = 0; i < staticCount; i++)
    delete staticList[i];

  // free lists
  delete[] staticList;
  delete[] dynamicList;
}

void			HardwareGLSceneDatabase::addStaticNode(SceneNode* object)
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

void			HardwareGLSceneDatabase::addDynamicNode(SceneNode* object)
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

void			HardwareGLSceneDatabase::addDynamicSphere(SphereSceneNode* n)
{
  // just add sphere -- don't need to break it up for hidden surfaces
  addDynamicNode(n);
}

void			HardwareGLSceneDatabase::removeDynamicNodes()
{
  dynamicCount = 0;
}

void			HardwareGLSceneDatabase::removeAllNodes()
{
  staticCount = 0;
  dynamicCount = 0;
}

bool			HardwareGLSceneDatabase::isOrdered()
{
  return false;
}

SceneIterator*		HardwareGLSceneDatabase::getRenderIterator()
{
  return new HardwareGLSceneIterator(this);
}

//
// ZSceneIterator
//

HardwareGLSceneIterator::HardwareGLSceneIterator(const HardwareGLSceneDatabase* _db) :
				SceneIterator(),
				db(_db)
{
  reset();
}

HardwareGLSceneIterator::~HardwareGLSceneIterator()
{
  // do nothing
}

void			HardwareGLSceneIterator::resetFrustum(const ViewFrustum*)
{
  // do nothing
}

void			HardwareGLSceneIterator::reset()
{
  staticIndex = 0;
  staticDone = (db->staticCount == staticIndex);
  dynamicIndex = 0;
  dynamicDone = (db->dynamicCount == dynamicIndex);
}

SceneNode*		HardwareGLSceneIterator::getNext()
{
  if (!staticDone) {
    SceneNode* node = db->staticList[staticIndex++];
    staticDone = (db->staticCount == staticIndex);
    return node;
  }
  if (!dynamicDone) {
    SceneNode* node = db->dynamicList[dynamicIndex++];
    dynamicDone = (db->dynamicCount == dynamicIndex);
    return node;
  }
  return NULL;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

