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

// interface header
#include "HUDuiElement.h"

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"


//
// HUDuiElement
//

// init static members
const GLfloat		HUDuiElement::dimTextColor[3] = { 0.7f, 0.7f, 0.7f };
const GLfloat		HUDuiElement::moreDimTextColor[3] = { 0.4f, 0.4f, 0.4f };
const GLfloat		HUDuiElement::textColor[3] = { 1.0f, 1.0f, 1.0f };

HUDuiElement::HUDuiElement()
{
  elementFontFace = -1;
  elementFontSize = 10;
  elementX = 0.0f;
  elementY = 0.0f;
  elementWidth = 1.0f;
  elementHeight = 1.0f;
  fontHeight = 10.0f;
  desiredLabelWidth = 0.0f;
  trueLabelWidth = 0.0f;
  skipRenderLabel = false;
}

HUDuiElement::~HUDuiElement()
{
  // nothing really to do here
}

float			HUDuiElement::getLabelWidth() const
{
  return desiredLabelWidth;
}

std::string		HUDuiElement::getLabel() const
{
  return BundleMgr::getCurrentBundle()->getLocalString(label);
}

int			HUDuiElement::getFontFace() const
{
  return elementFontFace;
}

float			HUDuiElement::getFontSize() const
{
  return elementFontSize;
}

void			HUDuiElement::setPosition(float _x, float _y)
{
  elementX = _x;
  elementY = _y;
}

void			HUDuiElement::setSize(float _width, float _height)
{
  elementWidth = _width;
  elementHeight = _height;
}

void			HUDuiElement::setLabelWidth(float labelWidth)
{
  desiredLabelWidth = labelWidth;
}

void			HUDuiElement::setLabel(const std::string& _label)
{

  label = _label;
  if (elementFontFace >= 0) {
    FontManager &fm = FontManager::instance();
    trueLabelWidth = fm.getStringWidth(elementFontFace,
				     elementFontSize, std::string(getLabel() + "99").c_str());
  }
}

void			HUDuiElement::setFontFace(int _fontFace)
{
  elementFontFace = _fontFace;
  onSetFont();
}

void			HUDuiElement::setFontSize(float size)
{
  elementFontSize = size;
  onSetFont();
}

void			HUDuiElement::onSetFont()
{
  if (elementFontFace >= 0) {
    FontManager &fm = FontManager::instance();
    fontHeight = fm.getStringHeight(elementFontFace, elementFontSize);
    trueLabelWidth = fm.getStringWidth(elementFontFace, elementFontSize, std::string(getLabel() + "99").c_str());
  } else {
    fontHeight = 10.0f;
    trueLabelWidth = 0.0f;
  }
}

void			HUDuiElement::renderLabel()
{
  std::string theLabel = getLabel();
  if (theLabel.length() > 0 && elementFontFace >= 0) {
    FontManager &fm = FontManager::instance();
    const float dx = (desiredLabelWidth > trueLabelWidth)
      ? desiredLabelWidth : trueLabelWidth;
    fm.drawString(elementX - dx, elementY, 0,
		  elementFontFace, elementFontSize, theLabel.c_str());
  }
}

void			HUDuiElement::render()
{
  if (!skipRenderLabel) renderLabel();
  doRender();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
