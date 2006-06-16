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
#include "BZDBCache.h"

//
// HUDuiFrame
//

HUDuiFrame::HUDuiFrame() : lineWidth(1.0f),
			   style(RectangleStyle)
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

void HUDuiFrame::drawArc(float x, float y, float r, int sides,
			 float atAngle, float arcAngle)
{
  // sanity
  if (sides <= 0)
     sides = 1;

  float i = arcAngle / sides;

  glPushMatrix();

  // center
  glTranslatef(x, y, 0);

  // draw
  glBegin(GL_LINE_STRIP);
    for (float a = atAngle; a - (atAngle + arcAngle) < 0.001f; a += i) {
      glVertex2f(r * cos(a), r * sin(a));
    }
  glEnd();

  glPopMatrix();
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

  // set up appearance
  OpenGLGState::resetState();  // fixme: shouldn't be needed
  glLineWidth(lineWidth);
  glColor4fv(color);

  const bool antialias = BZDBCache::blend && BZDBCache::smooth;
  if (antialias) {
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
  }

  // render frame
  switch (style) {
    case RectangleStyle:
      {
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
      }
      break;
    case RoundedRectStyle:
      {
	glBegin(GL_LINES);
	  glVertex2f(frameX, frameY - labelGap);
	  glVertex2f(frameX, frameY - height + labelGap);

	  glVertex2f(frameX + labelGap, frameY - height);
	  glVertex2f(frameX + frameWidth - labelGap, frameY - height);

	  glVertex2f(frameX + frameWidth, frameY - height + labelGap);
	  glVertex2f(frameX + frameWidth, frameY - labelGap);

	  if (width > labelWidth + labelGap) {
	    glVertex2f(frameX + frameWidth - labelGap, frameY);
	    glVertex2f(frameX + labelWidth + labelGap, frameY);
	  }
	glEnd();
	
	const float ninety = (float) M_PI / 2.0f;
	drawArc(frameX + frameWidth - labelGap, frameY - labelGap, labelGap, 20,
		0, ninety);
	drawArc(frameX + labelGap, frameY - labelGap, labelGap, 20,
		ninety, ninety);
	drawArc(frameX + labelGap, frameY - height + labelGap, labelGap, 20,
		2*ninety, ninety);
	drawArc(frameX + frameWidth - labelGap, frameY - height + labelGap, labelGap, 20,
		3*ninety, ninety);
      }
      break;
  }

  if (antialias) {
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
  }

  // render label
  fm.drawString(x, y, 0, fontFace, fontSize, getLabel());
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

