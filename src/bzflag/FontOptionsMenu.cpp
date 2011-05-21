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
#include "FontOptionsMenu.h"

/* common implementation headers */
#include "BundleMgr.h"
#include "BZDBCache.h"
#include "TextUtils.h"
#include "FontManager.h"

/* local implementation headers */
#include "FontSizer.h"
#include "MainMenu.h"
#include "World.h"
#include "HUDDialogStack.h"
#include "HUDuiList.h"
#include "HUDuiLabel.h"
#include "ScoreboardRenderer.h"
#include "SceneRenderer.h"
#include "LocalFontFace.h"
#include "playing.h"
#include "guiplaying.h"


FontOptionsMenu* FontOptionsMenu::fontOptionsMenu = NULL;


//============================================================================//

static void addFaces(HUDuiList* option) {
  FontManager& fm = FontManager::instance();
  std::vector<std::string>& options = option->getList();
  const int faceCount = fm.getNumFaces();
  for (int face = 0; face < faceCount; face++) {
    options.push_back(fm.getFaceName(face));
  }
  option->update();
}


static void setOptionFont(const std::string& faceName,
                          LocalFontFace*& facePtr, HUDuiList* list) {
  if (facePtr != NULL) {
    LocalFontFace::release(facePtr);
  }
  facePtr = LocalFontFace::create(faceName);
  list->setFontFace(facePtr);
  const std::vector<std::string>& faceNames = list->getList();
  for (size_t i = 0; i < faceNames.size(); i++) {
    if (TextUtils::tolower(faceName) == TextUtils::tolower(faceNames[i])) {
      list->setIndex(i);
      return;
    }
  }
  list->setIndex(0);
}


//============================================================================//

FontOptionsMenu::FontOptionsMenu()
  : consoleFont(NULL)
  , serifFont(NULL)
  , sansSerifFont(NULL) {
  fontOptionsMenu = this;

  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  consoleFont   = LocalFontFace::create(BZDB.get("consoleFont"));
  serifFont     = LocalFontFace::create(BZDB.get("serifFont"));
  sansSerifFont = LocalFontFace::create(BZDB.get("sansSerifFont"));

  // add controls
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Font Settings");
  addControl(label, false);

  HUDuiList* option;

  // Console Font
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Console Font:");
  option->setCallback(callback, (void*)"c");
  addFaces(option);
  setOptionFont(BZDB.get("consoleFont"), consoleFont, option);
  addControl(option);

  // Serif Font
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Serif Font:");
  option->setCallback(callback, (void*)"s");
  addFaces(option);
  setOptionFont(BZDB.get("serifFont"), serifFont, option);
  addControl(option);

  // Sans Serif Font
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Sans Serif Font:");
  option->setCallback(callback, (void*)"S");
  addFaces(option);
  setOptionFont(BZDB.get("sansSerifFont"), sansSerifFont, option);
  addControl(option);

  initNavigation();
}


FontOptionsMenu::~FontOptionsMenu() {
  LocalFontFace::release(consoleFont);
  LocalFontFace::release(serifFont);
  LocalFontFace::release(sansSerifFont);
  fontOptionsMenu = NULL;
}


void FontOptionsMenu::execute() {
}


void FontOptionsMenu::resize(int _width, int _height) {
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  FontManager& fm = FontManager::instance();
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace->getFMFace(), "headerFontSize");

  fs.setMin(0, 30);
  const float fontSize = fs.getFontSize(fontFace->getFMFace(), "menuFontSize");

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(fontFace->getFMFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStringHeight(fontFace->getFMFace(), titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.54f * (float)_width;
  y -= 1.0f * titleHeight;
  const float h = fm.getStringHeight(fontFace->getFMFace(), fontSize);
  const int count = (const int)listHUD.size();
  for (int i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  // load current settings
  int i = 1;
  setOptionFont(BZDB.get("consoleFont"),   consoleFont, (HUDuiList*)listHUD[i++]);
  setOptionFont(BZDB.get("serifFont"),     serifFont, (HUDuiList*)listHUD[i++]);
  setOptionFont(BZDB.get("sansSerifFont"), sansSerifFont, (HUDuiList*)listHUD[i++]);
}


void FontOptionsMenu::callback(HUDuiControl* w, void* data) {
  FontOptionsMenu* menu = fontOptionsMenu;
  if (menu == NULL) {
    return;
  }

  HUDuiList* list = (HUDuiList*)w;
  std::vector<std::string>& listStrs = list->getList();
  const std::string& faceName = listStrs[list->getIndex()];

  switch (((const char*)data)[0]) {
    case 'c': {
      BZDB.set("consoleFont", faceName);
      setOptionFont(faceName, menu->consoleFont, list);
      break;
    }
    case 's': {
      BZDB.set("serifFont", faceName);
      setOptionFont(faceName, menu->serifFont, list);
      break;
    }
    case 'S': {
      BZDB.set("sansSerifFont", faceName);
      setOptionFont(faceName, menu->sansSerifFont, list);
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
