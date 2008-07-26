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

/*
 * HUDuiServerListItem:
 *	User interface class for server list items. Acts as a UI wrapper
 *	for a ServerItem.
 */

#ifndef	__HUDUISERVERLISTITEM_H__
#define	__HUDUISERVERLISTITEM_H__

// ancestor class
#include "HUDuiControl.h"

#include "ServerItem.h"
#include <string>

class HUDuiServerListItem : public HUDuiControl {
  public:
      HUDuiServerListItem();
      HUDuiServerListItem(ServerItem item);
      ~HUDuiServerListItem();
	
    void setSize(float width, float height);
    void setPosition(float x, float y);
	
    std::string getDomainName();
    std::string getServerName();
    std::string getPlayerCount();
    std::string getServerPing();

    std::string getServerKey();

  protected:
    void doRender();
    void resize();
    std::string shorten(std::string string, float width);

  private:
    std::string serverKey;

    std::string domainName;
    std::string serverName;
    std::string playerCount;
    std::string serverPing;

    std::string displayDomain;
    std::string displayServer;
    std::string displayPlayer;
    std::string displayPing;

    float domainX;
    float serverX;
    float playerX;
    float pingX;

    float spacerWidth;
	
    static const float DOMAIN_PERCENTAGE;
    static const float SERVER_PERCENTAGE;
    static const float PLAYER_PERCENTAGE;
    static const float PING_PERCENTAGE;
};

#endif // __HUDuiServerListItem_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8