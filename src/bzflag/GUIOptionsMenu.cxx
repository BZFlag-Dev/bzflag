/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "GUIOptionsMenu.h"

/* system implementation headers */
#include <string>
#include <vector>

/* common implementation headers */
#include "BundleMgr.h"
#include "BZDBCache.h"
#include "TextUtils.h"
#include "FontManager.h"

/* local implementation headers */
#include "MainMenu.h"
#include "World.h"
#include "SceneRenderer.h"
#include "HUDDialogStack.h"
#include "HUDuiControl.h"
#include "HUDuiList.h"
#include "HUDuiLabel.h"

/* FIXME - from playing.h */
SceneRenderer* getSceneRenderer();


GUIOptionsMenu::GUIOptionsMenu()
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();

  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("GUI Options");
  list.push_back(label);

  HUDuiList* option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Enhanced radar:");
  option->setCallback(callback, (void*)"e");
  std::vector<std::string>* options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Controlpanel & Score FontSize:");
  option->setCallback(callback, (void*)"w");
  options = &option->getList();
  options->push_back(std::string("normal"));
  options->push_back(std::string("bigger"));
  option->update();
  list.push_back(option);

  // set Radar Translucency
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar & Panel Opacity:");
  option->setCallback(callback, (void*)"y");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // toggle coloring of shots on radar
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Colored shots on radar:");
  option->setCallback(callback, (void*)"z");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  // set radar shot length
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar Shot Length:");
  option->setCallback(callback, (void*)"l");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // set radar shot size
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar Shot Size:");
  option->setCallback(callback, (void*)"s");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // set radar size
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Radar & Panel Size:");
  option->setCallback(callback, (void*)"R");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // set maxmotion size
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Mouse Box Size:");
  option->setCallback(callback, (void*)"M");
  option->createSlider(11);
  option->update();
  list.push_back(option);

  // set locale
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Locale:");
  option->setCallback(callback, (void*)"L");
  options = &option->getList();
  std::vector<std::string> locales;
  if (BundleMgr::getLocaleList(&locales) == true) {
    options->push_back(std::string("default"));
    for (int i = 0; i < (int)locales.size(); i++) {
      options->push_back(locales[i]);
    }
    locales.erase(locales.begin(), locales.end());
  }
  else {
    // Something failed when trying to compile a list
    // of all the locales.
    options->push_back(std::string("default"));
  }

  for (int i = 0; i < (int)options->size(); i++) {
    if ((*options)[i].compare(World::getLocale()) == 0) {
      option->setIndex(i);
      break;
    }
  }
  option->update();
  list.push_back(option);

  // GUI coloring
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Control panel coloring:");
  option->setCallback(callback, (void*)"c");
  options = &option->getList();
  options->push_back(std::string("off"));
  options->push_back(std::string("on"));
  option->update();
  list.push_back(option);
  // Tabs
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Control panel tabs:");
  option->setCallback(callback, (void*)"t");
  options = &option->getList();
  options->push_back(std::string("off"));
  options->push_back(std::string("left"));
  options->push_back(std::string("right"));
  option->update();
  list.push_back(option);
  // Underline color
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Underline color:");
  option->setCallback(callback, (void*)"u");
  options = &option->getList();
  options->push_back(std::string("cyan"));
  options->push_back(std::string("grey"));
  options->push_back(std::string("text"));
  option->update();
  list.push_back(option);
  // Killer Highlight
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Killer Highlight:");
  option->setCallback(callback, (void*)"k");
  options = &option->getList();
  options->push_back(std::string("Blinking"));
  options->push_back(std::string("Underline"));
  options->push_back(std::string("None"));
  option->update();
  list.push_back(option);

  initNavigation(list, 1,list.size()-1);
}

GUIOptionsMenu::~GUIOptionsMenu()
{
}

void			GUIOptionsMenu::execute()
{
}

void			GUIOptionsMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)height / 15.0f;
  const float fontSize = (float)height / 45.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width + 0.5f * titleWidth);
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  const int count = list.size();
  for (int i = 1; i < count; i++) {
    list[i]->setFontSize(fontSize);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  // load current settings
  SceneRenderer* renderer = getSceneRenderer();
  if (renderer) {
    int i = 1;
    ((HUDuiList*)list[i++])->setIndex(BZDBCache::enhancedRadar ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("bigfont") ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex((int)(10.0f * renderer->getPanelOpacity() + 0.5));
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("coloredradarshots") ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(static_cast<int>(BZDB.eval("linedradarshots")));
    ((HUDuiList*)list[i++])->setIndex(static_cast<int>(BZDB.eval("sizedradarshots")));
    ((HUDuiList*)list[i++])->setIndex(renderer->getRadarSize());
    ((HUDuiList*)list[i++])->setIndex(renderer->getMaxMotionFactor());
    i++; // locale
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("colorful") ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(static_cast<int>(BZDB.eval("showtabs")));

    // underline color - does this HAVE to be so complicated?
    std::vector<std::string>* options = &((HUDuiList*)list[i++])->getList();
    std::string uColor = BZDB.get("underlineColor");
    std::vector<std::string>::iterator itr;
    int j = 0;
    for (itr = options->begin(); itr != options->end(); itr++) {
      if (uColor == (*itr)) {
        ((HUDuiList*)list[i])->setIndex(j);
	break;
      }
      j++;
    }

    ((HUDuiList*)list[i++])->setIndex(static_cast<int>(BZDB.eval("killerhighlight")));
  }
}

void			GUIOptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  SceneRenderer* sceneRenderer = getSceneRenderer();
  switch (((const char*)data)[0]) {
    case 'e':
      BZDB.set("enhancedradar", list->getIndex() ? "1" : "0");
      break;

    case 'w':
      BZDB.set("bigfont", list->getIndex() ? "1" : "0");
      break;

    case 'y':
      {
	sceneRenderer->setPanelOpacity(((float)list->getIndex()) / 10.0f);
	break;
      }

    case 'z':
      BZDB.set("coloredradarshots", list->getIndex() ? "1" : "0");
      break;

    case 'l':
      BZDB.set("linedradarshots", string_util::format("%d", list->getIndex()));
      break;

    case 's':
      BZDB.set("sizedradarshots", string_util::format("%d", list->getIndex()));
      break;

    case 'R':
      {
	sceneRenderer->setRadarSize(list->getIndex());
	break;
      }

    case 'M':
      {
	sceneRenderer->setMaxMotionFactor(list->getIndex());
	break;
      }

    case 'c':
      {
	BZDB.set("colorful", list->getIndex() ? "1" : "0");
	break;
      }

    case 't':
      {
	BZDB.set("showtabs", string_util::format("%d", list->getIndex()));
	break;
      }

    case 'u':
      {
	std::vector<std::string>* options = &list->getList();
	std::string color = (*options)[list->getIndex()];
	BZDB.set("underlineColor", color);
	break;
      }

    case 'k':
      {
	BZDB.set("killerhighlight", string_util::format("%d", list->getIndex()));
	break;
      }

    case 'L':
      {
	std::vector<std::string>* options = &list->getList();
	std::string locale = (*options)[list->getIndex()];

	World::setLocale(locale);
	BZDB.set("locale", locale);
	World::getBundleMgr()->getBundle(locale, true);

	GUIOptionsMenu *menu = (GUIOptionsMenu *) HUDDialogStack::get()->top();
	if (menu)
	  menu->resize(menu->getWidth(), menu->getHeight());
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
