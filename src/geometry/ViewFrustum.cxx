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

	setProjection(45.0f, 1.0f, 1.0f, 100.0f);
	setView(defaultEye, defaultTarget);
}

ViewFrustum::~ViewFrustum()
{
	// do nothing
}

void					ViewFrustum::setView(const float* _eye,
												const float* target)
{
	// set eye point
	eye[0] = _eye[0];
	eye[1] = _eye[1];
	eye[2] = _eye[2];

	// compute forward vector and normalize
	forward[0] = target[0] - eye[0];
	forward[1] = target[1] - eye[1];
	forward[2] = target[2] - eye[2];
	float d = 1.0f / hypotf(forward[0], hypotf(forward[1], forward[2]));
	forward[0] *= d;
	forward[1] *= d;
	forward[2] *= d;

	// compute left vector (by crossing forward with
	// world-up [0 0 1]T and normalizing)
	right[0] =  forward[1];
	right[1] = -forward[0];
	d = 1.0f / hypotf(right[0], right[1]);
	right[0] *= d;
	right[1] *= d;
	right[2] = 0.0f;

	// compute local up vector (by crossing right and forward,
	// normalization unnecessary)
	up[0] =  right[1] * forward[2];
	up[1] = -right[0] * forward[2];
	up[2] =  right[0] * forward[1] - right[1] * forward[0];

	// build view matrix, including a transformation bringing
	// world up [0 0 1 0]T to eye up [0 1 0 0]T, world north
	// [0 1 0 0]T to eye forward [0 0 -1 0]T.
	viewMatrix[0] = right[0];
	viewMatrix[4] = right[1];
	viewMatrix[8] = 0.0f;

	viewMatrix[1] = up[0];
	viewMatrix[5] = up[1];
	viewMatrix[9] = up[2];

	viewMatrix[2] =  -forward[0];
	viewMatrix[6] =  -forward[1];
	viewMatrix[10] = -forward[2];

	viewMatrix[12] = -(viewMatrix[0] * eye[0] +
						viewMatrix[4] * eye[1] +
						viewMatrix[8] * eye[2]);
	viewMatrix[13] = -(viewMatrix[1] * eye[0] +
						viewMatrix[5] * eye[1] +
						viewMatrix[9] * eye[2]);
	viewMatrix[14] = -(viewMatrix[2] * eye[0] +
						viewMatrix[6] * eye[1] +
						viewMatrix[10] * eye[2]);
}

void					ViewFrustum::setProjection(
								float fov, float aspectRatio,
								float _m_near, float _m_far)
{
	// do easy stuff
	m_near = _m_near;
	m_far  = _m_far;
	fovx   = fov;

	// compute projectionMatrix
	const float s = 1.0f / tanf(fovx * M_PI / 360.0f);
	projectionMatrix[0] = s;
	projectionMatrix[5] = s * aspectRatio;
	projectionMatrix[8] = 0.0f;
	projectionMatrix[9] = 0.0f;
	projectionMatrix[10] = -(m_far + m_near) / (m_far - m_near);
	projectionMatrix[11] = -1.0f;
	projectionMatrix[12] = 0.0f;
	projectionMatrix[14] = -2.0f * m_far * m_near / (m_far - m_near);
	projectionMatrix[15] = 0.0f;

	// get field of view in y direction
	fovy = atanf(1.0f / projectionMatrix[5]) * 360.0f / M_PI;
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

const float*				ViewFrustum::getTransform()
{
	// this matrix undoes the coordinate transform that setView() does.
	//
	// note that, as formatted, the matrix is transposed wrt to how
	// the matrix is used (due to the OpenGL matrix layout).
	static const float matrix[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return matrix;
}
