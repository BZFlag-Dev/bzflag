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
#include "CustomSphere.h"

/* system headers */
#include <vector>
#include "math.h"

/* common implementation headers */
#include "MeshObstacle.h"
#include "vectors.h"

/* bzfs implementation headers */
#include "ParseMaterial.h"
#include "MeshUtils.h"


CustomSphere::CustomSphere()
{
  divisions = 4;
  pos[2] = 10.0f;
  size[0] = size[1] = size[2] = 10.0f;
  material.texture = "boxwall";
  return;
}


CustomSphere::~CustomSphere()
{
  return;
}


bool CustomSphere::read(const char *cmd, std::istream& input)
{
  bool materror;
  
  if (strcasecmp(cmd, "divisions") == 0) {
    input >> divisions;
  }
  else if (strcasecmp(cmd, "radius") == 0) {
    float radius;
    input >> radius;
    size[0] = size[1] = size[2] = radius;
  }
  else if (parseMaterial(cmd, input, material, materror)) {
    if (materror) {
      return false;
    }
  }
  else {
    return WorldFileObstacle::read(cmd, input);
  }

  return true;
}


void CustomSphere::write(WorldInfo *world) const
{
  int i, j, q;
  cfvec3 v, n;
  cfvec2 t;
  
  // setup the coordinates
  std::vector<char> checkTypes;
  std::vector<cfvec3> checkPoints;
  std::vector<cfvec3> vertices;
  std::vector<cfvec3> normals;
  std::vector<cfvec2> texcoords;
  
  // the center vertices
  v[0] = pos[0];
  v[1] = pos[1];
  v[2] = pos[2] + size[2];
  vertices.push_back(v);
  v[2] = pos[2] - size[2];
  vertices.push_back(v);
  n[0] = 0.0f;
  n[1] = 0.0f;
  n[2] = 1.0f;
  normals.push_back(n);
  n[2] = -1.0f;
  normals.push_back(n);
  t[0] = 0.5f; // weirdness
  t[1] = 1.0f;
  texcoords.push_back(t);
  t[1] = 0.0f;
  texcoords.push_back(t);

  // the rest of the vertices
  for (i = 0; i < divisions; i++) {
    for (j = 0; j < (4 * (i + 1)); j++) {
      float h_angle = ((M_PI * 2.0f) *
                      (float)j / (float)(4 * (i + 1)));
      float v_angle = ((M_PI / 2.0f) * 
                      (float)(divisions - i - 1) / (float)(divisions));
      float delta[3];
      delta[0] = size[0] * (cos(h_angle) * cos(v_angle));
      delta[1] = size[1] * (sin(h_angle) * cos(v_angle));
      delta[2] = size[2] * sin(v_angle);
      // vertex
      v[0] = pos[0] + delta[0];
      v[1] = pos[1] + delta[1];
      v[2] = pos[2] + delta[2];
      vertices.push_back(v);
      // normal
      const float len = 1.0f / sqrtf(vec3dot(delta, delta));
      n[0] = delta[0] * len;
      n[1] = delta[1] * len;
      n[2] = delta[2] * len;
      normals.push_back(v);
      // texcoord
      t[0] = (float)j / (float)(4 * (i + 1));
      t[1] = (float)(divisions - i - 1) / (float)divisions;
      t[1] = 0.5f + (0.5f * t[1]);
      texcoords.push_back(t);

      // the bottom hemisphere
      if (i != (divisions - 1)) {
        // vertex
        v[2] = (2 * pos[2]) - v[2];
        vertices.push_back(v);
        // normal
        n[2] = -n[2];
        normals.push_back(v);
        // texcoord
        t[1] = 1.0f - t[1];
        texcoords.push_back(t);
      }
    }
  }
  
  // the closing strip of texture coordinates
//  const int texStripOffset = (2 * ringOffset) + (divisions * 4) + 1;
  const int texStripOffset = texcoords.size();
  t[0] = 0.5f; // weirdness
  t[1] = 1.0f;
  texcoords.push_back(t);
  t[1] = 0.0f;
  texcoords.push_back(t);
  for (i = 0; i < divisions; i++) {
    t[0] = 1.0f;
    t[1] = (float)(divisions - i - 1) / (float)divisions;
    t[1] = 0.5f + (0.5f * t[1]);
    texcoords.push_back(t);
    // the bottom hemisphere
    if (i != (divisions - 1)) {
      t[1] = 1.0f - t[1];
      texcoords.push_back(t);
    }
  }

  // add the checkpoint (one is sufficient)
  
  v[0] = pos[0];
  v[1] = pos[1];
  v[2] = pos[2];
  checkPoints.push_back(v);
  checkTypes.push_back(MeshObstacle::CheckInside);

  // make the mesh
  int faceCount = (divisions * divisions) * 8;
  MeshObstacle* mesh =
    new MeshObstacle(checkTypes, checkPoints, vertices, normals, texcoords,
                     faceCount, driveThrough, shootThrough);
                     
  // add the faces to the mesh
  std::vector<int> verticesList;
  std::vector<int> normalsList;
  std::vector<int> texcoordsList;

  int k = (divisions - 1);
  const int ringOffset = 1 + (((k*k)+k)*2);
  
  for (q = 0; q < 4; q++) {
    for (i = 0; i < divisions; i++) {
      for (j = 0; j < (i + 1); j++) {
        int a, b, c, d, ta, tc;
        const bool lastStrip = ((q == 3) && (j == i));
        const bool lastCircle = (i == (divisions - 1));

        // setup 'a'
        if (i > 0) {
          if (lastStrip) {
            int k = (i - 1);
            a = 1 + (((k*k)+k)*2);
          } else {
            int k = (i - 1);
            a = 1 + (((k*k)+k)*2) + (q*(k+1)) + j;
          }
        } else {
          a = 0;
        }
        
        // setup 'b'
        b = 1 + (((i*i)+i)*2) + (q*(i+1)) + j;
        
        // setup 'c'
        if (lastStrip) {
          c = 1 + (((i*i)+i)*2);
        } else {
          c = b + 1;
        }
        
        // setup 'd' for the down-pointing triangle
        int k = (i + 1);
        d = 1 + (((k*k)+k)*2) + (q*(k+1)) + (j + 1);
        

        // top hemisphere
        a = a * 2;
        if (!lastCircle) {
          b = b * 2;
          c = c * 2;
        } else {
          b = b + ringOffset;
          c = c + ringOffset;
        }
        if (i != (divisions - 2)) {
          d = d * 2;
        } else {
          d = d + ringOffset;
        }

        // deal with the last strip of texture coordinates
        if (!lastStrip) {
          ta = a;
          tc = c;
        } else {
          ta = texStripOffset + (i * 2);
          tc = texStripOffset + ((i + 1) * 2);
        }
        
        push3Ints(verticesList, a, b, c);
        push3Ints(normalsList, a, b, c);
        push3Ints(texcoordsList, ta, b, tc);
        addFace(mesh, verticesList, normalsList, texcoordsList, material);
        if (!lastCircle) {
          push3Ints(verticesList, b, d, c);
          push3Ints(normalsList, b, d, c);
          push3Ints(texcoordsList, b, d, tc);
          addFace(mesh, verticesList, normalsList, texcoordsList, material);
        }

        // bottom hemisphere
        a = a + 1;
        ta = ta + 1;
        if (!lastCircle) {
          b = b + 1;
          c = c + 1;
          tc = tc + 1;
        }
        if (i != (divisions - 2)) {
          d = d + 1;
        }
        push3Ints(verticesList, a, c, b);
        push3Ints(normalsList, a, c, b);
        push3Ints(texcoordsList, ta, tc, b);
        addFace(mesh, verticesList, normalsList, texcoordsList, material);
        if (!lastCircle) {
          push3Ints(verticesList, b, c, d);
          push3Ints(normalsList, b, c, d);
          push3Ints(texcoordsList, b, tc, d);
          addFace(mesh, verticesList, normalsList, texcoordsList, material);
        }
      }
    }
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
