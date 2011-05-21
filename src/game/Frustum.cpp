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

#include "common.h"
#include <math.h>
#include <string.h>
#include "Frustum.h"


Frustum::Frustum()
{
  static fvec3 defaultEye(0.0f, 0.0f, 0.0f);
  static fvec3 defaultTarget(0.0f, 1.0f, 0.0f);
  static float identity[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
  };

  // initialize view and projection matrices to identity
  ::memcpy(viewMatrix, identity, sizeof(viewMatrix));
  ::memcpy(billboardMatrix, identity, sizeof(billboardMatrix));
  ::memcpy(projectionMatrix, identity, sizeof(projectionMatrix));
  ::memcpy(deepProjectionMatrix, identity, sizeof(deepProjectionMatrix));

  setProjection((float)(M_PI/4.0), 1.0f, 100.0f, 1000.0f, 1, 1, 1);
  setView(defaultEye, defaultTarget);
}


Frustum::~Frustum()
{
  // do nothing
}


float Frustum::getEyeDepth(const float* p) const
{
  return (viewMatrix[2]  * p[0]) +
         (viewMatrix[6]  * p[1]) +
         (viewMatrix[10] * p[2]) + viewMatrix[14];
}


void Frustum::setView(const fvec3& _eye, const fvec3& _target)
{
  // set eye and target points
  eye    = _eye;
  target = _target;

  // compute forward vector and normalize
  const fvec3 dir = (target - eye).normalize();

  // compute left vector (by crossing forward with
  // world-up [0 0 1]T and normalizing)
  right.x =  dir.y;
  right.y = -dir.x;
  const float rd = 1.0f / hypotf(right.x, right.y);
  right.x *= rd;
  right.y *= rd;
  right.z = 0.0f;

  // compute local up vector (by crossing right and forward,
  // normalization unnecessary)
  up.x =  right.y * dir.z;
  up.y = -right.x * dir.z;
  up.z = (right.x * dir.y) - (right.y * dir.x);

  // build view matrix, including a transformation bringing
  // world up [0 0 1 0]T to eye up [0 1 0 0]T, world north
  // [0 1 0 0]T to eye forward [0 0 -1 0]T.
  viewMatrix[0] = right.x;
  viewMatrix[4] = right.y;
  viewMatrix[8] = 0.0f;

  viewMatrix[1] = up.x;
  viewMatrix[5] = up.y;
  viewMatrix[9] = up.z;

  viewMatrix[2] =  -dir.x;
  viewMatrix[6] =  -dir.y;
  viewMatrix[10] = -dir.z;

  viewMatrix[12] = -(viewMatrix[0]  * eye.x +
                     viewMatrix[4]  * eye.y +
                     viewMatrix[8]  * eye.z);
  viewMatrix[13] = -(viewMatrix[1]  * eye.x +
                     viewMatrix[5]  * eye.y +
                     viewMatrix[9]  * eye.z);
  viewMatrix[14] = -(viewMatrix[2]  * eye.x +
                     viewMatrix[6]  * eye.y +
                     viewMatrix[10] * eye.z);

  // build billboard matrix.  billboard matrix performs rotation
  // so that polygons drawn in the xy plane face the camera.
  billboardMatrix[0] = viewMatrix[0];
  billboardMatrix[1] = viewMatrix[4];
  billboardMatrix[2] = viewMatrix[8];
  billboardMatrix[4] = viewMatrix[1];
  billboardMatrix[5] = viewMatrix[5];
  billboardMatrix[6] = viewMatrix[9];
  billboardMatrix[8] = viewMatrix[2];
  billboardMatrix[9] = viewMatrix[6];
  billboardMatrix[10] = viewMatrix[10];

  // compute vectors of frustum edges
  const float xs = fabsf(1.0f / projectionMatrix[0]);
  const float ys = fabsf(1.0f / projectionMatrix[5]);
  fvec3 edge[4];
  edge[0] = dir - (xs * right) - (ys * up);
  edge[1] = dir + (xs * right) - (ys * up);
  edge[2] = dir + (xs * right) + (ys * up);
  edge[3] = dir - (xs * right) + (ys * up);

  // make frustum planes
  plane[0] = fvec4(dir, -fvec3::dot(eye, dir));
  makePlane(edge[0], edge[3], 1);
  makePlane(edge[2], edge[1], 2);
  makePlane(edge[1], edge[0], 3);
  makePlane(edge[3], edge[2], 4);

  plane[5] = -plane[0];
  plane[5].w += m_far;

  // make far corners
  for (int i = 0; i < 4; i++) {
    farCorner[i] = eye + (m_far * edge[i]);
  }

  // setup tilt and angle
  tilt     = (float)((180.0 / M_PI) * atan2((double)dir[2], 1.0));
  rotation = (float)((180.0 / M_PI) * atan2((double)dir[1], (double)dir[2]));
}


