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
//#include "../tcp-net/include/TCPConnection.h"
#include "Config.h"
#include "Log.h"

INSTANTIATE_SINGLETON(NetHandler);

OpcodeEntry opcodeTable[NUM_OPCODES] = {
  {"MSG_HANDSHAKE",             &PacketHandler::handleInvalid           },
  {"CMSG_AUTH_REQUEST",         &PacketHandler::handleAuthRequest       },
  {"DMSG_AUTH_FAIL",            &PacketHandler::handleInvalid           },
  {"DMSG_AUTH_CHALLENGE",       &PacketHandler::handleInvalid           },
  {"CMSG_AUTH_RESPONSE",        &PacketHandler::handleAuthResponse      },
  {"DMSG_AUTH_SUCCESS",         &PacketHandler::handleInvalid           },
  {"CMSG_REGISTER_GET_FORM",    &PacketHandler::handleRegisterGetForm   },
  {"DMSG_REGISTER_FAIL",        &PacketHandler::handleInvalid           },
  {"DMSG_REGISTER_SEND_FORM",   &PacketHandler::handleInvalid           },
  {"CMSG_REGISTER_REQUEST",     &PacketHandler::handleRegisterRequest   },
  {"DMSG_REGISTER_CHALLENGE",   &PacketHandler::handleInvalid           },
  {"CMSG_REGISTER_RESPONSE",    &PacketHandler::handleRegisterResponse  },
  {"DMSG_REGISTER_SUCCESS",     &PacketHandler::handleInvalid           },
  {"SMSG_TOKEN_VALIDATE",       &PacketHandler::handleTokenValidate     },
  {"DMSG_TOKEN_VALIDATE",       &PacketHandler::handleInvalid           }
};

const char *getOpcodeName(Packet &packet)
{
  return opcodeTable[packet.getOpcode()].name;
}

bool PacketHandler::handleInvalid(Packet &packet)
{
  sLog.outError("received an invalid opcode %s (%d)", getOpcodeName(packet), packet.getOpcode());
  return true;
}

bool PacketHandler::handleNull(Packet &packet)
{
  sLog.outDebug("received an unhandled opcode %s (%d)", getOpcodeName(packet), packet.getOpcode());
  return true;
}

void NetListenSocket::onReadData(ConnectSocket *socket, PacketHandlerBase *&handler, Packet *packet)
{
  if(!handler)
  {
    if(packet->getOpcode() != MSG_HANDSHAKE)
    {
      sLog.outError("invalid opcode %d received, handshake must be first", packet->getOpcode());
      socket->disconnect();
    }
    else
      if(!(handler = PacketHandler::handleHandshake(*packet, socket)))
        socket->disconnect();
  }
  else
  {
    if(!(((PacketHandler*)handler)->*opcodeTable[packet->getOpcode()].handler)(*packet))
    {
      sLog.outError("received %s: invalid packet format (length: %d)", getOpcodeName(*packet), (uint16)packet->getLength());
      socket->disconnect();
    }
  }
}

bool NetListenSocket::onConnect(TCPsocket &)
{
  return true;
}

void NetListenSocket::onDisconnect(ConnectSocket *)
{
  return;
}


NetHandler::NetHandler()
{
  localServer = new NetListenSocket;
}

bool NetHandler::initialize()
{
  // init the net library
  if(net_Init() != 0)
  {
    sLog.outError("NetHandler: Cannot initialize the net library");
    return false;
  }

  uint32 listenPort = sConfig.getIntValue(CONFIG_LOCALPORT);

  teTCPError err = localServer->listen(listenPort, 32000);
  if(err != eTCPNoError)
  {
    sLog.outError("NetHandler: Cannot listen on port %d, error code %d", listenPort, err);
    return false;
  }

  sLog.outLog("NetHandler: initialized");
  return true;
}

void NetHandler::update()
{
  localServer->update();
}

NetHandler::~NetHandler()
{
  delete localServer;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8