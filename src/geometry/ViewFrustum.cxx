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

// interface headers
#include "ViewFrustum.h"

// System headers
#include <glm/gtc/type_ptr.hpp>

// common implementation headers
#include "bzfgl.h"

ViewFrustum::ViewFrustum()
{
}

ViewFrustum::~ViewFrustum()
{
    // do nothing
}

void            ViewFrustum::executeProjection() const
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projectionMatrix));
    glMatrixMode(GL_MODELVIEW);
}

void            ViewFrustum::executeDeepProjection() const
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(deepProjectionMatrix));
    glMatrixMode(GL_MODELVIEW);
}

void            ViewFrustum::executeView() const
{
    glMultMatrixf(glm::value_ptr(viewMatrix));
}

void            ViewFrustum::executeOrientation() const
{
    glMultMatrixf(glm::value_ptr(viewMatrix));
    glTranslatef(eye[0], eye[1], eye[2]);
}

void            ViewFrustum::executeBillboard() const
{
    glMultMatrixf(glm::value_ptr(billboardMatrix));
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
