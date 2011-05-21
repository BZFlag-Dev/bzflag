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

/* interface header */
#include "SaveMenu.h"

/* common implementation headers */
#include "FontManager.h"
#include "StateDatabase.h"

/* local implementation headers */
#include "FontSizer.h"
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "HUDuiLabel.h"
#include "HUDuiList.h"
#include "HUDuiFrame.h"
#include "HUDui.h"
#include "ConfigFileManager.h"
#include "clientConfig.h"
#include "LocalFontFace.h"

/* from bzflag.cpp */
extern void dumpResources();
extern std::string alternateConfig;


bool SaveMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  return MenuDefaultKey::keyPress(key);
}

bool SaveMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  return MenuDefaultKey::keyRelease(key);
}


SaveMenu::SaveMenu()
{
  // add controls
  HUDuiLabel* label;

  label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Config File: ");
  addControl(label, false);

  label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  addControl(label, false);

  label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("OK");
  addControl(label);

  initNavigation();

  // frame
  HUDuiFrame* frame = new HUDuiFrame;
  frame->setLabel("Configuration Saved");
  frame->setLineWidth(2.0f);
  frame->setStyle(HUDuiFrame::RoundedRectStyle);
  addControl(frame);
}

void SaveMenu::setFileName(std::string& fname)
{
     filename = fname;
}

std::string SaveMenu::getFileName()
{
     return filename;
}

SaveMenu::~SaveMenu()
{
}

void SaveMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  FontManager &fm = FontManager::instance();
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // use a big font
  fs.setMin(0, 10);
  float fontSize = fs.getFontSize(fontFace->getFMFace(), "headerFontSize");

  fs.setMin(0,20);
  float smallFontSize = fs.getFontSize(fontFace->getFMFace(), "menuFontSize");

  fs.setMin(0,30);
  float midFontSize = fs.getFontSize(fontFace->getFMFace(), "menuFontSize");


  // heights
  const float fontHeight = fm.getStringHeight(fontFace->getFMFace(), fontSize);

  // get stuff
  std::vector<HUDuiElement*>& listHUD = getElements();

  float x, y;

  //config File
  HUDuiLabel* label = (HUDuiLabel*)listHUD[0];
  label->setFontSize(midFontSize);
  x = (float)_width / 4.0f;
  y = (float)_height - 3.5f * fontHeight;
  label->setPosition(x, y);

  //filename
  label = (HUDuiLabel*)listHUD[1];
  label->setFontSize(midFontSize);
  x = (float)_width / 4.0f;
  label->setString(getFileName());
  label->setPosition(1.5f * x, y);

  // OK
  label = (HUDuiLabel*)listHUD[2];
  label->setFontSize(fontSize);
  x = (float)_width / 4.0f;
  y = (float)_height - 5.5f * fontHeight;
  label->setPosition(2.0f * x, y);

  // frame
  HUDuiFrame* frame = (HUDuiFrame*)listHUD[3];
  const float gapSize = fm.getStringHeight(fontFace->getFMFace(), fontSize);
  frame->setFontFace(fontFace);
  frame->setFontSize(smallFontSize);
  frame->setPosition(x - gapSize, (float)_height - 3.0f * fontHeight);
  frame->setSize(0.5f * getWidth() + 2.0f * gapSize, fontHeight * 4.0f);
}

void SaveMenu::execute()
{
  HUDuiElement* _focus = getNav().get();
  std::vector<HUDuiElement*>& listHUD = getElements();
  if (_focus == listHUD[2]){ //OK
    HUDDialogStack::get()->pop();
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
