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

/* interface header */
#include "CustomArc.h"

/* system headers */
#include <sstream>
#include <vector>

/* common implementation headers */
#include "ArcObstacle.h"
#include "PhysicsDriver.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"


const char* CustomArc::sideNames[MaterialCount] = {
  "top",
  "bottom",
  "inside",
  "outside",
  "startside",
  "endside"
};
    

CustomArc::CustomArc()
{
  // default to a (radius=10, height=10) cylinder
  divisions = 16;
  size[0] = size[1] = size[2] = 10.0f;
  ratio = 1.0f;
  angle = 360.0f;
  texsize[0] = texsize[1] = texsize[2] = texsize[3] = -4.0f;
  phydrv = -1;
  useNormals = true;
  smoothBounce = false;

  // setup the default textures
  materials[Top].setTexture("roof");
  materials[Bottom].setTexture("roof");
  materials[Inside].setTexture("boxwall");
  materials[Outside].setTexture("boxwall");
  materials[StartFace].setTexture("wall");
  materials[EndFace].setTexture("wall");

  return;
}


CustomArc::~CustomArc()
{
  return;
}


bool CustomArc::read(const char *cmd, std::istream& input)
{
  bool materror;

  if (strcasecmp(cmd, "divisions") == 0) {
    if (!(input >> divisions)) {
      return false;
    }
  }
  else if (strcasecmp(cmd, "angle") == 0) {
    if (!(input >> angle)) {
      return false;
    }
  }
  else if (strcasecmp(cmd, "ratio") == 0) {
    if (!(input >> ratio)) {
      return false;
    }
  }
  else if (strcasecmp(cmd, "texsize") == 0) {
    if (!(input >> texsize[0] >> texsize[1] >> texsize[2] >> texsize[3])) {
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
    if (phydrv == -1) {
      std::cout << "couldn't find PhysicsDriver: " << drvname << std::endl;
    }
  }
  else if ((strcasecmp(cmd, "ricosuavez") == 0) ||
           (strcasecmp(cmd, "smoothbounce") == 0)) {
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


void CustomArc::write(WorldInfo *world) const
{
  int i;
  const BzMaterial* mats[MaterialCount];
  for (i = 0; i < MaterialCount; i++) {
    mats[i] = MATERIALMGR.addMaterial(&materials[i]);
  }
  ArcObstacle* arc = new ArcObstacle(pos, size, rotation, angle, ratio,
                                     texsize, useNormals, divisions, mats,
                                     phydrv,
                                     smoothBounce, driveThrough, shootThrough);
  if (arc->isValid()) {
    arc->getMesh()->setIsLocal(true);
    world->addArc(arc);
  } else {
    delete arc;
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
