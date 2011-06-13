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
#include "GUIOptionsMenu.h"

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


GUIOptionsMenu::GUIOptionsMenu() {
  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // add controls
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("GUI Settings");
  addControl(label, false);

  HUDuiList* option;
  std::vector<std::string>* options;

  // scoreboard sorting
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Scoreboard Sort:");
  option->setCallback(callback, (void*)"p");
  options = &option->getList();
  const char** sortLabels = ScoreboardRenderer::getSortLabels();
  while (*sortLabels != NULL) {
    options->push_back(std::string(*sortLabels++));
  }
  option->update();
  addControl(option);

  // always show team scores
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Always Show Team Scores:");
  option->setCallback(callback, (void*)"q");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  // set observer info
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Extended Observer Info:");
  option->setCallback(callback, (void*)"O");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  options->push_back(std::string("On With Apparent Speeds"));
  option->update();
  addControl(option);

  // set Radar Translucency
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar & Panel Opacity:");
  option->setCallback(callback, (void*)"y");
  option->createSlider(10);
  options = &option->getList();
  options->push_back(std::string("Opaque"));
  option->update();
  addControl(option);

  // set radar size
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar & Panel Size:");
  option->setCallback(callback, (void*)"R");
  option->createSlider(maxRadarSize + 1);
  option->update();
  addControl(option);

  // set maxmotion size
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Mouse Box Size:");
  option->setCallback(callback, (void*)"M");
  option->createSlider(22);
  option->update();
  addControl(option);

  // Tabs
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Control panel tabs:");
  option->setCallback(callback, (void*)"t");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Left"));
  options->push_back(std::string("Right"));
  option->update();
  addControl(option);

  // Time/date display settings
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Time / Date Display:");
  option->setCallback(callback, (void*)"h");
  options = &option->getList();
  options->push_back(std::string("time"));
  options->push_back(std::string("date"));
  options->push_back(std::string("both"));
  option->update();
  addControl(option);

  // HUD Reload timer
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Reload timer on HUD:");
  option->setCallback(callback, (void*)"T");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  initNavigation();
}


GUIOptionsMenu::~GUIOptionsMenu() {
}


void GUIOptionsMenu::execute() {
}


void GUIOptionsMenu::resize(int _width, int _height) {
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
  ((HUDuiList*)listHUD[i++])->setIndex(ScoreboardRenderer::getSort());
  ((HUDuiList*)listHUD[i++])->setIndex(ScoreboardRenderer::getAlwaysTeamScore());
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("showVelocities"));
  ((HUDuiList*)listHUD[i++])->setIndex((int)(10.0f * RENDERER.getPanelOpacity() + 0.5));
  ((HUDuiList*)listHUD[i++])->setIndex(RENDERER.getRadarSize());
  ((HUDuiList*)listHUD[i++])->setIndex(RENDERER.getMaxMotionFactor() + 11);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("showtabs"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("timedate"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("displayReloadTimer") ? 1 : 0);
}


void GUIOptionsMenu::callback(HUDuiControl* w, void* data) {
  HUDuiList* list = (HUDuiList*)w;

  switch (((const char*)data)[0]) {
    case 'h': {
      BZDB.setInt("timedate", list->getIndex());
      break;
    }
    case 'O': {
      BZDB.setInt("showVelocities", list->getIndex());
      getMainWindow()->getWindow()->callResizeCallbacks();
      break;
    }
    case 'y': {
      RENDERER.setPanelOpacity(((float)list->getIndex()) / 10.0f);
      break;
    }
    case 'R': {
      RENDERER.setRadarSize(list->getIndex());
      break;
    }
    case 'M': {
      RENDERER.setMaxMotionFactor(list->getIndex() - 11);
      break;
    }
    case 't': {
      BZDB.set("showtabs", TextUtils::format("%d", list->getIndex()));
      break;
    }
    case 'T': {
      BZDB.set("displayReloadTimer", list->getIndex() ? "1" : "0");
      break;
    }
    case 'p': {
      ScoreboardRenderer::setSort(list->getIndex());
      break;
    }
    case 'q': {
      ScoreboardRenderer::setAlwaysTeamScore(list->getIndex() ? true : false);
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
