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
#include "TextOptionsMenu.h"

/* common implementation headers */
#include "BundleMgr.h"
#include "BZDBCache.h"
#include "TextUtils.h"
#include "FontManager.h"

/* local implementation headers */
#include "FontOptionsMenu.h"
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


static const int fontStep = 3;


TextOptionsMenu::TextOptionsMenu()
: fontMenu(NULL)
{
  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // add controls
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Text Settings");
  addControl(label, false);

  HUDuiList* option;
  std::vector<std::string>* options;

  // Locale
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Locale:");
  option->setCallback(callback, (void*)"L");
  options = &option->getList();
  std::vector<std::string> locales;
  if (BundleMgr::getLocaleList(&locales) == true) {
    options->push_back(std::string("English"));
    for (int i = 0; i < (int)locales.size(); i++) {
      options->push_back(locales[i]);
    }
    locales.erase(locales.begin(), locales.end());
  } else {
    // Something failed when trying to compile a list
    // of all the locales.
    options->push_back(std::string("English"));
  }

  for (int i = 0; i < (int)options->size(); i++) {
    if ((*options)[i].compare(World::getLocale()) == 0) {
      option->setIndex(i);
      break;
    }
  }
  option->update();
  addControl(option);

  // Font Options
  fontOptions = new HUDuiLabel;
  fontOptions->setFontFace(fontFace);
  fontOptions->setLabel("Font Options");
  addControl(fontOptions);

  // Font Outlines
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Font Outlines:");
  option->setCallback(callback, (void*)"o");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Outlines"));
  options->push_back(std::string("Drop Shadows"));
  option->update();
  addControl(option);

  // Scoreboard Font Size
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Scoreboard Font Size:");
  option->setCallback(callback, (void*)"S");
  options = &option->getList();
  options->push_back(std::string("Auto"));
  option->createSlider(10);
  option->update();
  addControl(option);

  // ControlPanel Font Size
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("ControlPanel Font Size:");
  option->setCallback(callback, (void*)"C");
  options = &option->getList();
  options->push_back(std::string("Auto"));
  option->createSlider(10);
  option->update();
  addControl(option);

  // Text coloring
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Control panel coloring:");
  option->setCallback(callback, (void*)"c");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  // Underline color
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Underline color:");
  option->setCallback(callback, (void*)"u");
  options = &option->getList();
  options->push_back(std::string("Cyan"));
  options->push_back(std::string("Grey"));
  options->push_back(std::string("Text"));
  option->update();
  addControl(option);

  // Killer Highlight
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Killer Highlight:");
  option->setCallback(callback, (void*)"k");
  options = &option->getList();
  options->push_back(std::string("None"));
  options->push_back(std::string("Pulsating"));
  options->push_back(std::string("Underline"));
  option->update();
  addControl(option);

  // Pulsate Rate
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Pulsation Rate:");
  option->setCallback(callback, (void*)"r");
  option->createSlider(9);
  option->update();
  addControl(option);

  // Pulsate Depth
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Pulsation Depth:");
  option->setCallback(callback, (void*)"d");
  option->createSlider(9);
  option->update();
  addControl(option);

  initNavigation();
}


TextOptionsMenu::~TextOptionsMenu()
{
  delete fontMenu;
}


void TextOptionsMenu::execute()
{
  HUDuiControl* _focus = getNav().get();
  if (_focus == fontOptions) {
    if (fontMenu == NULL) {
      fontMenu = new FontOptionsMenu;
    }
    HUDDialogStack::get()->push(fontMenu);
  }
}


void TextOptionsMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  FontManager &fm = FontManager::instance();
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
  i++; // locale
  i++; // font options
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("fontOutline"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("scoreFontSize") / fontStep);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("consoleFontSize") / fontStep);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("colorful") ? 1 : 0);

  // underline color - find index of mode string in options
  const std::vector<std::string> &opts = ((HUDuiList*)listHUD[i])->getList();
  std::string uColor = BZDB.get("underlineColor");
  ((HUDuiList*)listHUD[i++])->setIndex((int)(std::find(opts.begin(), opts.end(), uColor) -
					 opts.begin()));

  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("killerhighlight"));
  ((HUDuiList*)listHUD[i++])->setIndex(int(BZDB.eval("pulseRate") * 5.0f) - 1);
  ((HUDuiList*)listHUD[i++])->setIndex(int(BZDB.eval("pulseDepth") * 10.0f) - 1);
}


void TextOptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  switch (((const char*)data)[0]) {
    case 'L': {
      std::vector<std::string>* options = &list->getList();
      std::string locale = (*options)[list->getIndex()];

      World::setLocale(locale);
      BZDB.set("locale", locale);
      World::getBundleMgr()->getBundle(locale, true);

      TextOptionsMenu *menu = (TextOptionsMenu *) HUDDialogStack::get()->top();
      if (menu) {
        menu->resize(menu->getWidth(), menu->getHeight());
      }
      break;
    }
    case 'o': {
      BZDB.setInt("fontOutline", list->getIndex());
      break;
    }
    case 'S': {
      if (list->getIndex() > 0) {
        BZDB.setInt("scoreFontSize", list->getIndex() * fontStep);
      } else {
        BZDB.set("scoreFontSize", BZDB.getDefault("scoreFontSize"));
      }
      getMainWindow()->getWindow()->callResizeCallbacks();
      break;
    }
    case 'C': {
      if (list->getIndex() > 0) {
        BZDB.setInt("consoleFontSize", list->getIndex() * fontStep);
      } else {
        BZDB.set("consoleFontSize", BZDB.getDefault("consoleFontSize"));
      }
      getMainWindow()->getWindow()->callResizeCallbacks();
      break;
    }
    case 'c': {
      BZDB.set("colorful", list->getIndex() ? "1" : "0");
      break;
    }
    case 'u': {
      std::vector<std::string>* options = &list->getList();
      std::string color = (*options)[list->getIndex()];
      BZDB.set("underlineColor", color);
      break;
    }
    case 'k': {
      BZDB.set("killerhighlight", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case 'r': {
      BZDB.set("pulseRate",
               TextUtils::format("%f", (float)(list->getIndex() + 1) / 5.0f));
      break;
    }
    case 'd': {
      BZDB.set("pulseDepth",
               TextUtils::format("%f", (float)(list->getIndex() + 1) / 10.0f));
      break;
    }
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
