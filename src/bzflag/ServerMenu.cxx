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

/* common implementation headers */
#include "version.h"
#include "bzsignal.h"
#include "Ping.h"
#include "Protocol.h"
#include "TimeKeeper.h"
#include "TextUtils.h"

/* local implementation headers */
#include "ServerListCache.h"
#include "MainMenu.h"
#include "StartupInfo.h"
#include "HUDDialogStack.h"
#include "ErrorHandler.h"

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
				pingBcastSocket(-1),
				selectedIndex(0),
				numListServers(0),
				serverCache(ServerListCache::get())
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
  label->setFont(MainMenu::getFont());
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
    index = servers.size() - 1;
  else if (index != 0 && index >= (int)servers.size())
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
      if (base + i < (int)servers.size()) {
	label->setString(servers[base + i].description);
	if (servers[base + i].cached ){
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
    if ((int)servers.size() > NumItems) {
      char msg[50];
      std::vector<std::string> args;
      sprintf(msg, "%d", newPage + 1);
      args.push_back(msg);
      sprintf(msg, "%ld", (long int)(servers.size() + NumItems - 1) / NumItems);
      args.push_back(msg);
      pageLabel->setString("Page {1} of {2}", &args);
    }
  }

  // set focus to selected item
  if (servers.size() > 0) {
    const int indexOnPage = selectedIndex % NumItems;
    getControls()[NumReadouts + indexOnPage]->setFocus();
  }

  // update readouts
  pick();
}

void ServerMenu::pick()
{
  if (servers.size() == 0)
    return;

  // get server info
  const ServerItem& item = servers[selectedIndex];
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

  if (ping.gameStyle & InertiaGameStyle)
    ((HUDuiLabel*)list[16])->setString("Inertia");
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
  // clear server list
  servers.clear();
  addedCacheToList = false;

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

  char buffer[80];

  // add cache items w/o re-caching them
  int numItemsAdded = 0;
  for (SRV_STR_MAP::const_iterator iter = serverCache->begin();
       iter != serverCache->end(); iter++) {
    // if maxCacheAge is 0 we add nothing
    // if the item is young enough we add it
    if (serverCache->getMaxCacheAge() != 0 && iter->second.getAgeMinutes() < serverCache->getMaxCacheAge()) {
      ServerItem aCopy = iter->second;
      addToList(aCopy);
      numItemsAdded ++;
    }
  }

  std::vector<std::string> args;
  sprintf(buffer, "%d", numItemsAdded);
  args.push_back(buffer);
  setStatus("Servers found: {1}", &args);
  pageLabel->setString("");
  selectedIndex = -1;
  setSelected(0);

  // focus to no-server
  setFocus(status);

  // schedule lookup of server list url.  dereference URL chain every
  // time instead of only first time just in case one of the pointers
  // has changed.
  const StartupInfo* info = getStartupInfo();
  if (info->listServerURL.size() == 0)
    phase = -1;
  else
    phase = 0;

  // also try broadcast
  pingBcastSocket = openBroadcast(BroadcastPort, NULL, &pingBcastAddr);
  if (pingBcastSocket != -1)
    PingPacket::sendRequest(pingBcastSocket, &pingBcastAddr);

  // listen for echos
  addPlayingCallback(&playingCB, this);
}

void			ServerMenu::execute()
{
  if (selectedIndex < 0 || selectedIndex >= (int)servers.size())
    return;

  // update startup info
  StartupInfo* info = getStartupInfo();
  strcpy(info->serverName, servers[selectedIndex].name.c_str());
  info->serverPort = ntohs((unsigned short)
				servers[selectedIndex].ping.serverId.port);

  // all done
  HUDDialogStack::get()->pop();
}

void			ServerMenu::dismiss()
{
  // no more callbacks
  removePlayingCallback(&playingCB, this);

  // close server list sockets
  for (int i = 0; i < numListServers; i++)
    if (listServers[i].socket != -1) {
      close(listServers[i].socket);
      listServers[i].socket = -1;
    }
  numListServers = 0;

  // close input multicast socket
  closeMulticast(pingBcastSocket);
  pingBcastSocket = -1;
}

