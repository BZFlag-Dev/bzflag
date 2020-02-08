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
#ifndef _OPENGLCOMMON_H
#define _OPENGLCOMMON_H

#include "common.h"

// System headers
#include <glm/fwd.hpp>

class OpenGLCommon
{
public:
    static void getFogColor(glm::vec4 &fogColor);
    static void setFogColor(const glm::vec4 &fogColor);
    static void setEyePlanes(const glm::vec4 &sPlane, const glm::vec4 &tPlane);
    static void getViewPort(glm::ivec4 &viewport);
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
