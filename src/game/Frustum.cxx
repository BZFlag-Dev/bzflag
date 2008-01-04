/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
  static float defaultEye[3] = { 0.0, 0.0, 0.0 };
  static float defaultTarget[3] = { 0.0, 1.0, 0.0 };
  static float identity[16] = { 1.0, 0.0, 0.0, 0.0,
				  0.0, 1.0, 0.0, 0.0,
				  0.0, 0.0, 1.0, 0.0,
				  0.0, 0.0, 0.0, 1.0 };

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
  return viewMatrix[2] * p[0] + viewMatrix[6] * p[1] +
	viewMatrix[10] * p[2] + viewMatrix[14];
}


void Frustum::setView(const float* _eye, const float* _target)
{
  // set eye and target points
  eye[0] = _eye[0];
  eye[1] = _eye[1];
  eye[2] = _eye[2];
  target[0] = _target[0];
  target[1] = _target[1];
  target[2] = _target[2];

  // compute forward vector and normalize
  plane[0][0] = target[0] - eye[0];
  plane[0][1] = target[1] - eye[1];
  plane[0][2] = target[2] - eye[2];
  float d = 1.0f / sqrtf(plane[0][0] * plane[0][0] +
			   plane[0][1] * plane[0][1] +
			   plane[0][2] * plane[0][2]);
  plane[0][0] *= d;
  plane[0][1] *= d;
  plane[0][2] *= d;

  // compute left vector (by crossing forward with
  // world-up [0 0 1]T and normalizing)
  right[0] =  plane[0][1];
  right[1] = -plane[0][0];
  d = 1.0f / hypotf(right[0], right[1]);
  right[0] *= d;
  right[1] *= d;
  right[2] = 0.0f;

  // compute local up vector (by crossing right and forward,
  // normalization unnecessary)
  up[0] =  right[1] * plane[0][2];
  up[1] = -right[0] * plane[0][2];
  up[2] =  right[0] * plane[0][1] - right[1] * plane[0][0];

  // build view matrix, including a transformation bringing
  // world up [0 0 1 0]T to eye up [0 1 0 0]T, world north
  // [0 1 0 0]T to eye forward [0 0 -1 0]T.
  viewMatrix[0] = right[0];
  viewMatrix[4] = right[1];
  viewMatrix[8] = 0.0f;

  viewMatrix[1] = up[0];
  viewMatrix[5] = up[1];
  viewMatrix[9] = up[2];

  viewMatrix[2] =  -plane[0][0];
  viewMatrix[6] =  -plane[0][1];
  viewMatrix[10] = -plane[0][2];

  viewMatrix[12] = -(viewMatrix[0] * eye[0] +
			viewMatrix[4] * eye[1] +
			viewMatrix[8] * eye[2]);
  viewMatrix[13] = -(viewMatrix[1] * eye[0] +
			viewMatrix[5] * eye[1] +
			viewMatrix[9] * eye[2]);
  viewMatrix[14] = -(viewMatrix[2] * eye[0] +
			viewMatrix[6] * eye[1] +
			viewMatrix[10] * eye[2]);

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
  float edge[4][3];
  edge[0][0] = plane[0][0] - xs * right[0] - ys * up[0];
  edge[0][1] = plane[0][1] - xs * right[1] - ys * up[1];
  edge[0][2] = plane[0][2] - xs * right[2] - ys * up[2];
  edge[1][0] = plane[0][0] + xs * right[0] - ys * up[0];
  edge[1][1] = plane[0][1] + xs * right[1] - ys * up[1];
  edge[1][2] = plane[0][2] + xs * right[2] - ys * up[2];
  edge[2][0] = plane[0][0] + xs * right[0] + ys * up[0];
  edge[2][1] = plane[0][1] + xs * right[1] + ys * up[1];
  edge[2][2] = plane[0][2] + xs * right[2] + ys * up[2];
  edge[3][0] = plane[0][0] - xs * right[0] + ys * up[0];
  edge[3][1] = plane[0][1] - xs * right[1] + ys * up[1];
  edge[3][2] = plane[0][2] - xs * right[2] + ys * up[2];

  // make frustum planes
  plane[0][3] = -(eye[0] * plane[0][0] + eye[1] * plane[0][1] +
			eye[2] * plane[0][2] + m_near);
  makePlane(edge[0], edge[3], 1);
  makePlane(edge[2], edge[1], 2);
  makePlane(edge[1], edge[0], 3);
  makePlane(edge[3], edge[2], 4);

  plane[5][0] = -plane[0][0];
  plane[5][1] = -plane[0][1];
  plane[5][2] = -plane[0][2];
  plane[5][3] = -plane[0][3] + m_far;

  // make far corners
  for (int i = 0; i < 4; i++) {
    farCorner[i][0] = eye[0] + m_far * edge[i][0];
    farCorner[i][1] = eye[1] + m_far * edge[i][1];
    farCorner[i][2] = eye[2] + m_far * edge[i][2];
  }

  // setup tilt and angle
  const float* dir = plane[0];
  tilt = (float)((180.0 / M_PI) * atan2((double)dir[2], 1.0));
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
			    float _m_near, float _m_far, float m_deep_far,
			    int width, int height, int viewHeight)
{
  // do easy stuff
  m_near = _m_near;
  m_far = _m_far;
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


void Frustum::makePlane(const float* v1, const float* v2, int index)
{
  // get normal by crossing v1 and v2 and normalizing
  float n[3];
  n[0] = v1[1] * v2[2] - v1[2] * v2[1];
  n[1] = v1[2] * v2[0] - v1[0] * v2[2];
  n[2] = v1[0] * v2[1] - v1[1] * v2[0];
  float d = 1.0f / sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
  plane[index][0] = d * n[0];
  plane[index][1] = d * n[1];
  plane[index][2] = d * n[2];
  plane[index][3] = -(eye[0] * plane[index][0] + eye[1] * plane[index][1] +
						eye[2] * plane[index][2]);
}


// these next two functions should be more generic
// flipX, flipY, flipZ, all with and offset along the axis
void Frustum::flipVertical()
{
  eye[2] = -eye[2];
  target[2] = -target[2];
  setView(eye, target);
  projectionMatrix[5] = -projectionMatrix[5];
  deepProjectionMatrix[5] = -deepProjectionMatrix[5];

  return;
}


void Frustum::flipHorizontal()
{
  eye[0] = -eye[0];
  target[0] = -target[0];
  setView(eye, target);
  projectionMatrix[0] = -projectionMatrix[0];
  deepProjectionMatrix[0] = -deepProjectionMatrix[0];
  return;
}


// used for radar culling, not really a frustum
void Frustum::setOrthoPlanes(const Frustum& view, float width, float breadth)
{
  // setup the eye, and the clipping planes
  memcpy(eye, view.getEye(), sizeof(float[3]));

  float front[2], left[2];
  const float* dir = view.getDirection();
  float len = (dir[0] * dir[0]) + (dir[1] * dir[1]);
  if (len != 0) {
    len = 1.0f / sqrtf(len);
    front[0] = dir[0] * len;
    front[1] = dir[1] * len;
  } else {
    front[0] = 1.0f;
    front[1] = 0.0f;
  }

  left[0] = -front[1];
  left[1] = +front[0];

  plane[1][0] = +left[0];
  plane[1][1] = +left[1];
  plane[1][3] = -((eye[0] * plane[1][0]) + (eye[1] * plane[1][1])) + width;

  plane[2][0] = -left[0];
  plane[2][1] = -left[1];
  plane[2][3] = -((eye[0] * plane[2][0]) + (eye[1] * plane[2][1])) + width;

  plane[3][0] = +front[0];
  plane[3][1] = +front[1];
  plane[3][3] = -((eye[0] * plane[3][0]) + (eye[1] * plane[3][1])) + breadth;

  plane[4][0] = -front[0];
  plane[4][1] = -front[1];
  plane[4][3] = -((eye[0] * plane[4][0]) + (eye[1] * plane[4][1])) + breadth;

  plane[1][2] = 0.0f;
  plane[2][2] = 0.0f;
  plane[3][2] = 0.0f;
  plane[4][2] = 0.0f;

  // disable the near and far planes
  plane[0][0] = plane[0][1] = 0.0f;
  plane[0][2] = 1.0f;
  plane[0][3] = -1.0e6;
  plane[5][0] = plane[0][1] = 0.0f;
  plane[5][2] = 1.0f;
  plane[5][3] = -1.0e6;

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