void			ServerMenu::resize(int width, int height)
{
  // remember size
  HUDDialog::resize(width, height);

  // get number of servers
  std::vector<HUDuiControl*>& list = getControls();

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;

  // reposition title
  float x, y;
  {
    HUDuiLabel* title = (HUDuiLabel*)list[0];
    title->setFontSize(titleFontWidth, titleFontHeight);
    const OpenGLTexFont& titleFont = title->getFont();
    const float titleWidth = titleFont.getWidth(title->getString());
    x = 0.5f * ((float)width - titleWidth);
    y = (float)height - titleFont.getHeight();
    title->setPosition(x, y);
  }

  // reposition server readouts
  int i;
  const float y0 = y;
  float fontWidth = (float)height / 36.0f;
  float fontHeight = (float)height / 36.0f;
  for (i = 1; i < NumReadouts - 2; i++) {
    if (i % 7 == 1) {
      x = (0.125f + 0.25f * (float)((i - 1) / 7)) * (float)width;
      y = y0;
    }

    HUDuiLabel* label = (HUDuiLabel*)list[i];
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    y -= 1.0f * font.getHeight();
    label->setPosition(x, y);
  }

  y = ((HUDuiLabel*)list[7])->getY(); //reset bottom to last team label

  // reposition search status readout
  {
    fontWidth = (float)height / 24.0f;
    fontHeight = (float)height / 24.0f;
    status->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = status->getFont();
    const float statusWidth = font.getWidth(status->getString());
    x = 0.5f * ((float)width - statusWidth);
    y -= 0.8f * font.getHeight();
    status->setPosition(x, y);
  }

  // position page readout and server item list
  fontWidth = (float)height / 36.0f;
  fontHeight = (float)height / 36.0f;
  x = 0.125f * (float)width;
  for (i = -1; i < NumItems; ++i) {
    HUDuiLabel* label = (HUDuiLabel*)list[i + NumReadouts];
    label->setFontSize(fontWidth, fontHeight);
    const OpenGLTexFont& font = label->getFont();
    y -= 1.0f * font.getHeight();
    label->setPosition(x, y);
  }
}

void			ServerMenu::setStatus(const char* msg, const std::vector<std::string> *parms)
{
  status->setString(msg, parms);
  const OpenGLTexFont& font = status->getFont();
  const float statusWidth = font.getWidth(status->getString());
  status->setPosition(0.5f * ((float)width - statusWidth), status->getY());
}

