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

#include "common.h"

/* interface header */
#include "OptionsMenu.h"

/* system implementation headers */
#include <vector>
#include <string>

/* common implementation headers */
#include "OpenGLTexFont.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "SceneRenderer.h"
#include "TextUtils.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "ControlPanel.h"
#include "StartupInfo.h"

/* FIXME - from playing.h */
SceneRenderer* getSceneRenderer();
void setSceneDatabase();
StartupInfo* getStartupInfo();
extern ControlPanel* controlPanel;


OptionsMenu::OptionsMenu() : guiOptionsMenu(NULL), saveWorldMenu(NULL),
			     inputMenu(NULL), audioMenu(NULL),
			     displayMenu(NULL)
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Options");
  list.push_back(label);

  HUDuiList* option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Dithering:");
  option->setCallback(callback, (void*)"1");
  std::vector<std::string>* options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Blending:");
  option->setCallback(callback, (void*)"2");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Smoothing:");
  option->setCallback(callback, (void*)"3");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Lighting:");
  option->setCallback(callback, (void*)"4");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Texturing:");
  option->setCallback(callback, (void*)"5");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("Nearest"));
  options->push_back(std::string("Linear"));
  options->push_back(std::string("Nearest Mipmap Nearest"));
  options->push_back(std::string("Linear Mipmap Nearest"));
  options->push_back(std::string("Nearest Mipmap Linear"));
  options->push_back(std::string("Linear Mipmap Linear"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Quality:");
  option->setCallback(callback, (void*)"6");
  options = &option->getList();
  options->push_back(std::string("Low"));
  options->push_back(std::string("Medium"));
  options->push_back(std::string("High"));
  options->push_back(std::string("Experimental"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Shadows:");
  option->setCallback(callback, (void*)"7");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Depth Buffer:");
  option->setCallback(callback, (void*)"8");
  options = &option->getList();
  GLint value;
  glGetIntegerv(GL_DEPTH_BITS, &value);
  if (value == 0) {
    options->push_back(std::string("Not available"));
  } else {
    options->push_back(std::string("Off"));
    options->push_back(std::string("On"));
  }
  option->update();
  list.push_back(option);

#if defined(DEBUG_RENDERING)
  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Hidden Line:");
  option->setCallback(callback, (void*)"a");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Wireframe:");
  option->setCallback(callback, (void*)"b");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Depth Complexity:");
  option->setCallback(callback, (void*)"c");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);
#endif

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("UDP network connection:");
  option->setCallback(callback, (void*)"U");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Server List Cache:");
  option->setCallback(callback, (void*)"S");
  options = &option->getList();
  options->push_back(std::string("Off / Backup Mode"));
  options->push_back(std::string("5 Minutes"));
  options->push_back(std::string("15 Minutes"));
  options->push_back(std::string("30 Minutes"));
  options->push_back(std::string("1 Hour"));
  options->push_back(std::string("5 Hours"));
  options->push_back(std::string("15 Hours"));
  options->push_back(std::string("1 day"));
  options->push_back(std::string("15 days"));
  options->push_back(std::string("30 days"));
  option->update();
  list.push_back(option);

  clearCache = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Clear Server List Cache");
  list.push_back(label);

  guiOptions = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("GUI Options");
  list.push_back(label);

  saveWorld = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Save World");
  list.push_back(label);

  inputSetting = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Input Setting");
  list.push_back(label);

  audioSetting = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Audio Setting");
  list.push_back(label);

  displaySetting = label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setLabel("Display Setting");
  list.push_back(label);

  initNavigation(list, 1,list.size()-1);
}

OptionsMenu::~OptionsMenu()
{
  delete guiOptionsMenu;
  delete saveWorldMenu;
  delete inputMenu;
  delete audioMenu;
  delete displayMenu;
}

void OptionsMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == guiOptions) {
    if (!guiOptionsMenu) guiOptionsMenu = new GUIOptionsMenu;
    HUDDialogStack::get()->push(guiOptionsMenu);
  } else if (focus == clearCache) {
    if ((ServerListCache::get())->clearCache()){
      controlPanel->addMessage("Cache Cleared");
    } else {
      // already cleared -- do nothing
    }
  } else if (focus == saveWorld) {
    if (!saveWorldMenu) saveWorldMenu = new SaveWorldMenu;
    HUDDialogStack::get()->push(saveWorldMenu);
  } else if (focus == inputSetting) {
    if (!inputMenu) inputMenu = new InputMenu;
    HUDDialogStack::get()->push(inputMenu);
  } else if (focus == audioSetting) {
    if (!audioMenu) audioMenu = new AudioMenu;
    HUDDialogStack::get()->push(audioMenu);
  } else if (focus == displaySetting) {
    if (!displayMenu) displayMenu = new DisplayMenu;
    HUDDialogStack::get()->push(displayMenu);
  }
}

