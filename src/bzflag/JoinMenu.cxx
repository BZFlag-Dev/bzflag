/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include "JoinMenu.h"

/* common implementation headers */
#include "FontManager.h"
#include "Protocol.h"
#include "BundleMgr.h"
#include "Bundle.h"

/* local implementation headers */
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "ServerMenu.h"
#include "ServerStartMenu.h"
#include "TextureManager.h"
#include "playing.h"
#include "HUDui.h"

JoinMenu* JoinMenu::activeMenu = NULL;


JoinMenu::JoinMenu() : serverStartMenu(NULL), serverMenu(NULL)
{
  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  StartupInfo* info = getStartupInfo();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Join Game");
  listHUD.push_back(label);

  findServer = new HUDuiLabel;
  findServer->setFontFace(fontFace);
  findServer->setString("Find Server");
  listHUD.push_back(findServer);

  connectLabel = new HUDuiLabel;
  connectLabel->setFontFace(fontFace);
  connectLabel->setString("Connect");
  listHUD.push_back(connectLabel);

  callsign = new HUDuiTypeIn;
  callsign->setFontFace(fontFace);
  callsign->setLabel("Callsign:");
  callsign->setMaxLength(CallSignLen - 1);
  callsign->setString(info->callsign);
  listHUD.push_back(callsign);

  password = new HUDuiTypeIn;
  password->setObfuscation(true);
  password->setFontFace(fontFace);
  password->setLabel("Password:");
  password->setMaxLength(CallSignLen - 1);
  password->setString(info->password);
  listHUD.push_back(password);

  team = new HUDuiList;
  team->setFontFace(fontFace);
  team->setLabel("Team:");
  team->setCallback(teamCallback, this);
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
  listHUD.push_back(team);

  teamIcon = new HUDuiTextureLabel;
  teamIcon->setFontFace(fontFace);
  teamIcon->setString(" ");
  updateTeamTexture();
  listHUD.push_back(teamIcon);

  server = new HUDuiTypeIn;
  server->setFontFace(fontFace);
  server->setLabel("Server:");
  server->setMaxLength(64);
  server->setString(info->serverName);
  listHUD.push_back(server);

  char buffer[10];
  sprintf(buffer, "%d", info->serverPort);
  port = new HUDuiTypeIn;
  port->setFontFace(fontFace);
  port->setLabel("Port:");
  port->setMaxLength(5);
  port->setString(buffer);
  listHUD.push_back(port);

  email = new HUDuiTypeIn;
  email->setFontFace(fontFace);
  email->setLabel("Email:");
  email->setMaxLength(EmailLen - 1);
  email->setString(info->email);
  listHUD.push_back(email);

  startServer = new HUDuiLabel;
  startServer->setFontFace(fontFace);
  startServer->setString("Start Server");
  listHUD.push_back(startServer);

  status = new HUDuiLabel;
  status->setFontFace(fontFace);
  status->setString("");
  listHUD.push_back(status);

  failedMessage = new HUDuiLabel;
  failedMessage->setFontFace(fontFace);
  failedMessage->setString("");
  listHUD.push_back(failedMessage);

  initNavigation(listHUD, 1, listHUD.size() - 3);

  // cut teamIcon out of the nav loop
  team->setNext(server);
  server->setPrev(team);
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
  if (strcmp(info->callsign, callsign->getString().c_str())) {
    strcpy(info->callsign, callsign->getString().c_str());
    info->token[0] = '\0';
  }
  if (strcmp(info->password, password->getString().c_str())) {
    strcpy(info->password, password->getString().c_str());
    info->token[0] = '\0';
  }
  info->team = getTeam();
  strcpy(info->serverName, server->getString().c_str());
  info->serverPort = atoi(port->getString().c_str());
  strcpy(info->email, email->getString().c_str());
}

void JoinMenu::execute()
{
  HUDuiControl* _focus = HUDui::getFocus();
  if (_focus == startServer) {

    if (!serverStartMenu) serverStartMenu = new ServerStartMenu;
    HUDDialogStack::get()->push(serverStartMenu);

  } else if (_focus == findServer) {

    if (!serverMenu) serverMenu = new ServerMenu;
    HUDDialogStack::get()->push(serverMenu);

  } else if (_focus == connectLabel) {

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

    // let user know we're trying
    setStatus("Trying...");
		HUDDialogStack::get()->setFailedMessage("");

    // schedule attempt to join game
    joinGame();
  }
}

void JoinMenu::setFailedMessage(const char* msg)
{
  failedMessage->setString(msg);

  FontManager &fm = FontManager::instance();
  const float _width = fm.getStrLength(MainMenu::getFontFace(),
	failedMessage->getFontSize(), failedMessage->getString());
  failedMessage->setPosition(center - 0.5f * _width, failedMessage->getY());
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
  const float _width = fm.getStrLength(status->getFontFace(),
		status->getFontSize(), status->getString());
  status->setPosition(center - 0.5f * _width, status->getY());
}

void JoinMenu::teamCallback(HUDuiControl*, void* source)
{
  ((JoinMenu*)source)->updateTeamTexture();
}

void JoinMenu::updateTeamTexture()
{
  TextureManager &tm = TextureManager::instance();
  FontManager &fm = FontManager::instance();

  // load the appropriate texture
  std::string texture;
  if (getTeam() == AutomaticTeam)
    texture = "automatic_";
  else
    texture = Team::getImagePrefix(getTeam());
  texture += "icon";
  int id = tm.getTextureID(texture.c_str());
  teamIcon->setTexture(id);

  // make it big enough
  teamIcon->setFontSize(team->getFontSize() * 1.5f);

  // put it at the end of the text
  Bundle *bdl = BundleMgr::getCurrentBundle();
  const float x = team->getX() + fm.getStrLength(team->getFontFace(),
	  team->getFontSize(),
	  bdl->getLocalString(team->getList()[team->getIndex()]) + "x");
  teamIcon->setPosition(x, team->getY());
}

void JoinMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  const float fontSize = (float)_height / 36.0f;
  center = 0.5f * (float)_width;

  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, "");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width - 0.5f * titleWidth);
  y -= 0.6f * titleHeight;
  listHUD[1]->setFontSize(fontSize);
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, "");
  const int count = listHUD.size();
  for (int i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    if (i != 5)
      y -= 1.0f * h;
    if (i <= 2 || i == 9) y -= 0.5f * h;
  }

  updateTeamTexture();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
