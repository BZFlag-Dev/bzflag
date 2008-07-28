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
  if (net_CheckSockets(socketSet, 1) < 1)
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
      onReadData(itr->first, itr->second, packet);
      delete packet;
    }

    if(!itr->first->isConnected())
    {
      onDisconnect(itr->first);
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

void ListenSocket::disconnect()
{
  if(socket) net_TCP_Close(socket);   // but it still seems to leave a mem leak :(
  if(socketSet) net_FreeSocketSet(socketSet);
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
  poz = 0;
}

void ConnectSocket::disconnect()
{
  connected = false;
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
      remainingData = *(uint16*)(buffer+2);
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
    uint16 opcode = *(uint16*)buffer;
    uint16 len = *(uint16*)(buffer+2);
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
  uint16 opcode = packet.getOpcode();
  uint16 len = (uint16)packet.getLength();

  // send the header first
  char header[4];
  *(uint16*)header = opcode;
  *(uint16*)(header + 2) = len;
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
