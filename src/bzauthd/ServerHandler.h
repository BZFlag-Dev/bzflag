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

#ifndef __BZAUTHD_SERVERHANDLER_H__
#define __BZAUTHD_SERVERHANDLER_H__

#include "NetHandler.h"
#include <list>
#include <string>
#include "UserStorage.h"

/* Server specific packet handler */
class ServerHandler : public PacketHandlerTemplate<ServerHandler>
{
public:
  ServerHandler(ConnectSocket *socket)
    : PacketHandlerTemplate<ServerHandler>(socket) {}

  static void initHandlerTable();

  bool handleTokenValidate(Packet &packet);
  bool handleGroupList(Packet &packet);
private:
  std::list<GroupId> m_groups;
};

#endif // __BZAUTHD_SERVERHANDLER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
