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
#include "JoinMenu.h"

/* common implementation headers */
#include "BzfMedia.h"
#include "FontManager.h"
#include "PlatformFactory.h"
#include "Protocol.h"
#include "ServerList.h"
#include "StartupInfo.h"
#include "TimeKeeper.h"

/* local implementation headers */
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "MenuDefaultKey.h"
#include "ServerMenu.h"
#include "ServerStartMenu.h"


/* from playing.h */
StartupInfo* getStartupInfo();
void joinGame();

JoinMenu* JoinMenu::activeMenu = NULL;


JoinMenu::JoinMenu() : serverStartMenu(NULL), serverMenu(NULL)
{
  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  StartupInfo* info = getStartupInfo();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Join Game");
  list.push_back(label);

  findServer = new HUDuiLabel;
  findServer->setFontFace(fontFace);
  findServer->setString("Find Server");
  list.push_back(findServer);

  connectLabel = new HUDuiLabel;
  connectLabel->setFontFace(fontFace);
  connectLabel->setString("Connect");
  list.push_back(connectLabel);

  callsign = new HUDuiTypeIn;
  callsign->setFontFace(fontFace);
  callsign->setLabel("Callsign:");
  callsign->setMaxLength(CallSignLen - 1);
  callsign->setString(info->callsign);
  list.push_back(callsign);

  password = new HUDuiTypeIn;
  password->setObfuscation(true);
  password->setFontFace(fontFace);
  password->setLabel("Password:");
  password->setMaxLength(CallSignLen - 1);
  password->setString(info->password);
  list.push_back(password);

  team = new HUDuiList;
  team->setFontFace(fontFace);
  team->setLabel("Team:");
  team->setCallback(teamCallback, NULL);
  std::vector<std::string>& teams = team->getList();
  // these do not need to be in enum order, but must match getTeam() & setTeam()
  teams.push_back(std::string(Team::getName(AutomaticTeam)));
  teams.push_back(std::string(Team::getName(RogueTeam)));
  teams.push_back(std::string(Team::getName(RedTeam)));
  teams.push_back(std::string(Team::getName(GreenTeam)));
  teams.push_back(std::string(Team::getName(BlueTeam)));
  teams.push_back(std::string(Team::getName(PurpleTeam)));
  teams.push_back(std::string(Team::getName(ObserverTeam)));
  team->update();
  setTeam(info->team);
  list.push_back(team);

  server = new HUDuiTypeIn;
  server->setFontFace(fontFace);
  server->setLabel("Server:");
  server->setMaxLength(64);
  server->setString(info->serverName);
  list.push_back(server);

  char buffer[10];
  sprintf(buffer, "%d", info->serverPort);
  port = new HUDuiTypeIn;
  port->setFontFace(fontFace);
  port->setLabel("Port:");
  port->setMaxLength(5);
  port->setString(buffer);
  list.push_back(port);

  email = new HUDuiTypeIn;
  email->setFontFace(fontFace);
  email->setLabel("Email:");
  email->setMaxLength(EmailLen - 1);
  email->setString(info->email);
  list.push_back(email);

  startServer = new HUDuiLabel;
  startServer->setFontFace(fontFace);
  startServer->setString("Start Server");
  list.push_back(startServer);

  status = new HUDuiLabel;
  status->setFontFace(fontFace);
  status->setString("");
  list.push_back(status);

  failedMessage = new HUDuiLabel;
  failedMessage->setFontFace(fontFace);
  failedMessage->setString("");
  list.push_back(failedMessage);

  initNavigation(list, 1, list.size() - 3);
}

JoinMenu::~JoinMenu()
{
  delete serverStartMenu;
  delete serverMenu;
}

HUDuiDefaultKey* JoinMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}

void JoinMenu::show()
{
  activeMenu = this;

  StartupInfo* info = getStartupInfo();

  // set fields
  callsign->setString(info->callsign);
  password->setString(info->password);
  setTeam(info->team);

  server->setString(info->serverName);
  char buffer[10];
  sprintf(buffer, "%d", info->serverPort);
  port->setString(buffer);

  // clear status
  setStatus("");
  setFailedMessage("");
}

