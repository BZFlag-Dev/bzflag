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
#include "ServerMenu.h"

/* system implementation headers */
#include <sys/types.h>
#if !defined(_WIN32)
#include <errno.h>
#endif
#include <vector>
#include <string>

/* common implementation headers */
#include "version.h"
#include "bzsignal.h"
#include "Ping.h"
#include "Protocol.h"
#include "TimeKeeper.h"
#include "TextUtils.h"
#include "FontManager.h"

/* local implementation headers */
#include "ServerListCache.h"
#include "MainMenu.h"
#include "StartupInfo.h"
#include "HUDDialogStack.h"
#include "ErrorHandler.h"
#include "HUDui.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"

/* from playing.h */
StartupInfo* getStartupInfo();
typedef void (*PlayingCallback)(void*);
void addPlayingCallback(PlayingCallback, void* data);
void removePlayingCallback(PlayingCallback, void* data);


const int ServerMenu::NumReadouts = 24;
const int ServerMenu::NumItems = 10;

bool ServerMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 0) switch (key.button) {
    case BzfKeyEvent::Up:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() - 1);
      }
      return true;

    case BzfKeyEvent::Down:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() + 1);
      }
      return true;

    case BzfKeyEvent::PageUp:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() - ServerMenu::NumItems);
      }
      return true;

    case BzfKeyEvent::PageDown:
      if (HUDui::getFocus()) {
	menu->setSelected(menu->getSelected() + ServerMenu::NumItems);
      }
      return true;
  }

  else if (key.ascii == '\t') {
    if (HUDui::getFocus()) {
      menu->setSelected(menu->getSelected() + 1);
    }
    return true;
  }

  return MenuDefaultKey::keyPress(key);
}

bool ServerMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.button) {
    case BzfKeyEvent::Up:
    case BzfKeyEvent::Down:
    case BzfKeyEvent::PageUp:
    case BzfKeyEvent::PageDown:
      return true;
  }
  switch (key.ascii) {
    case 27:	// escape
    case 13:	// return
      return true;
  }
  return false;
}

ServerMenu::ServerMenu() : defaultKey(this),
				selectedIndex(0),
				serversFound(0)
{
  // add controls
  addLabel("Servers", "");
  addLabel("Players", "");
  addLabel("Rogue", "");
  addLabel("Red", "");
  addLabel("Green", "");
  addLabel("Blue", "");
  addLabel("Purple", "");
  addLabel("Observers", "");
  addLabel("", "");			// max shots
  addLabel("", "");			// capture-the-flag/free-style/rabbit chase
  addLabel("", "");			// super-flags
  addLabel("", "");			// antidote-flag
  addLabel("", "");			// shaking time
  addLabel("", "");			// shaking wins
  addLabel("", "");			// jumping
  addLabel("", "");			// ricochet
  addLabel("", "");			// inertia
  addLabel("", "");			// time limit
  addLabel("", "");			// max team score
  addLabel("", "");			// max player score
  addLabel("", "");			// cached status
  addLabel("", "");			// cached age
  addLabel("", "");			// search status
  addLabel("", "");			// page readout
  status = (HUDuiLabel*)(getControls()[NumReadouts - 2]);
  pageLabel = (HUDuiLabel*)(getControls()[NumReadouts - 1]);

  // add server list items
  for (int i = 0; i < NumItems; ++i)
    addLabel("", "");

  // set initial focus
  setFocus(status);
}



void ServerMenu::addLabel(const char* msg, const char* _label)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString(msg);
  label->setLabel(_label);
  getControls().push_back(label);
}

int ServerMenu::getSelected() const
{
  return selectedIndex;
}

