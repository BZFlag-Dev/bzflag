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
  texsize[0] = texsize[1] = texsize[2] = texsize[3] = -4.0f;
  useNormals = true;

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

  if (strcasecmp(cmd, "divisions") == 0) {
    input >> divisions;
  }
  else if (strcasecmp(cmd, "angle") == 0) {
    input >> angle;
  }
  else if (strcasecmp(cmd, "ratio") == 0) {
    input >> ratio;
  }
  else if (strcasecmp(cmd, "texsize") == 0) {
    input >> texsize[0] >> texsize[1] >> texsize[2] >> texsize[3];
    printf ("%f %f %f %f\n", texsize[0], texsize[1], texsize[2], texsize[3]);
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


bool CustomArc::parseSideMaterials(const char* cmd, std::istream& input,
                                   bool& error)
{
  // NOTE: "end" can not be used because it will be picked-off
  //       as a block terminator at the BZWReader level.
  const char* sideNames[MaterialCount] =
    { "top", "bottom", "inside", "outside", "startside", "endside" };

  error = false;

  for (int n = 0; n < MaterialCount; n++) {
    if (strcasecmp (cmd, sideNames[n]) == 0) {
      std::string line, matcmd;
      std::getline(input, line);
      std::istringstream parms(line);
      if (!(parms >> matcmd)) {
        input.putback('\n');
        error = true;
      } else {
        // put the material command string back into the stream
        for (int i = 0; i < (int)(line.size() - matcmd.size()); i++) {
          input.putback(line[line.size() - i]);
        }
        parseMaterial(matcmd.c_str(), input, materials[n], error);
      }
      return true;
    }
  }

  return false;
}


void CustomArc::write(WorldInfo *world) const
{
  bool isPie = false;    // has no inside edge
  bool isCircle = false; // angle of 360 degrees
  const float minSize = 1.0e-6f; // cheezy / lazy

  // absolute the sizes
  float sz[3];
  sz[0] = fabsf(size[0]);
  sz[1] = fabsf(size[1]);
  sz[2] = fabsf(size[2]);

  // validity checking
  if ((sz[0] < minSize) || (sz[1] < minSize) || (sz[2] < minSize) ||
      (fabsf(texsize[0]) < minSize) || (fabsf(texsize[1]) < minSize) ||
      (fabsf(texsize[2]) < minSize) || (fabsf(texsize[3]) < minSize) ||
      (ratio < 0.0f) || (ratio > 1.0f)) {
    return;
  }

  // adjust the texture sizes   FIXME: finish texsz[2] & texsz[3]
  float texsz[4];
  memcpy (texsz, texsize, sizeof(float[4]));
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
  if ((int) ((a + minSize) / (M_PI * 0.5f)) > divisions) {
    return;
  }

  if (fabsf (M_PI - fmodf (a + M_PI, M_PI * 2.0f)) < minSize) {
    isCircle = true;
  }

  // FIXME - check angle vs. divisions
  // setup the radii
  float inrad = sz[0] * (1.0f - ratio);
  float outrad = sz[0];
  if (inrad > outrad) {
    const float tmp = inrad;
    inrad = outrad;
    outrad = tmp;
  }
  if ((outrad < minSize) || ((outrad - inrad) < minSize)) {
    return;
  }
  if (inrad < minSize) {
    isPie = true;
  }
  const float squish = sz[1] / sz[0];

  if (isPie) {
    makePie(isCircle, a, r, sz[2], outrad, squish, texsz, world);
  } else {
    makeRing(isCircle, a, r, sz[2], inrad, outrad, squish, texsz, world);
  }

  return;
}


void CustomArc::makePie(bool isCircle, float a, float r, float h,
                        float radius, float squish, float texsz[4],
                        WorldInfo* world) const
{
  int i;

  // setup the coordinates
  std::vector<char> checkTypes;
  std::vector<cfvec3> checkPoints;
  std::vector<cfvec3> vertices;
  std::vector<cfvec3> normals;
  std::vector<cfvec2> texcoords;

  // setup the texsize across the disc
  if (texsz[2] < 0.0f) {
    texsz[2] = -((2.0f * radius) / texsz[2]);
  }
  if (texsz[3] < 0.0f) {
    texsz[3] = -((2.0f * radius * squish) / texsz[3]);
  }

  const float astep = a / (float) divisions;

  for (i = 0; i < (divisions + 1); i++) {
    float ang = r + (astep * (float)i);
    float cos_val = cos(ang);
    float sin_val = sin(ang);

    // vertices and normals
    if (!isCircle || (i != divisions)) {
      cfvec3 v, n;
      float delta[2];
      delta[0] = cos_val * radius;
      delta[1] = (sin_val * radius) * squish;
      // vertices
      v[0] = pos[0] + delta[0];
      v[1] = pos[1] + delta[1];
      v[2] = pos[2];
      vertices.push_back(v);
      v[2] = v[2] + h;
      vertices.push_back(v);
      // normal
      if (useNormals) {
        n[2] = 0.0f;
        n[0] = cos_val * squish;
        n[1] = sin_val;
        float len = 1.0f / sqrtf((n[0] * n[0]) + (n[1] * n[1]));
        n[0] = n[0] * len;
        n[1] = n[1] * len;
        normals.push_back(n);
      }
    }

    // texture coordinates (around the edge)
    cfvec2 t;
    t[0] = (float) i / (float) divisions;
    t[0] = texsz[0] * t[0];
    t[1] = 0.0f;
    texcoords.push_back(t);
    // outside texcoord
    t[1] = texsz[1] * 1.0f;
    texcoords.push_back(t);
  }

  // texture coordinates (around the disc)
  for (i = 0; i < (divisions + 1); i++) {
    float ang = r + (astep * (float)i);
    float cos_val = cos(ang);
    float sin_val = sin(ang);
    cfvec2 t;
    t[0] = texsz[2] * (0.5f + (0.5f * cos_val));
    t[1] = texsz[3] * (0.5f + (0.5f * sin_val));
    texcoords.push_back(t);
  }

  // the central coordinates
  cfvec3 v;
  v[0] = pos[0];
  v[1] = pos[1];
  v[2] = pos[2];
  vertices.push_back(v); // bottom
  v[2] = pos[2] + h;
  vertices.push_back(v); // top
  cfvec2 t;
  t[0] = texsz[2] * 0.5f;
  t[1] = texsz[3] * 0.5f;
  texcoords.push_back(t);

  // setup the face count
  int fcount = (divisions * 3);
  if (!isCircle) {
    fcount = fcount + 2; // add the start and end faces
  }

  MeshObstacle* mesh =
    new MeshObstacle(checkTypes, checkPoints, vertices, normals, texcoords,
                     fcount, driveThrough, shootThrough);

  // now make the faces
  int vlen, nlen;
  if (isCircle) {
    vlen = divisions * 2;
    nlen = divisions;
  } else {
    vlen = (divisions + 1) * 2;
    nlen = (divisions + 1);
  }

  const int vtop = vlen + 1;
  const int vbot = vlen;
  const int tmid = ((divisions + 1) * 3);

  std::vector<int> vlist;
  std::vector<int> nlist;
  std::vector<int> tlist;

  for (i = 0; i < divisions; i++) {

// handy macros
#define PV(x) (((x) + (i * 2)) % vlen)
#define PN(x) (((x) + i) % nlen)
#define PTO(x) ((x) + (i * 2))                     // outside edge
#define PTC(x) (((divisions + 1) * 2) + (x) + i)   // around the disc
#define PTCI(x) (((divisions + 1) * 3) - (x) - i - 1)

    // outside
    push4Ints(vlist, PV(0), PV(2), PV(3), PV(1));
    if (useNormals) push4Ints(nlist, PN(0), PN(1), PN(1), PN(0));
    push4Ints(tlist, PTO(0), PTO(2), PTO(3), PTO(1));
    addFace(mesh, vlist, nlist, tlist, materials[Outside]);

    // top
    push3Ints(vlist, vtop, PV(1), PV(3));
    push3Ints(tlist, tmid, PTC(0), PTC(1));
    addFace(mesh, vlist, nlist, tlist, materials[Top]);

    // bottom
    push3Ints(vlist, vbot, PV(2), PV(0));
    push3Ints(tlist, tmid, PTCI(1), PTCI(0));
    addFace(mesh, vlist, nlist, tlist, materials[Bottom]);
  }


  if (!isCircle) {
    int tc = (divisions * 2);
    // start face
    push4Ints(vlist, vbot, 0, 1, vtop);
    push4Ints(tlist, 0, tc + 0, tc + 1, 1);
    addFace(mesh, vlist, nlist, tlist, materials[StartFace]);
    // end face
    int e = divisions * 2;
    push4Ints(vlist, e + 0, vbot, vtop, e + 1);
    push4Ints(tlist, 0, tc + 0, tc + 1, 1);
    addFace(mesh, vlist, nlist, tlist, materials[EndFace]);
  }


  // add the mesh
  mesh->finalize();
  world->addMesh(mesh);

  return;
}


void CustomArc::makeRing(bool isCircle, float a, float r, float h,
                         float inrad, float outrad, float squish,
                         float texsz[4], WorldInfo* world) const
{
  int i;

  // setup the coordinates
  std::vector<char> checkTypes;
  std::vector<cfvec3> checkPoints;
  std::vector<cfvec3> vertices;
  std::vector<cfvec3> normals;
  std::vector<cfvec2> texcoords;

  const float astep = a / (float) divisions;

  for (i = 0; i < (divisions + 1); i++) {
    float ang = r + (astep * (float)i);
    float cos_val = cos(ang);
    float sin_val = sin(ang);

    // vertices and normals
    if (!isCircle || (i != divisions)) {
      cfvec3 v, n;
      // inside points
      v[0] = pos[0] + (cos_val * inrad);
      v[1] = pos[1] + (squish * (sin_val * inrad));
      v[2] = pos[2];
      vertices.push_back(v);
      v[2] = v[2] + h;
      vertices.push_back(v);
      // outside points
      v[0] = pos[0] + (cos_val * outrad);
      v[1] = pos[1] + (squish * (sin_val * outrad));
      v[2] = pos[2];
      vertices.push_back(v);
      v[2] = v[2] + h;
      vertices.push_back(v);
      // inside normal
      if (useNormals) {
        n[2] = 0.0f;
        n[0] = -cos_val;
        n[1] = -sin_val * squish;
        float len = 1.0f / sqrtf((n[0] * n[0]) + (n[1] * n[1]));
        n[0] = n[0] * len;
        n[1] = n[1] * len;
        normals.push_back(n);
        // outside normal
        n[0] = -n[0];
        n[1] = -n[1];
        normals.push_back(n);
      }
    }

    // texture coordinates
    cfvec2 t;
    // inside texcoord
    t[0] = (float) i / (float) divisions;
    t[0] = texsz[0] * t[0];
    t[1] = 0.0f;
    texcoords.push_back(t);
    // outside texcoord
    t[1] = texsz[1] * 1.0f;
    texcoords.push_back(t);
  }

  // setup the face count
  int fcount = (divisions * 4);
  if (!isCircle) {
    fcount = fcount + 2; // add the start and end faces
  }

  MeshObstacle* mesh =
    new MeshObstacle(checkTypes, checkPoints, vertices, normals, texcoords,
                     fcount, driveThrough, shootThrough);

  // now make the faces
  int vlen, nlen;
  if (isCircle) {
    vlen = divisions * 4;
    nlen = divisions * 2;
  } else {
    vlen = (divisions + 1) * 4;
    nlen = (divisions + 1) * 2;
  }

  std::vector<int> vlist;
  std::vector<int> nlist;
  std::vector<int> tlist;

  for (i = 0; i < divisions; i++) {

// handy macros
#define RV(x) (((x) + (i * 4)) % vlen)
#define RN(x) (((x) + (i * 2)) % nlen)
#define RT(x) ((x) + (i * 2))
#define RIT(x) ((divisions + ((x)%2))*2 - ((x) + (i * 2)))

    // inside
    push4Ints(vlist, RV(4), RV(0), RV(1), RV(5));
    if (useNormals) push4Ints(nlist, RN(2), RN(0), RN(0), RN(2));
    push4Ints(tlist, RIT(2), RIT(0), RIT(1), RIT(3));
    addFace(mesh, vlist, nlist, tlist, materials[Inside]);

    // outside
    push4Ints(vlist, RV(2), RV(6), RV(7), RV(3));
    if (useNormals) push4Ints(nlist, RN(1), RN(3), RN(3), RN(1));
    push4Ints(tlist, RT(0), RT(2), RT(3), RT(1));
    addFace(mesh, vlist, nlist, tlist, materials[Outside]);

    // top
    push4Ints(vlist, RV(3), RV(7), RV(5), RV(1));
    push4Ints(tlist, RT(0), RT(2), RT(3), RT(1));
    addFace(mesh, vlist, nlist, tlist, materials[Top]);

    // bottom
    push4Ints(vlist, RV(0), RV(4), RV(6), RV(2));
    push4Ints(tlist, RT(0), RT(2), RT(3), RT(1));
    addFace(mesh, vlist, nlist, tlist, materials[Bottom]);
  }

  if (!isCircle) {
    int tc = (divisions * 2);
    // start face
    push4Ints(vlist, 0, 2, 3, 1);
    push4Ints(tlist, 0, tc + 0, tc + 1, 1);
    addFace(mesh, vlist, nlist, tlist, materials[StartFace]);
    // end face
    int e = divisions * 4;
    push4Ints(vlist, e + 2, e + 0, e + 1, e + 3);
    push4Ints(tlist, 0, tc + 0, tc + 1, 1);
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
