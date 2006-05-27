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

HUDuiElement::HUDuiElement() : fontFace(-1), fontSize(11),
				x(0.0f), y(0.0f),
				width(1.0f), height(1.0f),
				fontHeight(11.0f),
				desiredLabelWidth(0.0f),
				trueLabelWidth(0.0f)
{
  // nothing really to do here
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
  return fontFace;
}

float			HUDuiElement::getFontSize() const
{
  return fontSize;
}

void			HUDuiElement::setPosition(float _x, float _y)
{
  x = _x;
  y = _y;
}

void			HUDuiElement::setSize(float _width, float _height)
{
  width = _width;
  height = _height;
}

void			HUDuiElement::setLabelWidth(float labelWidth)
{
  desiredLabelWidth = labelWidth;
}

void			HUDuiElement::setLabel(const std::string& _label)
{

  label = _label;
  if (fontFace >= 0) {
    FontManager &fm = FontManager::instance();
    trueLabelWidth = fm.getStrLength(fontFace, fontSize, getLabel() + "99");
  }
}

void			HUDuiElement::setFontFace(int _fontFace)
{
  fontFace = _fontFace;
  onSetFont();
}

void			HUDuiElement::setFontSize(float size)
{
  fontSize = size;
  onSetFont();
}

void			HUDuiElement::onSetFont()
{
  if (fontFace >= 0) {
    FontManager &fm = FontManager::instance();
    fontHeight = fm.getStrHeight(fontFace, fontSize, getLabel());
    trueLabelWidth = fm.getStrLength(fontFace, fontSize, getLabel() + "99");
  } else {
    fontHeight = 11.0f;
    trueLabelWidth = 0.0f;
  }
}

void			HUDuiElement::renderLabel()
{
  std::string theLabel = getLabel();
  if (theLabel.length() > 0 && fontFace >= 0) {
    FontManager &fm = FontManager::instance();
    const float dx = (desiredLabelWidth > trueLabelWidth)
      ? desiredLabelWidth : trueLabelWidth;
    fm.drawString(x - dx, y, 0, fontFace, fontSize, theLabel);
  }
}

void			HUDuiElement::render()
{
  renderLabel();
  doRender();
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

