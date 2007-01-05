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

// interface headers
#include "HUDuiImage.h"

// common implementation headers
#include "bzfgl.h"
#include "TextureManager.h"
#include "OpenGLTexture.h"
#include "OpenGLGState.h"

//
// HUDuiImage
//

HUDuiImage::HUDuiImage() : HUDuiElement()
{
}

HUDuiImage::~HUDuiImage()
{
}

void			HUDuiImage::setTexture(const int t)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(t);
  builder.setBlending();
  gstate = builder.getState();
  texture = t;
}

void			HUDuiImage::doRender()
{
  const float _height = getHeight();
  TextureManager &tm = TextureManager::instance();
  const float _width = _height * (1.0f / tm.GetAspectRatio(texture));
  const float xx = getX();
  const float yy = getY();
  gstate.setState();
  glColor3fv(textColor);
  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(xx, yy);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(xx + _width, yy);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(xx + _width, yy + _height);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(xx, yy + _height);
  glEnd();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

