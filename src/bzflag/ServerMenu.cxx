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

bool ServerMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  ServerList &serverList = ServerList::instance();

  HUDuiControl* activeTabControl = menu->tabbedControl->getActiveTab();
  HUDuiServerList* activeServerList = NULL;

  // The currently active tab is the custom tab creation control. We don't handle
  // any key presses for that here, so pass it on, and return the result we get.
  if (activeTabControl == menu->customTabControl) // Is this the right way to check?
    return MenuDefaultKey::keyPress(key);
  // The currently active tab is NOT the custom tab, so let's try casting it as a
  // HUDuiServerList control. For the server browser we only expect controls of type
  // HUDuiServerListCustomTab, or HUDuiServerList. However, since we are using a
  // HUDuiTabbedControl, we could have any subclass of HUDuiControl, so we have to
  // be careful to check what class we are dealing with when executing key presses.
  else
    activeServerList = dynamic_cast<HUDuiServerList*>(activeTabControl);

  // The active tab is not the custom tab control, and is not a HUDuiServerList
  // Right now we're handling input that affects either custom tab creation, or
  // server list browsing. Since this is neither, return false, we don't handle it.
  if ((activeServerList == 0)||(activeServerList == NULL))
    // dynamic_cast returns 0 if it fails
    return false;

  // These key presses should only be processed when the user has a server selected.
  // As such, we should check to see if the server list, or the tabbed control have
  // focus, and if they do, ignore these key presses for now. Also check that the
  // user has a server selected.
  if ((!activeServerList->hasFocus())&&(!menu->tabbedControl->hasFocus())&&
     (activeServerList->getSelectedServer() != NULL)){
    // The favorite key was pressed
    if (key.chr == 'f') {
	  // Check to see if we're on the favorites list, if we are, ignore keypress
	  if (activeServerList == menu->favoritesList) {
	    return false;
	  }
	  else {
        // Mark it as a favorite server
	    serverList.markAsFavorite(activeServerList->getSelectedServer());
	    return true;
	  }
    }

	// The remove server key was pressed
    if (key.chr == 'h') {
	  // Design decision: Don't let users remove any servers from the normal server
      // list. Let's keep this as a pure representation of all the available servers
      if (activeServerList == menu->normalList)
        return false;

	  // If we're on the favorites list, we need to unmark the server
	  // as a favorite server in order to remove it from the list
      if (activeServerList == menu->favoritesList)
        serverList.unmarkAsFavorite(activeServerList->getSelectedServer());

	  // If we're on the recents list, we need to unmark the server
	  // as a recent server in order to remove it from the list
      if (activeServerList == menu->recentList)
        serverList.unmarkAsRecent(activeServerList->getSelectedServer());

      // Remove the selected server from the active server list
      activeServerList->removeItem(activeServerList->getSelectedServer());
      return true;
    }
  }

  // The clear server list key was pressed
  if (key.chr == 'c') {
	// Design decision: Don't let users remove any servers from the normal server
    // list. Let's keep this as a pure representation of all the available servers
    if (activeServerList == menu->normalList)
      return false;

	// If we're on the favorites list, cycle through all favorite servers and unmark
	// them as favorites. This may be a bad idea, it's fairly easy to accidentally
	// clear your favorites list using this functionality.
    if (activeServerList == menu->favoritesList) {
      for (size_t i=0; i<activeServerList->getSize(); i++) {
        serverList.unmarkAsFavorite(activeServerList->get(i)->getServer());
      }
    }

	// If we're on the recent server list, cycle through all
	// recent servers and unmark them as a recent server. 
    if (activeServerList == menu->recentList) {
      for (size_t i=0; i<activeServerList->getSize(); i++) {
        serverList.unmarkAsRecent(activeServerList->get(i)->getServer());
      }
    }

	// Clear the list
    activeServerList->clearList();
    return true;
  }

  // Remove the current tab. This can be pressed at any
  // time, but only works if the user is on a custom tab.
  if (key.chr == 'v') {
    // Do nothing if the user is on any of the standard tabs
    if ((activeServerList == menu->favoritesList) ||
	(activeServerList == menu->normalList) ||
	(activeServerList == menu->recentList))
      return false;

	// Remove the current tab from the tabbed control
    std::string tabName = menu->tabbedControl->getActiveTabName();
    HUDuiServerListCache::instance().removeList(activeServerList, tabName);
    menu->tabbedControl->removeTab(activeServerList, tabName);
	return true;
  }

  // Refresh the server list. This can be done at any time.
  if (key.chr == 'r') {
    ServerList::instance().clear();
    ServerList::instance().startServerPings(getStartupInfo());
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

ServerMenu::PingsMap ServerMenu::activePings;

ServerMenu::ServerMenu()
  : normalList(0)
  , recentList(0)
  , favoritesList(0)
  , tabbedControl(0)
  , serverInfo(new HUDuiServerInfo())
  , customTabControl(new HUDuiServerListCustomTab())
  , listsCache(HUDuiServerListCache::instance())
  , serverList(ServerList::instance())
  , defaultKey(this)
  , title(0)
  , help(0)
{
  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  addPlayingCallback(&playingCB, this);

  serverList.updateFromCache();
  serverList.startServerPings(getStartupInfo());
  listsCache.loadCache();

  std::vector<std::pair<HUDuiServerList*, std::string> > cachedLists = listsCache.readCachedLists();

  if (cachedLists.size() > (size_t) 0) {
    normalList = cachedLists[0].first;
    recentList = cachedLists[1].first;
    favoritesList = cachedLists[2].first;
  } else {
    normalList = new HUDuiServerList;
    favoritesList = new HUDuiServerList;
    recentList = new HUDuiServerList;
    listsCache.addNewList(normalList, "");
    listsCache.addNewList(favoritesList, "");
    listsCache.addNewList(recentList, "");
  }

  normalList->setFontFace(fontFace);
  favoritesList->setFontFace(fontFace);
  recentList->setFontFace(fontFace);
  serverInfo->setFontFace(fontFace);
  customTabControl->setFontFace(fontFace);

  title = new HUDuiLabel();
  title->setString("Servers");
  title->setFontFace(fontFace);

  help = new HUDuiLabel();
  help->setString("f - add server to favorites      h - remove server from tab      c - clear tab      v - delete tab      r - refresh server list");
  help->setFontFace(fontFace);

  tabbedControl = new HUDuiTabbedControl;
  tabbedControl->setFontFace(fontFace);
  tabbedControl->addTab(normalList, "All");
  tabbedControl->addTab(recentList, "Recent");
  tabbedControl->addTab(favoritesList, "Favorites");

  for (size_t i=3; i<cachedLists.size(); i++) {
    cachedLists[i].first->setFontFace(fontFace);
    tabbedControl->addTab(cachedLists[i].first, cachedLists[i].second);
  }

  tabbedControl->addTab(customTabControl, "Create New Tab");
  tabbedControl->setActiveTab(0);

  serverList.addServerCallback(newServer, normalList);
  serverList.addFavoriteServerCallback(newServer, favoritesList);
  serverList.addRecentServerCallback(newServer, recentList);

  addControl(title, false);
  addControl(tabbedControl);
  addControl(serverInfo);
  addControl(help, false);

  initNavigation();
}

ServerMenu::~ServerMenu()
{
  serverList.removeServerCallback(newServer, normalList);
  serverList.removeFavoriteServerCallback(newServer, favoritesList);
  serverList.removeRecentServerCallback(newServer, recentList);
}

void ServerMenu::newServer(ServerItem* addedServer, void* data)
{
  HUDuiServerList* serverData( static_cast<HUDuiServerList*>(data) );

  // Server already has a ping
  if (addedServer->ping.pingTime != 0) {
    serverData->addItem(addedServer);
    return;
  }

  // Pinging in process, add on to the list vector
  if (addedServer->ping.pinging) {
    ServerMenu::activePings[addedServer->getServerKey()].second.push_back(serverData);
  }


  ServerPing* newping = new ServerPing(addedServer->ping.serverId.serverHost, ntohs(addedServer->ping.serverId.port));
  newping->start();
  std::vector<HUDuiServerList*> serverListsVector;
  serverListsVector.push_back(serverData);
  ServerMenu::activePings.insert(PingsMap::value_type(addedServer->getServerKey(), std::pair<ServerPing*, std::vector<HUDuiServerList*> >(newping, serverListsVector)));
  addedServer->ping.pinging = true;
}

void ServerMenu::execute()
{
  if ((tabbedControl->getActiveTab() == customTabControl) &&
      (dynamic_cast<HUDuiServerListCustomTab*>(tabbedControl->getActiveTab())->createNew->hasFocus()))
  {
    HUDuiServerList* newServerList = customTabControl->createServerList();
    listsCache.addNewList(newServerList, customTabControl->tabName->getString());
    tabbedControl->addTab(newServerList, customTabControl->tabName->getString(), tabbedControl->getTabCount() - 1);
    for (size_t i=0; i<serverList.size(); i++) {
      newServerList->addItem(serverList.getServerAt(i));
    }
    //newServerList->searchServers(customTabControl->serverName->getString());
    dynamic_cast<HUDuiNestedContainer*>(tabbedControl->getTab(tabbedControl->getTabCount() - 2))->getNav().set(1);
    return;
  }

  if ((tabbedControl->hasFocus())||
      (tabbedControl->getActiveTab()->hasFocus())||
      (tabbedControl->getActiveTabName() == "Create New Tab"))
    return;

  // update startup info
  StartupInfo* info = getStartupInfo();
  ServerItem* selectedServer = dynamic_cast<HUDuiServerList*>(tabbedControl->getActiveTab())->getSelectedServer();
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

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace->getFMFace(), "headerFontSize");

  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(fontFace->getFMFace(), titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(fontFace->getFMFace(), titleFontSize);
  x = 0.5f * ((float)_width - titleWidth);
  y = (float)_height - titleHeight;
  title->setPosition(x, y);

  fs.setMin(40,10);
  const float fontSize = fs.getFontSize(fontFace->getFMFace(), "alertFontSize");

  float edgeSpacer = fm.getStringWidth(fontFace->getFMFace(), fontSize, "X");
  float bottomSpacer = fm.getStringHeight(fontFace->getFMFace(), fontSize);

  float remainingSpace = y - bottomSpacer*2;
  float listHeight = remainingSpace*0.65f;

  tabbedControl->setSize((_width - (2*edgeSpacer)),listHeight);
  serverInfo->setSize((_width - (2*edgeSpacer)), (remainingSpace - 4*bottomSpacer)*0.35f);

  y = y - (titleHeight/2) - tabbedControl->getHeight();

  float helpLength = fm.getStringWidth(fontFace->getFMFace(), fontSize, help->getString().c_str());

  tabbedControl->setPosition(edgeSpacer, y);
  serverInfo->setPosition(edgeSpacer, bottomSpacer/2 + bottomSpacer);
  help->setPosition((_width - helpLength)/2, bottomSpacer/2);
  tabbedControl->setFontSize(fontSize);
  serverInfo->setFontSize(fontSize);
  help->setFontSize(fontSize);
}

void ServerMenu::updateStatus()
{
  if (tabbedControl->hasFocus() ||
      tabbedControl->getActiveTab()->hasFocus() ||
      tabbedControl->getActiveTabName() == "Create New Tab")
    serverInfo->setServerItem(NULL);
  else
    serverInfo->setServerItem(dynamic_cast<HUDuiServerList*>(tabbedControl->getActiveTab())->getSelectedServer());
}

void ServerMenu::playingCB(void* _self)
{
  ServerList &list = ServerList::instance();
  for (PingsMap::iterator i = ServerMenu::activePings.begin();
       i != ServerMenu::activePings.end();) {
    i->second.first->doPings();
    if (i->second.first->done()) {
      ServerItem* server = list.lookupServer(i->first);
      if (server == NULL)
	break;
      server->ping.pingTime = i->second.first->calcLag();
      server->ping.pinging = false;
      for (size_t j=0; j<(i->second.second.size()); j++) {
	i->second.second[j]->addItem(server);
      }
      delete i->second.first;
      ServerMenu::activePings.erase(i++);
      continue;
    }
    ++i;
  }

  static_cast<ServerMenu*>(_self)->serverList.checkEchos(getStartupInfo());
  static_cast<ServerMenu*>(_self)->updateStatus();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
