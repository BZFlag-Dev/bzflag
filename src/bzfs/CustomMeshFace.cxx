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

/* system headers */
#include <sstream>
#include <iostream>


CustomMeshFace::CustomMeshFace(const MeshMaterial& _material)
{
  material = _material;
  return;
}


static void getIntList (std::istream& input, std::vector<int>& list)
{
  std::string args;
  int value;

  list.clear();
  std::getline(input, args);
  std::istringstream parms(args);

  while (parms >> value) {
    list.push_back(value);
  }
  input.putback('\n');
  
  return;
}


bool CustomMeshFace::read(const char *cmd, std::istream& input)
{
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
  else if (strcasecmp(cmd, "notexture") == 0) {
    material.texture = "";
  }
  else if (strcasecmp(cmd, "texture") == 0) {
    input >> material.texture;
  }
  else if (strcasecmp(cmd, "texmat") == 0) {
    input >> material.textureMatrix;
  }
  else if (strcasecmp(cmd, "dyncol") == 0) {
    input >> material.dynamicColor;
  }
  else if (strcasecmp(cmd, "ambient") == 0) {
    input >> material.ambient[0] >> material.ambient[1] >> 
             material.ambient[2] >> material.ambient[3];
  }
  else if (strcasecmp(cmd, "diffuse") == 0) {
    input >> material.diffuse[0] >> material.diffuse[1] >> 
             material.diffuse[2] >> material.diffuse[3];
  }
  else if (strcasecmp(cmd, "specular") == 0) {
    input >> material.specular[0] >> material.specular[1] >> 
             material.specular[2] >> material.specular[3];
  }
  else if (strcasecmp(cmd, "emission") == 0) {
    input >> material.emission[0] >> material.emission[1] >> 
             material.emission[2] >> material.emission[3];
  }
  else if (strcasecmp(cmd, "shininess") == 0) {
    input >> material.shininess;
  }
  else {
    std::cout << "unknown mesh face property: " << cmd << std::endl;
    return false;
  }

  return true;
}


void CustomMeshFace::write(MeshObstacle *mesh) const
{
  mesh->addFace(vertices, normals, texcoords, material);
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
