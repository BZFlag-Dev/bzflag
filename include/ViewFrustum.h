/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* ViewFrustum
 *	Encapsulates a camera.
 */

#ifndef	BZF_VIEW_FRUSTUM_H
#define	BZF_VIEW_FRUSTUM_H

#include "common.h"
#include "bzfgl.h"

// FIXME -- will need a means for off center projections for
//	looking through teleporters

class ViewFrustum {
  public:
			ViewFrustum();
			~ViewFrustum();

    const GLfloat*	getEye() const;
    const GLfloat*	getDirection() const;
    const GLfloat*	getUp() const;
    const GLfloat*	getRight() const;
    const GLfloat*	getSide(int index) const;
    const GLfloat*	getFarCorner(int index) const;
    GLfloat		getNear() const;
    GLfloat		getFar() const;
    const GLfloat*	getViewMatrix() const;
    GLfloat		getFOVx() const;
    GLfloat		getFOVy() const;
    const GLfloat*	getProjectionMatrix() const;
    GLfloat		getEyeDepth(const GLfloat*) const;
    GLfloat		getAreaFactor() const;

    void		setView(const GLfloat* eye, const GLfloat* target);
    void		setProjection(GLfloat fov, GLfloat m_near, GLfloat m_far,
					GLint width, GLint height, GLint viewHeight);
    void		setOffset(GLfloat eyeOffset, GLfloat focalPlane);

    void		executeProjection() const;
    void		executeDeepProjection() const;
    void		executeView() const;
    void		executeOrientation() const;
    void		executePosition() const;
    void		executeBillboard() const;

  private:
    void		makePlane(const GLfloat* v1, const GLfloat* v2, int);

  private:
    GLfloat		eye[3];
    GLfloat		right[3], up[3];
    GLfloat		plane[5][4];		// pointing in
    GLfloat		farCorner[4][3];
    GLfloat		viewMatrix[16];
    GLfloat		billboardMatrix[16];
    GLfloat		m_near, m_far;
    GLfloat		fovx, fovy;
    GLfloat		areaFactor;
    GLfloat		projectionMatrix[16];
    GLfloat		deepProjectionMatrix[16];
};

//
// ViewFrustum
//

inline const GLfloat*	ViewFrustum::getEye() const
{
  return eye;
}

inline const GLfloat*	ViewFrustum::getDirection() const
{
  return plane[0];
}

inline const GLfloat*	ViewFrustum::getSide(int index) const
{
  return plane[index];
}

inline const GLfloat*	ViewFrustum::getFarCorner(int index) const
{
  return farCorner[index];
}

inline const GLfloat*	ViewFrustum::getUp() const
{
  return up;
}

inline const GLfloat*	ViewFrustum::getRight() const
{
  return right;
}

inline GLfloat		ViewFrustum::getNear() const
{
  return m_near;
}

inline GLfloat		ViewFrustum::getFar() const
{
  return m_far;
}

inline GLfloat		ViewFrustum::getFOVx() const
{
  return fovx;
}

inline GLfloat		ViewFrustum::getFOVy() const
{
  return fovy;
}

inline const GLfloat*	ViewFrustum::getViewMatrix() const
{
  return viewMatrix;
}

inline const GLfloat*	ViewFrustum::getProjectionMatrix() const
{
  return projectionMatrix;
}

inline GLfloat		ViewFrustum::getAreaFactor() const
{
  return areaFactor;
}

#endif // BZF_VIEW_FRUSTUM_H
// ex: shiftwidth=2 tabstop=8
