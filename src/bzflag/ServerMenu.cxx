/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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

// system headers
#include <string.h>

/* common implementation headers */
#include "FontManager.h"
#include "TextUtils.h"
#include "AnsiCodes.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "ServerListFilterMenu.h"
#include "ServerListFilterHelpMenu.h"
#include "playing.h"
#include "HUDui.h"
#include "ServerListFilter.h"

const int ServerMenu::NumReadouts = 24;
const int ServerMenu::NumItems = 10;

ServerMenuDefaultKey::~ServerMenuDefaultKey()
{
  delete serverListFilterMenu;
  ServerListFilterHelpMenu::done();
}

bool ServerMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 0) {
    switch (key.button) {
      case BzfKeyEvent::Up: {
	if (HUDui::getFocus()) {
	  if (!menu->getFind()) {
	    menu->setSelected(menu->getSelected() - 1);
	  } else {
	    menu->setFind(false);
	  }
	}
	return true;
      }
      case BzfKeyEvent::Down: {
	if (HUDui::getFocus()) {
	  if (!menu->getFind()) {
	    menu->setSelected(menu->getSelected() + 1);
	  } else {
	    menu->setFind(false);
	  }
	}
	return true;
      }
      case BzfKeyEvent::PageUp: {
	if (HUDui::getFocus()) {
	  if (!menu->getFind()) {
	    menu->setSelected(menu->getSelected() - ServerMenu::NumItems);
	  } else {
	    menu->setFind(false);
	  }
	}
	return true;
      }
      case BzfKeyEvent::PageDown: {
	if (HUDui::getFocus()) {
	  if (!menu->getFind()) {
	    menu->setSelected(menu->getSelected() + ServerMenu::NumItems);
	  } else {
	    menu->setFind(false);
	  }
	}
	return true;
      }
    }
  }
  else if (key.ascii == '\t') {
    if (HUDui::getFocus()) {
      menu->setSelected(menu->getSelected() + 1);
    }
    return true;
  }
  else if (key.ascii == '/') {
    if (HUDui::getFocus() && !menu->getFind()) {
      menu->setFind(true, (key.shift & BzfKeyEvent::AltKey) != 0);
      return true;
    }
  }
  else if (key.ascii == 'f') {
    if (HUDui::getFocus() && !menu->getFind()) {
      menu->toggleFavView();
      return true;
    }
  }
  else if (key.ascii == '+') {
    if (HUDui::getFocus() && !menu->getFind()) {
      menu->setFav(true);
      return true;
    }
  }
  else if (key.ascii == '-') {
    if (HUDui::getFocus() && !menu->getFind()) {
      menu->setFav(false);
      return true;
    }
  }
  else if ((key.ascii >= '0') && (key.ascii <= '9')) {
    if (HUDui::getFocus() && !menu->getFind()) {
      menu->setFindIndex(key.ascii - '0');
      return true;
    }
  }
  else if (key.ascii == 'e') {
    if (HUDui::getFocus() && !menu->getFind()) {
      if (!serverListFilterMenu) serverListFilterMenu = new ServerListFilterMenu;
      HUDDialogStack::get()->push(serverListFilterMenu);
      return true;
    }
  }
  else if (key.ascii == '?') {
    if (HUDui::getFocus() && !menu->getFind()) {
      HUDDialogStack::get()->push(ServerListFilterHelpMenu::getServerListFilterHelpMenu());
      return true;
    }
  }
  else if (key.ascii == 27) {
    if (HUDui::getFocus()) {
      // escape drops out of find mode
      // note that this is handled by MenuDefaultKey if we're not in find mode
      if (menu->getFind()) {
	menu->setFind(false);
	return true;
      }
    }
  }

  return MenuDefaultKey::keyPress(key);
}


bool ServerMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.button) {
    case BzfKeyEvent::Up:
    case BzfKeyEvent::Down:
    case BzfKeyEvent::PageUp:
    case BzfKeyEvent::PageDown: {
      return true;
    }
  }
  switch (key.ascii) {
    case 27:   // escape
    case 13: { // return
      return true;
    }
  }
  return false;
}


