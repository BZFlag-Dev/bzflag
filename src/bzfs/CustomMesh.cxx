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
#include "CustomMesh.h"

/* bzfs implementation headers */
#include "CustomMeshFace.h"


CustomMesh::CustomMesh()
{
  face = NULL;
  driveThrough = false;
  shootThrough = false;
  return;
}


CustomMesh::~CustomMesh()
{
  std::vector<CustomMeshFace*>::iterator face_it;
  for (face_it = faces.begin(); face_it != faces.end(); face_it++) {
    CustomMeshFace* face = *face_it;
    delete face;
  }
  return;
}


bool CustomMesh::read(const char *cmd, std::istream& input)
{
  if (strcasecmp(cmd, "endface") == 0) {
    if (face == NULL) {
      std::cout << "extra 'endface' keyword found" << std::endl;
    } else {
      faces.push_back(face);
      face = NULL;
    }
  }
  else if (face) {
    // currently processing a face
    return face->read(cmd, input);
  }
  else if (strcasecmp(cmd, "face") == 0) {
    if (face != NULL) {
      std::cout << "discarding incomplete mesh face" << std::endl;
      delete face;
    }
    face = new CustomMeshFace (material);
  }
  else if (strcasecmp(cmd, "reset") == 0) {
    material.reset();
  }
  else if (strcasecmp(cmd, "inside") == 0) {
    cfvec3 inside;
    input >> inside[0] >> inside[1] >> inside[2];
    checkTypes.push_back(MeshObstacle::CheckInside);
    checkPoints.push_back(inside);
  }
  else if (strcasecmp(cmd, "outside") == 0) {
    cfvec3 outside;
    input >> outside[0] >> outside[1] >> outside[2];
    checkTypes.push_back(MeshObstacle::CheckOutside);
    checkPoints.push_back(outside);
  }
  else if (strcasecmp(cmd, "vertex") == 0) {
    cfvec3 vertex;
    input >> vertex[0] >> vertex[1] >> vertex[2];
    vertices.push_back(vertex);
  }
  else if (strcasecmp(cmd, "normal") == 0) {
    cfvec3 normal;
    input >> normal[0] >> normal[1] >> normal[2];
    normals.push_back(normal);
  }
  else if (strcasecmp(cmd, "texcoord") == 0) {
    cfvec2 texcoord;
    input >> texcoord[0] >> texcoord[1];
    texcoords.push_back(texcoord);
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
  else if (strcasecmp(cmd, "drivethrough") == 0) {
    driveThrough = true;
  }
  else if (strcasecmp(cmd, "shootthrough") == 0) {
    shootThrough = true;
  }
  else if (strcasecmp(cmd, "passable") == 0) {
    driveThrough = shootThrough = true;
  }
  else {
    // NOTE: we don't use a WorldFileObstacle
    return WorldFileObject::read(cmd, input);
  }

  return true;
}


void CustomMesh::write(WorldInfo *world) const
{
  if (face != NULL) {
    std::cout << "discarding incomplete mesh face" << std::endl;
    delete face;
  }
  
  MeshObstacle* mesh = new MeshObstacle(
    checkTypes, checkPoints, vertices, normals, texcoords, 
    faces.size(), driveThrough, shootThrough);

  std::vector<CustomMeshFace*>::const_iterator face_it;
  for (face_it = faces.begin(); face_it != faces.end(); face_it++) {
    const CustomMeshFace* face = *face_it;
    face->write(mesh);
  }
  mesh->finalize();
  
  world->addMesh(mesh);
  
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
