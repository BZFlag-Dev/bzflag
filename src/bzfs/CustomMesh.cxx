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
#include "ParseMaterial.h"


CustomMesh::CustomMesh()
{
  face = NULL;
  driveThrough = false;
  shootThrough = false;
  fragments = true;
  smoothBounce = false;
  material.setTexture("mesh");
  
  return;
}


CustomMesh::~CustomMesh()
{
  if (face != NULL) {
    std::cout << "discarded incomplete mesh face" << std::endl;
    delete face;
  }

  std::vector<CustomMeshFace*>::iterator face_it;
  for (face_it = faces.begin(); face_it != faces.end(); face_it++) {
    CustomMeshFace* face = *face_it;
    delete face;
  }

  return;
}


bool CustomMesh::read(const char *cmd, std::istream& input)
{
  bool materror;

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
    face = new CustomMeshFace (material, smoothBounce,
                               driveThrough, shootThrough);
  }
  else if (strcasecmp(cmd, "inside") == 0) {
    cfvec3 inside;
    if (!(input >> inside[0] >> inside[1] >> inside[2])) {
      return false;
    }
    checkTypes.push_back(MeshObstacle::CheckInside);
    checkPoints.push_back(inside);
  }
  else if (strcasecmp(cmd, "outside") == 0) {
    cfvec3 outside;
    if (!(input >> outside[0] >> outside[1] >> outside[2])) {
      return false;
    }
    checkTypes.push_back(MeshObstacle::CheckOutside);
    checkPoints.push_back(outside);
  }
  else if (strcasecmp(cmd, "vertex") == 0) {
    cfvec3 vertex;
    if (!(input >> vertex[0] >> vertex[1] >> vertex[2])) {
      return false;
    }
    vertices.push_back(vertex);
  }
  else if (strcasecmp(cmd, "normal") == 0) {
    cfvec3 normal;
    if (!(input >> normal[0] >> normal[1] >> normal[2])) {
      return false;
    }
    normals.push_back(normal);
  }
  else if (strcasecmp(cmd, "texcoord") == 0) {
    cfvec2 texcoord;
    if (!(input >> texcoord[0] >> texcoord[1])) {
      return false;
    }
    texcoords.push_back(texcoord);
  }
  else if ((strcasecmp(cmd, "ricosuavez") == 0) ||
           (strcasecmp(cmd, "smoothbounce") == 0)) {
    smoothBounce = true;
  }
  else if ((strcasecmp(cmd, "nofrags") == 0) ||
           (strcasecmp(cmd, "nofragments") == 0)) {
    fragments = false;
  }
  else if (parseMaterials(cmd, input, &material, 1, materror)) {
    if (materror) {
      return false;
    }
  }
  else {
    // NOTE: the position, size, and rotation
    //       parameters are currently ignored
    return WorldFileObstacle::read(cmd, input);
  }

  return true;
}


void CustomMesh::write(WorldInfo *world) const
{
  MeshObstacle* mesh = new MeshObstacle(
    checkTypes, checkPoints, vertices, normals, texcoords,
    faces.size(), smoothBounce, driveThrough, shootThrough);
    
  mesh->setFragments(fragments);

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
