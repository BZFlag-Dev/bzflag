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

#include "NetHandler.h"
#include "ClientHandler.h"
#include "ServerHandler.h"
#include "ConfigMgr.h"
#include "Log.h"

INSTANTIATE_SINGLETON(NetHandler)

PacketHandler* PacketHandler::handleHandshake(Packet &packet, ConnectSocket *socket)
{
  uint8_t peerType = 0;
  uint16_t protoVersion = 0;
  if(!(packet >> peerType >> protoVersion)) return NULL;

  PacketHandler *handler = NULL;

  switch (peerType) {
    case BZAUTHD_PEER_CLIENT: {
      sLog.outLog("received %s: client using protocol %d", getOpcodeName(packet), protoVersion);
      uint32_t cliVersion = 0;
      uint8_t commType = 0;
      if(!(packet >> cliVersion >> commType)) break;

      handler = new ClientHandler(socket);
      sLog.outLog("Handshake: client (%d) connected, requesting comm type %d", cliVersion, commType);
      
      bool success = false;
      switch (commType) {
        case 0: success = ((ClientHandler*)handler)->handleAuthRequest(packet); break;
        case 1: success = ((ClientHandler*)handler)->handleRegisterGetForm(packet); break;
        case 2: success = ((ClientHandler*)handler)->handleRegisterRequest(packet); break;
        default:
          sLog.outError("Handshake: invalid commType received : %d", commType);
      }
      if(!success) {
        delete handler;
        handler = NULL;
      }
    } break;
    case BZAUTHD_PEER_SERVER: {
      sLog.outLog("received %s: server using protocol %d", getOpcodeName(packet), protoVersion);
      handler = new ServerHandler(socket);
    } break;
    case BZAUTHD_PEER_DAEMON: {
      sLog.outLog("received %s: daemon using protocol %d", getOpcodeName(packet), protoVersion);
      handler = NULL/*new DaemonHandler(socket)*/;
    } break;
    default: {
      sLog.outError("received %s: unknown peer type %d", getOpcodeName(packet));
    }
  }

  return handler;
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
    if(!((PacketHandler*)handler)->handle(packet))
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

  ClientHandler::initHandlerTable();
  ServerHandler::initHandlerTable();

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
  delete sockHandler;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
