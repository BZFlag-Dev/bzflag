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
#include "CustomMeshFace.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"

/* system headers */
#include <sstream>
#include <iostream>


CustomMeshFace::CustomMeshFace(const MeshMaterial& _material,
                               bool bounce, bool drive, bool shoot)
{
  material = _material;
  smoothBounce = bounce;
  driveThrough = drive;
  shootThrough = shoot;
  return;
}


static void getIntList (std::istream& input, std::vector<int>& list)
{
  std::string args;
  int value;

  list.clear();
  std::getline(input, args);
  std::istringstream parms(args);
  input.putback('\n');

  while (parms >> value) {
    list.push_back(value);
  }

  return;
}


bool CustomMeshFace::read(const char *cmd, std::istream& input)
{
  bool materror;

  if (strcasecmp(cmd, "vertices") == 0) {
    getIntList (input, vertices);
    if (vertices.size() < 3) {
      std::cout << "mesh faces need at least 3 vertices" << std::endl;
      return false;
    }
  }
  else if (strcasecmp(cmd, "normals") == 0) {
    getIntList (input, normals);
    if (normals.size() < 3) {
      std::cout << "mesh faces need at least 3 normals" << std::endl;
      return false;
    }
  }
  else if (strcasecmp(cmd, "texcoords") == 0) {
    getIntList (input, texcoords);
    if (texcoords.size() < 3) {
      std::cout << "mesh faces need at least 3 texcoords" << std::endl;
      return false;
    }
  }
  else if ((strcasecmp(cmd, "ricosuavez") == 0) ||
           (strcasecmp(cmd, "smoothbounce") == 0)) {
    smoothBounce = true;
  }
  else if (strcasecmp(cmd, "drivethrough") == 0) {
    driveThrough = true;
  }
  else if (strcasecmp(cmd, "shootthrough") == 0) {
    shootThrough = true;
  }
  else if (strcasecmp(cmd, "passable") == 0) {
    driveThrough = shootThrough = true;
  }
  else if (parseMaterial(cmd, input, material, materror)) {
    if (materror) {
      return false;
    }
  }
  else {
    std::cout << "unknown mesh face property: " << cmd << std::endl;
    return false;
  }

  return true;
}


void CustomMeshFace::write(MeshObstacle *mesh) const
{
  mesh->addFace(vertices, normals, texcoords, material,
                smoothBounce, driveThrough, shootThrough);
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
