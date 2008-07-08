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

#include "common.h"
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
  {"CMSG_REGISTER_RESPONSE",    &PacketHandler::handleNull              },
  {"DMSG_REGISTER_SUCCESS",     &PacketHandler::handleInvalid           },
  {"SMSG_TOKEN_VALIDATE",       &PacketHandler::handleNull              },
  {"DMSG_TOKEN_VALIDATE",       &PacketHandler::handleInvalid           }
};

bool PacketHandler::handleInvalid(Packet &packet)
{
  sLog.outError("received an invalid opcode %s (%d)", packet.getOpcodeName(), packet.getOpcode());
  return true;
}

bool PacketHandler::handleNull(Packet &packet)
{
  sLog.outDebug("received an unhandled opcode %s (%d)", packet.getOpcodeName(), packet.getOpcode());
  return true;
}

teTCPError ListenSocket::listen(uint16 port, uint32 connections)
{
  //disconnect();

  if ( port == 0)
    return eTCPBadPort;

  maxUsers = connections;

  serverIP.host = INADDR_ANY;
  serverIP.port = port;

  socket = net_TCP_Open(&serverIP);
  if ( socket == NULL )
    return eTCPConnectionFailed;

  socketSet = net_AllocSocketSet(getMaxConnections()+1);
  if (!socketSet)
    return eTCPSocketNFG;

  IPaddress serverIP;
  net_ResolveHost(&serverIP, NULL, getPort());
  socket = net_TCP_Open(&serverIP);
  net_TCP_AddSocket(socketSet,socket);

  return eTCPNoError;
}

bool ListenSocket::update()
{
  if (net_CheckSockets(socketSet, ~0) < 1)
    return true;

  // check for new connections
  if ( net_SocketReady(socket) )
  {
    TCPsocket newsock;
    while ((newsock = net_TCP_Accept(socket)) != NULL)
    {
      if(onConnect(newsock))
      {
        net_TCP_AddSocket(socketSet, newsock);
        socketMap[new ConnectSocket(newsock, true)] = NULL;
      }
      else
        net_TCP_Close(newsock);
    }
  }

  for(SocketMapType::iterator itr = socketMap.begin(); itr != socketMap.end();)
  {
    Packet *packet;
    while((packet = itr->first->readData()) != NULL)
    {
      if(!itr->second)
      {
        if(packet->getOpcode() != MSG_HANDSHAKE)
        {
          sLog.outError("invalid opcode %d received, handshake must be first", packet->getOpcode());
          itr->first->disconnect();
        }
        else
          if(!(itr->second = PacketHandler::handleHandshake(*packet)))
            itr->first->disconnect();
      }
      else
      {
        if(!(itr->second->*opcodeTable[packet->getOpcode()].handler)(*packet))
        {
          sLog.outError("received %s: invalid packet format (length: %d)", packet->getOpcodeName(), (uint16)packet->getLength());
          itr->first->disconnect();
        }
      }
    }

    if(!itr->first->isConnected())
    {
      net_TCP_DelSocket(socketSet, itr->first->getSocket());
      net_TCP_Close(itr->first->getSocket());
      delete itr->first;
      if(itr->second) delete itr->second;
      socketMap.erase(itr++);
    }
    else
      ++itr;
  }

  return true;
}

bool ListenSocket::onConnect(TCPsocket &)
{
  return true;
}

void ListenSocket::disconnect()
{
  // TODO
}

ConnectSocket::ConnectSocket(const TCPsocket &s, bool isConn)
  : Socket(s), connected(isConn)
{
  initRead();
}

void ConnectSocket::initRead()
{
  remainingHeader = 4;
  remainingData = 0;
}

void ConnectSocket::disconnect()
{
  connected = false;
}

Packet * ConnectSocket::readData()
{
  if(!net_SocketReady(socket))
    return NULL;

  int read;

  if(remainingHeader)
  {
    read = net_TCP_Recv(socket, buffer + poz, remainingHeader);
    if(read <= 0)
    {
      disconnect();
      return NULL;
    }

    remainingHeader -= read;
    poz += read;

    if(!remainingHeader)
      remainingData = *(uint16*)(buffer+2);
    else
      return NULL;
  }

  read = net_TCP_Recv(socket, buffer + poz, remainingData);
  if(read <= 0)
  {
    disconnect();
    return NULL;
  }

  remainingData -= read;
  poz += read;

  if(!remainingData)
  {
    uint16 opcode = *(uint16*)buffer;
    uint16 len = *(uint16*)(buffer+2);
    Packet * ret = new Packet(opcode, buffer + 4, (size_t)len);
    initRead();
    return ret;
  }

  return NULL;
}

NetHandler::NetHandler()
{
  localServer = new ListenSocket;
}

bool NetHandler::initialize()
{
  // init the net library
  if(!net_Init()) return false;

  uint32 listenPort = sConfig.getIntValue(CONFIG_LOCALPORT);

  teTCPError err = localServer->listen(listenPort, 32000);
  if(err != eTCPNoError)
  {
    sLog.outError("NetHandler: Cannot listen on port %d, error code %d", listenPort, err);
    return false;
  }

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