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

#ifndef	BZF_CS_SCENE_DATABASE_H
#define	BZF_CS_SCENE_DATABASE_H

#include "common.h"

#include <crystalspace.h>

#include "SceneDatabase.h"

class CSSceneDatabase : public SceneDatabase {
 public:
  CSSceneDatabase();
  virtual ~CSSceneDatabase();

  bool addStaticNode(SceneNode*, bool dontFree);
  void addDynamicNode(SceneNode*) {};
  void addDynamicSphere(SphereSceneNode*) {};
  void finalizeStatics() {};
  void removeDynamicNodes() {};
  void removeAllNodes() {};
  bool isOrdered() { return true;};
  void updateNodeStyles() {};
  void addLights(SceneRenderer& renderer) {};
  void addShadowNodes(SceneRenderer &renderer) {};
  void addRenderNodes(SceneRenderer& renderer) {};
  void renderRadarNodes(const ViewFrustum&) {};
  void drawCuller() {};
 private:
  /// A pointer to the 3D engine.
  csRef<iEngine> engine;

  /// A pointer to the sector the camera will be in.
  iSector       *room;
};

#endif // BZF_CS_SCENE_DATABASE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

