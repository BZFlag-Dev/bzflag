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

// DUD INFORMATION FOR TESTING
HUDuiServerListItem::HUDuiServerListItem() : HUDuiControl()
{
  domainName = "Test";
  displayDomain = domainName;
  serverName = "THE SHIT SERVEr";
  displayServer = serverName;
  playerCount = "1000";
  displayPlayer = playerCount;
  serverPing = "200";
  displayPing = serverPing;
}

HUDuiServerListItem::HUDuiServerListItem(ServerItem item) : HUDuiControl()
{
  char temp[50];

  sprintf(temp, "%ld", item.ping.pingTime);
  std::string ping = temp;

  std::string players = "";

  sprintf(temp, "%ld", item.getPlayerCount());
  players.append(temp);
  players.append("/");

  sprintf(temp, "%ld", item.ping.maxPlayers);
  players.append(temp);

  std::string addr = stripAnsiCodes(item.description.c_str());
  std::string desc;
  std::string::size_type pos = addr.find_first_of(';');
  if (pos != std::string::npos) {
    desc = addr.substr(pos > 0 ? pos+1 : pos);
    addr.resize(pos);
  }

  domainName = addr;
  displayDomain = domainName;

  serverName = desc;
  displayServer = serverName;

  playerCount = players;
  displayPlayer = playerCount;

  serverPing = ping;
  displayPing = serverPing;
}

HUDuiServerListItem::~HUDuiServerListItem()
{
  // do nothing
}

// Set the scrollable list item's position on the screen
void HUDuiServerListItem::setPosition(float x, float y)
{
  HUDuiControl::setPosition(x, y);

  float _x = x;
  domainX = _x;
  _x = _x + (DOMAIN_PERCENTAGE*getWidth());
  serverX = _x;
  _x = _x + (SERVER_PERCENTAGE*getWidth());
  playerX = _x;
  _x = _x + (PLAYER_PERCENTAGE*getWidth());
  pingX = _x;
}

void HUDuiServerListItem::setSize(float width, float height)
{
  HUDuiControl::setSize(width, height);

  resize();
  setPosition(getX(), getY());
}

void HUDuiServerListItem::resize()
{
  if (getFontFace() == NULL)
    return;

  displayDomain = shorten(domainName, (DOMAIN_PERCENTAGE*getWidth()));
  displayServer = shorten(serverName, (SERVER_PERCENTAGE*getWidth()));
  displayPlayer = shorten(playerCount, (PLAYER_PERCENTAGE*getWidth()));
  displayPing = shorten(serverPing, (PING_PERCENTAGE*getWidth()));
}

std::string HUDuiServerListItem::shorten(std::string string, float width)
{
  // Trim string to fit our available space
  FontManager &fm = FontManager::instance();

  // Skip if it already fits
  if (fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), string.c_str()) <= width)
    return string;

  // Iterate through each character. Expensive.
  for (int i=0; i<=(int)string.size(); i++)
  {
    float temp = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), string.substr(0, i).c_str());
    // Is it too big yet?
    if (fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), string.substr(0, i).c_str()) > width)
    {
      return string.substr(0, i - 1);
    }
  }

  return "";
}

// Returns the domain name of the server list item
std::string HUDuiServerListItem::getDomainName()
{
  return domainName;
}

// Returns the server name of the server list item
std::string HUDuiServerListItem::getServerName()
{
  return serverName;
}

// Returns the player count of the server list item
std::string HUDuiServerListItem::getPlayerCount()
{
  return playerCount;
}

// Returns the server ping of the server list item
std::string HUDuiServerListItem::getServerPing()
{
  return serverPing;
}

// Render the scrollable list item
void HUDuiServerListItem::doRender()
{
  if (getFontFace() < 0) {
    return;
  }

  FontManager &fm = FontManager::instance();
  float darkness;
  if (hasFocus()) {
    darkness = 1.0f;
  } else {
    darkness = 0.7f;
  }
  fm.setDarkness(darkness);
  fm.drawString(domainX, getY(), 0,
		getFontFace()->getFMFace(), getFontSize(),
		displayDomain.c_str());
  fm.drawString(serverX, getY(), 0,
		getFontFace()->getFMFace(), getFontSize(),
		displayServer.c_str());
  fm.drawString(playerX, getY(), 0,
		getFontFace()->getFMFace(), getFontSize(),
		displayPlayer.c_str());
  fm.drawString(pingX, getY(), 0,
		getFontFace()->getFMFace(), getFontSize(),
		displayPing.c_str());
  fm.setDarkness(1.0f);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8