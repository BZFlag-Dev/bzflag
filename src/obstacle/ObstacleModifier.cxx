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

#include "common.h"

// implementation header
#include "ObstacleModifier.h"

// system headers
#include <string.h>
#include <string>
#include <vector>
#include <iostream>

// common headers
#include "ObstacleMgr.h"
#include "BzMaterial.h"

// obstacle headers
#include "Obstacle.h"
#include "BaseBuilding.h"
#include "MeshObstacle.h"


ObstacleModifier::ObstacleModifier()
{
  // team
  modifyTeam = false;
  team = 0;
  // tint
  modifyColor = false;
  tint[0] = 1.0f;
  tint[1] = 1.0f;
  tint[2] = 1.0f;
  tint[3] = 1.0f;
  // phydrv
  modifyPhysicsDriver = false;
  phydrv = -1;

  return;
}


ObstacleModifier::ObstacleModifier(const ObstacleModifier& obsMod,
				   const GroupInstance& grpinst)
{
  modifyTeam = false;
  modifyColor = false;
  modifyPhysicsDriver = false;

  if (grpinst.modifyTeam || obsMod.modifyTeam) {
    modifyTeam = true;
    if (obsMod.modifyTeam) {
      team = obsMod.team;
    } else {
      team = grpinst.team;
    }
  }

  if (grpinst.modifyColor || obsMod.modifyColor) {
    modifyColor = true;
    if (grpinst.modifyColor) {
      tint[0] = grpinst.tint[0] * obsMod.tint[0];
      tint[1] = grpinst.tint[1] * obsMod.tint[1];
      tint[2] = grpinst.tint[2] * obsMod.tint[2];
      tint[3] = grpinst.tint[3] * obsMod.tint[3];
    } else {
      tint[0] = obsMod.tint[0];
      tint[1] = obsMod.tint[1];
      tint[2] = obsMod.tint[2];
      tint[3] = obsMod.tint[3];
    }
  } else {
    tint[0] = 1.0f;
    tint[1] = 1.0f;
    tint[2] = 1.0f;
    tint[3] = 1.0f;
  }

  if (grpinst.modifyPhysicsDriver || obsMod.modifyPhysicsDriver) {
    modifyPhysicsDriver = true;
    if (obsMod.modifyPhysicsDriver) {
      phydrv = obsMod.phydrv;
    } else {
      phydrv = grpinst.phydrv;
    }
  }

  return;
}


ObstacleModifier::~ObstacleModifier()
{
  return;
}


static const BzMaterial* getTintedMaterial(const float tint[4],
					   const BzMaterial* mat)
{
  BzMaterial tintmat(*mat);
  float color[4];
  color[0] = mat->getDiffuse()[0] * tint[0];
  color[1] = mat->getDiffuse()[1] * tint[1];
  color[2] = mat->getDiffuse()[2] * tint[2];
  color[3] = mat->getDiffuse()[3] * tint[3];
  tintmat.setDiffuse(color);
  const BzMaterial* newmat = MATERIALMGR.addMaterial(&tintmat);
  return newmat;
}


void ObstacleModifier::execute(Obstacle* obstacle) const
{
  if (modifyTeam) {
    if (obstacle->getType() == BaseBuilding::getClassName()) {
      BaseBuilding* base = (BaseBuilding*) obstacle;
      base->team = team;
    }
  }
  if (modifyColor) {
    if (obstacle->getType() == MeshObstacle::getClassName()) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	face->bzMaterial = getTintedMaterial(tint, face->bzMaterial);
      }
    }
  }
  if (modifyPhysicsDriver) {
    if (obstacle->getType() == MeshObstacle::getClassName()) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	// only modify faces that already have a physics driver
	if (face->phydrv >= 0) {
	  face->phydrv = phydrv;
	}
      }
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

