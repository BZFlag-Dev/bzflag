/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "ObstacleModifier.h"

// system headers
#include <string.h>
#include <string>
#include <vector>
#include <map>

// common headers
#include "ObstacleMgr.h"
#include "BzMaterial.h"
#include "WorldText.h"

// obstacle headers
#include "Obstacle.h"
#include "BaseBuilding.h"
#include "MeshObstacle.h"


//============================================================================//

void ObstacleModifier::init()
{
  // team
  modifyTeam = false;
  team = 0;
  // tint
  modifyColor = false;
  tint = fvec4(0.0f, 0.0f, 0.0f, 0.0f);
  // phydrv
  modifyPhysicsDriver = false;
  phydrv = -1;
  // material;
  modifyMaterial = false;
  material = NULL;
  // passable bits
  driveThrough = 0;
  shootThrough = 0;
  ricochet = false;

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
      tint = grpinst.tint * obsMod.tint;
    }
    else if (obsMod.modifyColor) {
      tint = obsMod.tint;
    }
    else {
      tint = grpinst.tint;
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

  if (obsMod.modifyMaterial) {
    modifyMaterial = true;
    material = obsMod.material;
  }
  else if (obsMod.matMap.size() > 0) {
    if (grpinst.modifyMaterial) {
      modifyMaterial = true;
      MaterialMap::const_iterator find;
      find = obsMod.matMap.find(grpinst.material);
      if (find != obsMod.matMap.end()) {
	material = find->second;
      } else {
	material = grpinst.material;
      }
    }
    else {
      matMap = obsMod.matMap;
      MaterialMap::const_iterator it;
      for (it = grpinst.matMap.begin(); it != grpinst.matMap.end(); it++) {
	MaterialMap::const_iterator find_it;
	find_it = obsMod.matMap.find(it->second);
	if (find_it != obsMod.matMap.end()) {
	  matMap[it->first] = find_it->second;
	} else {
	  matMap[it->first] = it->second;
	}
      }
    }
  }
  else if (grpinst.modifyMaterial) {
    modifyMaterial = true;
    material = grpinst.material;
  }
  else if (grpinst.matMap.size() > 0) {
    matMap = grpinst.matMap;
  }

  driveThrough = grpinst.driveThrough | obsMod.driveThrough;
  shootThrough = grpinst.shootThrough | obsMod.shootThrough;
  ricochet     = grpinst.ricochet    || obsMod.ricochet;

  return;
}


ObstacleModifier::~ObstacleModifier()
{
  return;
}


//============================================================================//

static const BzMaterial* getTintedMaterial(const fvec4& tint,
					   const BzMaterial* mat)
{
  BzMaterial tintmat(*mat);
  // diffuse
  const fvec4& oldColor = mat->getDiffuse();
  tintmat.setDiffuse(oldColor * tint);
  // ambient, specular, and emission are intentionally unmodifed
  const BzMaterial* newmat = MATERIALMGR.addMaterial(&tintmat);
  return newmat;
}


void ObstacleModifier::execute(Obstacle* obstacle) const
{
  if (modifyTeam) {
    if (obstacle->getTypeID() == baseType) {
      BaseBuilding* base = (BaseBuilding*) obstacle;
      base->team = team;
    }
    else if (obstacle->getTypeID() == meshType) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	// only modify faces that already have a team
	if (face->isBaseFace()) {
	  face->specialData->baseTeam = team;
	}
      }
    }
  }

  if (modifyColor || modifyMaterial || (matMap.size() > 0)) {
    if (obstacle->getTypeID() == meshType) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	if (modifyMaterial) {
	  face->bzMaterial = material;
	}
	else if (matMap.size() > 0) {
	  MaterialMap::const_iterator it = matMap.find(face->bzMaterial);
	  if (it != matMap.end()) {
	    face->bzMaterial = it->second;
	  }
	}
	if (modifyColor) {
	  face->bzMaterial = getTintedMaterial(tint, face->bzMaterial);
	}
      }
    }
  }

  if (modifyPhysicsDriver) {
    if (obstacle->getTypeID() == meshType) {
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
    obstacle->driveThrough = 0xFF;
    if (obstacle->getTypeID() == meshType) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	face->driveThrough = 0xFF;
      }
    }
  }

  if (shootThrough) {
    obstacle->shootThrough = 0xFF;
    if (obstacle->getTypeID() == meshType) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	face->shootThrough = 0xFF;
      }
    }
  }

  if (ricochet) {
    obstacle->ricochet = true;
    if (obstacle->getTypeID() == meshType) {
      const MeshObstacle* mesh = (MeshObstacle*) obstacle;
      for (int i = 0; i < mesh->getFaceCount(); i++) {
	MeshFace* face = (MeshFace*) mesh->getFace(i);
	face->ricochet = true;
      }
    }
  }

  return;
}


void ObstacleModifier::execute(WorldText* text) const
{
  if (modifyMaterial) {
    text->bzMaterial = material;
  }
  else if (matMap.size() > 0) {
    MaterialMap::const_iterator it = matMap.find(text->bzMaterial);
    if (it != matMap.end()) {
      text->bzMaterial = it->second;
    }
  }
  if (modifyColor) {
    text->bzMaterial = getTintedMaterial(tint, text->bzMaterial);
  }
}


//============================================================================//

void ObstacleModifier::getMaterialMap(const MaterialSet& matSet,
				      MaterialMap& materialMap) const
{
  materialMap.clear();

  if (modifyColor || modifyMaterial || (matMap.size() > 0)) {
    MaterialSet::const_iterator it;
    for (it = matSet.begin(); it != matSet.end(); it++) {
      const BzMaterial* origMat = *it;
      const BzMaterial* convMat = *it;
      if (modifyMaterial) {
	convMat = material;
      } else if (matMap.size() > 0) {
	MaterialMap::const_iterator swap_it = matMap.find(origMat);
	if (swap_it != matMap.end()) {
	  convMat = swap_it->second;
	}
      }
      if (modifyColor) {
	convMat = getTintedMaterial(tint, convMat);
      }
      if (convMat != origMat) {
	materialMap[origMat] = convMat;
      }
    }
  }
  return;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
