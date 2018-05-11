/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 * Writen By Jeffrey Myers
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef __OPENGLUTILS_H__
#define __OPENGLUTILS_H__

#include "common.h"

// system headers
#include <string>
#include <map>
#include <vector>

// common headers
#include "bzfgl.h"
#include "vectors.h"


class BzMaterial;
class OpenGLGState;


#define INVALID_GL_ID 0xffffffff


extern void bzMat2gstate(const BzMaterial* bzmat, OpenGLGState& gstate,
			 fvec4& color, const fvec4*& colorPtr);

extern bool parseBlendFactors(const std::string& s, GLenum& src, GLenum& dst);

extern float getFloatColor(int val);
extern void setColor(float dstColor[3], int r, int g, int b);
extern void glSetColor(const float c[3], float alpha = 1.0f);
extern void glTranslatefv(const float v[3]);

typedef enum {
  eCenter,
  eLowerLeft,
  eLowerRight,
  eUpperLeft,
  eUpperRight,
  eCenterLeft,
  eCenterRight,
  eCenterTop,
  eCenterBottom
} eAlignment;


#endif // __OPENGLUTILS_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
