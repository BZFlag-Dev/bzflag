/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "TextUtils.h"
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

HUDuiServerListItem::HUDuiServerListItem(std::string key) : HUDuiControl(), serverList(ServerList::instance()), domain_percentage(0.0f), server_percentage(0.0f), player_percentage(0.0f), ping_percentage(0.0f), fm(FontManager::instance())
{
  if (key == "")
    return;

  serverKey = key;

  displayModes = modes = calculateModes();
  displayDomain = domainName = calculateDomainName();
  displayServer = serverName = calculateServerName();
  displayPlayer = playerCount = calculatePlayers();
  displayPing = serverPing = calculatePing();
}

HUDuiServerListItem::~HUDuiServerListItem()
{
  // do nothing
}

std::string HUDuiServerListItem::calculateModes()
{
  ServerItem* server = serverList.lookupServer(serverKey);

  if (server == NULL)
    return "";

  std::string modesText;
  if (BZDB.isTrue("listIcons")) {
    // game mode
    if ((server->ping.observerMax == 16) && (server->ping.maxPlayers == 200))
      modesText += ANSI_STR_FG_CYAN "*  "; // replay
    else if (server->ping.gameType == ClassicCTF)
      modesText += ANSI_STR_FG_RED "*  "; // ctf
    else if (server->ping.gameType == RabbitChase)
      modesText += ANSI_STR_FG_WHITE "*  "; // white rabbit
    else
      modesText += ANSI_STR_FG_YELLOW "*  "; // free-for-all

    // jumping?
    if (server->ping.gameOptions & JumpingGameStyle)
      modesText += ANSI_STR_BRIGHT ANSI_STR_FG_MAGENTA "J ";
    else
      modesText += ANSI_STR_DIM ANSI_STR_FG_WHITE "J ";

    // superflags ?
    if (server->ping.gameOptions & SuperFlagGameStyle)
      modesText += ANSI_STR_BRIGHT ANSI_STR_FG_BLUE "F ";
    else
      modesText += ANSI_STR_DIM ANSI_STR_FG_WHITE "F ";

    // ricochet?
    if (server->ping.gameOptions & RicochetGameStyle)
      modesText += ANSI_STR_BRIGHT ANSI_STR_FG_GREEN "R ";
    else
      modesText += ANSI_STR_DIM ANSI_STR_FG_WHITE "R ";

    // LuaWorld -- S = scripted
    if (server->ping.gameOptions & LuaWorldAvailable) {
      if (server->ping.gameOptions & LuaWorldRequired) {
        modesText += ANSI_STR_BRIGHT ANSI_STR_FG_RED "S ";
      } else {
        modesText += ANSI_STR_BRIGHT ANSI_STR_FG_GREEN "S ";
      }
    } else {
      modesText += ANSI_STR_DIM ANSI_STR_FG_WHITE "S ";
    }

    // lag?
    if (server->ping.pingTime <= 0)
      modesText += ANSI_STR_BRIGHT ANSI_STR_FG_BLACK "L";
    else if (server->ping.pingTime < BZDB.eval("pingLow"))
      modesText += ANSI_STR_BRIGHT ANSI_STR_FG_GREEN "L";
    else if (server->ping.pingTime < BZDB.eval("pingMed"))
      modesText += ANSI_STR_BRIGHT ANSI_STR_FG_YELLOW "L";
    else if (server->ping.pingTime < BZDB.eval("pingHigh"))
      modesText += ANSI_STR_BRIGHT ANSI_STR_FG_ORANGE "L";
    else if (server->ping.pingTime >= BZDB.eval("pingHigh") && server->ping.pingTime < INT_MAX)
      modesText += ANSI_STR_BRIGHT ANSI_STR_FG_RED "L";
    else if (server->ping.pingTime >= BZDB.eval("pingHigh"))
      modesText += ANSI_STR_PULSATING ANSI_STR_FG_RED "L";
    else
      // shouldn't reach here
      modesText += ANSI_STR_BRIGHT ANSI_STR_FG_BLACK "L";
  }

  return modesText;
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

  if (server->ping.pingTime == INT_MAX)
    return "[nr]";

  return TextUtils::format("%d", server->ping.pingTime);
}

