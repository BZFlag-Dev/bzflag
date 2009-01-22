/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "QuitMenu.h"

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

/* from bzflag.cxx */
extern void dumpResources();
extern std::string alternateConfig;


bool QuitMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  // TODO: not localizable
  if (key.chr == 'Y' || key.chr == 'y') {
    CommandsStandard::quit();
    return true;
  }
  return MenuDefaultKey::keyPress(key);
}

bool QuitMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.chr == 'Y' || key.chr == 'y')
    return true;
  return MenuDefaultKey::keyRelease(key);
}


QuitMenu::QuitMenu()
{
  // add controls
  HUDuiLabel* label;

  label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Yes, quit");
  addControl(label);

  label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("No, return to game");
  addControl(label);

  HUDuiList* list;
  list = new HUDuiList;
  list->setFontFace(MainMenu::getFontFace());
  list->setLabel("Save Settings?");
  std::vector<std::string>& listList = list->getList();
  listList.push_back("No");
  listList.push_back("Yes");
  list->setIndex(BZDB.evalInt("saveSettings"));
  addControl(list);

  initNavigation();

  // frame
  HUDuiFrame* frame = new HUDuiFrame;
  frame->setFontFace(MainMenu::getFontFace());
  frame->setLabel("Really Quit?");
  frame->setLineWidth(2.0f);
  frame->setStyle(HUDuiFrame::RoundedRectStyle);
  addControl(frame);
}

QuitMenu::~QuitMenu()
{
}

void QuitMenu::resize(int _width, int _height)
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

  // heights
  const float fontHeight = fm.getStringHeight(fontFace->getFMFace(), fontSize);

  // get stuff
  std::vector<HUDuiElement*>& listHUD = getElements();

  float x, y;

  // yes
  HUDuiLabel* label = (HUDuiLabel*)listHUD[0];
  label->setFontSize(fontSize);
  x = (float)_width / 4.0f;
  y = (float)_height - 4.25f * fontHeight;
  label->setPosition(x, y);

  // no
  label = (HUDuiLabel*)listHUD[1];
  label->setFontSize(fontSize);
  y = (float)_height - 5.5f * fontHeight;
  label->setPosition(x, y);

  // save settings
  HUDuiList* list = (HUDuiList*)listHUD[2];
  list->setFontSize(smallFontSize);
  const float stringWidth = fm.getStringWidth(fontFace->getFMFace(), smallFontSize, std::string(list->getLabel() + "99").c_str());
  y = (float)_height - 6.25f * fontHeight;
  list->setPosition(x + stringWidth, y);

  // frame
  HUDuiFrame* frame = (HUDuiFrame*)listHUD[3];
  const float gapSize = fm.getStringHeight(fontFace->getFMFace(), fontSize);
  frame->setFontFace(fontFace);
  frame->setFontSize(smallFontSize);
  frame->setPosition(x - gapSize, (float)_height - 3.0f * fontHeight);
  frame->setSize(0.5f * getWidth() + 2.0f * gapSize, fontHeight * 4.0f);
}

void QuitMenu::execute()
{
  HUDuiElement* _focus = getNav().get();
  std::vector<HUDuiElement*>& listHUD = getElements();
  if (_focus == listHUD[0]) { // yes
    const bool permanentSave = BZDB.isTrue("saveSettings");
    const bool tempSave = (((HUDuiList*)listHUD[2])->getIndex() != 0);
    if (tempSave && !permanentSave) {
      // save this time, but not usually
      dumpResources();
      if (alternateConfig == "") {
	CFGMGR.write(getCurrentConfigFileName());
      } else {
	CFGMGR.write(alternateConfig);
      }
    } else if (permanentSave && !tempSave) {
      // save usually, but not this time
      BZDB.setBool("saveSettings", false);
    }
    CommandsStandard::quit();
  } else if (_focus == listHUD[1]) { //no
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
