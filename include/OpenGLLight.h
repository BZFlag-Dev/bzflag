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

/* OpenGLLight:
 *	Encapsulates an OpenGL (point or directional) light source.
 */

#ifndef	BZF_OPENGL_LIGHT_H
#define	BZF_OPENGL_LIGHT_H

#include "bzfgl.h"
#include "common.h"
#include "AList.h"

BZF_DEFINE_ALIST(OpenGLLightDLStack, GLuint);

class OpenGLLight {
  friend class OpenGLLightCleanup;
  public:
			OpenGLLight();
			OpenGLLight(const OpenGLLight&);
			~OpenGLLight();
    OpenGLLight&	operator=(const OpenGLLight&);

    const GLfloat*	getPosition() const;
    const GLfloat*	getColor() const;
    const GLfloat*	getAttenuation() const;

    void		setDirection(const GLfloat* xyz);
    void		setPosition(const GLfloat* xyz);
    void		setColor(GLfloat r, GLfloat g, GLfloat b);
    void		setColor(const GLfloat* rgb);
    void		setAttenuation(const GLfloat* clq);
    void		setAttenuation(int index, GLfloat value);

    GLfloat		getImportance(const GLfloat* pos) const;

    void		execute(int index) const;

    static GLint	getMaxLights();
    static void		enableLight(int index, boolean = True); // const

  protected:
    void		makeLists();
    void		freeLists();
    void		genLight(GLenum light) const;
    static void		cleanup();

  private:
    static void		initContext(void*);

  private:
    GLfloat		pos[4];
    GLfloat		color[4];
    GLfloat		atten[3];
    GLuint		listBase;
    GLuint*		list;
    GLuint		mailbox;
    static GLint	maxLights;
    static OpenGLLightDLStack oldLists;
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

#endif // BZF_OPENGL_LIGHT_H
// ex: shiftwidth=2 tabstop=8
