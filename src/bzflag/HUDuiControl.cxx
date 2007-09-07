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
#include "HUDuiControl.h"

// common implementation headers
#include "TextureManager.h"

// local implementation headers
#include "HUDui.h"
#include "HUDNavigationQueue.h"


//
// HUDuiControl
//

// init static members
OpenGLGState*		HUDuiControl::gstate = NULL;
int			HUDuiControl::arrow = -1;
int			HUDuiControl::arrowFrame = 0;
TimeKeeper		HUDuiControl::lastTime;
int			HUDuiControl::totalCount = 0;

HUDuiControl::HUDuiControl() : showingFocus(true), navList(NULL), cb(NULL),
				userData(NULL)
{
  if (totalCount == 0) {
    // load arrow texture
    TextureManager &tm = TextureManager::instance();
    arrow = tm.getTextureID( "menu_arrow" );

    // make gstate for focus arrow
    gstate = new OpenGLGState;
    OpenGLGStateBuilder builder(*gstate);
    builder.setTexture(arrow);
    builder.setBlending();
//    builder.setSmoothing();
//    builder.setTextureEnvMode(GL_TEXTURE_2D);
    *gstate = builder.getState();

    // get start time for animation
    lastTime = TimeKeeper::getCurrent();
  }

  totalCount++;
}

HUDuiControl::~HUDuiControl()
{
  if (--totalCount == 0) {
    delete gstate;
    arrow = -1;
    gstate = NULL;
  }
}

HUDuiCallback		HUDuiControl::getCallback() const
{
  return cb;
}

void*			HUDuiControl::getUserData() const
{
  return userData;
}

void			HUDuiControl::setCallback(HUDuiCallback _cb, void* _ud)
{
  cb = _cb;
  userData = _ud;
}

bool			HUDuiControl::hasFocus() const
{
  return this == HUDui::getFocus();
}

void			HUDuiControl::showFocus(bool _showingFocus)
{
  showingFocus = _showingFocus;
}

void			HUDuiControl::doCallback()
{
  if (cb) (*cb)(this, userData);
}

void			HUDuiControl::renderFocus()
{
  float fh2;

  TextureManager &tm = TextureManager::instance();
  const ImageInfo &info = tm.getInfo(arrow);

  const float x = getX();
  const float y = getY();

  if (gstate->isTextured()) { 
    /* draw a fancy textured/image cursor */

    float imageSize = (float)info.y;
    // assumes there are w/h frames of animation h x h in each image
    int uFrames = 1;
    if (imageSize != 0)
      uFrames = int(info.x/imageSize); // 4;
    int vFrames = 1; // 4;
    float du = 1.0f / (float)uFrames;
    float dv = 1.0f / (float)vFrames;

    float u = (float)(arrowFrame % uFrames) / (float)uFrames;
    float v = (float)(arrowFrame / uFrames) / (float)vFrames;
    fh2 = floorf(1.5f * fontHeight) - 1.0f; // this really should not scale the image based on the font,
    gstate->setState();			    // best would be to load an image for each size
    glColor3f(1.0f, 1.0f, 1.0f);
    float imageXShift = 0.55f;
    float imageYShift = -fh2 * 0.29f;
    float outputSize = fh2;

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS); {
      glTexCoord2f(u, v);
      glVertex2f(x + imageXShift - outputSize, y + imageYShift);
      glTexCoord2f(u + du, v);
      glVertex2f(x + imageXShift , y + imageYShift);
      glTexCoord2f(u + du, v + dv);
      glVertex2f(x + imageXShift , y + outputSize + imageYShift);
      glTexCoord2f(u, v + dv);
      glVertex2f(x + imageXShift - outputSize, y + outputSize + imageYShift);
    } glEnd();

    TimeKeeper nowTime = TimeKeeper::getCurrent();
    if (nowTime - lastTime > 0.07f) {
      lastTime = nowTime;
      if (++arrowFrame == uFrames * vFrames) arrowFrame = 0;
    }
  } else {
    /* draw a generic triangle cursor */

    fh2 = floorf(0.5f * fontHeight);
    gstate->setState();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
      glVertex2f(x - fh2 - fontHeight, y + fontHeight - 1.0f);
      glVertex2f(x - fh2 - fontHeight, y);
      glVertex2f(x - fh2 - 1.0f, y + 0.5f * (fontHeight - 1.0f));
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
      glVertex2f(x - fh2 - fontHeight, y + fontHeight - 1.0f);
      glVertex2f(x - fh2 - fontHeight, y);
      glVertex2f(x - fh2 - 1.0f, y + 0.5f * (fontHeight - 1.0f));
    glEnd();
  }
}

void			HUDuiControl::render()
{
  if (hasFocus() && showingFocus) renderFocus();
  glColor3fv(hasFocus() ? textColor : dimTextColor);
  HUDuiElement::render();
}

void			HUDuiControl::setNavQueue(HUDNavigationQueue* _navList)
{
  navList = _navList;
}

bool			HUDuiControl::doKeyPress(const BzfKeyEvent& key)
{
  if (!navList) return false;

  if (key.ascii == 0) switch (key.button) {
    case BzfKeyEvent::Up:
      navList->prev();
      break;

    case BzfKeyEvent::Down:
      navList->next();
      break;

    default:
      return false;
  }

  if (key.ascii == '\t') {
    navList->next();
    return true;
  }

  return false;
}

bool			HUDuiControl::doKeyRelease(const BzfKeyEvent&)
{
  return false;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