ServerMenu::ServerMenu()
: defaultKey(this)
, selectedIndex(0)
, serversFound(0)
, realServersFound(0)
, findMode(false)
, favView(false)
, newfilter(true)
, listFilter(BZDB.get("listFilter"))
, lastWidth(0)
, lastHeight(0)
{
  if (debugLevel > 0) {
    listFilter.print();
  }

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

  // find server
  search = new HUDuiTypeIn;
  search->setFontFace(MainMenu::getFontFace());
  search->setMaxLength(42);
  search->setString(listFilter.getSource());
  search->setColorFunc(ServerListFilter::colorizeSearch);
  getControls().push_back(search);
  setFind(false);

  // short key help
  help1 = new HUDuiLabel;
  help1->setFontFace(MainMenu::getFontFace());
  help1->setString("Press +/- to add/remove favorites, f to toggle favorites-only list,");
  getControls().push_back(help1);

  help2 = new HUDuiLabel;
  help2->setFontFace(MainMenu::getFontFace());
  help2->setString("1 to 9 for quick filters, 0 to clear, e to edit quick filters, ? for filter help");
  getControls().push_back(help2);

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


void ServerMenu::setFindLabel(const std::string& label)
{
  search->setLabel(label);
  if (lastWidth && lastHeight) {
    resize(lastWidth, lastHeight); // overkill...
  }
}


void ServerMenu::setFind(bool mode, bool clear)
{
  findMode = mode;

  const std::string oldFilterSource = listFilter.getSource();

  if (clear) {
    search->setString("");
  }

  if (mode) { // filter is being typed in
    setFindLabel(ANSI_STR_FG_GREEN "Edit Filter:");
    search->setFocus();
  }
  else {
    listFilter.parse(search->getString());
    BZDB.set("listFilter", search->getString());
    if (listFilter.getSource().empty()) {
      setFindLabel("Press '/' to search");
    } else {
      std::string filter_number;
      for (int i = 1; i <= 9; i++) {
	if (listFilter.getSource() == BZDB.get(TextUtils::format("listFilter%d", i))) {
	  filter_number = TextUtils::format(" %d", i);
	  break;
	}
      }
      setFindLabel(ANSI_STR_FG_RED "Using filter" + filter_number + ":");
    }
    // select the first item in the list
    setSelected(0, true);
  }

  if (debugLevel > 0) {
    listFilter.print();
  }

  newfilter = (listFilter.getSource() != oldFilterSource);
}


void ServerMenu::setFindIndex(int index)
{
  if ((index >= 1) && (index <= 9)) {
    std::string name = "listFilter";
    name += (index + '0');
    search->setString(BZDB.get(name));
  } else {
    search->setString("");
  }

  setFind(false);
}


bool ServerMenu::getFind() const
{
  return findMode;
}


void ServerMenu::toggleFavView()
{
  favView = !favView;
  newfilter = true;
  updateStatus();
}


void ServerMenu::setFav(bool fav)
{
  if (selectedIndex < 0 || (int)serverList.size() <= selectedIndex)
    return;	// no such entry (server list may be empty)
  const ServerItem& item = serverList.getServers()[selectedIndex];
  std::string addrname = item.getAddrName();
  ServerListCache *cache = ServerListCache::get();
  ServerListCache::SRV_STR_MAP::iterator i = cache->find(addrname);
  if (i != cache->end()) {
    i->second.favorite = fav;
  } else {
    // FIXME  should not ever come here, but what to do?
  }
  realServerList.markFav(addrname, fav);
  serverList.markFav(addrname, fav);

  setSelected(getSelected()+1, true);
}


int ServerMenu::getSelected() const
{
  return selectedIndex;
}


void ServerMenu::setSelected(int index, bool forcerefresh)
{
  // clamp index
  if (index < 0)
    index = serverList.size() - 1;
  else if (index != 0 && index >= (int)serverList.size())
    index = 0;

  // ignore if no change
  if (!forcerefresh && selectedIndex == index) {
    return;
  }

  // update selected index and get old and new page numbers
  const int oldPage = (selectedIndex < 0) ? -1 : (selectedIndex / NumItems);
  selectedIndex = index;
  const int newPage = (selectedIndex / NumItems);

  // if page changed then load items for this page
  if (oldPage != newPage || forcerefresh) {
    // fill items
    std::vector<HUDuiControl*>& listHUD = getControls();
    const int base = newPage * NumItems;
    for (int i = 0; i < NumItems; ++i) {
      HUDuiLabel* label = (HUDuiLabel*)listHUD[i + NumReadouts];
      if (base + i < (int)serverList.size()) {
	const ServerItem &server = serverList.getServers()[base + i];
	const short gameType = server.ping.gameType;
	const short gameOptions = server.ping.gameOptions;
	std::string fullLabel;
	if (BZDB.isTrue("listIcons")) {
	  // game mode
	  if ((server.ping.observerMax == 16) &&
	      (server.ping.maxPlayers == 200)) {
	    fullLabel += ANSI_STR_FG_CYAN "*  "; // replay
	  } else if (gameType == ClassicCTF) {
	    fullLabel += ANSI_STR_FG_RED "*  "; // ctf
	  } else if (gameType == RabbitChase) {
	    fullLabel += ANSI_STR_FG_WHITE "*  "; // white rabbit
	  } else {
	    fullLabel += ANSI_STR_FG_YELLOW "*  "; // free-for-all
	  }
	  // jumping?
	  if (gameOptions & JumpingGameStyle) {
	    fullLabel += ANSI_STR_BRIGHT ANSI_STR_FG_MAGENTA "J ";
	  } else {
	    fullLabel += ANSI_STR_DIM ANSI_STR_FG_WHITE "J ";
	  }
	  // superflags ?
	  if (gameOptions & SuperFlagGameStyle) {
	    fullLabel += ANSI_STR_BRIGHT ANSI_STR_FG_BLUE "F ";
	  } else {
	    fullLabel += ANSI_STR_DIM ANSI_STR_FG_WHITE "F ";
	  }
	  // ricochet?
	  if (gameOptions & RicochetGameStyle) {
	    fullLabel += ANSI_STR_BRIGHT ANSI_STR_FG_GREEN "R";
	  } else {
	    fullLabel += ANSI_STR_DIM ANSI_STR_FG_WHITE "R";
	  }
	  fullLabel += ANSI_STR_RESET "   ";

	  // colorize server descriptions by shot counts
	  const int maxShots = server.ping.maxShots;
	  if (maxShots <= 0) {
	    label->setColor(0.4f, 0.0f, 0.6f); // purple
	  }
	  else if (maxShots == 1) {
	    label->setColor(0.25f, 0.25f, 1.0f); // blue
	  }
	  else if (maxShots == 2) {
	    label->setColor(0.25f, 1.0f, 0.25f); // green
	  }
	  else if (maxShots == 3) {
	    label->setColor(1.0f, 1.0f, 0.25f); // yellow
	  }
	  else {
	    // graded orange/red
	    const float shotScale =
	      std::min(1.0f, log10f((float)(maxShots - 3)));
	    const float rf = 1.0f;
	    const float gf = 0.4f * (1.0f - shotScale);
	    const float bf = 0.25f * gf;
	    label->setColor(rf, gf, bf);
	  }
	}
	else {
	  // colorize servers: many shots->red, jumping->green, CTF->blue
	  const float rf = std::min(1.0f, logf((float)server.ping.maxShots) / logf(20.0f));
	  const float gf = gameOptions & JumpingGameStyle ? 1.0f : 0.0f;
	  const float bf = (gameType == ClassicCTF) ? 1.0f : 0.0f;
	  label->setColor(0.5f + rf * 0.5f, 0.5f + gf * 0.5f, 0.5f + bf * 0.5f);
	}

	std::string addr, desc;
	server.splitAddrTitle(addr, desc);
	if (server.favorite)
	  fullLabel += ANSI_STR_FG_ORANGE;
	else
	  fullLabel += ANSI_STR_FG_WHITE;
	fullLabel += addr;
	if (!desc.empty()) {
	  fullLabel += ANSI_STR_RESET "  ";
	  fullLabel += desc;
	}
	label->setString(fullLabel);
	label->setDarker(server.cached);
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

  // set focus to selected item unless we are typing into the search field
  if (serverList.size() > 0 && ! findMode) {
    const int indexOnPage = selectedIndex % NumItems;
    getControls()[NumReadouts + indexOnPage]->setFocus();
  }

  // update readouts
  pick();
}


void ServerMenu::pick()
{
  if (serverList.size() == 0) {
    return;
  }

  // get server info
  const ServerItem& item = serverList.getServers()[selectedIndex];
  const PingPacket& ping = item.ping;

  // update server readouts
  char buf[60];
  std::vector<HUDuiControl*>& listHUD = getControls();

  const uint8_t maxes [] = { ping.maxPlayers, ping.rogueMax, ping.redMax, ping.greenMax,
      ping.blueMax, ping.purpleMax, ping.observerMax };

  // if this is a cached item set the player counts to "?/max count"
  if (item.cached && item.getPlayerCount() == 0) {
    for (int i = 1; i <=7; i ++) {
      sprintf(buf, "?/%d", maxes[i-1]);
      ((HUDuiLabel*)listHUD[i])->setLabel(buf);
    }
  }
  else {  // not an old item, set players #s to info we have
    sprintf(buf, "%d/%d", ping.rogueCount + ping.redCount + ping.greenCount +
	ping.blueCount + ping.purpleCount + ping.observerCount, ping.maxPlayers);
    ((HUDuiLabel*)listHUD[1])->setLabel(buf);

    if (ping.rogueMax == 0)
      buf[0]=0;
    else if (ping.rogueMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.rogueCount);
    else
      sprintf(buf, "%d/%d", ping.rogueCount, ping.rogueMax);
    ((HUDuiLabel*)listHUD[2])->setLabel(buf);

    if (ping.redMax == 0)
      buf[0]=0;
    else if (ping.redMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.redCount);
    else
      sprintf(buf, "%d/%d", ping.redCount, ping.redMax);
    ((HUDuiLabel*)listHUD[3])->setLabel(buf);

    if (ping.greenMax == 0)
      buf[0]=0;
    else if (ping.greenMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.greenCount);
    else
      sprintf(buf, "%d/%d", ping.greenCount, ping.greenMax);
    ((HUDuiLabel*)listHUD[4])->setLabel(buf);

    if (ping.blueMax == 0)
      buf[0]=0;
    else if (ping.blueMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.blueCount);
    else
      sprintf(buf, "%d/%d", ping.blueCount, ping.blueMax);
    ((HUDuiLabel*)listHUD[5])->setLabel(buf);

    if (ping.purpleMax == 0)
      buf[0]=0;
    else if (ping.purpleMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.purpleCount);
    else
      sprintf(buf, "%d/%d", ping.purpleCount, ping.purpleMax);
    ((HUDuiLabel*)listHUD[6])->setLabel(buf);

    if (ping.observerMax == 0)
      buf[0]=0;
    else if (ping.observerMax >= ping.maxPlayers)
      sprintf(buf, "%d", ping.observerCount);
    else
      sprintf(buf, "%d/%d", ping.observerCount, ping.observerMax);
    ((HUDuiLabel*)listHUD[7])->setLabel(buf);
  }

  std::vector<std::string> args;
  sprintf(buf, "%d", ping.maxShots);
  args.push_back(buf);

  if (ping.maxShots == 1)
    ((HUDuiLabel*)listHUD[8])->setString("{1} Shot", &args );
  else
    ((HUDuiLabel*)listHUD[8])->setString("{1} Shots", &args );

  if (ping.gameType == ClassicCTF)
    ((HUDuiLabel*)listHUD[9])->setString("Classic Capture-the-Flag");
  else if (ping.gameType == RabbitChase)
    ((HUDuiLabel*)listHUD[9])->setString("Rabbit Chase");
  else if (ping.gameType == OpenFFA)
    ((HUDuiLabel*)listHUD[9])->setString("Open (Teamless) Free-For-All");
  else
    ((HUDuiLabel*)listHUD[9])->setString("Free-style");

  if (ping.gameOptions & SuperFlagGameStyle)
    ((HUDuiLabel*)listHUD[10])->setString("Super Flags");
  else
    ((HUDuiLabel*)listHUD[10])->setString("");

  if (ping.gameOptions & AntidoteGameStyle)
    ((HUDuiLabel*)listHUD[11])->setString("Antidote Flags");
  else
    ((HUDuiLabel*)listHUD[11])->setString("");

  if ((ping.gameOptions & ShakableGameStyle) && ping.shakeTimeout != 0) {
    std::vector<std::string> dropArgs;
    sprintf(buf, "%.1f", 0.1f * float(ping.shakeTimeout));
    dropArgs.push_back(buf);
    if (ping.shakeWins == 1)
      ((HUDuiLabel*)listHUD[12])->setString("{1} sec To Drop Bad Flag",
					    &dropArgs);
    else
      ((HUDuiLabel*)listHUD[12])->setString("{1} secs To Drop Bad Flag",
					    &dropArgs);
  }
  else {
    ((HUDuiLabel*)listHUD[12])->setString("");
  }

  if ((ping.gameOptions & ShakableGameStyle) && ping.shakeWins != 0) {
    std::vector<std::string> dropArgs;
    sprintf(buf, "%d", ping.shakeWins);
    dropArgs.push_back(buf);
    dropArgs.push_back(ping.shakeWins == 1 ? "" : "s");
    if (ping.shakeWins == 1)
      ((HUDuiLabel*)listHUD[12])->setString("{1} Win Drops Bad Flag",
					    &dropArgs);
    else
      ((HUDuiLabel*)listHUD[12])->setString("{1} Wins Drops Bad Flag",
					    &dropArgs);
  }
  else {
    ((HUDuiLabel*)listHUD[12])->setString("");
  }

  if (ping.gameOptions & NoTeamKillsGameStyle) {
    ((HUDuiLabel*)listHUD[13])->setString("No Teamkills");
  } else {
    ((HUDuiLabel*)listHUD[13])->setString("");
  }

  if (ping.gameOptions & JumpingGameStyle) {
    ((HUDuiLabel*)listHUD[14])->setString("Jumping");
  } else {
    ((HUDuiLabel*)listHUD[14])->setString("");
  }

  if (ping.gameOptions & RicochetGameStyle) {
    ((HUDuiLabel*)listHUD[15])->setString("Ricochet");
  } else {
    ((HUDuiLabel*)listHUD[15])->setString("");
  }

  if (ping.gameOptions & HandicapGameStyle) {
    ((HUDuiLabel*)listHUD[16])->setString("Handicap");
  } else {
    ((HUDuiLabel*)listHUD[16])->setString("");
  }

  if (ping.maxTime != 0) {
    std::vector<std::string> pingArgs;
    if (ping.maxTime >= 3600)
      sprintf(buf, "%d:%02d:%02d", ping.maxTime / 3600, (ping.maxTime / 60) % 60, ping.maxTime % 60);
    else if (ping.maxTime >= 60)
      sprintf(buf, "%d:%02d", ping.maxTime / 60, ping.maxTime % 60);
    else
      sprintf(buf, "0:%02d", ping.maxTime);
    pingArgs.push_back(buf);
    ((HUDuiLabel*)listHUD[17])->setString("Time limit: {1}", &pingArgs);
  } else {
    ((HUDuiLabel*)listHUD[17])->setString("");
  }

  if (ping.maxTeamScore != 0) {
    std::vector<std::string> scoreArgs;
    sprintf(buf, "%d", ping.maxTeamScore);
    scoreArgs.push_back(buf);
    ((HUDuiLabel*)listHUD[18])->setString("Max team score: {1}", &scoreArgs);
  }
  else {
    ((HUDuiLabel*)listHUD[18])->setString("");
  }


  if (ping.maxPlayerScore != 0) {
    std::vector<std::string> scoreArgs;
    sprintf(buf, "%d", ping.maxPlayerScore);
    scoreArgs.push_back(buf);
    ((HUDuiLabel*)listHUD[19])->setString("Max player score: {1}", &scoreArgs);
  } else {
    ((HUDuiLabel*)listHUD[19])->setString("");
  }

  if (item.cached) {
    ((HUDuiLabel*)listHUD[20])->setString("Cached");
    ((HUDuiLabel*)listHUD[21])->setString(item.getAgeString());
  }
  else {
    ((HUDuiLabel*)listHUD[20])->setString("");
    ((HUDuiLabel*)listHUD[21])->setString("");
  }
}


void ServerMenu::show()
{
  // clear server readouts
  std::vector<HUDuiControl*>& listHUD = getControls();
  ((HUDuiLabel*)listHUD[1])->setLabel("");
  ((HUDuiLabel*)listHUD[2])->setLabel("");
  ((HUDuiLabel*)listHUD[3])->setLabel("");
  ((HUDuiLabel*)listHUD[4])->setLabel("");
  ((HUDuiLabel*)listHUD[5])->setLabel("");
  ((HUDuiLabel*)listHUD[6])->setLabel("");
  ((HUDuiLabel*)listHUD[7])->setLabel("");
  ((HUDuiLabel*)listHUD[8])->setString("");
  ((HUDuiLabel*)listHUD[9])->setString("");
  ((HUDuiLabel*)listHUD[10])->setString("");
  ((HUDuiLabel*)listHUD[11])->setString("");
  ((HUDuiLabel*)listHUD[12])->setString("");
  ((HUDuiLabel*)listHUD[13])->setString("");
  ((HUDuiLabel*)listHUD[14])->setString("");
  ((HUDuiLabel*)listHUD[15])->setString("");
  ((HUDuiLabel*)listHUD[16])->setString("");
  ((HUDuiLabel*)listHUD[17])->setString("");
  ((HUDuiLabel*)listHUD[18])->setString("");
  ((HUDuiLabel*)listHUD[19])->setString("");
  ((HUDuiLabel*)listHUD[20])->setString("");
  ((HUDuiLabel*)listHUD[21])->setString("");

  // add cache items w/o re-caching them
  serversFound = 0;
  realServerList.updateFromCache();

  // update the status
  updateStatus();

  // focus to no-server
  setFocus(status);

  // *** NOTE *** start ping here
  // listen for echos
  addPlayingCallback(&playingCB, this);
  realServerList.startServerPings(getStartupInfo());
}


void ServerMenu::execute()
{
  const bool endFind = (HUDui::getFocus() == search);
  if (endFind) {
    setFind(false);
    setSelected(0);
    return;
  }

  if (selectedIndex < 0 || selectedIndex >= (int)serverList.size()) {
    return;
  }

  // update startup info
  StartupInfo* info = getStartupInfo();
  strncpy(info->serverName, serverList.getServers()[selectedIndex].name.c_str(), sizeof(info->serverName) - 1);
  info->serverName[sizeof(info->serverName) - 1] = '\0';
  info->serverPort = ntohs((unsigned short)
			   serverList.getServers()[selectedIndex].ping.serverId.port);

  // all done
  HUDDialogStack::get()->pop();
}


void ServerMenu::dismiss()
{
  // no more callbacks
  removePlayingCallback(&playingCB, this);
  // save any new token we got
  // FIXME myTank.token = serverList.token;
}


void ServerMenu::resize(int _width, int _height)
{
  lastWidth = _width;
  lastHeight = _height;

  // remember size
  HUDDialog::resize(_width, _height);

  std::vector<HUDuiControl*>& listHUD = getControls();

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  float x, y;
  {
    HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
    title->setFontSize(titleFontSize);
    const float titleWidth = fm.getStrLength(title->getFontFace(), titleFontSize, title->getString());
    const float titleHeight = fm.getStrHeight(title->getFontFace(), titleFontSize, " ");
    x = 0.5f * ((float)_width - titleWidth);
    y = (float)_height - titleHeight;
    title->setPosition(x, y);
  }

  // reposition server readouts
  int i;
  const float y0 = y;
  float fontSize = (float)_height / 54.0f;
  float fontHeight = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  for (i = 1; i < NumReadouts - 2; i++) {
    if (i % 7 == 1) {
      x = (0.125f + 0.25f * (float)((i - 1) / 7)) * (float)_width;
      y = y0;
    }

    HUDuiLabel* label = (HUDuiLabel*)listHUD[i];
    label->setFontSize(fontSize);
    y -= 1.0f * fontHeight;
    label->setPosition(x, y);
  }

  y = ((HUDuiLabel*)listHUD[7])->getY(); //reset bottom to last team label

  // reposition search status readout
  {
    fontSize = (float)_height / 36.0f;
    float fontHt = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
    status->setFontSize(fontSize);
    const float statusWidth = fm.getStrLength(status->getFontFace(), fontSize, status->getString());
    x = 0.5f * ((float)_width - statusWidth);
    y -= 1.5f * fontHt;
    status->setPosition(x, y);
  }

  // reposition find server input
  {
    fontSize = (float)_height / 36.0f;
    float fontHt = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
    search->setFontSize(fontSize);
    const float searchWidth = fm.getStrLength(search->getFontFace(), fontSize, search->getLabel());
    x = (0.1f * (float)_width) + searchWidth;
    search->setPosition(x, fontHt * 2 /* near bottom of screen */);
  }

  // reposition key help
  {
    fontSize = (float)_height / 54.0f;
    float fontHt = fm.getStrHeight(help1->getFontFace(), fontSize, " ");
    help1->setFontSize(fontSize);
    help2->setFontSize(fontSize);
    const float help1Width = fm.getStrLength(help1->getFontFace(), fontSize, help1->getString());
    const float help2Width = fm.getStrLength(help2->getFontFace(), fontSize, help2->getString());
    help1->setPosition(0.5f * ((float)_width - help1Width), fontHt * 1.5f /* near bottom of screen */);
    help2->setPosition(0.5f * ((float)_width - help2Width), fontHt * 0.5f /* near bottom of screen */);
  }

  // position page readout and server item list
  fontSize = (float)_height / 54.0f;
  fontHeight = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  x = 0.125f * (float)_width;
  const bool useIcons = BZDB.isTrue("listIcons");
  for (i = -1; i < NumItems; ++i) {
    HUDuiLabel* label = (HUDuiLabel*)listHUD[i + NumReadouts];
    label->setFontSize(fontSize);
    y -= 1.0f * fontHeight;
    if (useIcons && (i >= 0)) {
      const float offset = fm.getStrLength(status->getFontFace(), fontSize, "*  J F R   ");
      label->setPosition(x - offset, y);
    } else {
      label->setPosition(x, y);
    }
  }
}


void ServerMenu::setStatus(const char* msg, const std::vector<std::string> *parms)
{
  status->setString(msg, parms);
  FontManager &fm = FontManager::instance();
  const float statusWidth = fm.getStrLength(status->getFontFace(), status->getFontSize(), status->getString());
  status->setPosition(0.5f * ((float)width - statusWidth), status->getY());
}


void ServerMenu::updateStatus() {
  // nothing here to see
  if (!realServerList.serverFound()) {
    setStatus("Searching");
    return;
  }

  // don't run unnecessarily
  if (realServersFound == realServerList.size() && !newfilter) {
    return;
  }

  // do filtering and counting
  int playerCount = 0;
  int observerCount = 0;
  serverList.clear();
  for (unsigned int i = 0; i < realServerList.size(); ++i) {
    const ServerItem &item = realServerList.getServers()[i];
    const PingPacket& ping = item.ping;
    playerCount += ping.rogueCount;
    playerCount += ping.redCount;
    playerCount += ping.greenCount;
    playerCount += ping.blueCount;
    playerCount += ping.purpleCount;
    observerCount += ping.observerCount;
    // filter is already lower case.  do case insensitive matching.
    if (listFilter.check(item) && (!favView || item.favorite)) {
      serverList.addToList(item);
    }
  }
  newfilter = false;

  // update the status label
  std::vector<std::string> args;
  char buffer [80];
  sprintf(buffer, "%d", (unsigned int)serverList.size());
  args.push_back(buffer);
  sprintf(buffer, "%d", (unsigned int)realServerList.size());
  args.push_back(buffer);
  sprintf(buffer, "%d", playerCount);
  args.push_back(buffer);
  sprintf(buffer, "%d", observerCount);
  args.push_back(buffer);
  if (favView) {
    setStatus("Favorite servers: {1}/{2}  ({3} players, {4} observers)", &args);
  } else {
    setStatus("Servers found: {1}/{2}  ({3} players, {4} observers)", &args);
  }
  pageLabel->setString("");
  selectedIndex = -1;
  setSelected(0);

  serversFound = serverList.size();
  realServersFound = realServerList.size();
}


void ServerMenu::playingCB(void* _self)
{
  ((ServerMenu*)_self)->realServerList.checkEchos(getStartupInfo());

  ((ServerMenu*)_self)->updateStatus();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
