/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include "HUDuiFrame.h"

// system headers
#include <iostream>

// common headers
#include "bzfgl.h"
#include "OpenGLGState.h"
#include "FontManager.h"

//
// HUDuiFrame
//

HUDuiFrame::HUDuiFrame() : lineWidth(1.0f)
{
  color[0] = 1.0f;
  color[1] = 1.0f;
  color[2] = 1.0f;
  color[3] = 1.0f;
  skipRenderLabel = true;
}

HUDuiFrame::~HUDuiFrame()
{
}

void			HUDuiFrame::doRender()
{
  const float height = getHeight();
  const float width = getWidth();
  const int fontFace = getFontFace();
  const float fontSize = getFontSize();
  const float x = getX();
  const float y = getY();

  FontManager &fm = FontManager::instance();
  const float labelWidth = std::max(getLabelWidth(),
    fm.getStrLength(fontFace, fontSize, getLabel()));
  const float labelGap = fm.getStrLength(fontFace, fontSize, "9");

  const float frameY = y + fontSize / 2;
  const float frameX = x - labelGap;
  const float frameWidth = labelGap * 2 + width;

  OpenGLGState::resetState();  // fixme: shouldn't be needed
  glLineWidth(lineWidth);
  glColor4fv(color);
  glBegin(GL_LINES);
    glVertex2f(frameX, frameY);
    glVertex2f(frameX, frameY - height);

    glVertex2f(frameX, frameY - height);
    glVertex2f(frameX + frameWidth, frameY - height);

    glVertex2f(frameX + frameWidth, frameY - height);
    glVertex2f(frameX + frameWidth, frameY);

    if (width > labelWidth + labelGap) {
      glVertex2f(frameX + frameWidth, frameY);
      glVertex2f(frameX + labelWidth + labelGap, frameY);

      glVertex2f(frameX, frameY);
      glVertex2f(frameX + labelGap, frameY);
    }
  glEnd();

  fm.drawString(x, y, 0, fontFace, fontSize, getLabel());
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

