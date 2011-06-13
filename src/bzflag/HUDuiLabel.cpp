/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "LocalFontFace.h"
#include "3D/FontManager.h"
#include "common/Bundle.h"
#include "common/BundleMgr.h"


//
// HUDuiLabel
//

HUDuiLabel::HUDuiLabel() : HUDuiControl() {
  darker = false;
  params = NULL;
  color = textColor;
}


HUDuiLabel::~HUDuiLabel() {
  if (params) {
    delete params;
  }
}


std::string HUDuiLabel::getString() const {
  std::string theString;
  Bundle* bdl = BundleMgr::getCurrentBundle();
  if (params) {
    theString = bdl->formatMessage(label, params);
  }
  else {
    theString = bdl->getLocalString(label);
  }

  return theString;
}


void HUDuiLabel::setString(const std::string& _string, const std::vector<std::string> *_params) {
  label = _string;
  if (_params) {
    if (params != NULL) {
      delete params;
    }

    params = new std::vector<std::string>(*_params);
  }
  onSetFont();
}


void HUDuiLabel::onSetFont() {
  HUDuiControl::onSetFont();
}


bool HUDuiLabel::doKeyPress(const BzfKeyEvent& key) {
  if (HUDuiControl::doKeyPress(key)) {
    return true;
  }

  switch (key.unicode) {
    case 13:
    case 27:
    case 0:
      return false;
  }
  return true;
}


bool HUDuiLabel::doKeyRelease(const BzfKeyEvent&) {
  return false;
}


void HUDuiLabel::setDarker(bool d) {
  darker = d;
}


void HUDuiLabel::setColor(float r, float g, float b) {
  color.r = r;
  color.g = g;
  color.b = b;
}


void HUDuiLabel::doRender() {
  if (getFontFace() < 0) {
    return;
  }
  // render string
  FontManager& fm = FontManager::instance();
  float darkness;
  if (hasFocus()) {
    darkness = 1.0f;
  }
  else if (!darker) {
    darkness = 0.7f;
  }
  else {
    darkness = 0.4f;
  }
  fm.setDarkness(darkness);
  fm.drawString(getX(), getY(), 0, getFontFace()->getFMFace(), getFontSize(),
                getString(), &color);
  fm.setDarkness(1.0f);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
