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
#include <assert.h>

/** ---- SocketHandler ---- **/

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

bool SocketHandler::addSocket(Socket *socket)
{
  net_TCP_AddSocket(socketSet,socket->getSocket());
  if(updating == socket) return false;
  socketMap[socket] = NULL;
  return true;
}

bool SocketHandler::removeSocket(Socket *socket)
{
  SocketMapType::iterator const &itr = socketMap.find(socket);
  if(itr != socketMap.end()) {
     _removeSocket(itr);
    if(updating == socket) return false;
    socketMap.erase(itr);
  }
  return true;
}

void SocketHandler::_removeSocket(SocketMapType::iterator const &itr)
{
  net_TCP_DelSocket(socketSet, itr->first->getSocket());
  if(itr->second) delete itr->second;
}

bool SocketHandler::global_init()
{
  return net_Init() == 0;
}

/** update all active sockets in the handler */
void SocketHandler::update()
{
  if (net_CheckSockets(socketSet, 1) < 1)
    return;

  for(SocketMapType::iterator itr = socketMap.begin(); itr != socketMap.end();) {
    // mark the socket that is currently being updated
    updating = itr->first;
    if(!itr->first->update(itr->second)) {
      // avoid searching for the socket again
      itr->first->_disconnect();
      delete itr->first;
      socketMap.erase(itr++);
    } else
      ++itr;
  }
  updating = NULL;
}

/** ---- Socket ---- **/

/** deleting the socket is the same as disconnecting it,
    but it must not be done during a socket update */
Socket::~Socket() {
  if(socket) {
    if(sockHandler) assert(sockHandler->removeSocket(this));
    _disconnect();
  }
}

/** disconnect the underlying socket and remove from the socket handler,
    if called during an update, the socket will be deleted afterwards,
    unless the socket is reconnected in the same update */
void Socket::disconnect()
{
  if(socket) {
    if(sockHandler) sockHandler->removeSocket(this);
    _disconnect();
  }
}

/** disconnect without removing from the handler */
void Socket::_disconnect()
{
  if(socket) { net_TCP_Close(socket); socket = NULL; }
}

/** ---- ListenSocket ---- **/

/** listen for incoming connections */
teTCPError ListenSocket::listen(uint16_t port)
{
  disconnect();

  if ( port == 0)
    return eTCPBadPort;

  serverIP.host = INADDR_ANY;
  serverIP.port = port;

  net_ResolveHost(&serverIP, NULL, getPort());
  socket = net_TCP_Open(&serverIP);
  if ( socket == NULL )
    return eTCPConnectionFailed;

  if(sockHandler)
    sockHandler->addSocket(this);

  return eTCPNoError;
}

/** check for new connections */
bool ListenSocket::update(PacketHandlerBase *&)
{
  if ( net_SocketReady(socket) )
  {
    TCPsocket newsock;
    while ((newsock = net_TCP_Accept(socket)) != NULL) {
      if(ConnectSocket *clisock = onConnect(newsock)) {
        sockHandler->addSocket(clisock);
        // inherit the socket handler from parent
        clisock->sockHandler = sockHandler;
      } else
        net_TCP_Close(newsock);
    }
  }
  return true;
}

/** ---- ConnectSocket ---- **/

ConnectSocket::ConnectSocket(SocketHandler *h, const TCPsocket &s)
  : Socket(h, s)
{
  initRead();
}

ConnectSocket::ConnectSocket(SocketHandler *h)
  : Socket(h)
{
  initRead();
}

void ConnectSocket::initRead()
{
  remainingHeader = 4;
  remainingData = 0;
  poz = 0;
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

  disconnect();

  if ( serverIP.host == INADDR_NONE )
    return eTCPBadAddress;

  if ( serverIP.host == 0 || serverIP.port == 0 || serverIP.host == INADDR_NONE )
    return eTCPBadAddress;

  socket = net_TCP_Open(&serverIP);
  if ( socket == NULL )
    return eTCPConnectionFailed;

  if(sockHandler)
    sockHandler->addSocket(this);

  return eTCPNoError;
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
