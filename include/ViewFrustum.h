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

/* ViewFrustum
 *	Encapsulates a camera.
 */

#ifndef BZF_VIEW_FRUSTUM_H
#define BZF_VIEW_FRUSTUM_H

#include "common.h"
#include "math3D.h"

class ViewFrustum {
public:
						ViewFrustum();
						~ViewFrustum();

	const float*		getEye() const;
	const float*		getDirection() const;
	const float*		getUp() const;
	const float*		getRight() const;
	float				getFOVx() const;
	float				getFOVy() const;
	float				getNear() const;
	float				getFar() const;
	const float*		getViewMatrix() const;
	const float*		getProjectionMatrix() const;

	void				setView(const float* eye, const float* target);
	void				setProjection(float fovx, float aspectRatio,
							float m_near, float m_far);
	void				setOffset(float eyeOffset, float focalPlane);

	static const Matrix&	getTransform();

private:
	float				eye[3];
	float				forward[3], right[3], up[3];
	float				m_near, m_far;
	float				fovx, fovy;
	float				viewMatrix[16];
	float				projectionMatrix[16];
};

//
// ViewFrustum
//

inline
const float*			ViewFrustum::getEye() const
{
	return eye;
}

inline
const float*			ViewFrustum::getDirection() const
{
	return forward;
}

inline
const float*			ViewFrustum::getUp() const
{
	return up;
}

inline
const float*			ViewFrustum::getRight() const
{
	return right;
}

inline
float					ViewFrustum::getFOVx() const
{
	return fovx;
}

inline
float					ViewFrustum::getFOVy() const
{
	return fovy;
}

inline
float					ViewFrustum::getNear() const
{
	return m_near;
}

inline
float					ViewFrustum::getFar() const
{
	return m_far;
}

inline
const float*			ViewFrustum::getViewMatrix() const
{
	return viewMatrix;
}

inline
const float*			ViewFrustum::getProjectionMatrix() const
{
	return projectionMatrix;
}

#endif // BZF_VIEW_FRUSTUM_H
