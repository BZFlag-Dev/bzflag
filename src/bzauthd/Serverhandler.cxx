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

#include <common.h>
#include "NetHandler.h"
#include "TokenMgr.h"
#include "UserStorage.h"

bool PacketHandler::handleTokenValidate(Packet &packet)
{
  char n;
  if(!(packet >> n)) return false;

  Packet response(DMSG_TOKEN_VALIDATE, n*4);
  response << n;
  for(int i = 0; i < n; i++) {
    uint8_t callsign[MAX_CALLSIGN_LEN+1];
    uint32_t token;
    if(!(packet >> token)) return false;
    if(!packet.read_string(callsign, MAX_CALLSIGN_LEN+1)) return false;

    response.append(callsign, strlen((char*)callsign)+1);
    if(sTokenMgr.checkToken((char *)callsign, token))
      response << (uint32_t)2;                          // registered, verified
    else if(sUserStore.isRegistered((char*)callsign))
      response << (uint32_t)1;                          // registered, not verified
    else
      response << (uint32_t)0;                          // not registered
  }
  m_socket->sendData(response);

  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8