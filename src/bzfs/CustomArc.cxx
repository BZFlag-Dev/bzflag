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
#include <vector>
#include "math.h"

/* common implementation headers */
#include "MeshObstacle.h"
#include "vectors.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"
#include "MeshUtils.h"

CustomArc::CustomArc()
{
  // default to a (radius=10, height=10) cylinder
  divisions = 16;
  size[0] = size[1] = size[2] = 10.0f;
  ratio = 1.0f;
  angle = 360.0f;

  // setup the default textures
  materials[Top].texture = "roof";
  materials[Bottom].texture = "roof";
  materials[Inside].texture = "boxwall";
  materials[Outside].texture = "boxwall";
  materials[StartFace].texture = "wall";
  materials[EndFace].texture = "wall";
  
  return;
}


CustomArc::~CustomArc()
{
  return;
}


bool CustomArc::read(const char *cmd, std::istream& input)
{
  bool materror;
  MeshMaterial modedMat;
  MeshMaterial origMat; // default material
  
  if (strcasecmp(cmd, "divisions") == 0) {
    input >> divisions;
  }
  else if (strcasecmp(cmd, "angle") == 0) {
    input >> angle;
  }
  else if (strcasecmp(cmd, "ratio") == 0) {
    input >> ratio;
  }
  else if (parseMaterial(cmd, input, modedMat, materror)) {
    if (materror) {
      return false;
    }
    for (int i = 0; i < 6; i++) {
      materials[i].copyDiffs(modedMat, origMat);
    }
  }
  else {
    return WorldFileObstacle::read(cmd, input);
  }

  return true;
}


