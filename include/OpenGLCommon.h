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

#include "common.h"

// System headers
#include <glm/vec4.hpp>

class OpenGLCommon
{
public:
    static void ClipPlane(int id, glm::vec4 plane);
    static void Ortho(float left, float right, float bottom, float top, float nearVal, float farVal);
    static void ClearDepth();
    static void DepthRange(float depthRange, float depthRangeSize);
    static void LightModelLocalViewer(bool enable);
    static void LightModelColorControl();
};

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