void Frustum::setFarPlaneCull(bool useCulling)
{
  // far clip plane
  if (useCulling) {
    planeCount = 6;
  } else {
    planeCount = 5;
  }
}


void Frustum::setProjection(float fov,
			    float _m_near, float _m_far, float _m_deep_far,
			    int width, int height, int viewHeight)
{
  // do easy stuff
  m_near     = _m_near;
  m_far      = _m_far;
  m_deep_far = _m_deep_far;

  fovx = fov;

  // clear the far plane culling here
  planeCount = 5;

  // compute projectionMatrix
  const float s = 1.0f / tanf(fov / 2.0f);
  const float fracHeight = 1.0f - float(viewHeight) / float(height);
  projectionMatrix[0] = s;
  projectionMatrix[5] = (1.0f - fracHeight) * s * float(width) / float(viewHeight);
  projectionMatrix[8] = 0.0f;
  projectionMatrix[9] = -fracHeight;
  projectionMatrix[10] = -(m_far + m_near) / (m_far - m_near);
  projectionMatrix[11] = -1.0f;
  projectionMatrix[12] = 0.0f;
  projectionMatrix[14] = -2.0f * m_far * m_near / (m_far - m_near);
  projectionMatrix[15] = 0.0f;

  deepProjectionMatrix[0] = projectionMatrix[0];
  deepProjectionMatrix[5] = projectionMatrix[5];
  deepProjectionMatrix[8] = projectionMatrix[8];
  deepProjectionMatrix[9] = projectionMatrix[9];
  deepProjectionMatrix[11] = projectionMatrix[11];
  deepProjectionMatrix[12] = projectionMatrix[12];
  deepProjectionMatrix[15] = projectionMatrix[15];
  deepProjectionMatrix[10] = -(m_deep_far + m_near) / (m_deep_far - m_near);
  deepProjectionMatrix[14] = -2.0f * m_deep_far * m_near / (m_deep_far - m_near);

  // get field of view in y direction
  fovy = 2.0f * atanf(1.0f / projectionMatrix[5]);

  // compute areaFactor
  areaFactor = 0.25f * s * float(height);
  areaFactor = (float)(M_PI * areaFactor * areaFactor);
}


void Frustum::setOffset(float eyeOffset, float focalPlane)
{
  projectionMatrix[12] = 0.5f * eyeOffset * projectionMatrix[0];
  projectionMatrix[8] = projectionMatrix[12] / focalPlane;
  deepProjectionMatrix[8] = projectionMatrix[8];
  deepProjectionMatrix[12] = projectionMatrix[12];
}


void Frustum::makePlane(const fvec3& v1, const fvec3& v2, int index)
{
  // get normal by crossing v1 and v2 and normalizing
  const fvec3 dir = fvec3::cross(v1, v2).normalize();
  const float dist = -fvec3::dot(eye, dir);
  plane[index] = fvec4(dir, dist);
}


// these next two functions should be more generic
// flipX, flipY, flipZ, all with and offset along the axis
void Frustum::flipVertical()
{
  eye.z = -eye.z;
  target.z = -target.z;
  setView(eye, target);
  projectionMatrix[5] = -projectionMatrix[5];
  deepProjectionMatrix[5] = -deepProjectionMatrix[5];

  return;
}


void Frustum::flipHorizontal()
{
  eye.x = -eye.x;
  target.x = -target.x;
  setView(eye, target);
  projectionMatrix[0] = -projectionMatrix[0];
  deepProjectionMatrix[0] = -deepProjectionMatrix[0];
  return;
}


// used for radar culling, not really a frustum
void Frustum::setOrthoPlanes(const Frustum& view, float width, float breadth)
{
  // setup the eye, and the clipping planes
  eye = view.getEye();

  const fvec3& dir = view.getDirection();
  fvec2 front = dir.xy();
  if (!fvec2::normalize(front)) {
    front = fvec2(1.0f, 0.0f);
  }

  const fvec2 left(-front.y, +front.x);

  plane[1].xy() =  left;
  plane[1].z = 0.0f;
  plane[1].w = -fvec2::dot(eye.xy(), plane[1].xy()) + width;

  plane[2].xy() = -left;
  plane[2].z = 0.0f;
  plane[2].w = -fvec2::dot(eye.xy(), plane[2].xy()) + width;

  plane[3].xy() =  front;
  plane[3].z = 0.0f;
  plane[3].w = -fvec2::dot(eye.xy(), plane[3].xy()) + breadth;

  plane[4].xy() = -front;
  plane[4].z = 0.0f;
  plane[4].w = -fvec2::dot(eye.xy(), plane[4].xy()) + breadth;


  // disable the near and far planes
  plane[0] = fvec4(0.0f, 0.0f, 1.0f, -1.0e6f);
  plane[5] = fvec4(0.0f, 0.0f, 1.0f, -1.0e6f);

  planeCount = 5;

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
