/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "Socket.h"

SocketHandler::~SocketHandler()
{
  if(socketSet) { net_FreeSocketSet(socketSet); socketSet = NULL; }
}

teTCPError SocketHandler::initialize(uint32_t connections)
{
  maxUsers = connections;

  socketSet = net_AllocSocketSet(getMaxConnections()+1);
  if (!socketSet)
    return eTCPSocketNFG;

  is_init = true;

  return eTCPNoError;
}

void SocketHandler::addSocket(Socket *socket)
{
  net_TCP_AddSocket(socketSet,socket->getSocket());
  socketMap[socket] = NULL;
}

void SocketHandler::removeSocket(Socket *socket)
{
  removeSocket(socketMap.find(socket));
}

void SocketHandler::removeSocket(SocketMapType::iterator &itr)
{
  if(itr == socketMap.end()) return;
  net_TCP_DelSocket(socketSet, itr->first->getSocket());
  net_TCP_Close(itr->first->getSocket());
  delete itr->first;
  if(itr->second) delete itr->second;
  socketMap.erase(itr++);
}

bool SocketHandler::global_init()
{
  return net_Init() == 0;
}

teTCPError ListenSocket::listen(uint16_t port)
{
  //disconnect();

  if ( port == 0)
    return eTCPBadPort;

  serverIP.host = INADDR_ANY;
  serverIP.port = port;

  socket = net_TCP_Open(&serverIP);
  if ( socket == NULL )
    return eTCPConnectionFailed;

  IPaddress serverIP;
  net_ResolveHost(&serverIP, NULL, getPort());
  socket = net_TCP_Open(&serverIP);

  sockHandler->addSocket(this);

  return eTCPNoError;
}

void SocketHandler::update()
{
  if (net_CheckSockets(socketSet, 1) < 1)
    return;

  for(SocketMapType::iterator itr = socketMap.begin(); itr != socketMap.end();)
    if(!itr->first->update(itr->second))
      removeSocket(itr);
    else
      ++itr;
}

bool ListenSocket::update(PacketHandlerBase *&)
{
  // check for new connections
  if ( net_SocketReady(socket) )
  {
    TCPsocket newsock;
    while ((newsock = net_TCP_Accept(socket)) != NULL)
      if(ConnectSocket *socket = onConnect(newsock))
        sockHandler->addSocket(socket);
      else
        net_TCP_Close(newsock);
  }
  return true;
}

bool ConnectSocket::update(PacketHandlerBase *& handler)
{
  Packet *packet;
  while((packet = readData()) != NULL)
  {
    onReadData(handler, *packet);
    delete packet;
  }

  if(!isConnected()) {
    onDisconnect();
    return false;
  } else
    return true;
}

void ListenSocket::disconnect()
{
  if(socket) { net_TCP_Close(socket); socket = NULL; } // but it still seems to leave a mem leak :(
}

ConnectSocket::ConnectSocket(SocketHandler *h, const TCPsocket &s)
  : Socket(h, s), connected(true)
{
  initRead();
}

ConnectSocket::ConnectSocket(SocketHandler *h)
  : Socket(h), connected(true)
{
  initRead();
}

void ConnectSocket::initRead()
{
  remainingHeader = 4;
  remainingData = 0;
  poz = 0;
}

void ConnectSocket::disconnect()
{
  connected = false;
}

teTCPError ConnectSocket::connect(std::string server_and_port)
{
  int port = 0;
  std::string::size_type pos = server_and_port.find(':');
  if(pos != std::string::npos)
    port = atoi(server_and_port.substr(pos+1).c_str());
  return connect(server_and_port.substr(0, pos), port);
}

teTCPError ConnectSocket::connect(std::string server, uint16_t port)
{
  if ( net_ResolveHost(&serverIP, server.c_str(), port))
    return eTCPUnknownError;

  if ( serverIP.host == INADDR_NONE )
    return eTCPBadAddress;

  disconnect();

  if ( serverIP.host == 0 || serverIP.port == 0 || serverIP.host == INADDR_NONE )
    return eTCPBadAddress;

  socket = net_TCP_Open(&serverIP);
  if ( socket == NULL )
    return eTCPConnectionFailed;

  sockHandler->addSocket(this);
  connected = true;

  return eTCPNoError;
}

Packet * ConnectSocket::readData()
{
  if(!isConnected() || !net_SocketReady(socket))
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
    {
      remainingData = *(uint16_t*)(buffer+2);
      if(remainingData == 0)
      {
        uint16_t opcode = *(uint16_t*)buffer;
        Packet * ret = new Packet(opcode, (uint8_t*)"", 0);
        initRead();
        return ret;
      }
    }
    //else       - return even if there might be more data, will be read later
    // otherwise if no more data was sent, the next recv will return -1 and that
    // will be incorrectly interpreted as a closed socket
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
    uint16_t opcode = *(uint16_t*)buffer;
    uint16_t len = *(uint16_t*)(buffer+2);
    Packet * ret = new Packet(opcode, buffer + 4, (size_t)len);
    initRead();
    return ret;
  }

  return NULL;
}

teTCPError ConnectSocket::sendData(Packet &packet)
{
  if (!isConnected())
    return eTCPSocketNFG;

  void *data = (void*)packet.getData();
  uint16_t opcode = packet.getOpcode();
  uint16_t len = (uint16_t)packet.getLength();

  // send the header first
  char header[4];
  *(uint16_t*)header = opcode;
  *(uint16_t*)(header + 2) = len;
  int lenSent = net_TCP_Send(socket, header, 4);
  if (lenSent < 4)
    return eTCPConnectionFailed;

  if(data && len >= 1)
  {
    lenSent = net_TCP_Send(socket, data, len);

    if (lenSent < len)
      return eTCPConnectionFailed;
  }

  return eTCPNoError;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
