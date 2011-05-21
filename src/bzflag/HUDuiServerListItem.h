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

/*
 * HUDuiServerListItem:
 *  User interface class for server list items. Acts as a UI wrapper
 *  for a ServerItem.
 */

#ifndef __HUDUISERVERLISTITEM_H__
#define __HUDUISERVERLISTITEM_H__

// ancestor class
#include "HUDuiControl.h"

#include "FontManager.h"

#include "ServerItem.h"
#include "ServerList.h"
#include <string>

class HUDuiServerListItem : public HUDuiControl {
  public:
    HUDuiServerListItem();
    HUDuiServerListItem(ServerItem* item);
    HUDuiServerListItem(std::string key);
    ~HUDuiServerListItem();

    void setSize(float width, float height);
    void setFontSize(float size);
    void setFontFace(const LocalFontFace* face);

    void setColumnSizes(float modes_percent, float domain, float server, float player, float ping);

    std::string getDomainName() { return domainName; }
    std::string getServerName() { return serverName; }
    std::string getPlayerCount() { return playerCount; }
    std::string getServerPing() { return serverPing; }

    std::string calculateModes();
    std::string calculateDomainName();
    std::string calculateServerName();
    std::string calculatePlayers();
    std::string calculatePing();

    ServerItem* getServer() { return serverList.lookupServer(serverKey); }
    std::string getServerKey() { return serverKey; }

  protected:
    void doRender();
    void resize();
    std::string shorten(std::string string, float width);

  private:
    ServerList& serverList;
    std::string serverKey;

    std::string modes;
    std::string domainName;
    std::string serverName;
    std::string playerCount;
    std::string serverPing;

    std::string displayModes;
    std::string displayDomain;
    std::string displayServer;
    std::string displayPlayer;
    std::string displayPing;

    float modes_percentage;
    float domain_percentage;
    float server_percentage;
    float player_percentage;
    float ping_percentage;

    float spacerWidth;

    FontManager& fm;
};

#endif // __HUDuiServerListItem_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
