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
#include "CustomCone.h"

/* system headers */
#include <sstream>
#include <vector>
#include "math.h"

/* common implementation headers */
#include "ConeObstacle.h"
#include "vectors.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"


CustomCone::CustomCone()
{
  // default to a (radius=10, height=10) cylinder
  divisions = 16;
  size[0] = size[1] = size[2] = 10.0f;
  texsize[0] = texsize[1] = -4.0f;
  angle = 360.0f;
  useNormals = true;
  smoothBounce = false;

  // setup the default textures
  materials[Edge].setTexture("boxwall");
  materials[Bottom].setTexture("roof");
  materials[StartFace].setTexture("wall");
  materials[EndFace].setTexture("wall");

  return;
}


CustomCone::~CustomCone()
{
  return;
}


bool CustomCone::read(const char *cmd, std::istream& input)
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
  else if (strcasecmp(cmd, "texsize") == 0) {
    if (!(input >> texsize[0] >> texsize[1])) {
      return false;
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
  else if (parseSideMaterials(cmd, input, materror)) {
    if (materror) {
      return false;
    }
  }
  else {
    return WorldFileObstacle::read(cmd, input);
  }

  return true;
}


bool CustomCone::parseSideMaterials(const char* cmd, std::istream& input,
                                   bool& error)
{
  const char* sideNames[MaterialCount] = 
    { "edge", "bottom", "startside", "endside" };

  error = false;

  for (int n = 0; n < MaterialCount; n++) {
    if (strcasecmp (cmd, sideNames[n]) == 0) {
      std::string line, matcmd;
      std::getline(input, line);
      std::istringstream parms(line);
      if (!(parms >> matcmd)) {
        error = true;
      } else {
        // put the material command string back into the stream
        for (int i = 0; i < (int)(line.size() - matcmd.size()); i++) {
          input.putback(line[line.size() - i]);
        }
        if (!parseMaterials(matcmd.c_str(), input, &materials[n], 1, error)) {
          error = true;
        }
      }
      input.putback('\n');
      return true;
    }
  }

  return false;
}


void CustomCone::write(WorldInfo *world) const
{
  int i;
  const BzMaterial* mats[MaterialCount];
  for (i = 0; i < MaterialCount; i++) {
    mats[i] = MATERIALMGR.addMaterial(&materials[i]);
  }
  ConeObstacle* cone = new ConeObstacle(pos, size, rotation, angle,
                                        texsize, useNormals, divisions, mats,
                                        smoothBounce, driveThrough, shootThrough);

  if (cone->isValid()) {
    cone->getMesh()->setIsLocal(true);
    world->addCone(cone);
  } else {
    delete cone;
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

