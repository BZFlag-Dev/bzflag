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

class ViewFrustum {
public:
						ViewFrustum();
						~ViewFrustum();

	const float*		getEye() const;
	const float*		getDirection() const;
	const float*		getUp() const;
	const float*		getRight() const;
	const float*		getSide(int index) const;
	const float*		getFarCorner(int index) const;
	float				getNear() const;
	float				getFar() const;
	const float*		getViewMatrix() const;
	float				getFOVx() const;
	float				getFOVy() const;
	const float*		getProjectionMatrix() const;
	float				getEyeDepth(const float*) const;
	float				getAreaFactor() const;

	void				setView(const float* eye, const float* target);
	void				setProjection(float fov,
							float m_near, float m_far,
							int width, int height);
	void				setOffset(float eyeOffset, float focalPlane);

	static const float*	getTransform();

private:
	void				makePlane(const float* v1, const float* v2, int);

private:
	float				eye[3];
	float				right[3], up[3];
	float				plane[5][4];		      // pointing in
	float				farCorner[4][3];
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
	return plane[0];
}

inline
const float*			ViewFrustum::getSide(int index) const
{
	return plane[index];
}

inline
const float*			ViewFrustum::getFarCorner(int index) const
{
	return farCorner[index];
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
