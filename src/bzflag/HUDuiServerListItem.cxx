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
#include "LocalFontFace.h"
#include "bzUnicode.h"

//
// HUDuiServerListItem
//

HUDuiServerListItem::HUDuiServerListItem() : HUDuiControl(), fm(FontManager::instance())
{
  // Do nothing
}

HUDuiServerListItem::HUDuiServerListItem(ServerItem item) : HUDuiControl(), domain_percentage(0.0f), server_percentage(0.0f), player_percentage(0.0f), ping_percentage(0.0f), fm(FontManager::instance())
{
  serverKey = item.description;
  char temp[50];

  sprintf(temp, "%d", item.ping.pingTime);
  std::string ping = temp;

  sprintf(temp, "%d/%d", item.getPlayerCount(), item.ping.maxPlayers);
  std::string players = temp;

  std::string addr = stripAnsiCodes(item.description.c_str());
  std::string desc;
  std::string::size_type pos = addr.find_first_of(';');
  if (pos != std::string::npos) {
    desc = addr.substr(pos > 0 ? pos+1 : pos);
    addr.resize(pos);
  }

  displayDomain = domainName = addr;

  displayServer = serverName = desc;

  displayPlayer = playerCount = players;

  displayPing = serverPing = ping;
}

HUDuiServerListItem::~HUDuiServerListItem()
{
  // do nothing
}

std::string HUDuiServerListItem::getServerKey()
{
  return serverKey;
}

void HUDuiServerListItem::setSize(float width, float height)
{
  HUDuiControl::setSize(width, height);

  resize();
}

void HUDuiServerListItem::setFontSize(float size)
{
  HUDuiControl::setFontSize(size);

  resize();
}

void HUDuiServerListItem::setFontFace(const LocalFontFace *face)
{
  HUDuiControl::setFontFace(face);

  resize();
}

void HUDuiServerListItem::setColumnSizes(float domain, float server, float player, float ping)
{
  domain_percentage = domain;
  server_percentage = server;
  player_percentage = player;
  ping_percentage = ping;

  resize();
}

void HUDuiServerListItem::resize()
{
  if (getFontFace() == NULL)
    return;

  spacerWidth = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), "I");

  displayDomain = shorten(domainName, (domain_percentage*getWidth())-2*spacerWidth);
  displayServer = shorten(serverName, (server_percentage*getWidth())-2*spacerWidth);
  displayPlayer = shorten(playerCount, (player_percentage*getWidth())-2*spacerWidth);
  displayPing = shorten(serverPing, (ping_percentage*getWidth())-2*spacerWidth);
}

std::string HUDuiServerListItem::shorten(std::string string, float width)
{
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

  float darkness;
  if (hasFocus()) {
    darkness = 1.0f;
  } else {
    darkness = 0.7f;
  }
  
  float domainX = getX() + spacerWidth;
  float serverX = getX() + domain_percentage*getWidth() + spacerWidth;
  float playerX = getX() + domain_percentage*getWidth() + server_percentage*getWidth() + spacerWidth;
  float pingX = getX() + domain_percentage*getWidth() + server_percentage*getWidth() + player_percentage*getWidth() + spacerWidth;

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
