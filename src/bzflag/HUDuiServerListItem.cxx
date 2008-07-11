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

// interface headers
#include "HUDuiServerListItem.h"

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"
#include "LocalFontFace.h"
#include "HUDui.h"
#include "bzUnicode.h"

//
// HUDuiServerListItem
//

// Percentages of the server list item label dedicated to each column
const float HUDuiServerListItem::DOMAIN_PERCENTAGE = 0.375;
const float HUDuiServerListItem::SERVER_PERCENTAGE = 0.375;
const float HUDuiServerListItem::PLAYER_PERCENTAGE = 0.125;
const float HUDuiServerListItem::PING_PERCENTAGE = 0.125;

HUDuiServerListItem::HUDuiServerListItem() : HUDuiControl()
{
  domainName = new HUDuiLabel;
  domainName->setString("Test");
  domainName->setFontFace(getFontFace());

  serverName = new HUDuiLabel;
  serverName->setString("THE SHIT SERVEr");
  serverName->setFontFace(getFontFace());

  playerCount = new HUDuiLabel;
  playerCount->setString("1000");
  playerCount->setFontFace(getFontFace());

  serverPing = new HUDuiLabel;
  serverPing->setString("200");
  serverPing->setFontFace(getFontFace());
}

// FILLED IN WITH ALL DUD INFORMATION AT THE MOMENT
HUDuiServerListItem::HUDuiServerListItem(ServerItem item) : HUDuiControl()
{
  std::vector<std::string> args;
  char msg[50];

  sprintf(msg, "%ld", item.getPlayerCount());
  args.push_back(msg);
  sprintf(msg, "%ld", item.ping.maxPlayers);
  args.push_back(msg);

  domainName = new HUDuiLabel;
  domainName->setString("Domain");
  domainName->setFontFace(getFontFace());

  serverName = new HUDuiLabel;
  serverName->setString("Server");
  serverName->setFontFace(getFontFace());

  playerCount = new HUDuiLabel;
  playerCount->setString("99");
  playerCount->setFontFace(getFontFace());

  serverPing = new HUDuiLabel;
  serverPing->setString("200");
  serverPing->setFontFace(getFontFace());
  
  //serverPing = new HUDuiScrollListItem(item.ping.pingTime);
}

HUDuiServerListItem::~HUDuiServerListItem()
{
  // do nothing
}

// Set the font size for the scrollable list item
void HUDuiServerListItem::setFontSize(float size)
{
  HUDuiControl::setFontSize(size);

  domainName->setFontSize(size);
  serverName->setFontSize(size);
  playerCount->setFontSize(size);
  serverPing->setFontSize(size);
}

// Set the font face for the scrollable list item
void HUDuiServerListItem::setFontFace(const LocalFontFace* fontFace)
{
  HUDuiControl::setFontFace(fontFace);

  domainName->setFontFace(fontFace);
  serverName->setFontFace(fontFace);
  playerCount->setFontFace(fontFace);
  serverPing->setFontFace(fontFace);
}

// Set the scrollable list item's position on the screen
void HUDuiServerListItem::setPosition(float x, float y)
{
  HUDuiControl::setPosition(x, y);

  float _x = x;
  domainName->setPosition(_x, y);
  _x = _x + domainName->getWidth();
  serverName->setPosition(_x, y);
  _x = _x + serverName->getWidth();
  playerCount->setPosition(_x, y);
  _x = _x + playerCount->getWidth();
  serverPing->setPosition(_x, y);
}

void HUDuiServerListItem::setSize(float width, float height)
{
  HUDuiControl::setSize(width, height);
  
  domainName->setSize((DOMAIN_PERCENTAGE * width), height);
  serverName->setSize((SERVER_PERCENTAGE * width), height);
  playerCount->setSize((PLAYER_PERCENTAGE * width), height);
  serverPing->setSize((PING_PERCENTAGE * width), height);
}

// Returns the domain name of the server list item
std::string HUDuiServerListItem::getDomainName()
{
  return domainName->getString();
}

// Returns the server name of the server list item
std::string HUDuiServerListItem::getServerName()
{
  return serverName->getString();
}

// Returns the player count of the server list item
std::string HUDuiServerListItem::getPlayerCount()
{
  return playerCount->getString();
}

// Returns the server ping of the server list item
std::string HUDuiServerListItem::getServerPing()
{
  return serverPing->getString();
}

// Render the scrollable list item
void HUDuiServerListItem::doRender()
{
  domainName->render();
  serverName->render();
  playerCount->render();
  serverPing->render();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8