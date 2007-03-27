/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// implementation header
#include "CSSceneDatabase.h"

#include "SceneNode.h"

CSSceneDatabase::CSSceneDatabase(){
  engine = csQueryRegistry<iEngine>
    (csApplicationFramework::GetObjectRegistry());
  if (!engine)
    csApplicationFramework::ReportError("Failed to locate 3D Engine!");

  // We find the sector called "room".
  room = engine->FindSector("room");
}

CSSceneDatabase::~CSSceneDatabase() {
}

bool CSSceneDatabase::addStaticNode(SceneNode* object, bool /*dontFree*/) {
  object->addToEngine(engine, room);
  return false;
};

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
