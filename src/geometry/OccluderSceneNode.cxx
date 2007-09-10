/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// bzflag common header
#include "common.h"

// interface header
#include "OccluderSceneNode.h"

// system headers
#include <math.h>
#include <string.h>

// common implementation headers
#include "MeshFace.h"
#include "Intersect.h"
#include "ViewFrustum.h"


OccluderSceneNode::OccluderSceneNode(const MeshFace* face)
{
  int i;

  noPlane = false;
  setOccluder(true);

  // record plane info
  memcpy(plane, face->getPlane(), sizeof(float[4]));

  // record extents info
  extents = face->getExtents();

  // record vertex info
  vertexCount = face->getVertexCount();
  vertices = new GLfloat3[vertexCount];
  for (i = 0; i < vertexCount; i++) {
    memcpy(vertices[i], face->getVertex(i), sizeof(float[3]));
  }

  // record sphere info
  GLfloat mySphere[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  for (i = 0; i < vertexCount; i++) {
    const float* v = vertices[i];
    mySphere[0] += v[0];
    mySphere[1] += v[1];
    mySphere[2] += v[2];
  }
  mySphere[0] /= (float)vertexCount;
  mySphere[1] /= (float)vertexCount;
  mySphere[2] /= (float)vertexCount;

  for (i = 0; i < vertexCount; i++) {
    const float* v = vertices[i];
    const float dx = mySphere[0] - v[0];
    const float dy = mySphere[1] - v[1];
    const float dz = mySphere[2] - v[2];
    GLfloat r = ((dx * dx) + (dy * dy) + (dz * dz));
    if (r > mySphere[3]) {
      mySphere[3] = r;
    }
  }
  setSphere(mySphere);

  return;
}


OccluderSceneNode::~OccluderSceneNode()
{
  delete[] vertices;
  return;
}


bool OccluderSceneNode::cull(const ViewFrustum& frustum) const
{
  // cull if eye is behind (or on) plane
  const GLfloat* eye = frustum.getEye();
  if (((eye[0] * plane[0]) + (eye[1] * plane[1]) + (eye[2] * plane[2]) +
       plane[3]) <= 0.0f) {
    return true;
  }

  // if the Visibility culler tells us that we're
  // fully visible, then skip the rest of these tests
  if (octreeState == OctreeVisible) {
    return false;
  }

  const Frustum* f = (const Frustum *) &frustum;
  if (testAxisBoxInFrustum(extents, f) == Outside) {
    return true;
  }

  // probably visible
  return false;
}


bool OccluderSceneNode::inAxisBox (const Extents& exts) const
{
  if (!extents.touches(exts)) {
    return false;
  }

  return testPolygonInAxisBox (vertexCount, vertices, plane, exts);
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

