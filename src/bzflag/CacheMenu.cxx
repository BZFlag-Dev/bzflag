/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "CacheMenu.h"

/* common implementation headers */
#include "FontManager.h"

/* local implementation headers */
#include "MainMenu.h"
#include "Downloads.h"
#include "CacheManager.h"
#include "ServerListCache.h"
#include "HUDuiList.h"
#include "playing.h"
#include "HUDui.h"

CacheMenu::CacheMenu()
{
  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();

  // the menu label
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Cache Settings");
  listHUD.push_back(label);

  // the menu options
  HUDuiList* option;
  std::vector<std::string>* options;


  // Server List Cache Time
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Server List Cache:");
  option->setCallback(callback, (void*)"s");
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
  listHUD.push_back(option);

  // Server List Cache Clear
  clearServerListCache = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Clear Server List Cache");
  listHUD.push_back(label);


  // Cache Size (MegaBytes)
  cacheSize = new HUDuiTypeIn;
  cacheSize->setFontFace(MainMenu::getFontFace());
  cacheSize->setLabel("Cache Size (MB):");
  cacheSize->setMaxLength(4);
  cacheSize->setString(BZDB.get("maxCacheMB"));
  listHUD.push_back(cacheSize);

  // Clear Download Cache
  clearDownloadCache = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Clear Download Cache");
  listHUD.push_back(label);


  // Automatic Downloads
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Automatic Downloads:");
  option->setCallback(callback, (void*)"d");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  // Connection Updates
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Connection Updates:");
  option->setCallback(callback, (void*)"u");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  listHUD.push_back(option);

  // Update Download Cache
  updateDownloadCache = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Update Downloads");
  listHUD.push_back(label);


  // Failed Message  (download status)
  failedMessage = new HUDuiLabel;
  failedMessage->setFontFace(fontFace);
  failedMessage->setString("");
  listHUD.push_back(failedMessage);


  initNavigation(listHUD, 1, listHUD.size() - 2);

  return;
}


CacheMenu::~CacheMenu()
{
  return;
}


void CacheMenu::execute()
{
  HUDuiControl* _focus = HUDui::getFocus();

  if (_focus == cacheSize) {
    BZDB.set("maxCacheMB", cacheSize->getString().c_str());
    int maxCacheMB = BZDB.evalInt("maxCacheMB");
    if (maxCacheMB < 0) {
      BZDB.set("maxCacheMB", "0");
      HUDuiTypeIn* inputField = (HUDuiTypeIn*) _focus;
      inputField->setString("0");
    }
  } else if (_focus == updateDownloadCache) {
    controlPanel->addMessage("Updating Downloads");
    Downloads::startDownloads(true, true, true);
  } else if (_focus == clearDownloadCache) {
    const std::string oldSize = BZDB.get("maxCacheMB");
    BZDB.set("maxCacheMB", "0");
    CACHEMGR.loadIndex();
    CACHEMGR.limitCacheSize();
    CACHEMGR.saveIndex();
    BZDB.set("maxCacheMB", oldSize);
    controlPanel->addMessage("Download Cache Cleared");
  } else if (_focus == clearServerListCache) {
    if ((ServerListCache::get())->clearCache()){
      controlPanel->addMessage("Server List Cache Cleared");
    } else {
      // already cleared -- do nothing
    }
  }
}


void CacheMenu::setFailedMessage(const char* msg)
{
  failedMessage->setString(msg);

  FontManager &fm = FontManager::instance();
  const float _width = fm.getStrLength(MainMenu::getFontFace(),
	failedMessage->getFontSize(), failedMessage->getString());
  failedMessage->setPosition(center - 0.5f * _width, failedMessage->getY());
}


void CacheMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  center = 0.5f * (float)_width;

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  const float fontSize = (float)_height / 45.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth =
    fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight =
    fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * (float)_width;
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  const int count = listHUD.size();
  int i;
  for (i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    if ((i == 2) || (i == 4) || (i == 7)) {
      y -= 1.75f * h;
    } else {
      y -= 1.0f * h;
    }
  }

  // load current settings
  i = 1;

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
  ((HUDuiList*)listHUD[i++])->setIndex(index);
  i++; // clear cache label

  i++; // cache size
  i++; // clear downloads cache

  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("doDownloads") ? 1 : 0);
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("updateDownloads") ? 1 : 0);
  i++; // update downloads now

  return;
}


void CacheMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  switch (((const char*)data)[0]) {
    case 'd': {
      BZDB.set("doDownloads", list->getIndex() ? "1" : "0");
      break;
    }
    case 'u': {
      BZDB.set("updateDownloads", list->getIndex() ? "1" : "0");
      break;
    }
    case 's': { // server cache
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
  }

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
