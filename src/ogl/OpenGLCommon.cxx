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

void OpenGLCommon::ClipPlane(int id, glm::vec4 plane)
{
    const GLdouble myPlane[] = { plane[0], plane[1], plane[2], plane[3]};
    glClipPlane(GL_CLIP_PLANE0 + id, myPlane);
}

void OpenGLCommon::Ortho(float left, float right, float bottom, float top, float nearVal, float farVal)
{
    glLoadIdentity();
    glOrtho(left, right, bottom, top, nearVal, farVal);
}

void OpenGLCommon::ClearDepth()
{
    glClearDepth(1.0);
}

void OpenGLCommon::DepthRange(float depthRange, float depthRangeSize)
{
    GLclampd x_near = (GLclampd)depthRange * depthRangeSize;
    glDepthRange(x_near, x_near + depthRangeSize);
}

void OpenGLCommon::LightModelLocalViewer(bool enable)
{
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, enable ? GL_TRUE : GL_FALSE);
}

void OpenGLCommon::LightModelColorControl()
{
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
