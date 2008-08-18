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
#include "ConfigMgr.h"
#include "Log.h"

INSTANTIATE_SINGLETON(NetHandler);

PHFunc handlerTable[NUM_OPCODES];

void PacketHandler::initHandlerTable()
{
  for(int i = 0; i < NUM_OPCODES; i++)
    handlerTable[i] = &PacketHandler::handleInvalid;

  handlerTable[CMSG_AUTH_REQUEST]       = &PacketHandler::handleAuthRequest;
  handlerTable[CMSG_AUTH_RESPONSE]      = &PacketHandler::handleAuthResponse;
  handlerTable[CMSG_REGISTER_GET_FORM]  = &PacketHandler::handleRegisterGetForm;
  handlerTable[CMSG_REGISTER_REQUEST]   = &PacketHandler::handleRegisterRequest;
  handlerTable[CMSG_REGISTER_RESPONSE]  = &PacketHandler::handleRegisterResponse;
  handlerTable[SMSG_TOKEN_VALIDATE]     = &PacketHandler::handleTokenValidate;
}

const char *getOpcodeName(Packet &packet)
{
  return bzAuthOpcodeNames[packet.getOpcode()];
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

void NetConnectSocket::onReadData(PacketHandlerBase *&handler, Packet &packet)
{
  if(!handler)
  {
    if(packet.getOpcode() != MSG_HANDSHAKE)
    {
      sLog.outError("invalid opcode %d received, handshake must be first", packet.getOpcode());
      disconnect();
    }
    else
      if(!(handler = PacketHandler::handleHandshake(packet, this)))
        disconnect();
  }
  else
  {
    if(!(((PacketHandler*)handler)->*handlerTable[packet.getOpcode()])(packet))
    {
      sLog.outError("received %s: invalid packet format (length: %d)", getOpcodeName(packet), (uint16_t)packet.getLength());
      disconnect();
    }
  }
}

ConnectSocket *NetListenSocket::onConnect(TCPsocket &s)
{
  return new NetConnectSocket(NULL, s);
}

void NetConnectSocket::onDisconnect()
{
  return;
}


NetHandler::NetHandler()
{
  sockHandler = new SocketHandler;
  localServer = new NetListenSocket(sockHandler);
}

bool NetHandler::initialize()
{
  // init the net library
  if(!SocketHandler::global_init())
  {
    sLog.outError("NetHandler: Cannot initialize the net library");
    return false;
  }

  sockHandler->initialize(32000);

  uint32_t listenPort = sConfig.getIntValue(CONFIG_LOCALPORT);

  teTCPError err = localServer->listen(listenPort);
  if(err != eTCPNoError)
  {
    sLog.outError("NetHandler: Cannot listen on port %d, error code %d", listenPort, err);
    return false;
  }

  PacketHandler::initHandlerTable();

  sLog.outLog("NetHandler: initialized");
  return true;
}

void NetHandler::update()
{
  sockHandler->update();
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