void ServerMenu::setSelected(int index)
{
  // clamp index
  if (index < 0)
    index = serverList.size() - 1;
  else if (index != 0 && index >= (int)serverList.size())
    index = 0;

  // ignore if no change
  if (selectedIndex == index)
    return;

  // update selected index and get old and new page numbers
  const int oldPage = (selectedIndex < 0) ? -1 : (selectedIndex / NumItems);
  selectedIndex = index;
  const int newPage = (selectedIndex / NumItems);

  // if page changed then load items for this page
  if (oldPage != newPage) {
    // fill items
    std::vector<HUDuiControl*>& list = getControls();
    const int base = newPage * NumItems;
    for (int i = 0; i < NumItems; ++i) {
      HUDuiLabel* label = (HUDuiLabel*)list[i + NumReadouts];
      if (base + i < (int)serverList.size()) {
	label->setString(serverList.getServers()[base + i].description);
	if (serverList.getServers()[base + i].cached ){
	  label->setDarker(true);
	}
	else {
	  label->setDarker(false);
	}
      }
      else {
	label->setString("");
      }
    }

    // change page label
    if ((int)serverList.size() > NumItems) {
      char msg[50];
      std::vector<std::string> args;
      sprintf(msg, "%d", newPage + 1);
      args.push_back(msg);
      sprintf(msg, "%ld", (long int)(serverList.size() + NumItems - 1) / NumItems);
      args.push_back(msg);
      pageLabel->setString("Page {1} of {2}", &args);
    }
  }

  // set focus to selected item
  if (serverList.size() > 0) {
    const int indexOnPage = selectedIndex % NumItems;
    getControls()[NumReadouts + indexOnPage]->setFocus();
  }

  // update readouts
  pick();
}

void ServerMenu::pick()
{
  if (serverList.size() == 0)
    return;

  // get server info
  const ServerItem& item = serverList.getServers()[selectedIndex];
  const PingPacket& ping = item.ping;

  // update server readouts
  char buf[60];
  std::vector<HUDuiControl*>& list = getControls();

  const uint8_t maxes [] = { ping.maxPlayers, ping.rogueMax, ping.redMax, ping.greenMax,
      ping.blueMax, ping.purpleMax, ping.observerMax };

  // if this is a cached item set the player counts to "?/max count"
  if (item.cached && item.getPlayerCount() == 0) {
    for (int i = 1; i <=7; i ++){
      sprintf(buf, "?/%d", maxes[i-1]);
      ((HUDuiLabel*)list[i])->setLabel(buf);
    }
  } else {  // not an old item, set players #s to info we have
    sprintf(buf, "%d/%d", ping.rogueCount + ping.redCount + ping.greenCount +
	ping.blueCount + ping.purpleCount + ping.observerCount, ping.maxPlayers);
    ((HUDuiLabel*)list[1])->setLabel(buf);

    if (ping.rogueMax == 0)
      buf[0]=0;
    else if (ping.rogueMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.rogueCount);
    else
      sprintf(buf, "%d/%d", ping.rogueCount, ping.rogueMax);
    ((HUDuiLabel*)list[2])->setLabel(buf);

    if (ping.redMax == 0)
      buf[0]=0;
    else if (ping.redMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.redCount);
    else
      sprintf(buf, "%d/%d", ping.redCount, ping.redMax);
    ((HUDuiLabel*)list[3])->setLabel(buf);

    if (ping.greenMax == 0)
      buf[0]=0;
    else if (ping.greenMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.greenCount);
    else
      sprintf(buf, "%d/%d", ping.greenCount, ping.greenMax);
    ((HUDuiLabel*)list[4])->setLabel(buf);

    if (ping.blueMax == 0)
      buf[0]=0;
    else if (ping.blueMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.blueCount);
    else
      sprintf(buf, "%d/%d", ping.blueCount, ping.blueMax);
    ((HUDuiLabel*)list[5])->setLabel(buf);

    if (ping.purpleMax == 0)
      buf[0]=0;
    else if (ping.purpleMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.purpleCount);
    else
      sprintf(buf, "%d/%d", ping.purpleCount, ping.purpleMax);
    ((HUDuiLabel*)list[6])->setLabel(buf);

    if (ping.observerMax == 0)
      buf[0]=0;
    else if (ping.observerMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.observerCount);
    else
      sprintf(buf, "%d/%d", ping.observerCount, ping.observerMax);
    ((HUDuiLabel*)list[7])->setLabel(buf);
  }

  std::vector<std::string> args;
  sprintf(buf, "%d", ping.maxShots);
  args.push_back(buf);

  if (ping.maxShots == 1)
    ((HUDuiLabel*)list[8])->setString("{1} Shot", &args );
  else
    ((HUDuiLabel*)list[8])->setString("{1} Shots", &args );

  if (ping.gameStyle & TeamFlagGameStyle)
    ((HUDuiLabel*)list[9])->setString("Capture-the-Flag");
  else if (ping.gameStyle & RabbitChaseGameStyle)
    ((HUDuiLabel*)list[9])->setString("Rabbit Chase");
  else
    ((HUDuiLabel*)list[9])->setString("Free-style");

  if (ping.gameStyle & SuperFlagGameStyle)
    ((HUDuiLabel*)list[10])->setString("Super Flags");
  else
    ((HUDuiLabel*)list[10])->setString("");

  if (ping.gameStyle & AntidoteGameStyle)
    ((HUDuiLabel*)list[11])->setString("Antidote Flags");
  else
    ((HUDuiLabel*)list[11])->setString("");

  if ((ping.gameStyle & ShakableGameStyle) && ping.shakeTimeout != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%.1f", 0.1f * float(ping.shakeTimeout));
    args.push_back(buf);
    if (ping.shakeWins == 1)
      ((HUDuiLabel*)list[12])->setString("{1} sec To Drop Bad Flag", &args);
    else
      ((HUDuiLabel*)list[12])->setString("{1} secs To Drop Bad Flag", &args);
  }
  else
    ((HUDuiLabel*)list[13])->setString("");

  if ((ping.gameStyle & ShakableGameStyle) && ping.shakeWins != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%d", ping.shakeWins);
    args.push_back(buf);
    args.push_back(ping.shakeWins == 1 ? "" : "s");
    if (ping.shakeWins == 1)
      ((HUDuiLabel*)list[12])->setString("{1} Win Drops Bad Flag", &args);
    else
      ((HUDuiLabel*)list[12])->setString("{1} Wins Drops Bad Flag", &args);
  }
  else
    ((HUDuiLabel*)list[13])->setString("");

  if (ping.gameStyle & JumpingGameStyle)
    ((HUDuiLabel*)list[14])->setString("Jumping");
  else
    ((HUDuiLabel*)list[14])->setString("");

  if (ping.gameStyle & RicochetGameStyle)
    ((HUDuiLabel*)list[15])->setString("Ricochet");
  else
    ((HUDuiLabel*)list[15])->setString("");

  if (ping.gameStyle & HandicapGameStyle)
    ((HUDuiLabel*)list[16])->setString("Handicap");
  else
    ((HUDuiLabel*)list[16])->setString("");

  if (ping.maxTime != 0) {
    std::vector<std::string> args;
    if (ping.maxTime >= 3600)
      sprintf(buf, "%d:%02d:%02d", ping.maxTime / 3600, (ping.maxTime / 60) % 60, ping.maxTime % 60);
    else if (ping.maxTime >= 60)
      sprintf(buf, "%d:%02d", ping.maxTime / 60, ping.maxTime % 60);
    else
      sprintf(buf, "0:%02d", ping.maxTime);
    args.push_back(buf);
    ((HUDuiLabel*)list[17])->setString("Time limit: {1}", &args);
  }
  else
    ((HUDuiLabel*)list[17])->setString("");

  if (ping.maxTeamScore != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%d", ping.maxTeamScore);
    args.push_back(buf);
    ((HUDuiLabel*)list[18])->setString("Max team score: {1}", &args);
  }
  else
    ((HUDuiLabel*)list[18])->setString("");


  if (ping.maxPlayerScore != 0) {
    std::vector<std::string> args;
    sprintf(buf, "%d", ping.maxPlayerScore);
    args.push_back(buf);
    ((HUDuiLabel*)list[19])->setString("Max player score: {1}", &args);
  }
  else
    ((HUDuiLabel*)list[19])->setString("");

  if (item.cached){
    ((HUDuiLabel*)list[20])->setString("Cached");
    ((HUDuiLabel*)list[21])->setString(item.getAgeString());
  }
  else {
    ((HUDuiLabel*)list[20])->setString("");
    ((HUDuiLabel*)list[21])->setString("");
  }
}