std::string HUDuiServerListItem::calculatePlayers()
{
  ServerItem* server = serverList.lookupServer(serverKey);

  if (server == NULL)
    return "";

  return TextUtils::format("%d/%d", server->getPlayerCount(), server->ping.maxPlayers);
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

void HUDuiServerListItem::setColumnSizes(float modes_percent, float domain,
                                         float server, float player, float ping)
{
  modes_percentage = modes_percent;
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

  displayModes = shorten(modes, (modes_percentage*getWidth())-2*spacerWidth);
  displayDomain = shorten(domainName, (domain_percentage*getWidth())-2*spacerWidth);
  displayServer = shorten(serverName, (server_percentage*getWidth())-2*spacerWidth);
  displayPlayer = shorten(playerCount, (player_percentage*getWidth())-2*spacerWidth);
  displayPing = shorten(serverPing, (ping_percentage*getWidth())-2*spacerWidth);
}

std::string HUDuiServerListItem::shorten(std::string string, float width)
{
  // Skip if it already fits
  if (fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), string) <= width)
    return string;

  // Iterate through each character. Expensive.
  for (int i=0; i<=(int)string.size(); i++) {
    // Is it too big yet?
    if (fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), string.substr(0, i)) > width) {
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
      (playerCount.compare(calculatePlayers()) == 0)||(serverPing.compare(calculatePing()) == 0)) {
    displayModes = modes = calculateModes();
    displayDomain = domainName = calculateDomainName();
    displayServer = serverName = calculateServerName();
    displayPlayer = playerCount = calculatePlayers();
    displayPing = serverPing = calculatePing();
    resize();
  }

  float modesX = getX() + spacerWidth;
  float domainX = getX() + modes_percentage*getWidth() + spacerWidth;
  float serverX = getX() + modes_percentage*getWidth() + domain_percentage*getWidth() + spacerWidth;
  float playerX = getX() + modes_percentage*getWidth() + domain_percentage*getWidth() + server_percentage*getWidth() + spacerWidth;
  float pingX = getX() + modes_percentage*getWidth() + domain_percentage*getWidth() + server_percentage*getWidth() + player_percentage*getWidth() + spacerWidth;

  int faceID = getFontFace()->getFMFace();

  fvec4 color(1.0f, 1.0f, 1.0f, 1.0f);

  // colorize server descriptions by shot counts
  const int maxShots = server->ping.maxShots;
  if (maxShots <= 0) {
    color.rgb() = fvec3(0.4f, 0.0f, 0.6f);
  } else if (maxShots == 1) {
    color.rgb() = fvec3(0.25f, 0.25f, 1.0f);
  } else if (maxShots == 2) {
    color.rgb() = fvec3(0.25f, 1.0f, 0.25f);
  } else if (maxShots == 3) {
    color.rgb() = fvec3(1.0f, 1.0f, 0.25f);
  } else {
    // graded orange/red
    const float shotScale = std::min(1.0f, log10f((float)(maxShots - 3)));
    color.r = 1.0f;
    color.g = 0.4f * (1.0f - shotScale);
    color.b = 0.25f * color.g;
  }

  const float fontSize = getFontSize();
  fm.setDarkness(darkness);
  fm.drawString(modesX,  getY(), 0, faceID, fontSize, displayModes);
  fm.drawString(domainX, getY(), 0, faceID, fontSize, displayDomain, &color);
  fm.drawString(serverX, getY(), 0, faceID, fontSize, displayServer, &color);
  fm.drawString(playerX, getY(), 0, faceID, fontSize, displayPlayer, &color);
  fm.drawString(pingX,   getY(), 0, faceID, fontSize, displayPing,   &color);
  fm.setDarkness(1.0f);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
