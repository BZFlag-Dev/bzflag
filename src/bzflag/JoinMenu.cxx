/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
#include "FontSizer.h"
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "ServerMenu.h"
#include "ServerStartMenu.h"
#include "TextureManager.h"
#include "playing.h"

JoinMenu* JoinMenu::activeMenu = NULL;


JoinMenu::JoinMenu() : serverStartMenu(NULL), serverMenu(NULL)
{
  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // add controls
  StartupInfo* info = getStartupInfo();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Join Game");
  addControl(label, false);

  findServer = new HUDuiLabel;
  findServer->setFontFace(fontFace);
  findServer->setString("Find Server");
  addControl(findServer);

  connectLabel = new HUDuiLabel;
  connectLabel->setFontFace(fontFace);
  connectLabel->setString("Connect");
  addControl(connectLabel);

  callsign = new HUDuiTypeIn;
  callsign->setFontFace(fontFace);
  callsign->setLabel("Callsign:");
  callsign->setMaxLength(CallSignLen - 1);
  callsign->setString(info->callsign);
  addControl(callsign);

  password = new HUDuiTypeIn;
  password->setObfuscation(true);
  password->setFontFace(fontFace);
  password->setLabel("Password:");
  password->setMaxLength(CallSignLen - 1);
  password->setString(info->password);
  addControl(password);

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
  addControl(team);

  server = new HUDuiTypeIn;
  server->setFontFace(fontFace);
  server->setLabel("Server:");
  server->setMaxLength(64);
  server->setString(info->serverName);
  addControl(server);

  char buffer[10];
  sprintf(buffer, "%d", info->serverPort);
  port = new HUDuiTypeIn;
  port->setFontFace(fontFace);
  port->setLabel("Port:");
  port->setMaxLength(5);
  port->setString(buffer);
  addControl(port);

  email = new HUDuiTypeIn;
  email->setFontFace(fontFace);
  email->setLabel("Email:");
  email->setMaxLength(EmailLen - 1);
  email->setString(info->email);
  addControl(email);

  startServer = new HUDuiLabel;
  startServer->setFontFace(fontFace);
  startServer->setString("Start Server");
  addControl(startServer);

  status = new HUDuiLabel;
  status->setFontFace(fontFace);
  status->setString("");
  addControl(status, false);

  failedMessage = new HUDuiLabel;
  failedMessage->setFontFace(fontFace);
  failedMessage->setString("");
  addControl(failedMessage, false);

  teamIcon = new HUDuiImage;
  updateTeamTexture();
  addControl(teamIcon);

  initNavigation();
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
  HUDuiControl* _focus = getNav().get();
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
  const float _width = fm.getStringWidth(MainMenu::getFontFace(),
	failedMessage->getFontSize(), failedMessage->getString().c_str());
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
  const float _width = fm.getStringWidth(status->getFontFace(),
		status->getFontSize(), status->getString().c_str());
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
  const float iconSize = team->getFontSize() * 1.5f;
  teamIcon->setSize(iconSize, iconSize);

  // put it at the end of the text
  Bundle *bdl = BundleMgr::getCurrentBundle();
  const float x = team->getX() + fm.getStringWidth(team->getFontFace(),
	  team->getFontSize(),
	  std::string(bdl->getLocalString(team->getList()[team->getIndex()]) + "x").c_str());
  teamIcon->setPosition(x, team->getY());
}

void JoinMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(MainMenu::getFontFace(), "headerFontSize");

  fs.setMin(0, 20);
  const float fontSize = fs.getFontSize(MainMenu::getFontFace(), "menuFontSize");

  center = 0.5f * (float)_width;

  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(MainMenu::getFontFace(), titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(MainMenu::getFontFace(), titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width - 0.5f * titleWidth);
  y -= 1.0f * titleHeight;
  listHUD[1]->setFontSize(fontSize);
  const float h = fm.getStringHeight(MainMenu::getFontFace(), fontSize);
  const int count = (const int)listHUD.size();
  for (int i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
    if (i <= 2 || i == 8) y -= 0.5f * h;
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
