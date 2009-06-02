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
#include "GUIOptionsMenu.h"

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


GUIOptionsMenu::GUIOptionsMenu()
{
  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // add controls
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("GUI Settings");
  addControl(label, false);

  HUDuiList* option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar Style:");
  option->setCallback(callback, (void*)"e");
  std::vector<std::string>* options = &option->getList();
  options->push_back(std::string("Normal"));
  options->push_back(std::string("Fast"));
  options->push_back(std::string("Fast Sorted"));
  options->push_back(std::string("Enhanced"));
  option->update();
  addControl(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Scoreboard Sort:");
  option->setCallback(callback, (void*)"p");
  options = &option->getList();
  const char **sortLabels = ScoreboardRenderer::getSortLabels();
  while ( *sortLabels != NULL)
    options->push_back(std::string(*sortLabels++));
  option->update();
  addControl(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Always Show Team Scores:");
  option->setCallback(callback, (void*)"q");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Scoreboard Font Size:");
  option->setCallback(callback, (void*)"S");
  options = &option->getList();
  options->push_back(std::string("Auto"));
  option->createSlider(10);
  option->update();
  addControl(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("ControlPanel Font Size:");
  option->setCallback(callback, (void*)"C");
  options = &option->getList();
  options->push_back(std::string("Auto"));
  option->createSlider(10);
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

  // set radar size
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar & Panel Size:");
  option->setCallback(callback, (void*)"R");
  option->createSlider(maxRadarSize+1);
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

  // set locale
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Locale:");
  option->setCallback(callback, (void*)"L");
  options = &option->getList();
  std::vector<std::string> locales;
  if (BundleMgr::getLocaleList(&locales) == true) {
    options->push_back(std::string("Default"));
    for (int i = 0; i < (int)locales.size(); i++) {
      options->push_back(locales[i]);
    }
    locales.erase(locales.begin(), locales.end());
  } else {
    // Something failed when trying to compile a list
    // of all the locales.
    options->push_back(std::string("Default"));
  }

  for (int i = 0; i < (int)options->size(); i++) {
    if ((*options)[i].compare(World::getLocale()) == 0) {
      option->setIndex(i);
      break;
    }
  }
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

  // GUI coloring
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


GUIOptionsMenu::~GUIOptionsMenu()
{
}


void GUIOptionsMenu::execute()
{
}


void GUIOptionsMenu::resize(int _width, int _height)
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
  ((HUDuiList*)listHUD[i++])->setIndex(BZDBCache::radarStyle);
  ((HUDuiList*)listHUD[i++])->setIndex(ScoreboardRenderer::getSort());
  ((HUDuiList*)listHUD[i++])->setIndex(ScoreboardRenderer::getAlwaysTeamScore());
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("scoreFontSize") / 8);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("consoleFontSize") / 8);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("showVelocities"));
  ((HUDuiList*)listHUD[i++])->setIndex((int)(10.0f * RENDERER.getPanelOpacity() + 0.5));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("coloredradarshots") ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("linedradarshots"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("sizedradarshots"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDBCache::leadingShotLine ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDBCache::showShotGuide ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(RENDERER.getRadarSize());
  ((HUDuiList*)listHUD[i++])->setIndex(RENDERER.getMaxMotionFactor() + 11);
  i++; // locale
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("showtabs"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("colorful") ? 1 : 0);

  // underline color - find index of mode string in options
  const std::vector<std::string> &opts = ((HUDuiList*)listHUD[i])->getList();
  std::string uColor = BZDB.get("underlineColor");
  ((HUDuiList*)listHUD[i++])->setIndex((int)(std::find(opts.begin(), opts.end(), uColor) -
					 opts.begin()));

  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("killerhighlight"));
  ((HUDuiList*)listHUD[i++])->setIndex((BZDB.evalInt("pulseRate") * 5) - 1);
  ((HUDuiList*)listHUD[i++])->setIndex((BZDB.evalInt("pulseDepth") * 10) - 1);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("timedate"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("displayReloadTimer") ? 1 : 0);
}


void GUIOptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  switch (((const char*)data)[0]) {
    case 'e': {
      BZDB.setInt("radarStyle", list->getIndex());
      break;
    }
    case 'C': {
      BZDB.setInt("consoleFontSize", list->getIndex() * 8);
      getMainWindow()->getWindow()->callResizeCallbacks();
      break;
    }
    case 'h': {
      BZDB.setInt("timedate", list->getIndex());
      break;
    }
    case 'S': {
      BZDB.setInt("scoreFontSize", list->getIndex() * 8);
      getMainWindow()->getWindow()->callResizeCallbacks();
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
    case 'R': {
      RENDERER.setRadarSize(list->getIndex());
      break;
    }
    case 'M': {
      RENDERER.setMaxMotionFactor(list->getIndex() - 11);
      break;
    }
    case 'c': {
      BZDB.set("colorful", list->getIndex() ? "1" : "0");
      break;
    }
    case 't': {
      BZDB.set("showtabs", TextUtils::format("%d", list->getIndex()));
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
    case 'L': {
      std::vector<std::string>* options = &list->getList();
      std::string locale = (*options)[list->getIndex()];

      World::setLocale(locale);
      BZDB.set("locale", locale);
      World::getBundleMgr()->getBundle(locale, true);

      GUIOptionsMenu *menu = (GUIOptionsMenu *) HUDDialogStack::get()->top();
      if (menu) {
        menu->resize(menu->getWidth(), menu->getHeight());
      }
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
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