void			ServerMenu::checkEchos()
{

  // counter used to print a status spinner
  static int counter=0;
  // how frequent to update spinner
  const float STATUS_UPDATE_FREQUENCY = 0.5; 
  // timer used to track the spinner update frequency
  static TimeKeeper lastUpdate = TimeKeeper::getCurrent();

  // print a spinning status message that updates periodically until we are 
  // actually receiving data from a list server (phase 3).  the loop below
  // is not entered until later -- so update the spinning status here too
  if (phase < 2) {
    if (TimeKeeper::getCurrent() - lastUpdate > STATUS_UPDATE_FREQUENCY) {
      /* a space trailing the spinning status icon adjusts for the
       * variable width font -- would be better to actually print
       * a status icon elsewhere or print spinning icon separate
       * from the text (and as a cool graphic).
       */
      setStatus(string_util::format("%s Searching", (counter%4==0)?"-":(counter%4==1)?" \\":(counter%4==2)?" |":" /").c_str());
      counter++;
      lastUpdate = TimeKeeper::getCurrent();
    }
  }

  // lookup server list in phase 0
  if (phase == 0) {
    int i;

    std::vector<std::string> urls;
    urls.push_back(getStartupInfo()->listServerURL);

    // check urls for validity
    numListServers = 0;
    for (i = 0; i < (int)urls.size(); ++i) {
	// parse url
	std::string protocol, hostname, path;
	int port = 80;
	Address address;
	if (!BzfNetwork::parseURL(urls[i], protocol, hostname, port, path) ||
	    protocol != "http" || port < 1 || port > 65535 ||
	    (address = Address::getHostAddress(hostname)).isAny()) {
	    std::vector<std::string> args;
	    args.push_back(urls[i]);
	    printError("Can't open list server: {1}", &args);
	    if (!addedCacheToList) {
	      addedCacheToList = true;
	      addCacheToList();
	    }
	    continue;
	}

	// add to list
	listServers[numListServers].address = address;
	listServers[numListServers].hostname = hostname;
	listServers[numListServers].pathname = path;
	listServers[numListServers].port    = port;
	listServers[numListServers].socket  = -1;
	listServers[numListServers].phase   = 2;
	numListServers++;
    }

    // do phase 1 only if we found a valid list server url
    if (numListServers > 0)
      phase = 1;
    else
      phase = -1;
  }

  // connect to list servers in phase 1
  else if (phase == 1) {
    phase = -1;
    for (int i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];

      // create socket.  give up on failure.
      listServer.socket = socket(AF_INET, SOCK_STREAM, 0);
      if (listServer.socket < 0) {
	printError("Can't create list server socket");
	listServer.socket = -1;
	if (!addedCacheToList) {
	  addedCacheToList = true;
	  addCacheToList();
	}
	continue;
      }

      // set to non-blocking.  we don't want to wait for the connection.
      if (BzfNetwork::setNonBlocking(listServer.socket) < 0) {
	printError("Error with list server socket");
	close(listServer.socket);
	listServer.socket = -1;
	if (!addedCacheToList){
	  addedCacheToList = true;
	  addCacheToList();
	}
	continue;
      }

      // start connection
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(listServer.port);
      addr.sin_addr = listServer.address;
      if (connect(listServer.socket, (CNCTType*)&addr, sizeof(addr)) < 0) {
#if defined(_WIN32)
#undef EINPROGRESS
#define EINPROGRESS EWOULDBLOCK
#endif
	if (getErrno() != EINPROGRESS) {
	  printError("Can't connect list server socket");
	  close(listServer.socket);
	  listServer.socket = -1;
	  if (!addedCacheToList){
	    addedCacheToList = true;
	    addCacheToList();
	  }
	  continue;
	}
      }

      // at least this socket is okay so proceed to phase 2
      phase = 2;
    }
  }

  // get echo messages
  while (1) {
    int i;

    // print a spinning status message that updates periodically until we are 
    // actually receiving data from a list server (phase 3).
    if (phase < 2) {
      if (TimeKeeper::getCurrent() - lastUpdate > STATUS_UPDATE_FREQUENCY) {
	/* a space trailing the spinning status icon adjusts for the
	 * variable width font -- would be better to actually print
	 * a status icon elsewhere or print spinning icon separate
	 * from the text. (and as a cool graphic)
	 */
	setStatus(string_util::format("%s Searching", (counter%4==0)?"-":(counter%4==1)?" \\":(counter%4==2)?" |":" /").c_str());
	counter++;
	lastUpdate = TimeKeeper::getCurrent();
      }
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 250;

    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    if (pingBcastSocket != -1) _FD_SET(pingBcastSocket, &read_set);
    int fdMax = pingBcastSocket;

    // check for list server connection or data
    for (i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];
      if (listServer.socket != -1) {
	if (listServer.phase == 2) {
	  _FD_SET(listServer.socket, &write_set);
	} else if (listServer.phase == 3) {
	  _FD_SET(listServer.socket, &read_set);
	}
	if (listServer.socket > fdMax) {
	  fdMax = listServer.socket;
	}
      }
    }

    const int nfound = select(fdMax+1, (fd_set*)&read_set,
					(fd_set*)&write_set, 0, &timeout);
    if (nfound <= 0) {
      break;
    }
    
    // check broadcast sockets
    ServerItem serverInfo;
    sockaddr_in addr;
    
    if (pingBcastSocket != -1 && FD_ISSET(pingBcastSocket, &read_set)) {
      if (serverInfo.ping.read(pingBcastSocket, &addr)) {
	serverInfo.ping.serverId.serverHost = addr.sin_addr;
	serverInfo.cached = false;
	addToListWithLookup(serverInfo);
      }
    }

    // check list servers
    for (i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];
      if (listServer.socket != -1) {
	// read more data from server
	if (FD_ISSET(listServer.socket, &read_set)) {
	  readServerList(i);
	}

	// send list request
	else if (FD_ISSET(listServer.socket, &write_set)) {
#if !defined(_WIN32)
	  // ignore SIGPIPE for this send
	  SIG_PF oldPipeHandler = bzSignal(SIGPIPE, SIG_IGN);
#endif
	  bool errorSending;
	  {
	    char url[1024];
	    snprintf(url, sizeof(url),
		     "GET %s?action=LIST&version=%s HTTP/1.1\r\nHost: %s\r\nCache-control: no-cache\r\n\r\n",
		     listServer.pathname.c_str(), getServerVersion(),
		     listServer.hostname.c_str());
	    errorSending = send(listServer.socket, url, strlen(url), 0)
	      != (int) strlen(url);
	  }
	  if (errorSending) {
	    // probably unable to connect to server
	    close(listServer.socket);
	    listServer.socket = -1;
	    if (!addedCacheToList){
	      addedCacheToList = true;
	      addCacheToList();
	     }
	  }
	  else {
	    listServer.phase = 3;
	    listServer.bufferSize = 0;
	  }
#if !defined(_WIN32)
	  bzSignal(SIGPIPE, oldPipeHandler);
#endif
	}
      }
    }
  }
}

