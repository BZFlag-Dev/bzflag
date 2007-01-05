/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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

/* interface header */
#include "CustomSphere.h"

/* system headers */
#include <iostream>
#include <sstream>
#include <vector>

/* common implementation headers */
#include "SphereObstacle.h"
#include "PhysicsDriver.h"
#include "ObstacleMgr.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"


const char* CustomSphere::sideNames[MaterialCount] = { "edge", "bottom" };


CustomSphere::CustomSphere()
{
  divisions = 4;
  pos[2] = 10.0f;
  size[0] = size[1] = size[2] = 10.0f;
  materials[Edge].setTexture("boxwall");
  materials[Bottom].setTexture("roof");
  texsize[0] = texsize[1] = -4.0f;
  hemisphere = false;
  phydrv = -1;
  useNormals = true;
  smoothBounce = false;
  return;
}


CustomSphere::~CustomSphere()
{
  return;
}


bool CustomSphere::read(const char *cmd, std::istream& input)
{
  bool materror;

  if (strcasecmp(cmd, "divisions") == 0) {
    if (!(input >> divisions)) {
      return false;
    }
  }
  else if (strcasecmp(cmd, "radius") == 0) {
    float radius;
    if (!(input >> radius)) {
      return false;
    }
    size[0] = size[1] = size[2] = radius;
  }
  else if ((strcasecmp(cmd, "hemi") == 0) ||
	   (strcasecmp(cmd, "hemisphere") == 0)) {
    hemisphere = true;
  }
  else if (strcasecmp(cmd, "texsize") == 0) {
    if (!(input >> texsize[0] >> texsize[1])) {
      return false;
    }
  }
  else if (strcasecmp(cmd, "phydrv") == 0) {
    std::string drvname;
    if (!(input >> drvname)) {
      std::cout << "missing Physics Driver parameter" << std::endl;
      return false;
    }
    phydrv = PHYDRVMGR.findDriver(drvname);
    if ((phydrv == -1) && (drvname != "-1")) {
      std::cout << "couldn't find PhysicsDriver: " << drvname << std::endl;
    }
  }
  else if (strcasecmp(cmd, "smoothbounce") == 0) {
    smoothBounce = true;
  }
  else if (strcasecmp(cmd, "flatshading") == 0) {
    useNormals = false;
  }
  else if (parseMaterials(cmd, input, materials, MaterialCount, materror)) {
    if (materror) {
      return false;
    }
  }
  else if (parseMaterialsByName(cmd, input, materials, sideNames,
				MaterialCount, materror)) {
    if (materror) {
      return false;
    }
  }
  else {
    return WorldFileObstacle::read(cmd, input);
  }

  return true;
}


void CustomSphere::writeToGroupDef(GroupDefinition *groupdef) const
{
  int i;
  const BzMaterial* mats[MaterialCount];
  for (i = 0; i < MaterialCount; i++) {
    mats[i] = MATERIALMGR.addMaterial(&materials[i]);
  }
  SphereObstacle* sphere = new SphereObstacle(transform, pos, size, rotation, texsize,
					      useNormals, hemisphere, divisions, mats,
					      phydrv,
					      smoothBounce, driveThrough, shootThrough);

  if (sphere->isValid()) {
    groupdef->addObstacle(sphere);
  } else {
    delete sphere;
  }

  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
