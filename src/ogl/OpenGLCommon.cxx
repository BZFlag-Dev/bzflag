/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "OpenGLCommon.h"

// System headers
#include <glm/gtc/type_ptr.hpp>

// Common headers
#include "bzfgl.h"

void OpenGLCommon::getFogColor(glm::vec4 &fogColor)
{
    glGetFloatv(GL_FOG_COLOR, glm::value_ptr(fogColor));
}

void OpenGLCommon::setFogColor(const glm::vec4 &fogColor)
{
    glFogfv(GL_FOG_COLOR, glm::value_ptr(fogColor));
}

void OpenGLCommon::setEyePlanes(const glm::vec4 &sPlane,
                                const glm::vec4 &tPlane)
{
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGenfv(GL_S, GL_EYE_PLANE, glm::value_ptr(sPlane));
    glTexGenfv(GL_T, GL_EYE_PLANE, glm::value_ptr(tPlane));
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
