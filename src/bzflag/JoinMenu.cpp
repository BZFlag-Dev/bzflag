/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "AnsiCodes.h"
#include "Bundle.h"
#include "BundleMgr.h"
#include "FontManager.h"
#include "Protocol.h"

/* local implementation headers */
#include "FontSizer.h"
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "ServerMenu.h"
#include "TextureManager.h"
#include "playing.h"
#include "LocalFontFace.h"


JoinMenu* JoinMenu::instance = NULL;


JoinMenu::JoinMenu() : serverMenu(NULL) {
  instance = this;

  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

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
  if (!LuaClientScripts::GetDevMode()) {
    callsign->setString(info->callsign);
  }
  else {
    callsign->setString(ANSI_STR_FG_BLACK "devmode");
    callsign->setEditing(false);
  }
  addControl(callsign);

  password = new HUDuiTypeIn;
  password->setObfuscation(true);
  password->setFontFace(fontFace);
  password->setLabel("Password:");
  password->setMaxLength(PasswordLen - 1);
  password->setString(info->password);
  addControl(password);

  motto = new HUDuiTypeIn;
  motto->setFontFace(fontFace);
  motto->setMaxLength(64);
  motto->setLabel("Motto:");
  motto->setString(info->motto);
  addControl(motto);

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
  if (!LuaClientScripts::GetDevMode()) {
    server->setString(info->serverName);
  }
  else {
    server->setString(ANSI_STR_FG_BLACK "127.0.0.1 (devmode)");
    server->setEditing(false);
  }
  addControl(server);

  char buffer[10];
  sprintf(buffer, "%d", info->serverPort);
  port = new HUDuiTypeIn;
  port->setFontFace(fontFace);
  port->setLabel("Port:");
  port->setMaxLength(5);
  port->setString(buffer);
  addControl(port);

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

JoinMenu::~JoinMenu() {
  instance = NULL;
  delete serverMenu;
}

HUDuiDefaultKey* JoinMenu::getDefaultKey() {
  return MenuDefaultKey::getInstance();
}

void JoinMenu::show() {
  StartupInfo* info = getStartupInfo();

  // set fields
  if (!LuaClientScripts::GetDevMode()) {
    callsign->setString(info->callsign);
  }
  else {
    callsign->setString(ANSI_STR_FG_BLACK "devmode");
  }

  password->setString(info->password);
  setTeam(info->team);

  if (!LuaClientScripts::GetDevMode()) {
    server->setString(info->serverName);
  }
  else {
    server->setString(ANSI_STR_FG_BLACK "127.0.0.1 (devmode)");
  }
  char buffer[10];
  sprintf(buffer, "%d", info->serverPort);
  port->setString(buffer);

  // clear status
  setStatus("");
  setFailedMessage("");
}

void JoinMenu::dismiss() {
  loadInfo();
}

void JoinMenu::loadInfo() {
  // load startup info with current settings
  StartupInfo* info = getStartupInfo();
  if (!LuaClientScripts::GetDevMode()) {
    if (strcmp(info->callsign, callsign->getString().c_str())) {
      strncpy(info->callsign, callsign->getString().c_str(), CallSignLen - 1);
      info->token[0] = '\0';
    }
  }
  if (strcmp(info->password, password->getString().c_str())) {
    strncpy(info->password, password->getString().c_str(), PasswordLen - 1);
    info->token[0] = '\0';
  }

  if (info->motto != motto->getString()) {
    info->motto = motto->getString();
  }

  info->team = getTeam();
  if (!LuaClientScripts::GetDevMode()) {
    strncpy(info->serverName, server->getString().c_str(), ServerNameLen - 1);
  }
  else {
    strcpy(info->serverName, "127.0.0.1");
  }
  info->serverPort = atoi(port->getString().c_str());
}

void JoinMenu::execute() {
  HUDuiControl* _focus = getNav().get();
  if (_focus == findServer) {

    if (!serverMenu) { serverMenu = new ServerMenu; }
    HUDDialogStack::get()->push(serverMenu);

  }
  else if (_focus == connectLabel) {

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

void JoinMenu::setFailedMessage(const char* msg) {
  failedMessage->setString(msg);

  FontManager& fm = FontManager::instance();
  const float _width = fm.getStringWidth(MainMenu::getFontFace()->getFMFace(),
                                         failedMessage->getFontSize(), failedMessage->getString());
  failedMessage->setPosition(center - 0.5f * _width, failedMessage->getY());
}

TeamColor JoinMenu::getTeam() const {
  return team->getIndex() == 0 ? AutomaticTeam : TeamColor(team->getIndex() - 1);
}

void JoinMenu::setTeam(TeamColor teamcol) {
  team->setIndex(teamcol == AutomaticTeam ? 0 : int(teamcol) + 1);
}

void JoinMenu::setStatus(const char* msg, const std::vector<std::string> *) {
  status->setString(msg);
  FontManager& fm = FontManager::instance();
  const float _width = fm.getStringWidth(status->getFontFace()->getFMFace(),
                                         status->getFontSize(), status->getString());
  status->setPosition(center - 0.5f * _width, status->getY());
}

void JoinMenu::teamCallback(HUDuiControl*, void* source) {
  ((JoinMenu*)source)->updateTeamTexture();
}

void JoinMenu::updateTeamTexture() {
  TextureManager& tm = TextureManager::instance();
  FontManager& fm = FontManager::instance();

  // load the appropriate texture
  std::string texture;
  if (getTeam() == AutomaticTeam) {
    texture = "automatic_";
  }
  else {
    texture = Team::getImagePrefix(getTeam());
  }
  texture += "icon";
  int id = tm.getTextureID(texture);
  teamIcon->setTexture(id);

  // leave the icon the same size as the text so it doesn't overlap other lines
  const float iconSize = team->getFontSize();
  teamIcon->setSize(iconSize, iconSize);

  // put it at the end of the text
  Bundle* bdl = BundleMgr::getCurrentBundle();
  const float x = team->getX() + fm.getStringWidth(team->getFontFace()->getFMFace(),
                                                   team->getFontSize(),
                                                   std::string(bdl->getLocalString(team->getList()[team->getIndex()]) + "x"));
  teamIcon->setPosition(x, team->getY());
}

void JoinMenu::resize(int _width, int _height) {
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(MainMenu::getFontFace()->getFMFace(), "headerFontSize");

  fs.setMin(0, 20);
  const float fontSize = fs.getFontSize(MainMenu::getFontFace()->getFMFace(), "menuFontSize");

  center = 0.5f * (float)_width;

  FontManager& fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(MainMenu::getFontFace()->getFMFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStringHeight(MainMenu::getFontFace()->getFMFace(), titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width - 0.5f * titleWidth);
  y -= 1.0f * titleHeight;
  listHUD[1]->setFontSize(fontSize);
  const float h = fm.getStringHeight(MainMenu::getFontFace()->getFMFace(), fontSize);
  const int count = (const int)listHUD.size();
  for (int i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
    if (i <= 2 || i == 8) { y -= 0.5f * h; }
  }

  updateTeamTexture();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
