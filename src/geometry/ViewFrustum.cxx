/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
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
#include "ViewFrustum.h"

ViewFrustum::ViewFrustum()
{
	static float defaultEye[3] = { 0.0, 0.0, 0.0 };
	static float defaultTarget[3] = { 0.0, 1.0, 0.0 };
	static float identity[16] = { 1.0, 0.0, 0.0, 0.0,
								  0.0, 1.0, 0.0, 0.0,
								  0.0, 0.0, 1.0, 0.0,
								  0.0, 0.0, 0.0, 1.0 };

	// initialize view and projection matrices to identity
	::memcpy(viewMatrix, identity, sizeof(viewMatrix));
	::memcpy(projectionMatrix, identity, sizeof(projectionMatrix));

	setProjection(45.0f, 1.0f, 100.0f, 1, 1);
	setView(defaultEye, defaultTarget);
}

ViewFrustum::~ViewFrustum()
{
	// do nothing
}

float					ViewFrustum::getEyeDepth(const float* p) const
{
	return viewMatrix[2] * p[0] + viewMatrix[6] * p[1] +
		viewMatrix[10] * p[2] + viewMatrix[14];
}

void					ViewFrustum::setView(const float* _eye,
												const float* target)
{
	// set eye point
	eye[0] = _eye[0];
	eye[1] = _eye[1];
	eye[2] = _eye[2];

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

	// compute vectors of frustum edges
	const float xs = 1.0f / projectionMatrix[0];
	const float ys = 1.0f / projectionMatrix[5];
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
	plane[0][3] = -(eye[0] * plane[0][0] +
						eye[1] * plane[0][1] +
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

void					ViewFrustum::setProjection(float fov,
								float _m_near, float _m_far,
								int width, int height)
{
	// do easy stuff
	m_near = _m_near;
	m_far = _m_far;
	fovx = fov;

	// compute projectionMatrix
	const float s = 1.0f / tanf(fov * M_PI / 360.0f);
	projectionMatrix[0] = s;
	projectionMatrix[5] = s * float(width) / float(height);
	projectionMatrix[8] = 0.0f;
	projectionMatrix[9] = 0.0f;
	projectionMatrix[10] = -(m_far + m_near) / (m_far - m_near);
	projectionMatrix[11] = -1.0f;
	projectionMatrix[12] = 0.0f;
	projectionMatrix[14] = -2.0f * m_far * m_near / (m_far - m_near);
	projectionMatrix[15] = 0.0f;

	// get field of view in y direction
	fovy = atanf(1.0f / projectionMatrix[5]) * 180.0f / M_PI;
}

void					ViewFrustum::setOffset(
								float eyeOffset, float focalPlane)
{
	if (focalPlane <= 0.0f) {
		eyeOffset  = 0.0f;
		focalPlane = 1.0f;
	}

	projectionMatrix[12] = 0.5f * eyeOffset * projectionMatrix[0];
	projectionMatrix[8]  = projectionMatrix[12] / focalPlane;
}

void					ViewFrustum::makePlane(const float* v1,
												const float* v2, int index)
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

const float*				ViewFrustum::getTransform()
{
	// this matrix undoes the coordinate transform that setView() does.
	//
	// note that, as formatter, the matrix is transposed wrt to how
	// the matrix is used.
	static const float matrix[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return matrix;
}
