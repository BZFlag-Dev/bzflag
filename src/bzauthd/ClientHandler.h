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

#ifndef __BZAUTHD_CLIENTHANDLER_H__
#define __BZAUTHD_CLIENTHANDLER_H__

#include "NetHandler.h"

/* Client specific packet handler */
class ClientHandler : public PacketHandlerTemplate<ClientHandler>
{
public:
  ClientHandler(ConnectSocket *socket) 
    : PacketHandlerTemplate<ClientHandler>(socket), m_authSession(NULL), m_regSession(NULL) {}
  ~ClientHandler() { delete m_authSession; delete m_regSession; }

  static void initHandlerTable();

  bool handleAuthRequest(Packet &packet);
  bool handleRegisterGetForm(Packet &packet);
  bool handleRegisterRequest(Packet &packet);
  bool handleRegisterResponse(Packet &packet);
  bool handleAuthResponse(Packet &packet);
private:
  AuthSession *m_authSession;
  RegisterSession *m_regSession;
};

#endif // __BZAUTHD_CLIENTHANDLER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
