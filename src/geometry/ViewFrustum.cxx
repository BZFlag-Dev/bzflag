/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <math.h>
#include <string.h>
#include "bzfgl.h"
#include "ViewFrustum.h"

ViewFrustum::ViewFrustum()
{
  static GLfloat defaultEye[3] = { 0.0, 0.0, 0.0 };
  static GLfloat defaultTarget[3] = { 0.0, 1.0, 0.0 };
  static GLfloat identity[16] = { 1.0, 0.0, 0.0, 0.0,
				  0.0, 1.0, 0.0, 0.0,
				  0.0, 0.0, 1.0, 0.0,
				  0.0, 0.0, 0.0, 1.0 };

  // initialize view and projection matrices to identity
  ::memcpy(viewMatrix, identity, sizeof(viewMatrix));
  ::memcpy(billboardMatrix, identity, sizeof(billboardMatrix));
  ::memcpy(projectionMatrix, identity, sizeof(projectionMatrix));
  ::memcpy(deepProjectionMatrix, identity, sizeof(deepProjectionMatrix));

  setProjection(M_PI/4.0, 1.0, 100.0, 1, 1);
  setView(defaultEye, defaultTarget);
}

ViewFrustum::~ViewFrustum()
{
  // do nothing
}

GLfloat			ViewFrustum::getEyeDepth(const GLfloat* p) const
{
  return viewMatrix[2] * p[0] + viewMatrix[6] * p[1] +
	viewMatrix[10] * p[2] + viewMatrix[14];
}

void			ViewFrustum::setView(const GLfloat* _eye,
						const GLfloat* target)
{
  // set eye point
  eye[0] = _eye[0];
  eye[1] = _eye[1];
  eye[2] = _eye[2];

  // compute forward vector and normalize
  plane[0][0] = target[0] - eye[0];
  plane[0][1] = target[1] - eye[1];
  plane[0][2] = target[2] - eye[2];
  GLfloat d = 1.0f / sqrtf(plane[0][0] * plane[0][0] +
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
  const GLfloat xs = 1.0f / projectionMatrix[0];
  const GLfloat ys = 1.0f / projectionMatrix[5];
  GLfloat edge[4][3];
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

  // make far corners
  for (int i = 0; i < 4; i++) {
    farCorner[i][0] = eye[0] + m_far * edge[i][0];
    farCorner[i][1] = eye[1] + m_far * edge[i][1];
    farCorner[i][2] = eye[2] + m_far * edge[i][2];
  }
}

void			ViewFrustum::setProjection(GLfloat fov, GLfloat _m_near,
						GLfloat _m_far, GLint width,
						GLint height)
{
  // do easy stuff
  m_near = _m_near;
  m_far = _m_far;
  fovx = fov;

  // compute projectionMatrix
  const GLfloat s = 1.0f / tanf(fov / 2.0f);
  projectionMatrix[0] = s;
  projectionMatrix[5] = (1.0f) * s * GLfloat(width) / GLfloat(height);
  projectionMatrix[8] = 0.0f;
  projectionMatrix[9] = 0.0;
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
  deepProjectionMatrix[10] = -(10.0f * m_far + m_near) / (10.0f * m_far - m_near);
  deepProjectionMatrix[14] = -20.0f * m_far * m_near / (10.0f * m_far - m_near);

  // get field of view in y direction
  fovy = atanf(1.0f / projectionMatrix[5]);

  // compute areaFactor
  areaFactor = 0.25f * s * GLfloat(height);
  areaFactor = M_PI * areaFactor * areaFactor;
}

void			ViewFrustum::setOffset(
				GLfloat eyeOffset, GLfloat focalPlane)
{
  projectionMatrix[12] = 0.5f * eyeOffset * projectionMatrix[0];
  projectionMatrix[8] = projectionMatrix[12] / focalPlane;
  deepProjectionMatrix[8] = projectionMatrix[8];
  deepProjectionMatrix[12] = projectionMatrix[12];
}

void			ViewFrustum::executeProjection() const
{
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projectionMatrix);
  glMatrixMode(GL_MODELVIEW);
}

void			ViewFrustum::executeDeepProjection() const
{
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(deepProjectionMatrix);
  glMatrixMode(GL_MODELVIEW);
}

void			ViewFrustum::executeView() const
{
  glMultMatrixf(viewMatrix);
}

void			ViewFrustum::executeOrientation() const
{
  glMultMatrixf(viewMatrix);
  glTranslatef(eye[0], eye[1], eye[2]);
}

void			ViewFrustum::executeBillboard() const
{
  glMultMatrixf(billboardMatrix);
}

void			ViewFrustum::makePlane(const GLfloat* v1,
						const GLfloat* v2, int index)
{
  // get normal by crossing v1 and v2 and normalizing
  GLfloat n[3];
  n[0] = v1[1] * v2[2] - v1[2] * v2[1];
  n[1] = v1[2] * v2[0] - v1[0] * v2[2];
  n[2] = v1[0] * v2[1] - v1[1] * v2[0];
  GLfloat d = 1.0f / sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
  plane[index][0] = d * n[0];
  plane[index][1] = d * n[1];
  plane[index][2] = d * n[2];
  plane[index][3] = -(eye[0] * plane[index][0] + eye[1] * plane[index][1] +
						eye[2] * plane[index][2]);
}
// ex: shiftwidth=2 tabstop=8