void CustomArc::write(WorldInfo *world) const
{
  bool isPie = false;    // has no inside edge
  bool isCircle = false; // angle of 360 degrees
  int i, count;

  // limit angle and rotation to [0, (M_PI * 2)]
  float r = rotation;
  float a = angle * (M_PI / 180.f); // convert to radians
/*  
  if (a < 0.0f) {
    // put us on the positive side
    a = a - (M_PI * 2.0f) * floorf (a / (M_PI * 2.0f));
  } else {
    a = fmodf(a, M_PI * 2.0f);
  }
  
  if (a < 0.0f) {
    r = r + a;
    a = fabsf(a);
  }
  r = fmodf(r, M_PI * 2.0f);
  if (r < 0.0f) {
    r = r - ((M_PI * 2.0f) * floorf (r / (M_PI * 2.0f)));
  }
*/
  if (fabsf (M_PI - fmodf (a + M_PI, M_PI * 2.0f)) < 1e-6) {
    isCircle = true;
    printf ("isCircle\n");
  } else {
    printf ("not isCircle\n");
  }
  
  // FIXME - check angle vs. divisions
  // setup the radii  
  float inrad = size[0] * (1.0f - ratio);
  float outrad = size[0];
  if (inrad > outrad) {
    const float tmp = inrad;
    inrad = outrad;
    outrad = tmp;
  }
  if ((outrad < 1.0e-6f) || ((outrad - inrad) < 1e-6f)) {
    return;
  }
  if (inrad < 1.0e-6f) {
    isPie = true;
    printf ("isPie\n");
  } else {
    printf ("not isPie\n");
  }
  const float squish = size[1] / size[0];
  printf ("inrad = %f, outrad = %f, squish = %f\n", inrad, outrad, squish);
  
  // setup the height
  float h = fabsf(size[2]);
  if (h < 1.0e-6f) {
    return;
  }

  // setup the coordinates
  std::vector<char> checkTypes;
  std::vector<cfvec3> checkPoints;
  std::vector<cfvec3> vertices;
  std::vector<cfvec3> normals;
  std::vector<cfvec2> texcoords;

  if (isPie) {
    cfvec3 v;
    v[0] = pos[0];
    v[1] = pos[1];
    v[2] = pos[2];
    vertices.push_back(v); // center bottom
    v[2] = v[2] + h;
    vertices.push_back(v); // center top
  }
  
  // the number of corners  
  int corners = divisions + 1;
  if (isCircle) {
    corners = divisions;
  }
  
  const float astep = a / (float) divisions;

  for (i = 0; i < corners; i++) {
    float ang = r + (astep * (float)i);
    float cos_val = cos(ang);
    float sin_val = sin(ang);
    
    // vertices
    cfvec3 v;
    if (!isPie) {
      // inside points
      v[0] = pos[0] + (cos_val * inrad);
      v[1] = pos[1] + (squish * (sin_val * inrad));
      v[2] = pos[2];
      vertices.push_back(v);
      v[2] = v[2] + h;
      vertices.push_back(v);
    }
    // outside points
    v[0] = pos[0] + (cos_val * outrad);
    v[1] = pos[1] + (squish * (sin_val * outrad));
    v[2] = pos[2];
    vertices.push_back(v);
    v[2] = v[2] + h;
    vertices.push_back(v);
    
    // normals
    v[2] = 0.0f;
    if (!isPie) {
      // inside normal
      v[0] = -cos_val;
      v[1] = -sin_val;
      normals.push_back(v);
    }
    // outside normal
    v[0] = +cos_val;
    v[1] = +sin_val;
    normals.push_back(v);
  }
  
  // setup the face count
  int fcount = 2 + (divisions * 4);
  if (isCircle) {
    fcount = fcount - 2; // no start/end faces
  }
  if (isPie) {
    fcount = fcount - divisions; // no inside faces
  }


  MeshObstacle* mesh =
    new MeshObstacle(checkTypes, checkPoints, vertices, normals, texcoords,
                     fcount, driveThrough, shootThrough);
                 

  // now make the faces                 
  std::vector<int> verticesList;
  std::vector<int> normalsList;
  std::vector<int> texcoordsList;

  // inside edge
  if (!isPie) {
    count = isCircle ? (divisions - 1) : divisions;
    for (i = 0; i < count; i++) {
      const int e = (i * 4);
      push4Ints(verticesList, e + 4, e + 0, e + 1, e + 5);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Inside]);
    }
    if (isCircle) {
      const int e = (count * 4);
      push4Ints(verticesList, 0, e + 0, e + 1, 1);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Inside]);
    }
  }
  
  // outside edge
  count = isCircle ? (divisions - 1) : divisions;
  for (i = 0; i < count; i++) {
    if (!isPie) {
      const int e = 2 + (i * 4);
      push4Ints(verticesList, e, e + 4, e + 5, e + 1);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Outside]);
    } else {
      const int e = 2 + (i * 2);
      push4Ints(verticesList, e, e + 2, e + 3, e + 1);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Outside]);
    }
  }
  if (isCircle) {
    if (!isPie) {
      const int e = 2 + (count * 4);
      push4Ints(verticesList, e + 0, 2, 3, e + 1);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Outside]);
    } else {
      const int e = 2 + (count * 2);
      push4Ints(verticesList, e + 0, 2, 3, e + 1);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Outside]);
    }
  }
  

  // top side
  if (!isPie) {
    count = isCircle ? (divisions - 1) : divisions;
    for (i = 0; i < count; i++) {
      const int e = 1 + (i * 4);
      push4Ints(verticesList, e + 0, e + 2, e + 6, e + 4);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Top]);
    }
    if (isCircle) {
      const int e = 1 + (count * 4);
      push4Ints(verticesList, e + 0, e + 2, 3, 1);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Top]);
    }
  } else {
    count = isCircle ? (divisions - 1) : divisions;
    for (i = 0; i < count; i++) {
      const int e = 3 + (i * 2);
      push3Ints(verticesList, 1, e + 0, e + 2);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Top]);
    }
    if (isCircle) {
      const int e = 3 + (count * 2);
      push3Ints(verticesList, 1, e + 0, 3);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[Top]);
    }
  }
/*  
  // bottom side
  mesh->addFace(verticesList, normalsList, texcoordsList, materials[5]);
  verticesList.clear();
  normalsList.clear();
  texcoordsList.clear();
*/

  
  if (!isCircle) {
    // start face
    push4Ints(verticesList, 0, 2, 3, 1);
    addFace(mesh, verticesList, normalsList, texcoordsList, materials[StartFace]);
  
    // end face
    if (!isPie) {
      const int e = (corners - 1) * 4;
      push4Ints(verticesList, e + 2, e + 0, e + 1, e + 3);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[EndFace]);
    } else {
      const int e = 2 + ((corners - 1) * 2);
      push4Ints(verticesList, e, 0, 1, e + 1);
      addFace(mesh, verticesList, normalsList, texcoordsList, materials[EndFace]);
    }
  }
  
  // add the mesh  
  mesh->finalize();
  world->addMesh(mesh);
  
  push3Ints(verticesList, 1, 2, 3);//FIXME
  
  return;
}

void makePie(bool isCircle)
{
  isCircle = isCircle;
  return;
}


void makeRing(bool isCircle)
{
  isCircle = isCircle;
  return;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
