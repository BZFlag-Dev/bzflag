/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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


void ObstacleModifier::init()
{
  // team
  modifyTeam = false;
  team = 0;
  // tint
  modifyColor = false;
  tint[0] = tint[1] = tint[2] = tint[3] = 1.0f;
  // phydrv
  modifyPhysicsDriver = false;
  phydrv = -1;
  // material;
  modifyMaterial = false;
  material = NULL;
  // passable bits
  driveThrough = false;
  shootThrough = false;

  return;
}


ObstacleModifier::ObstacleModifier()
{
  init();
  return;
}


ObstacleModifier::ObstacleModifier(const ObstacleModifier& obsMod,
				   const GroupInstance& grpinst)
{
  init();

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
    if (grpinst.modifyColor && obsMod.modifyColor) {
      tint[0] = grpinst.tint[0] * obsMod.tint[0];
      tint[1] = grpinst.tint[1] * obsMod.tint[1];
      tint[2] = grpinst.tint[2] * obsMod.tint[2];
      tint[3] = grpinst.tint[3] * obsMod.tint[3];
    }
    else if (obsMod.modifyColor) {
      memcpy (tint, obsMod.tint, sizeof(float[4]));
    }
    else {
      memcpy (tint, grpinst.tint, sizeof(float[4]));
    }
  }

  if (grpinst.modifyPhysicsDriver || obsMod.modifyPhysicsDriver) {
    modifyPhysicsDriver = true;
    if (obsMod.modifyPhysicsDriver) {
      phydrv = obsMod.phydrv;
    } else {
      phydrv = grpinst.phydrv;
    }
  }

  if (grpinst.modifyMaterial || obsMod.modifyMaterial) {
    modifyMaterial = true;
    if (obsMod.modifyMaterial) {
      material = obsMod.material;
    } else {
      material = grpinst.material;
    }
  }

  driveThrough = grpinst.driveThrough || obsMod.driveThrough;
  shootThrough = grpinst.shootThrough || obsMod.shootThrough;

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
  float newColor[4];
  const float* oldColor;

  // diffuse
  oldColor = mat->getDiffuse();
  newColor[0] = oldColor[0] * tint[0];
  newColor[1] = oldColor[1] * tint[1];
  newColor[2] = oldColor[2] * tint[2];
  newColor[3] = oldColor[3] * tint[3];
  tintmat.setDiffuse(newColor);
  // ambient, specular, and emission are intentionally unmodifed

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
  if (modifyMaterial) { // do this before tinting
    if (obstacle->getType() == MeshObstacle::getClassName()) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	face->bzMaterial = material;
      }
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
  if (driveThrough) {
    obstacle->driveThrough = true;
    if (obstacle->getType() == MeshObstacle::getClassName()) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	face->driveThrough = true;
      }
    }
  }
  if (shootThrough) {
    obstacle->shootThrough = true;
    if (obstacle->getType() == MeshObstacle::getClassName()) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	face->shootThrough = true;
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

