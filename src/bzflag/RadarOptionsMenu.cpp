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
#include "RadarOptionsMenu.h"

/* common implementation headers */
#include "3D/FontManager.h"
#include "common/BundleMgr.h"
#include "common/TextUtils.h"
#include "game/BZDBCache.h"

/* local implementation headers */
#include "FontSizer.h"
#include "HUDDialogStack.h"
#include "HUDuiLabel.h"
#include "HUDuiList.h"
#include "LocalFontFace.h"
#include "MainMenu.h"
#include "ScoreboardRenderer.h"
#include "World.h"
#include "guiplaying.h"
#include "playing.h"
#include "bzflag/SceneRenderer.h"


RadarOptionsMenu::RadarOptionsMenu() {
  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // add controls
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Radar Settings");
  addControl(label, false);

  HUDuiList* option;
  std::vector<std::string>* options;

  // radar style
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar Style:");
  option->setCallback(callback, (void*)"e");
  options = &option->getList();
  options->push_back(std::string("Normal"));      // NormalRadar
  options->push_back(std::string("Enhanced"));    // EnhancedRadar
  options->push_back(std::string("Fast"));        // FastRadar
  options->push_back(std::string("Fast Sorted")); // FastSortedRadar
  option->update();
  addControl(option);

  // toggle coloring of shots on radar
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Colored shots on radar:");
  option->setCallback(callback, (void*)"z");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  // set radar shot length
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar Shot Length:");
  option->setCallback(callback, (void*)"l");
  option->createSlider(11);
  option->update();
  addControl(option);

  // set radar shot size
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar Shot Size:");
  option->setCallback(callback, (void*)"s");
  option->createSlider(11);
  option->update();
  addControl(option);

  // radar shot leading line
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar Shot Line:");
  option->setCallback(callback, (void*)"F");
  options = &option->getList();
  options->push_back(std::string("Lagging"));
  options->push_back(std::string("Leading"));
  option->update();
  addControl(option);

  // radar shot guide on/off
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar Shot Guide:");
  option->setCallback(callback, (void*)"G");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  initNavigation();
}


RadarOptionsMenu::~RadarOptionsMenu() {
}


void RadarOptionsMenu::execute() {
}


void RadarOptionsMenu::resize(int _width, int _height) {
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
  ((HUDuiList*)listHUD[i++])->setIndex(BZDBCache::radarStyle);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("coloredradarshots") ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("linedradarshots"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("sizedradarshots"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDBCache::leadingShotLine ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDBCache::showShotGuide ? 1 : 0);
}


void RadarOptionsMenu::callback(HUDuiControl* w, void* data) {
  HUDuiList* list = (HUDuiList*)w;

  switch (((const char*)data)[0]) {
    case 'e': {
      BZDB.setInt("radarStyle", list->getIndex());
      break;
    }
    case 'z': {
      BZDB.set("coloredradarshots", list->getIndex() ? "1" : "0");
      break;
    }
    case 'l': {
      BZDB.set("linedradarshots", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case 's': {
      BZDB.set("sizedradarshots", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case 'F': {
      BZDB.set("leadingShotLine", list->getIndex() ? "1" : "0");
      break;
    }
    case 'G': {
      BZDB.set("showShotGuide", list->getIndex() ? "1" : "0");
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