void			ServerMenu::readServerList(int index)
{
  ListServer& listServer = listServers[index];

  // read more data into server list buffer
  int n = recv(listServer.socket, listServer.buffer + listServer.bufferSize,
				sizeof(listServer.buffer) -
					listServer.bufferSize - 1, 0);
  if (n > 0) {
    listServer.bufferSize += n;
    listServer.buffer[listServer.bufferSize] = 0;

    char* base = listServer.buffer;
    while (*base) {
      // find next newline
      char* scan = base;
      while (*scan && *scan != '\n') scan++;

      // if no newline then no more complete replies
      if (*scan != '\n') break;
      *scan++ = '\0';

      // parse server info
      char *scan2, *name, *version, *info, *address, *title;
      name = base;
      version = name;
      while (*version && !isspace(*version))  version++;
      while (*version &&  isspace(*version)) *version++ = 0;
      info = version;
      while (*info && !isspace(*info))  info++;
      while (*info &&  isspace(*info)) *info++ = 0;
      address = info;
      while (*address && !isspace(*address))  address++;
      while (*address &&  isspace(*address)) *address++ = 0;
      title = address;
      while (*title && !isspace(*title))  title++;
      while (*title &&  isspace(*title)) *title++ = 0;

      // extract port number from address
      int port = ServerPort;
      scan2 = strchr(name, ':');
      if (scan2) {
	port = atoi(scan2 + 1);
	*scan2 = 0;
      }

      // check info
      if (strcmp(version, getServerVersion()) == 0 &&
	  (int)strlen(info) == PingPacketHexPackedSize &&
	  port >= 1 && port <= 65535) {
	// store info
	ServerItem serverInfo;
	serverInfo.ping.unpackHex(info);
	int dot[4] = {127,0,0,1};
	if (sscanf(address, "%d.%d.%d.%d", dot+0, dot+1, dot+2, dot+3) == 4) {
	  if (dot[0] >= 0 && dot[0] <= 255 &&
	      dot[1] >= 0 && dot[1] <= 255 &&
	      dot[2] >= 0 && dot[2] <= 255 &&
	      dot[3] >= 0 && dot[3] <= 255) {
	    InAddr addr;
	    unsigned char* paddr = (unsigned char*)&addr.s_addr;
	    paddr[0] = (unsigned char)dot[0];
	    paddr[1] = (unsigned char)dot[1];
	    paddr[2] = (unsigned char)dot[2];
	    paddr[3] = (unsigned char)dot[3];
	    serverInfo.ping.serverId.serverHost = addr;
	  }
	}
	serverInfo.ping.serverId.port = htons((int16_t)port);
	serverInfo.name = name;

	// construct description
	serverInfo.description = serverInfo.name;
	if (port != ServerPort) {
	  char portBuf[20];
	  sprintf(portBuf, "%d", port);
	  serverInfo.description += ":";
	  serverInfo.description += portBuf;
	}
	if (strlen(title) > 0) {
	  serverInfo.description += "; ";
	  serverInfo.description += title;
	}

	serverInfo.cached = false;
	// add to list & add it to the server cache
	addToList(serverInfo,true);
      }

      // next reply
      base = scan;
    }

    // remove parsed replies
    listServer.bufferSize -= (base - listServer.buffer);
    memmove(listServer.buffer, base, listServer.bufferSize);
  }
  else if (n == 0) {
    // server hungup
    close(listServer.socket);
    listServer.socket = -1;
    listServer.phase = 4;
  }
  else if (n < 0) {
    close(listServer.socket);
    listServer.socket = -1;
    listServer.phase = -1;
  }
}

