/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _OPENGLAPI_H
#define _OPENGLAPI_H

// 1st
#include "common.h"

// System headers
#include <glm/gtc/type_ptr.hpp>

// common headers
#include "bzfgl.h"


inline void glColor(glm::vec3 c)
{
    glColor3fv(glm::value_ptr(c));
};

inline void glColor(glm::vec4 c)
{
    glColor4fv(glm::value_ptr(c));
};

inline void glVertex(glm::vec2 p)
{
    glVertex2fv(glm::value_ptr(p));
};

inline void glVertex(glm::vec3 p)
{
    glVertex3fv(glm::value_ptr(p));
};

inline void glTranslate(glm::vec3 v)
{
    glTranslatef(v.x, v.y, v.z);
};

inline void glNormal(glm::vec3 n)
{
    glNormal3fv(glm::value_ptr(n));
};

inline void glTexCoord(glm::vec2 t)
{
    glTexCoord2fv(glm::value_ptr(t));
};

inline void glSetFogColor(glm::vec4 fogColor)
{
    glFogfv(GL_FOG_COLOR, glm::value_ptr(fogColor));
};

#endif // _OPENGLAPI_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
