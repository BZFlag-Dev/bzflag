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
#include "ServerMenu.h"

/* common implementation headers */
#include "FontManager.h"
#include "StateDatabase.h"

/* local implementation headers */
#include "FontSizer.h"
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "clientConfig.h"
#include "ConfigFileManager.h"
#include "bzflag.h"
#include "LocalFontFace.h"
#include "playing.h"

#include "HUDuiList.h"
#include "HUDuiServerList.h"
#include "HUDuiServerListItem.h"

#include "HUDuiTabbedControl.h"


bool ServerMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.chr == 'f') {
    menu->favoritesList->addItem(*(((HUDuiServerList*)menu->tabbedControl->getActiveTab())->getSelectedServer()));
    return true;
  }
  if (key.chr == 'i') {
    menu->inverted = !menu->inverted;
    menu->refresh();
    return true;
  }

  return MenuDefaultKey::keyPress(key);
}

bool ServerMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.chr) {
  case 27: // escape
  case 13: // return
    return true;
  }

  return false;
}

ServerMenu::ServerMenu(): defaultKey(this), inverted(false)
{
  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  addPlayingCallback(&playingCB, this);
  serverList.startServerPings(getStartupInfo());

  normalList = new HUDuiServerList();
  normalList->setFontFace(fontFace);
  normalList->setSize(800,200);
  normalList->setServerList(&serverList);

  favoritesList = new HUDuiServerList();
  favoritesList->setFontFace(fontFace);
  favoritesList->setSize(800,200);
  favoritesList->setServerList(&serverList);

  recentList = new HUDuiServerList();
  recentList->setFontFace(fontFace);
  recentList->setSize(800,200);
  recentList->setServerList(&serverList);

  serverInfo = new HUDuiServerInfo();
  serverInfo->setFontFace(fontFace);
  serverInfo->setSize(500, 300);
  serverInfo->setFontSize(16.0f);
  serverInfo->setPosition(200, 200);

  title = new HUDuiLabel();
  title->setString("Servers");
  title->setFontFace(fontFace);

  tabbedControl = new HUDuiTabbedControl;
  tabbedControl->addTab(normalList, "All");
  tabbedControl->addTab(recentList, "Recent");
  tabbedControl->addTab(favoritesList, "Favorites");
  tabbedControl->setActiveTab(0);
  tabbedControl->setFontFace(fontFace);
  tabbedControl->setPosition(300,500);
  tabbedControl->setFontSize(12.0f);
  tabbedControl->setSize(800, 200);

  addControl(title, false);
  addControl(tabbedControl);
  addControl(serverInfo);

  initNavigation();
}

ServerMenu::~ServerMenu()
{
  // Blank
}

void ServerMenu::execute()
{
  if ((tabbedControl->hasFocus())||(tabbedControl->getActiveTab()->hasFocus()))
    return;

  // update startup info
  StartupInfo* info = getStartupInfo();
  ServerItem* selectedServer = ((HUDuiServerList*)tabbedControl->getActiveTab())->getSelectedServer();
  strncpy(info->serverName, selectedServer->name.c_str(), ServerNameLen-1);
  info->serverPort = ntohs((unsigned short) selectedServer->ping.serverId.port);

  // all done
  HUDDialogStack::get()->pop();
}

void ServerMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  FontManager &fm = FontManager::instance();
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // reposition title
  float x, y;
  //fontFace = title->getFontFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace->getFMFace(), "headerFontSize");

  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(fontFace->getFMFace(), titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(fontFace->getFMFace(), titleFontSize);
  x = 0.5f * ((float)_width - titleWidth);
  y = (float)_height - titleHeight;
  title->setPosition(x, y);

  //fontFace = MainMenu::getFontFace();

  fs.setMin(40,10);
  const float fontSize = fs.getFontSize(fontFace->getFMFace(), "alertFontSize");

  float edgeSpacer = fm.getStringWidth(fontFace->getFMFace(), fontSize, "X");
  float bottomSpacer = fm.getStringHeight(fontFace->getFMFace(), fontSize);

  float remainingSpace = y - bottomSpacer*2;
  float listHeight = remainingSpace*0.7f;

  HUDuiControl* topControl;
  HUDuiControl* bottomControl;

  tabbedControl->setSize((_width - (2*edgeSpacer)),listHeight);
  serverInfo->setSize((_width - (2*edgeSpacer)), (remainingSpace - bottomSpacer)*0.3f);

  if (inverted)
  {
    topControl = serverInfo;
    bottomControl = tabbedControl;
  }
  else
  {
    topControl = tabbedControl;
    bottomControl = serverInfo;
  }

  y = y - (titleHeight/2) - topControl->getHeight();

  topControl->setPosition(edgeSpacer, y);

  bottomControl->setPosition(edgeSpacer, bottomSpacer/2);

  tabbedControl->setFontSize(fontSize);

  serverInfo->setFontSize(fontSize);
}

void ServerMenu::callback(HUDuiControl* w, void* data)
{
  // Blank
}

void ServerMenu::refresh()
{
  resize(getWidth(), getHeight());
}

void ServerMenu::updateStatus()
{
  serverInfo->setServerItem(((HUDuiServerList*)tabbedControl->getActiveTab())->getSelectedServer());

  if ((tabbedControl->hasFocus())||(((HUDuiServerList*)tabbedControl->getActiveTab())->hasFocus()))
    serverInfo->setServerItem(NULL);

  if (serverList.size() == normalList->size())
    return;
  else if (serverList.size() < normalList->size())
    normalList->clear();

  for (int i = (int) normalList->size(); i < (int) serverList.size(); i++)
    normalList->addItem(*(serverList.getServerAt(i)));
}

void ServerMenu::playingCB(void* _self)
{
  ((ServerMenu*)_self)->serverList.checkEchos(getStartupInfo());

  ((ServerMenu*)_self)->updateStatus();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