void			ServerMenu::show()
{
  // clear server readouts
  std::vector<HUDuiControl*>& list = getControls();
  ((HUDuiLabel*)list[1])->setLabel("");
  ((HUDuiLabel*)list[2])->setLabel("");
  ((HUDuiLabel*)list[3])->setLabel("");
  ((HUDuiLabel*)list[4])->setLabel("");
  ((HUDuiLabel*)list[5])->setLabel("");
  ((HUDuiLabel*)list[6])->setLabel("");
  ((HUDuiLabel*)list[7])->setLabel("");
  ((HUDuiLabel*)list[8])->setString("");
  ((HUDuiLabel*)list[9])->setString("");
  ((HUDuiLabel*)list[10])->setString("");
  ((HUDuiLabel*)list[11])->setString("");
  ((HUDuiLabel*)list[12])->setString("");
  ((HUDuiLabel*)list[13])->setString("");
  ((HUDuiLabel*)list[14])->setString("");
  ((HUDuiLabel*)list[15])->setString("");
  ((HUDuiLabel*)list[16])->setString("");
  ((HUDuiLabel*)list[17])->setString("");
  ((HUDuiLabel*)list[18])->setString("");
  ((HUDuiLabel*)list[19])->setString("");
  ((HUDuiLabel*)list[20])->setString("");
  ((HUDuiLabel*)list[21])->setString("");

  // add cache items w/o re-caching them
  serversFound = 0;
  int numItemsAdded;
  numItemsAdded = serverList.updateFromCache();

  // update the status
  updateStatus();

  // focus to no-server
  setFocus(status);

  // *** NOTE *** start ping here
  // listen for echos
  addPlayingCallback(&playingCB, this);
  serverList.startServerPings();
}

