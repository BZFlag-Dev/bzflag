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

/* OpenGLLight:
 *	Encapsulates an OpenGL (point or directional) light source.
 */

#ifndef	BZF_OPENGL_LIGHT_H
#define	BZF_OPENGL_LIGHT_H

// common goes first
#include "common.h"

// system headers
#include <vector>

// common headers
#include "bzfgl.h"
#include "ViewFrustum.h"


class OpenGLLight {
  public:
			OpenGLLight();
			OpenGLLight(const OpenGLLight&);
			~OpenGLLight();
    OpenGLLight&	operator=(const OpenGLLight&);

    const GLfloat*	getPosition() const;
    const GLfloat*	getColor() const;
    const GLfloat*	getAttenuation() const;
    const GLfloat	getMaxDist() const;

    void		setDirection(const GLfloat* xyz);
    void		setPosition(const GLfloat* xyz);
    void		setColor(GLfloat r, GLfloat g, GLfloat b);
    void		setColor(const GLfloat* rgb);
    void		setAttenuation(const GLfloat* clq);
    void		setAttenuation(int index, GLfloat value);

    void		calculateImportance(const ViewFrustum& frustum);
    GLfloat		getImportance() const;

    void		setOnlyReal(bool value);
    bool		getOnlyReal() const;
    void		setOnlyGround(bool value);
    bool		getOnlyGround() const;

    void		execute(int index, bool useList) const;

    static GLint	getMaxLights();
    static void		enableLight(int index, bool on); // const

  protected:
    void		makeLists();
    void		freeLists();
    void		genLight(GLenum light) const;

  private:
    static void		freeContext(void*);
    static void		initContext(void*);

  private:
    GLfloat		pos[4];
    GLfloat		color[4];
    GLfloat		atten[3];
    GLfloat		maxDist;
    GLfloat		importance;
    bool		onlyReal;
    bool		onlyGround;
    GLuint*		lists;
    static GLint	maxLights;
};

//
// OpenGLLight
//

inline const GLfloat*	OpenGLLight::getPosition() const
{
  return pos;
}

inline const GLfloat*	OpenGLLight::getColor() const
{
  return color;
}

inline const GLfloat*	OpenGLLight::getAttenuation() const
{
  return atten;
}

inline const GLfloat	OpenGLLight::getMaxDist() const
{
  return maxDist;
}

inline GLfloat		 OpenGLLight::getImportance() const
{
  return importance;
}

inline bool		 OpenGLLight::getOnlyReal() const
{
  return onlyReal;
}

inline bool		 OpenGLLight::getOnlyGround() const
{
  return onlyGround;
}


#endif // BZF_OPENGL_LIGHT_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