void OptionsMenu::resize(int width, int height)
{
  int i;
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float fontWidth = (float)height / 36.0f;
  const float fontHeight = (float)height / 36.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition options in two columns
  x = 0.3f * (float)width;
  const float topY = y - 0.6f * titleFont.getHeight();
  y = topY;
  list[1]->setFontSize(fontWidth, fontHeight);
  const float h = list[1]->getFont().getHeight();
  const int count = list.size();
  const int mid = count / 2;

  for (i = 1; i <= mid; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 0.80f * (float)width;
  y = topY;
  for (i = mid + 1; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  // load current settings
  SceneRenderer* renderer = getSceneRenderer();
  if (renderer) {
    HUDuiList* tex;
    int i = 1;
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("dither"));
    ((HUDuiList*)list[i++])->setIndex(BZDBCache::blend);
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("smooth"));
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("lighting"));
    tex = (HUDuiList*)list[i++];
    ((HUDuiList*)list[i++])->setIndex(renderer->useQuality());
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("shadows"));
    ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("zbuffer"));
#if defined(DEBUG_RENDERING)
    ((HUDuiList*)list[i++])->setIndex(renderer->useHiddenLine() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useWireframe() ? 1 : 0);
    ((HUDuiList*)list[i++])->setIndex(renderer->useDepthComplexity() ? 1 : 0);
#endif

    const StartupInfo* info = getStartupInfo();

    // mind the ++i !
    ((HUDuiList*)list[i++])->setIndex(info->useUDPconnection ? 1 : 0);

    if (!BZDBCache::texture)
      tex->setIndex(0);
    else
      tex->setIndex(OpenGLTexture::getFilter());

    // server cache age
    int index = 0;
    switch ((ServerListCache::get())->getMaxCacheAge()){
      case 0: index = 0; break;
      case 5: index = 1; break;
      case 15: index = 2; break;
      case 30: index = 3; break;
      case 60: index = 4; break;
      case 60*5: index = 5; break;
      case 60*15: index = 6; break;
      case 60*24: index = 7; break;
      case 60*24*15: index = 8; break;
      case 60*24*30: index = 9; break;
      default: index = 4;
    }
    ((HUDuiList*)list[i++])->setIndex(index);
    i++; // clear cache label
  }
}

void OptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  SceneRenderer* sceneRenderer = getSceneRenderer();
  switch (((const char*)data)[0]) {
    case '1':
      BZDB.set("dither", list->getIndex() ? "1" : "0");
      sceneRenderer->notifyStyleChange();
      break;

    case '2':
      BZDB.set("blend", list->getIndex() ? "1" : "0");
      sceneRenderer->notifyStyleChange();
      break;

    case '3':
      BZDB.set("smooth", list->getIndex() ? "1" : "0");
      sceneRenderer->notifyStyleChange();
      break;

    case '4':
      BZDB.set("lighting", list->getIndex() ? "1" : "0");

      BZDB.set("_texturereplace", (!BZDB.isTrue("lighting") &&
				   sceneRenderer->useQuality() < 2) ? "1" : "0");
      BZDB.setPersistent("_texturereplace", false);
      sceneRenderer->notifyStyleChange();
      break;

    case '5':
#ifdef _MSC_VER
      // Suppose Pat want to remind himself
      { int patlabor_get_tm_to_set_texture; }
#endif
      /*
	OpenGLTexture::setFilter((OpenGLTexture::Filter)list->getIndex());
	BZDB.set("texture", OpenGLTexture::getFilterName());
	sceneRenderer->notifyStyleChange();
      */
      break;

    case '6':
      sceneRenderer->setQuality(list->getIndex());

      BZDB.set("_texturereplace", (!BZDB.isTrue("lighting") &&
				   sceneRenderer->useQuality() < 2) ? "1" : "0");
      BZDB.setPersistent("_texturereplace", false);
      sceneRenderer->notifyStyleChange();
      break;

    case '7':
      BZDB.set("shadows", list->getIndex() ? "1" : "0");
      sceneRenderer->notifyStyleChange();
      break;

    case '8':
      BZDB.set("zbuffer", list->getIndex() ? "1" : "0");
      // FIXME - test for whether the z buffer will work
      setSceneDatabase();
      sceneRenderer->notifyStyleChange();
      break;

    case 'U': {
      StartupInfo* info = getStartupInfo();
      info->useUDPconnection = (list->getIndex() != 0);
    }
      break;

    case 'S': { // server cache
      time_t minutes = 0;
      int index = list->getIndex();
      switch (index){
	case 0: minutes = 0; break;
	case 1: minutes = 5; break;
	case 2: minutes = 15; break;
	case 3: minutes = 30; break;
	case 4: minutes = 60; break;
	case 5: minutes = 60*5; break;
	case 6: minutes = 60*15; break;
	case 7: minutes = 60*24; break;
	case 8: minutes = 60*24*15; break;
	case 9: minutes = 60*24*30; break;
      }
      (ServerListCache::get())->setMaxCacheAge(minutes);
      break;
    }

#if defined(DEBUG_RENDERING)
    case 'a':
      sceneRenderer->setHiddenLine(list->getIndex() != 0);
      break;

    case 'b':
      sceneRenderer->setWireframe(list->getIndex() != 0);
      break;

    case 'c':
      sceneRenderer->setDepthComplexity(list->getIndex() != 0);
      break;
#endif

    case 'r':
      // do nothing -- wait for enter or t key
      break;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
