/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface headers
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "HUDuiTextureLabel.h"

// system headers
#include <iostream>
#include <assert.h>

// common implementation headers
#include "common.h"
#include "bzfgl.h"
#include "TextureManager.h"
#include "OpenGLGState.h"
#include "OpenGLTexture.h"

//
// HUDuiTextureLabel
//

HUDuiTextureLabel::HUDuiTextureLabel() : HUDuiLabel()
{
}

HUDuiTextureLabel::~HUDuiTextureLabel()
{
}

void			HUDuiTextureLabel::setTexture(const int t)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(t);
  builder.setBlending();
  gstate = builder.getState();
  texture = t;
}

void			HUDuiTextureLabel::doRender()
{
  if (getFontFace() < 0) return;

  // render string if texture filter is Off, otherwise draw the texture
  // about the same size and position as the string would be.
  if (OpenGLTexture::getFilter() == OpenGLTexture::Off || !gstate.isTextured() || texture < 0) {
    HUDuiLabel::doRender();
  } else { // why use a font? it's an image, use the image size, let every pixel be seen!!! :)
    const float height = getFontSize();//texture.getHeight();//
    TextureManager  &tm = TextureManager::instance();

    const float width = height * 1.0f/tm.GetAspectRatio(texture);//font.getWidth(getString());
    //const float descent = font.getDescent();
    const float descent = 0;
    const float x = getX();
    const float y = getY();
    gstate.setState();
    glColor3fv(textColor);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(x, y - descent);
      glTexCoord2f(1.0f, 0.0f);
      glVertex2f(x + width, y - descent);
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(x + width, y - descent + height);
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(x, y - descent + height);
    glEnd();
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

