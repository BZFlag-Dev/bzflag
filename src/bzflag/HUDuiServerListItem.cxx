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
  domainName = new HUDuiScrollListItem("");
  domainName->setFontFace(getFontFace());

  serverName = new HUDuiScrollListItem("");
  serverName->setFontFace(getFontFace());

  playerCount = new HUDuiScrollListItem("");
  playerCount->setFontFace(getFontFace());

  serverPing = new HUDuiScrollListItem("");
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
  
  domainName = new HUDuiScrollListItem("Domain");
  domainName->setFontFace(getFontFace());

  serverName = new HUDuiScrollListItem("Server");
  serverName->setFontFace(getFontFace());

  playerCount = new HUDuiScrollListItem("99");
  playerCount->setFontFace(getFontFace());
  
  //serverPing = new HUDuiScrollListItem(item.ping.pingTime);
  serverPing = new HUDuiScrollListItem("200");
  serverPing->setFontFace(getFontFace());
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
// NEEDS WORK
void HUDuiServerListItem::setPosition(float x, float y)
{
  HUDuiControl::setPosition(x, y);

  domainName->setPosition(x, y);
  serverName->setPosition((x + (DOMAIN_PERCENTAGE * getWidth())), y);
  playerCount->setPosition((x + (SERVER_PERCENTAGE * getWidth())), y);
  serverPing->setPosition((x+ (PLAYER_PERCENTAGE * getWidth())), y);
}

void HUDuiServerListItem::setSize(float width, float height)
{
  HUDuiControl::setSize(width, height);
  shorten(getWidth());
}

// Returns the domain name of the server list item
std::string HUDuiServerListItem::getDomainName()
{
  return domainName->getValue();
}

// Returns the server name of the server list item
std::string HUDuiServerListItem::getServerName()
{
  return serverName->getValue();
}

// Returns the player count of the server list item
std::string HUDuiServerListItem::getPlayerCount()
{
  return playerCount->getValue();
}

// Returns the server ping of the server list item
std::string HUDuiServerListItem::getServerPing()
{
  return serverPing->getValue();
}

// Shorten the item's label to fit
// MUST BE RE-DONE
void HUDuiServerListItem::shorten(float width)
{
  float listWidth = getWidth();
  
  domainName->shorten(listWidth * DOMAIN_PERCENTAGE);
  serverName->shorten(listWidth * SERVER_PERCENTAGE);
  playerCount->shorten(listWidth * PLAYER_PERCENTAGE);
  serverPing->shorten(listWidth * PING_PERCENTAGE);
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