void			ServerMenu::execute()
{
  if (selectedIndex < 0 || selectedIndex >= (int)serverList.size())
    return;

  // update startup info
  StartupInfo* info = getStartupInfo();
  strcpy(info->serverName, serverList.getServers()[selectedIndex].name.c_str());
  info->serverPort = ntohs((unsigned short)
				serverList.getServers()[selectedIndex].ping.serverId.port);

  // all done
  HUDDialogStack::get()->pop();
}

void			ServerMenu::dismiss()
{
  // no more callbacks
  removePlayingCallback(&playingCB, this);
  // save any new token we got
  // FIXME myTank.token = serverList.token;
}

void			ServerMenu::resize(int width, int height)
{
  // remember size
  HUDDialog::resize(width, height);

  // get number of serverList.getServers()
  std::vector<HUDuiControl*>& list = getControls();

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)height / 15.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  float x, y;
  {
    HUDuiLabel* title = (HUDuiLabel*)list[0];
    title->setFontSize(titleFontSize);
    const float titleWidth = fm.getStrLength(title->getFontFace(), titleFontSize, title->getString());
    const float titleHeight = fm.getStrHeight(title->getFontFace(), titleFontSize, " ");
    x = 0.5f * ((float)width - titleWidth);
    y = (float)height - titleHeight;
    title->setPosition(x, y);
  }

  // reposition server readouts
  int i;
  const float y0 = y;
  float fontSize = (float)height / 54.0f;
  float fontHeight = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  for (i = 1; i < NumReadouts - 2; i++) {
    if (i % 7 == 1) {
      x = (0.125f + 0.25f * (float)((i - 1) / 7)) * (float)width;
      y = y0;
    }

    HUDuiLabel* label = (HUDuiLabel*)list[i];
    label->setFontSize(fontSize);
    y -= 1.0f * fontHeight;
    label->setPosition(x, y);
  }

  y = ((HUDuiLabel*)list[7])->getY(); //reset bottom to last team label

  // reposition search status readout
  {
    fontSize = (float)height / 36.0f;
    float fontHeight = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
    status->setFontSize(fontSize);
    const float statusWidth = fm.getStrLength(status->getFontFace(), fontSize, status->getString());
    x = 0.5f * ((float)width - statusWidth);
    y -= 0.8f * fontHeight;
    status->setPosition(x, y);
  }

  // position page readout and server item list
  fontSize = (float)height / 54.0f;
  fontHeight = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  x = 0.125f * (float)width;
  for (i = -1; i < NumItems; ++i) {
    HUDuiLabel* label = (HUDuiLabel*)list[i + NumReadouts];
    label->setFontSize(fontSize);
    y -= 1.0f * fontHeight;
    label->setPosition(x, y);
  }
}

void			ServerMenu::setStatus(const char* msg, const std::vector<std::string> *parms)
{
  status->setString(msg, parms);
  FontManager &fm = FontManager::instance();
  const float statusWidth = fm.getStrLength(status->getFontFace(), status->getFontSize(), status->getString());
  status->setPosition(0.5f * ((float)width - statusWidth), status->getY());
}

void ServerMenu::updateStatus() {
  if (serverList.searchActive()) {
    setStatus("Searching");
  } else if (serversFound < serverList.size()) {
    std::vector<std::string> args;
    char buffer [80];
    sprintf(buffer, "%d", (unsigned int)serverList.size());
    args.push_back(buffer);
    setStatus("Servers found: {1}", &args);
    pageLabel->setString("");
    selectedIndex = -1;
    setSelected(0);

    serversFound = serverList.size();
  }
}


void			ServerMenu::playingCB(void* _self)
{
  ((ServerMenu*)_self)->serverList.checkEchos();

  ((ServerMenu*)_self)->updateStatus();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
