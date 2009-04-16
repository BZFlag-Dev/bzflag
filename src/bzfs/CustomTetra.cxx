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

#include "common.h"

/* interface header */
#include "CustomTetra.h"

/* system implementation headers */
#include <iostream>

/* common implementation headers */
#include "TextureMatrix.h"
#include "MeshObstacle.h"
#include "ObstacleMgr.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"


CustomTetra::CustomTetra()
{
  vertexCount = 0; // no vertices have yet been defined

  // reset the secondary coordinate states
  for (int i = 0; i < 4; i++) {
    useNormals[i] = false;
    useTexcoords[i] = false;
    materials[i].setTexture("mesh");
  }
  return;
}


bool CustomTetra::read(const char *cmd, std::istream& input)
{
  if (vertexCount > 4) {
    std::cout << "Extra tetrahedron vertex" << std::endl;
    // keep on chugging
    return true;
  }

  bool materror;
  if (vertexCount == 0) {
    // try to parse all 4 materials
    if (parseMaterials(cmd, input, materials, 4, materror)) {
      return !materror;
    }
  } else {
    // try to parse the specific vertex's material
    int vc = vertexCount - 1;
    if (vc > 3) {
      vc = 3;
    }
    if (parseMaterials(cmd, input, &materials[vc], 1, materror)) {
      return !materror;
    }
  }

  if (strcasecmp(cmd, "vertex") == 0) {
    if (vertexCount >= 4) {
      std::cout << "Extra tetrahedron vertex" << std::endl;
      // keep on chugging
    } else {
      fvec3& vertex = vertices[vertexCount];
      input >> vertex.x >> vertex.y >> vertex.z;
      vertexCount++;
    }
  } else if (strcasecmp(cmd, "normals") == 0) {
    if (vertexCount < 1) {
      std::cout << "Normals defined before any vertex" << std::endl;
      // keep on chugging
    } else if (vertexCount > 4) {
      std::cout << "Extra tetrahedron normals" << std::endl;
      // keep on chugging
    } else {
      useNormals[vertexCount - 1] = true;
      fvec3& normal = normals[vertexCount - 1];
      input >> normal.x >> normal.y >> normal.z;
    }
  } else if (strcasecmp(cmd, "texcoords") == 0) {
    if (vertexCount < 1) {
      std::cout << "Texcoords defined before any vertex" << std::endl;
      // keep on chugging
    } else if (vertexCount > 4) {
      std::cout << "Extra tetrahedron texcoords" << std::endl;
      // keep on chugging
    } else {
      useTexcoords[vertexCount - 1] = true;
      fvec2& texcoord = texcoords[vertexCount - 1];
      input >> texcoord.x >> texcoord.y;
    }
  } else {
    return WorldFileObstacle::read(cmd, input);
  }

  return true;
}


void CustomTetra::writeToGroupDef(GroupDefinition *groupdef) const
{
  if (vertexCount < 4) {
    std::cout << "Not creating tetrahedron, not enough vertices ("
	      << vertexCount << ")" << std::endl;
    return;
  }

  const int posFaceSets[4][3] = {
    { 0, 1, 3 },
    { 1, 2, 3 },
    { 2, 0, 3 },
    { 2, 1, 0 }
  };
  const int invFaceSets[4][3] = {
    { 1, 0, 3 },
    { 2, 1, 3 },
    { 0, 2, 3 },
    { 0, 1, 2 }
  };
  const fvec3 edge01 = vertices[1] - vertices[0];
  const fvec3 edge02 = vertices[2] - vertices[0];
  const fvec3 edge03 = vertices[3] - vertices[0];
  const fvec3 cross = fvec3::cross(edge01, edge02);
  const float polarity = fvec3::dot(edge03, cross);
  const int (*faceSets)[3] = (polarity >= 0.0f) ? posFaceSets : invFaceSets;

  std::vector<char>  checkTypes;
  std::vector<fvec3> checkPoints;
  std::vector<fvec3> verts;
  std::vector<fvec3> norms;
  std::vector<fvec2> txcds;

  if ((driveThrough == 0) && (fabsf(polarity) > 1.0e-3f)) {
    fvec3 center;
    for (int v = 0; v < 4; v++) {
      center += vertices[v];
    }
    center *= 0.25f;
    checkTypes.push_back(MeshObstacle::CheckInside);
    checkPoints.push_back(center);
  }

  int normOffset = 0;
  int txcdOffset = 0;
  int normOffsets[4];
  int txcdOffsets[4];

  for (int v = 0; v < 4; v++) {
    verts.push_back(vertices[v]);

    normOffsets[v] = normOffset;
    if (useNormals[v]) {
      //FIXME norms.push_back(normals[v]);
      normOffset += 1;
    }

    txcdOffsets[v] = txcdOffset;
    if (useTexcoords[v]) {
      //FIXME txcds.push_back(texcoords[v]);
      txcdOffset += 1;
    }
  }
    
  MeshObstacle* mesh = new MeshObstacle(transform,
                                        checkTypes, checkPoints,
					verts, norms, txcds, 4,
					false, false, 0, 0, false);
  mesh->setName(name.c_str());

  // get the material refs
  const BzMaterial* mats[4];
  for (int v = 0; v < 4; v++) {
    mats[v] = MATERIALMGR.addMaterial(&materials[v]);
  }

  // the index arrays
  std::vector<int> iv;
  std::vector<int> it;
  std::vector<int> in; // leave at 0 count (auto normals)

  for (int f = 0; f < 4; f++) {
    const int* fs = faceSets[f];
    iv.clear(); in.clear(); it.clear(); 
    iv.push_back(fs[0]); iv.push_back(fs[1]); iv.push_back(fs[2]);
    if (false && useNormals[f]) { // FIXME
      in.push_back(fs[0]); in.push_back(fs[1]); in.push_back(fs[2]);
    }
    if (false && useTexcoords[f]) { // FIXME
      it.push_back(fs[0]); it.push_back(fs[1]); it.push_back(fs[2]);
    }
    mesh->addFace(iv, in, it, mats[f], -1, false, false,
                  driveThrough, shootThrough, ricochet,  false);
  }

  // to be or not to be...
  if (mesh->isValid()) {
    groupdef->addObstacle(mesh);
  } else {
    std::cout << "Error generating tetra obstacle." << std::endl;
    delete mesh;
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
