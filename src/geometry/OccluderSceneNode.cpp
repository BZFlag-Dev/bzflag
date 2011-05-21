/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// bzflag common header
#include "common.h"

// interface header
#include "OccluderSceneNode.h"

// system headers
#include <math.h>
#include <string.h>

// common headers
#include "MeshFace.h"
#include "Intersect.h"
#include "ViewFrustum.h"


OccluderSceneNode::OccluderSceneNode(const MeshFace* face) {
  int i;

  noPlane = false;
  setOccluder(true);

  // record plane info
  plane = face->getPlane();

  // record extents info
  extents = face->getExtents();

  // record vertex info
  vertexCount = face->getVertexCount();
  vertices = new fvec3[vertexCount];
  for (i = 0; i < vertexCount; i++) {
    vertices[i] = face->getVertex(i);
  }

  // record sphere info
  fvec4 mySphere(0.0f, 0.0f, 0.0f, 0.0f);
  for (i = 0; i < vertexCount; i++) {
    mySphere.xyz() += vertices[i];
  }
  mySphere.xyz() /= (float)vertexCount;

  for (i = 0; i < vertexCount; i++) {
    const float distSq = (mySphere.xyz() - vertices[i]).lengthSq();
    if (mySphere.w < distSq) {
      mySphere.w = distSq;
    }
  }
  setSphere(mySphere);

  return;
}


OccluderSceneNode::~OccluderSceneNode() {
  delete[] vertices;
  return;
}


bool OccluderSceneNode::cull(const ViewFrustum& frustum) const {
  // cull if eye is behind (or on) plane
  const fvec3& eye = frustum.getEye();
  if (plane.planeDist(eye) <= 0.0f) {
    return true;
  }

  // if the Visibility culler tells us that we're
  // fully visible, then skip the rest of these tests
  if (octreeState == OctreeVisible) {
    return false;
  }

  const Frustum* f = (const Frustum*) &frustum;
  if (Intersect::testAxisBoxInFrustum(extents, f) == Intersect::Outside) {
    return true;
  }

  // probably visible
  return false;
}


bool OccluderSceneNode::inAxisBox(const Extents& exts) const {
  if (!extents.touches(exts)) {
    return false;
  }

  return Intersect::testPolygonInAxisBox(vertexCount,
                                         (const fvec3*)vertices, plane, exts);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