void			ServerMenu::addToListWithLookup(ServerItem& info)
{
  info.name = Address::getHostByAddress(info.ping.serverId.serverHost);

  // tack on port number to description if not default
  info.description = info.name;
  const int port = (int)ntohs((unsigned short)info.ping.serverId.port);
  if (port != ServerPort) {
    char portBuf[20];
    sprintf(portBuf, "%d", port);
    info.description += ":";
    info.description += portBuf;
  }

  addToList(info); // do not cache network lan - etc. servers
}


void			ServerMenu::addToList(ServerItem& info, bool doCache)
{
  // update if we already have it
  int i;

  // search and delete entry for this item if it exists
  // (updating info in place may "unsort" the list)
  for (i = 0; i < (int)servers.size(); i++) {
    ServerItem& server = servers[i];
    if (server.ping.serverId.serverHost.s_addr == info.ping.serverId.serverHost.s_addr
	&& server.ping.serverId.port == info.ping.serverId.port) {
      servers.erase(servers.begin() + i); // erase this item
    }
  }

  // find point to insert new player at
  int insertPoint = -1; // point to insert server into

  // insert new item before the first serveritem with is deemed to be less
  // in value than the item to be inserted -- cached items are less than
  // non-cached, items that have more players are more, etc..
  for (i = 0; i < (int)servers.size(); i++) {
    ServerItem& server = servers[i];
    if (server < info){
      insertPoint = i;
      break;
    }
  }

  if (insertPoint == -1){ // no spot to insert it into -- goes on back
    servers.push_back(info);
  }
  else {  // found a spot to insert it into
    servers.insert(servers.begin() + insertPoint,info);
  }

  // update display
  char buffer [80];
  std::vector<std::string> args;
  sprintf(buffer, "%d", (int)servers.size());
  args.push_back(buffer);
  setStatus("Servers found: {1}", &args);

  // force update
  const int oldSelectedIndex = selectedIndex;
  selectedIndex = -1;
  setSelected(oldSelectedIndex);

  if (doCache) {
    // update server cache if asked for
    // on save we delete at most as many items as we added
    // if the added list is normal, we weed most out, if we
    // get few items, we weed few items out
    serverCache->incAddedNum();

    // make string like "sdfsd.dmsd.com:123"
    char buffer [100];
    std::string serverAddress = info.name;
    sprintf(buffer,":%d",  ntohs((unsigned short) info.ping.serverId.port));
    serverAddress += buffer;
    info.cached = true; // values in cache are "cached"
    // update the last updated time to now
    info.setUpdateTime();

    SRV_STR_MAP::iterator iter;
    iter = serverCache->find(serverAddress);  // erase entry to allow update
    if (iter != serverCache->end()){ // if we find it, update it
      iter->second = info;
    }
    else {
      // insert into cache -- wasn't found
      serverCache->insert(serverAddress,info);
    }
  }
}

// add the entire cache to the server list
void			ServerMenu::addCacheToList()
{
  for (SRV_STR_MAP::iterator iter = serverCache->begin();
       iter != serverCache->end(); iter++){
    addToList(iter->second);
  }
}

void			ServerMenu::playingCB(void* _self)
{
  ((ServerMenu*)_self)->checkEchos();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
