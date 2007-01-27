/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag common header
#include "common.h"

// interface headers
#include "Frustum.h"
#include "ViewFrustum.h"

// common implementation headers
#include "bzfgl.h"

ViewFrustum::ViewFrustum()
{
}

ViewFrustum::~ViewFrustum()
{
  // do nothing
}

void			ViewFrustum::executeProjection() const
{
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projectionMatrix);
  glMatrixMode(GL_MODELVIEW);
}

void			ViewFrustum::executeDeepProjection() const
{
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(deepProjectionMatrix);
  glMatrixMode(GL_MODELVIEW);
}

void			ViewFrustum::executeView() const
{
  glMultMatrixf(viewMatrix);
}

void			ViewFrustum::executeOrientation() const
{
  glMultMatrixf(viewMatrix);
  glTranslatef(eye[0], eye[1], eye[2]);
}

void			ViewFrustum::executeBillboard() const
{
  glMultMatrixf(billboardMatrix);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