void JoinMenu::dismiss()
{
  loadInfo();
  activeMenu = NULL;
}

void JoinMenu::loadInfo()
{
  // load startup info with current settings
  StartupInfo* info = getStartupInfo();
  strcpy(info->callsign, callsign->getString().c_str());
  strcpy(info->password, password->getString().c_str());
  info->team = getTeam();
  strcpy(info->serverName, server->getString().c_str());
  info->serverPort = atoi(port->getString().c_str());
  strcpy(info->email, email->getString().c_str());
}

void JoinMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == startServer) {

    if (!serverStartMenu) serverStartMenu = new ServerStartMenu;
    HUDDialogStack::get()->push(serverStartMenu);

  } else if (focus == findServer) {

    if (!serverMenu) serverMenu = new ServerMenu;
    HUDDialogStack::get()->push(serverMenu);

  } else if (focus == connectLabel) {

    // load startup info
    loadInfo();

    // make sure we've got what we need
    StartupInfo* info = getStartupInfo();
    if (info->callsign[0] == '\0') {
      setStatus("You must enter a callsign.");
      return;
    }
    if (info->serverName[0] == '\0') {
      setStatus("You must enter a server.");
      return;
    }
    if (info->serverPort <= 0 || info->serverPort > 65535) {
      char buffer[60];
      sprintf(buffer, "Port is invalid.  Try %d.", ServerPort);
      setStatus(buffer);
      return;
    }

    // get token if we need to (have a password but no token)
    if ((info->token[0] == '\0') && (info->password[0] != '\0')) {
      BzfMedia* media = PlatformFactory::getMedia();
      ServerList* serverList = new ServerList;
      serverList->startServerPings(info);
      // wait no more than 10 seconds for a token
      for (int i = 0; i < 40; i++) {
	serverList->checkEchos(getStartupInfo());
	if (info->token[0] != '\0') break;
	media->sleep(0.25f);
      }
      delete serverList;
    }

    // let user know we're trying
    setStatus("Trying...");

    // don't let the bad token specifier slip through to the server, just erase it
    if (strcmp(info->token, "badtoken") == 0)
      info->token[0] = '\0';

    // schedule attempt to join game
    joinGame();
  }
}

void JoinMenu::setFailedMessage(const char* msg)
{
  failedMessage->setString(msg);

  FontManager &fm = FontManager::instance();
  const float width = fm.getStrLength(MainMenu::getFontFace(),
	failedMessage->getFontSize(), failedMessage->getString());
  failedMessage->setPosition(center - 0.5f * width, failedMessage->getY());
}

TeamColor JoinMenu::getTeam() const
{
  return team->getIndex() == 0 ? AutomaticTeam : TeamColor(team->getIndex() - 1);
}

void JoinMenu::setTeam(TeamColor teamcol)
{
  team->setIndex(teamcol == AutomaticTeam ? 0 : int(teamcol) + 1);
}

void JoinMenu::setStatus(const char* msg, const std::vector<std::string> *)
{
  status->setString(msg);
  FontManager &fm = FontManager::instance();
  const float width = fm.getStrLength(status->getFontFace(),
		status->getFontSize(), status->getString());
  status->setPosition(center - 0.5f * width, status->getY());
}

void JoinMenu::teamCallback(HUDuiControl*, void*)
{
  // do nothing (for now)
}

void JoinMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)height / 15.0f;
  const float fontSize = (float)height / 36.0f;
  center = 0.5f * (float)width;

  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, "");
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width - 0.5f * titleWidth);
  y -= 0.6f * titleHeight;
  list[1]->setFontSize(fontSize);
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, "");
  const int count = list.size();
  for (int i = 1; i < count; i++) {
    list[i]->setFontSize(fontSize);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
    if (i <= 2 || i == 8) y -= 0.5f * h;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
