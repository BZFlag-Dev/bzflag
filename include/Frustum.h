/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Frustum
 * Encapsulates a camera.
 */

#ifndef BZF_FRUSTUM_H
#define BZF_FRUSTUM_H

#include "common.h"

// FIXME -- will need a means for off center projections for
// looking through teleporters

class Frustum {
  public:
			Frustum();
			~Frustum();

    const float*	getEye() const;
    const float*	getDirection() const;
    const float*	getUp() const;
    const float*	getRight() const;
    const float*	getSide(int index) const;
    int			getPlaneCount() const;
    const float*	getFarCorner(int index) const;
    float		getTilt() const; // degrees
    float		getRotation() const; // degrees
    float		getNear() const;
    float		getFar() const;
    const float*	getViewMatrix() const;
    float		getFOVx() const;
    float		getFOVy() const;
    const float*	getProjectionMatrix() const;
    float		getEyeDepth(const float*) const;
    float		getAreaFactor() const;

    void		setView(const float* eye, const float* target);
    void		setProjection(float fov,
                                      float m_near, float m_far, float m_deep_far,
                                      int width, int height, int viewHeight);
    void		setOffset(float eyeOffset, float focalPlane);
    void		setFarPlaneCull(bool useCulling);
    void		flipVertical();
    void		flipHorizontal();

    // used for radar culling
    void		setOrthoPlanes(const Frustum& view,
				       float width, float breadth);

  protected:
    void		makePlane(const float* v1, const float* v2, int);

  protected:
    float		eye[3];
    float		target[3];
    float		right[3], up[3];
    float		plane[6][4];		// pointing in
    int			planeCount;
    float		farCorner[4][3];
    float		tilt;
    float		rotation;
    float		viewMatrix[16];
    float		billboardMatrix[16];
    float		m_near, m_far;
    float		fovx, fovy;
    float		areaFactor;
    float		projectionMatrix[16];
    float		deepProjectionMatrix[16];
};

//
// Frustum
//

inline const float*	Frustum::getEye() const
{
  return eye;
}

inline const float*	Frustum::getDirection() const
{
  return plane[0];
}

inline const float*	Frustum::getSide(int index) const
{
  return plane[index];
}

inline int		Frustum::getPlaneCount() const
{
  return planeCount;
}

inline const float*	Frustum::getFarCorner(int index) const
{
  return farCorner[index];
}

inline float		Frustum::getTilt() const
{
  return tilt;
}

inline float		Frustum::getRotation() const
{
  return rotation;
}

inline const float*	Frustum::getUp() const
{
  return up;
}

inline const float*	Frustum::getRight() const
{
  return right;
}

inline float		Frustum::getNear() const
{
  return m_near;
}

inline float		Frustum::getFar() const
{
  return m_far;
}

inline float		Frustum::getFOVx() const
{
  return fovx;
}

inline float		Frustum::getFOVy() const
{
  return fovy;
}

inline const float*	Frustum::getViewMatrix() const
{
  return viewMatrix;
}

inline const float*	Frustum::getProjectionMatrix() const
{
  return projectionMatrix;
}

inline float		Frustum::getAreaFactor() const
{
  return areaFactor;
}

#endif // BZF_FRUSTUM_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

