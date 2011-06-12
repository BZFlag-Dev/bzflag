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

#include "common.h"

/* interface header */
#include "HubMenu.h"

/* common implementation headers */
#include "game/BZDBCache.h"
#include "platform/BzfDisplay.h"
#include "3D/FontManager.h"
#include "bzflag/SceneRenderer.h"
#include "3D/TextureManager.h"

/* local implementation headers */
#include "FontSizer.h"
#include "HubLink.h"
#include "JoinMenu.h"
#include "game/StartupInfo.h"
#include "HUDDialogStack.h"
#include "HUDuiList.h"
#include "LocalFontFace.h"
#include "MainMenu.h"
#include "playing.h"
#include "guiplaying.h"


HubMenu::HubMenu() {
  std::vector<std::string>* options;

  // cache font face id
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // header
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Hub Settings");
  addControl(label, false);

  // sub-header
  HUDuiLabel* subLabel = new HUDuiLabel;
  subLabel->setFontFace(fontFace);
  subLabel->setString("(global chat)");
  addControl(subLabel, false);

  // AutoJoin -- Yes / NO
  autoJoinList = new HUDuiList;
  autoJoinList->setFontFace(fontFace);
  autoJoinList->setLabel("AutoJoin:");
  autoJoinList->setCallback(callback, (void*)"a");
  options = &autoJoinList->getList();
  options->push_back("Off");
  options->push_back("On");
  addControl(autoJoinList);

  // UpdateCode -- Yes / NO
  updateCode = new HUDuiList;
  updateCode->setFontFace(fontFace);
  updateCode->setLabel("UpdateCode:");
  updateCode->setCallback(callback, (void*)"c");
  options = &updateCode->getList();
  options->push_back("Off");
  options->push_back("On");
  addControl(updateCode);

  // Username -- text input
  usernameText = new HUDuiTypeIn;
  usernameText->setFontFace(fontFace);
  usernameText->setLabel("Username:");
  usernameText->setCallback(callback, (void*)"u");
  usernameText->setMaxLength(CallSignLen - 1);
  usernameText->setString(BZDB.get("hubUsername"));
  addControl(usernameText);

  // Password -- text input
  passwordText = new HUDuiTypeIn;
  passwordText->setObfuscation(true);
  passwordText->setFontFace(fontFace);
  passwordText->setLabel("Password:");
  passwordText->setCallback(callback, (void*)"p");
  passwordText->setMaxLength(PasswordLen - 1);
  passwordText->setString(BZDB.get("hubPassword"));
  addControl(passwordText);

  // Copy Login -- button
  copyLoginLabel = new HUDuiLabel;
  copyLoginLabel->setFontFace(fontFace);
  copyLoginLabel->setLabel("Copy Login");
  addControl(copyLoginLabel);

  // Connect -- button
  connectLabel = new HUDuiLabel;
  connectLabel->setFontFace(fontFace);
  connectLabel->setLabel("Connect");
  addControl(connectLabel);

  // Disconnect -- button
  disconnectLabel = new HUDuiLabel;
  disconnectLabel->setFontFace(fontFace);
  disconnectLabel->setLabel("Disconnect");
  addControl(disconnectLabel);

  initNavigation();
}


HubMenu::~HubMenu() {
}


void HubMenu::resize(int _width, int _height) {
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);
  int i;

  FontManager& fm = FontManager::instance();
  const LocalFontFace* fontFace = MainMenu::getFontFace();
  const int faceID = fontFace->getFMFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(faceID, "headerFontSize");
  fs.setMin(0, 20);
  const float fontSize = fs.getFontSize(faceID, "menuFontSize");
  const float subTitleFontSize = 0.6f * titleFontSize;

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  const float titleWidth = fm.getStringWidth(faceID, titleFontSize, title->getString());
  const float titleHeight = fm.getStringHeight(faceID, titleFontSize);
  const float tx = 0.5f * ((float)_width - titleWidth);
  const float ty = (float)_height - titleHeight;
  title->setFontSize(titleFontSize);
  title->setPosition(tx, ty);

  // reposition subTitle
  HUDuiLabel* subTitle = (HUDuiLabel*)listHUD[1];
  const float subTitleWidth = fm.getStringWidth(faceID, subTitleFontSize, subTitle->getString());
  const float subTitleHeight = fm.getStringHeight(faceID, subTitleFontSize);
  const float sx = 0.5f * ((float)_width - subTitleWidth);
  const float sy = ty - subTitleHeight;
  subTitle->setFontSize(subTitleFontSize);
  subTitle->setPosition(sx, sy);

  // reposition options
  float x = 0.5f * ((float)_width);
  float y = sy - titleHeight;
  const float h = fm.getStringHeight(faceID, fontSize);
  const int count = (int)listHUD.size();
  for (i = 2; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
    if ((listHUD[i] == updateCode) ||
        (listHUD[i] == copyLoginLabel)) {
      y -= 0.5f * h;
    }
  }

  // load current settings
  autoJoinList->setIndex(BZDB.isTrue("hubAutoJoin") ? 1 : 0);
  updateCode->setIndex(BZDB.isTrue("hubUpdateCode") ? 1 : 0);
  usernameText->setString(BZDB.get("hubUsername"));
  passwordText->setString(BZDB.get("hubPassword"));
}


void HubMenu::execute() {
  HUDuiControl* _focus = getNav().get();
  if (_focus == connectLabel) {
    if (!BZDB.get("hubServer").empty()) {
      delete hubLink;
      hubLink = new HubLink(BZDB.get("hubServer"));
    }
  }
  else if (_focus == disconnectLabel) {
    delete hubLink;
    hubLink = NULL;
  }
  else if (_focus == usernameText) {
    BZDB.set("hubUsername", usernameText->getString());
  }
  else if (_focus == passwordText) {
    BZDB.set("hubPassword", passwordText->getString());
  }
  else if (_focus == copyLoginLabel) {
    JoinMenu* joinMenu = JoinMenu::getInstance();
    if (joinMenu != NULL) {
      BZDB.set("hubUsername", joinMenu->callsign->getString());
      BZDB.set("hubPassword", joinMenu->password->getString());
      resize(width, height);
    }
    else {
      StartupInfo* info = getStartupInfo();
      if (info) {
        BZDB.set("hubUsername", info->callsign);
        BZDB.set("hubPassword", info->password);
        resize(width, height);
      }
    }
  }
}


void HubMenu::callback(HUDuiControl* w, void* data) {
  switch (((const char*)data)[0]) {
    case 'a': {
      HUDuiList* list = (HUDuiList*)w;
      BZDB.setBool("hubAutoJoin", (list->getIndex() == 0) ? false : true);
      break;
    }
    case 'c': {
      HUDuiList* list = (HUDuiList*)w;
      BZDB.setBool("hubUpdateCode", (list->getIndex() == 0) ? false : true);
      break;
    }
    case 'u': {
      HUDuiTypeIn* type = (HUDuiTypeIn*)w;
      BZDB.set("hubUsername", type->getString());
      break;
    }
    case 'p': {
      HUDuiTypeIn* type = (HUDuiTypeIn*)w;
      BZDB.set("hubPassword", type->getString());
      break;
    }
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
