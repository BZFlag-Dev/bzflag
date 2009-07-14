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
#include "ServerHandler.h"
#include "TokenMgr.h"
#include "UserStorage.h"

INSTANTIATE_PACKETHANDLER(ServerHandler)

void ServerHandler::initHandlerTable()
{
  for(int i = 0; i < NUM_OPCODES; i++)
    handlerTable[i] = &PacketHandler::handleInvalid;

  handlerTable[SMSG_TOKEN_VALIDATE]     = &ServerHandler::handleTokenValidate;
  handlerTable[SMSG_GROUP_LIST]         = &ServerHandler::handleGroupList;
}

bool ServerHandler::handleTokenValidate(Packet &packet)
{
  char n;
  if(!(packet >> n)) return false;

  Packet response(DMSG_TOKEN_VALIDATE, n*4);
  response << n;
  for(int i = 0; i < n; i++) {
    uint8_t callsign[MAX_CALLSIGN_LEN+1];
    uint32_t token = 0;
    if(!(packet >> token)) return false;
    if(!packet.read_string(callsign, MAX_CALLSIGN_LEN+1)) return false;

    response << callsign;
    uint32_t bzid = sTokenMgr.checkToken((char *)callsign, token);
    if(bzid) {
      response << (uint32_t)2;                          // registered, verified
      // send list of groups
      std::list<std::string> groups = sUserStore.intersectGroupList((char*)callsign, m_groups);
      response << (uint32_t)groups.size();
      for(std::list<std::string>::iterator itr = groups.begin(); itr != groups.end(); ++itr)
        response << itr->c_str();
      // send bzid as string for future flexibility
      char bzid_str[32];
      sprintf(bzid_str, "%d", bzid);
      response << bzid_str;
    } else if(sUserStore.isRegistered((char*)callsign))
      response << (uint32_t)1;                          // registered, not verified
    else
      response << (uint32_t)0;                          // not registered
  }
  m_socket->sendData(response);

  return true;
}

bool ServerHandler::handleGroupList(Packet &packet)
{
    int nr = 0;
    if(!(packet >> nr)) return false;

    m_groups.clear();
    for(int i = 0; i < nr; i++)
    {
        uint8_t group[MAX_GROUPNAME_LEN+1];
        if(!packet.read_string(group, MAX_GROUPNAME_LEN+1)) return false;
        /* TODO: validate group name */
        m_groups.push_back((char*)group);
    }

    return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
