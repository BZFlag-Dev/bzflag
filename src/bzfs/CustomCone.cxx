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
#include "MeshObstacle.h"
#include "vectors.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"
#include "MeshUtils.h"


CustomCone::CustomCone()
{
  // default to a (radius=10, height=10) cylinder
  divisions = 16;
  size[0] = size[1] = size[2] = 10.0f;
  texsize[0] = texsize[1] = -4.0f;
  angle = 360.0f;
  useNormals = true;

  // setup the default textures
  materials[Edge].texture = "boxwall";
  materials[Bottom].texture = "roof";
  materials[StartFace].texture = "wall";
  materials[EndFace].texture = "wall";

  return;
}


CustomCone::~CustomCone()
{
  return;
}


bool CustomCone::read(const char *cmd, std::istream& input)
{
  bool materror;
  MeshMaterial modedMat;

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
  else if (strcasecmp(cmd, "flatshading") == 0) {
    useNormals = false;
  }
  else if (parseMaterial(cmd, input, modedMat, materror)) {
    if (materror) {
      return false;
    }
    MeshMaterial defMat; // default material
    for (int i = 0; i < MaterialCount; i++) {
      materials[i].copyDiffs(modedMat, defMat);
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
        if (!parseMaterial(matcmd.c_str(), input, materials[n], error)) {
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
  bool isCircle = false; // angle of 360 degrees
  const float minSize = 1.0e-6f; // cheezy / lazy

  // absolute the sizes
  float sz[3];
  sz[0] = fabsf(size[0]);
  sz[1] = fabsf(size[1]);
  sz[2] = fabsf(size[2]);

  // validity checking
  if ((sz[0] < minSize) || (sz[1] < minSize) || (sz[2] < minSize) ||
      (fabs(texsize[0]) < minSize) || (fabs(texsize[1]) < minSize)) {
    return;
  }

  // adjust the texture sizes
  float texsz[2];
  memcpy (texsz, texsize, sizeof(float[2]));
  if (texsz[0] < 0.0f) {
    // unless you want to do elliptic integrals, here's
    // the Ramanujan approximation for the circumference
    // of an ellipse  (it will be rounded anyways)
    const float circ = 
      M_PI * ((3.0f * (sz[0] + sz[1])) -
              sqrtf ((sz[0] + (3.0f * sz[1])) * (sz[1] + (3.0f * sz[0]))));
    // make sure it's an integral number so that the edges line up
    texsz[0] = -floorf(circ / texsz[0]);
  }
  if (texsz[1] < 0.0f) {
    texsz[1] = -(sz[2] / texsz[1]);
  }
  
  // setup the angles  
  float r = rotation;
  float a = angle;
  if (a > +360.0f) {
    a = +360.0f;
  }
  if (a < -360.0f) {
    a = -360.0f;
  }
  a = a * (M_PI / 180.0f); // convert to radians
  if (a < 0.0f) {
    r = r + a;
    a = -a;
  }

  // more validity checking
  if (divisions <= (int) ((a + minSize) / M_PI)) {
    return;
  }

  if (fabsf (M_PI - fmodf (a + M_PI, M_PI * 2.0f)) < minSize) {
    isCircle = true;
  }

  // setup the coordinates
  std::vector<char> checkTypes;
  std::vector<cfvec3> checkPoints;
  std::vector<cfvec3> vertices;
  std::vector<cfvec3> normals;
  std::vector<cfvec2> texcoords;
  cfvec3 v, n;
  cfvec2 t;

  const float astep = a / (float) divisions;
  
  int i;
  for (i = 0; i < (divisions + 1); i++) {
    float ang = r + (astep * (float)i);
    float cos_val = cos(ang);
    float sin_val = sin(ang);

    // vertices
    if (!isCircle || (i != divisions)) {
      float delta[2];
      // vertices (around the edge)
      delta[0] = cos_val * sz[0];
      delta[1] = sin_val * sz[1];
      v[0] = pos[0] + delta[0];
      v[1] = pos[1] + delta[1];
      v[2] = pos[2];
      vertices.push_back(v);
      // normals (around the edge)
      if (useNormals) {
        // the horizontal normals
        n[0] = cos_val / sz[0];
        n[1] = sin_val / sz[1];
        n[2] = 1.0f / sz[2];
        // normalize
        float len = (n[0] * n[0]) + (n[1] * n[1]) + (n[2] * n[2]);
        len = 1.0f / sqrtf(len);
        n[0] *= len;
        n[1] *= len;
        n[2] *= len;
        normals.push_back(n);
      }
    }

    // texture coordinates (around the edge)
    cfvec2 t;
    t[0] = texsz[0] * (0.5f + (0.5f * cos(ang)));
    t[1] = texsz[1] * (0.5f + (0.5f * sin(ang)));
    texcoords.push_back(t);
  }

  // the central normals
  if (useNormals) {
    for (i = 0; i < divisions; i++) {
      float ang = r + (astep * (0.5f + (float)i));
      // the horizontal normals
      n[0] = cos(ang) / sz[0];
      n[1] = sin(ang) / sz[1];
      n[2] = 1.0f / sz[2];
      // normalize
      float len = (n[0] * n[0]) + (n[1] * n[1]) + (n[2] * n[2]);
      len = 1.0f / sqrtf(len);
      n[0] *= len;
      n[1] *= len;
      n[2] *= len;
      normals.push_back(n);
    }
  }
  
  // the central coordinates
  v[0] = pos[0];
  v[1] = pos[1];
  v[2] = pos[2];
  vertices.push_back(v); // bottom
  v[2] = pos[2] + sz[2];
  vertices.push_back(v); // top
  t[0] = texsz[0] * 0.5f;
  t[1] = texsz[1] * 0.5f;
  texcoords.push_back(t);

  // for the start/end faces
  if (!isCircle) {
    t[0] = texsz[0] * 0.0f;
    t[1] = texsz[1] * 0.0f;
    texcoords.push_back(t);
    t[0] = texsz[0] * 1.0f;
    t[1] = texsz[1] * 0.0f;
    texcoords.push_back(t);
    t[0] = texsz[0] * 1.0f;
    t[1] = texsz[1] * 1.0f;
    texcoords.push_back(t);
    t[0] = texsz[0] * 0.0f;
    t[1] = texsz[1] * 1.0f;
    texcoords.push_back(t);
  }

  // setup the face count
  int fcount = (divisions * 2);
  if (!isCircle) {
    fcount = fcount + 2; // add the start and end faces
  }

  MeshObstacle* mesh =
    new MeshObstacle(checkTypes, checkPoints, vertices, normals, texcoords,
                     fcount, driveThrough, shootThrough);

  // now make the faces
  int vlen;
  if (isCircle) {
    vlen = divisions;
  } else {
    vlen = divisions + 1;
  }

  std::vector<int> vlist;
  std::vector<int> nlist;
  std::vector<int> tlist;

  const int vtop = vlen + 1;
  const int vbot = vlen;
  const int tmid = divisions + 1;
  const int t00 = (divisions + 1) + 1 + 0;
  const int t10 = (divisions + 1) + 1 + 1;
  const int t11 = (divisions + 1) + 1 + 2;
  const int t01 = (divisions + 1) + 1 + 3;
  
  for (i = 0; i < divisions; i++) {

// handy macros
#define V(x) (((x) + i) % vlen)  // edge
#define NE(x) (((x) + i) % vlen) // edge
#define NC(x) (vlen + i)         // center
#define T(x) ((x) + i)           // edge
#define TI(x) (divisions - T(x)) // edge, backwards

    // edge
    push3Ints(vlist, vtop, V(0), V(1));
    if (useNormals) push3Ints(nlist, NC(0), NE(0), NE(1));
    push3Ints(tlist, tmid, T(0), T(1));
    addFace(mesh, vlist, nlist, tlist, materials[Edge]);

    // bottom
    push3Ints(vlist, vbot, V(1), V(0));
    push3Ints(tlist, tmid, TI(1), TI(0));
    addFace(mesh, vlist, nlist, tlist, materials[Bottom]);
  }

  if (!isCircle) {
    // start face
    push3Ints(vlist, vbot, 0, vtop);
    push3Ints(tlist, t00, t10, t01);
    addFace(mesh, vlist, nlist, tlist, materials[StartFace]);
    // end face
    push3Ints(vlist, vlen - 1, vbot, vtop);
    push3Ints(tlist, t00, t10, t11);
    addFace(mesh, vlist, nlist, tlist, materials[EndFace]);
  }


  // add the mesh
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

