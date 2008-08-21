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
#include "Protocol.h"

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "LocalFontFace.h"
#include "bzUnicode.h"

//
// HUDuiServerListItem
//

HUDuiServerListItem::HUDuiServerListItem() : HUDuiControl(), serverList(ServerList::instance()), fm(FontManager::instance())
{
  // Do nothing
}

HUDuiServerListItem::HUDuiServerListItem(ServerItem* item) : HUDuiControl(), serverList(ServerList::instance()), domain_percentage(0.0f), server_percentage(0.0f), player_percentage(0.0f), ping_percentage(0.0f), fm(FontManager::instance())
{
  if (item == NULL)
    return;

  serverKey = item->getServerKey();

  displayDomain = domainName = calculateDomainName();
  displayServer = serverName = calculateServerName();
  displayPlayer = playerCount = calculatePlayers();
  displayPing = serverPing = calculatePing();
}

HUDuiServerListItem::~HUDuiServerListItem()
{
  // do nothing
}

std::string HUDuiServerListItem::calculateDomainName()
{
  ServerItem* server = serverList.lookupServer(serverKey);

  if (server == NULL)
    return "";

  std::string addr = stripAnsiCodes(server->description.c_str());
  std::string desc;
  std::string::size_type pos = addr.find_first_of(';');
  if (pos != std::string::npos) {
    desc = addr.substr(pos > 0 ? pos+1 : pos);
    addr.resize(pos);
  }
  return addr;
}

std::string HUDuiServerListItem::calculateServerName()
{
  ServerItem* server = serverList.lookupServer(serverKey);

  if (server == NULL)
    return "";

  std::string addr = stripAnsiCodes(server->description.c_str());
  std::string desc;
  std::string::size_type pos = addr.find_first_of(';');
  if (pos != std::string::npos) {
    desc = addr.substr(pos > 0 ? pos+1 : pos);
    addr.resize(pos);
  }
  return desc;
}

std::string HUDuiServerListItem::calculatePing()
{
  ServerItem* server = serverList.lookupServer(serverKey);

  if (server == NULL)
    return "";

  char ping[5];
  sprintf(ping, "%d", server->ping.pingTime);
  return ping;
}

std::string HUDuiServerListItem::calculatePlayers()
{
  ServerItem* server = serverList.lookupServer(serverKey);

  if (server == NULL)
    return "";

  char players[20];
  sprintf(players, "%d/%d", server->getPlayerCount(), server->ping.maxPlayers);
  return players;
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

// Render the scrollable list item
void HUDuiServerListItem::doRender()
{
  if (getFontFace() < 0) {
    return;
  }

  ServerItem* server = serverList.lookupServer(serverKey);

  if (server == NULL)
    return;

  float darkness = 0.7f;

  if (hasFocus())
    darkness = 1.0f;

  if ((domainName.compare(calculateDomainName()) == 0)||(serverName.compare(calculateServerName()) == 0)||
      (playerCount.compare(calculatePlayers()) == 0)||(serverPing.compare(calculatePing()) == 0))
  {
    displayDomain = domainName = calculateDomainName();
    displayServer = serverName = calculateServerName();
    displayPlayer = playerCount = calculatePlayers();
    displayPing = serverPing = calculatePing();
    resize();
  }
  
  float domainX = getX() + spacerWidth;
  float serverX = getX() + domain_percentage*getWidth() + spacerWidth;
  float playerX = getX() + domain_percentage*getWidth() + server_percentage*getWidth() + spacerWidth;
  float pingX = getX() + domain_percentage*getWidth() + server_percentage*getWidth() + player_percentage*getWidth() + spacerWidth;

  int face = getFontFace()->getFMFace();

  fm.setDarkness(darkness);
  fm.drawString(domainX, getY(), 0, face, getFontSize(), displayDomain.c_str());
  fm.drawString(serverX, getY(), 0, face, getFontSize(), displayServer.c_str());
  fm.drawString(playerX, getY(), 0, face, getFontSize(), displayPlayer.c_str());
  fm.drawString(pingX, getY(), 0, face, getFontSize(), displayPing.c_str());
  fm.setDarkness(1.0f);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
