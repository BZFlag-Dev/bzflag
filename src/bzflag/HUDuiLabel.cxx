/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "HUDuiLabel.h"

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"
#include "LocalFontFace.h"

//
// HUDuiLabel
//

HUDuiLabel::HUDuiLabel() : HUDuiControl()
{
  darker = false;
  params = NULL;
  color[0] = textColor[0];
  color[1] = textColor[1];
  color[2] = textColor[2];
}

HUDuiLabel::~HUDuiLabel()
{
  if (params)
    delete params;
}

std::string		HUDuiLabel::getString() const
{
  std::string theString;
  Bundle *bdl = BundleMgr::getCurrentBundle();
  if (params)
    theString = bdl->formatMessage(string, params);
  else
    theString = bdl->getLocalString(string);

  return theString;
}

std::string		HUDuiLabel::getDisplayString() const
{
  std::string theString;
  Bundle *bdl = BundleMgr::getCurrentBundle();
  if (params)
    theString = bdl->formatMessage(displayString, params);
  else
    theString = bdl->getLocalString(displayString);

  return theString;
}

void			HUDuiLabel::setString(const std::string& _string, const std::vector<std::string> *_params)
{
  string = _string;
  displayString = _string;
  if (_params) {
    if (params != NULL)
      delete params;

    params = new std::vector<std::string>(*_params);
  }
  setSize(getWidth(), getHeight());
  onSetFont();
}

void			HUDuiLabel::onSetFont()
{
  HUDuiControl::onSetFont();
}

void HUDuiLabel::setSize(float width, float height)
{
  HUDuiControl::setSize(width, height);
  
  if ((width < 0)||(height < 0))
    return;

  // Trim string to fit our available space
  FontManager &fm = FontManager::instance();
  std::string tempStr = getString();

  if (getFontFace() == NULL)
    return;

  // Skip if it already fits
  if (fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), tempStr.c_str()) <= width)
    return;

  // Iterate through each character. Expensive.
  for (int i=0; i<=(int)tempStr.size(); i++)
  {
    float temp = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), tempStr.substr(0, i).c_str());
    // Is it too big yet?
    if (fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), tempStr.substr(0, i).c_str()) > width)
    {
      displayString = tempStr.substr(0, i - 1);
      break;
    }
  }
}

void HUDuiLabel::setFontSize(float size)
{
  HUDuiControl::setFontSize(size);
  
  setSize(getWidth(), getHeight());
}

void HUDuiLabel::setFontFace(const LocalFontFace* fontFace)
{
  HUDuiControl::setFontFace(fontFace);
  
  setSize(getWidth(), getHeight());
}

bool HUDuiLabel::doKeyPress(const BzfKeyEvent& key)
{
  if (HUDuiControl::doKeyPress(key))
    return true;

  switch (key.chr) {
    case 13:
    case 27:
    case 0:
      return false;
  }
  return false;
  //return true;
}

bool			HUDuiLabel::doKeyRelease(const BzfKeyEvent&)
{
  return false;
}

void			HUDuiLabel::setDarker(bool d)
{
  darker = d;
}

void			HUDuiLabel::setColor(GLfloat r, GLfloat g, GLfloat b)
{
  color[0] = r;
  color[1] = g;
  color[2] = b;
}

void			HUDuiLabel::doRender()
{
  if (getFontFace() < 0) {
    return;
  }
  // render string
  FontManager &fm = FontManager::instance();
  float darkness;
  if (hasFocus()) {
    darkness = 1.0f;
  } else if (!darker) {
    darkness = 0.7f;
  } else {
    darkness = 0.4f;
  }
  fm.setDarkness(darkness);
  fm.drawString(getX(), getY(), 0,
		getFontFace()->getFMFace(), getFontSize(),
		getDisplayString().c_str(), color);
  fm.setDarkness(1.0f);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
