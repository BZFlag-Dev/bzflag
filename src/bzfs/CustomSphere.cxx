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
  texsize[0] = texsize[1] = -4.0f;
  useNormals = true;
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
    if (!(input >> divisions)) {
      return false;
    }
  }
  else if (strcasecmp(cmd, "radius") == 0) {
    float radius;
    if (!(input >> radius)) {
      return false;
    }
    size[0] = size[1] = size[2] = radius;
  }
  else if (strcasecmp(cmd, "texsize") == 0) {
    if (!(input >> texsize[0] >> texsize[1])) {
      return false;
    }
  }
  else if (strcasecmp(cmd, "flatshading") == 0) {
    useNormals = false;
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
  float sz[3], texsz[2];
  const float minSize = 1.0e-6f; // cheezy / lazy

  // absolute the sizes
  sz[0] = fabsf(size[0]);
  sz[1] = fabsf(size[1]);
  sz[2] = fabsf(size[2]);
  
  // adjust the texture sizes
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
    texsz[1] = -((2.0f * sz[2]) / texsz[1]);
  }
  

  // validity checking
  if ((divisions < 1) || (texsz[0] < minSize) || (texsz[1] < minSize) ||
      (sz[0] < minSize) || (sz[1] < minSize) || (sz[2] < minSize)) {
    return;
  }

  // setup the coordinates
  std::vector<char> checkTypes;
  std::vector<cfvec3> checkPoints;
  std::vector<cfvec3> vertices;
  std::vector<cfvec3> normals;
  std::vector<cfvec2> texcoords;

  // the center vertices
  v[0] = pos[0];
  v[1] = pos[1];
  v[2] = pos[2] + sz[2];
  vertices.push_back(v);
  v[2] = pos[2] - sz[2];
  vertices.push_back(v);
  if (useNormals) {
    n[0] = 0.0f;
    n[1] = 0.0f;
    n[2] = 1.0f;
    normals.push_back(n);
    n[2] = -1.0f;
    normals.push_back(n);
  }
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
      float unit[3];
      unit[0] = cos(h_angle) * cos(v_angle);
      unit[1] = sin(h_angle) * cos(v_angle);
      unit[2] = sin(v_angle);
      // vertex
      v[0] = pos[0] + (sz[0] * unit[0]);
      v[1] = pos[1] + (sz[1] * unit[1]);
      v[2] = pos[2] + (sz[2] * unit[2]);
      vertices.push_back(v);
      // normal
      if (useNormals) {
        n[0] = unit[0] / sz[0];
        n[1] = unit[1] / sz[1];
        n[2] = unit[2] / sz[2];
        const float len = 1.0f / sqrtf(vec3dot(n.data, n.data));
        n[0] = n[0] * len;
        n[1] = n[1] * len;
        n[2] = n[2] * len;
        normals.push_back(n);
      }
      // texcoord
      t[0] = (float)j / (float)(4 * (i + 1));
      t[0] = t[0] * texsz[0];
      t[1] = (float)(divisions - i - 1) / (float)divisions;
      t[1] = 0.5f + (0.5f * t[1]);
      t[1] = t[1] * texsz[1];
      texcoords.push_back(t);

      // the bottom hemisphere
      if (i != (divisions - 1)) {
        // vertex
        v[2] = (2 * pos[2]) - v[2];
        vertices.push_back(v);
        // normal
        if (useNormals) {
          n[2] = -n[2];
          normals.push_back(n);
        }
        // texcoord
        t[1] = texsz[1] - t[1];
        texcoords.push_back(t);
      }
    }
  }

  // the closing strip of texture coordinates
//  const int texStripOffset = (2 * ringOffset) + (divisions * 4) + 1;
  const int texStripOffset = texcoords.size();
  t[0] = texsz[0] * 0.5f; // weirdness
  t[1] = texsz[1] * 1.0f;
  texcoords.push_back(t);
  t[1] = 0.0f;
  texcoords.push_back(t);
  for (i = 0; i < divisions; i++) {
    t[0] = texsz[0] * 1.0f;
    t[1] = (float)(divisions - i - 1) / (float)divisions;
    t[1] = 0.5f + (0.5f * t[1]);
    t[1] = texsz[1] * t[1];
    texcoords.push_back(t);
    // the bottom hemisphere
    if (i != (divisions - 1)) {
      t[1] = texsz[1] - t[1];
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
  std::vector<int> vlist;
  std::vector<int> nlist;
  std::vector<int> tlist;

  int k = (divisions - 1);
  const int ringOffset = 1 + (((k*k)+k)*2);

  for (q = 0; q < 4; q++) {
    for (i = 0; i < divisions; i++) {
      for (j = 0; j < (i + 1); j++) {
        int a, b, c, d, ta, tc;
        // a, b, c form the upwards pointing triangle
        // b, c, d form the downwards pointing triangle
        // ta and tc are the texcoords for a and c
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

        push3Ints(vlist, a, b, c);
        if (useNormals) push3Ints(nlist, a, b, c);
        push3Ints(tlist, ta, b, tc);
        addFace(mesh, vlist, nlist, tlist, material);
        if (!lastCircle) {
          push3Ints(vlist, b, d, c);
          if (useNormals) push3Ints(nlist, b, d, c);
          push3Ints(tlist, b, d, tc);
          addFace(mesh, vlist, nlist, tlist, material);
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
        push3Ints(vlist, a, c, b);
        if (useNormals) push3Ints(nlist, a, c, b);
        push3Ints(tlist, ta, tc, b);
        addFace(mesh, vlist, nlist, tlist, material);
        if (!lastCircle) {
          push3Ints(vlist, b, c, d);
          if (useNormals) push3Ints(nlist, b, c, d);
          push3Ints(tlist, b, tc, d);
          addFace(mesh, vlist, nlist, tlist, material);
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
