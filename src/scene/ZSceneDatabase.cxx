/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 *
 */

#include <string.h>
#include "ZSceneDatabase.h"
#include "SceneNode.h"
#include "SphereSceneNode.h"

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
  for (int i = 0; i < staticCount; i++)
    delete staticList[i];

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
				SceneIterator(),
				db(_db)
{
  reset();
}

ZSceneIterator::~ZSceneIterator()
{
  // do nothing
}

void			ZSceneIterator::resetFrustum(const ViewFrustum*)
{
  // do nothing
}

void			ZSceneIterator::reset()
{
  staticIndex = 0;
  staticDone = (db->staticCount == staticIndex);
  dynamicIndex = 0;
  dynamicDone = (db->dynamicCount == dynamicIndex);
}

SceneNode*		ZSceneIterator::getNext()
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
// ex: shiftwidth=2 tabstop=8
