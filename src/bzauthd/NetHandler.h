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

#ifndef __BZAUTHD_NETHANDLER_H__
#define __BZAUTHD_NETHANDLER_H__

#include <Socket.h>
#include <AuthProtocol.h>

class Packet;

class Peer
{
public:
private:
};

class Server : public Peer
{
public:
};

class Client : public Peer
{
public:
};

class Daemon : public Peer
{
public:
};


class Session
{
public:
};

class AuthSession : public Session
{
public:
};

class RegisterSession : public Session
{
public:
};

/* Main packet handler for the daemon */
class PacketHandler : public PacketHandlerBase
{
public:
  PacketHandler(ConnectSocket *socket) 
    : PacketHandlerBase(socket) {}
  virtual ~PacketHandler() {}

  virtual bool handle(Packet &packet) = 0;

  static PacketHandler* handleHandshake(Packet &packet, ConnectSocket *socket);

  bool handleNull(Packet &packet);
  bool handleInvalid(Packet &packet);
};

/* Helper class for the specific packet handlers
   which defines the packet handler table */
template < class T >
class PacketHandlerTemplate : public PacketHandler
{
public:
  typedef bool (T::*PHFunc)(Packet &packet);

  PacketHandlerTemplate(ConnectSocket *socket)
    : PacketHandler(socket) {}
  
  bool handle(Packet &packet)
  {
    ((static_cast<T*>(this))->*handlerTable[packet.getOpcode()])(packet);
    return true;
  }

protected:
  static PHFunc handlerTable[NUM_OPCODES];
};

#define INSTANTIATE_PACKETHANDLER(Handler) \
  template<> PacketHandlerTemplate<Handler>::PHFunc PacketHandlerTemplate<Handler>::handlerTable[NUM_OPCODES] = {};

const char *getOpcodeName(Packet &packet);

/** The socket that listens for incoming connections */
class NetListenSocket : public ListenSocket
{
public:
  NetListenSocket(SocketHandler *h) : ListenSocket(h) {}
  ~NetListenSocket() {}
  ConnectSocket* onConnect(TCPsocket &socket);
};

/** The socket created by the NetListenSocket for incoming connection */
class NetConnectSocket : public ConnectSocket
{
public:
  NetConnectSocket(SocketHandler *h, const TCPsocket &s) : ConnectSocket(h,s) {}
  NetConnectSocket(SocketHandler *h) : ConnectSocket(h) {}
  void onReadData(PacketHandlerBase *&handler, Packet &packet);
  void onDisconnect();
};

class NetHandler : public Singleton<NetHandler>
{
public:
  NetHandler();
  ~NetHandler();
  bool initialize();
  void update();
private:
  SocketHandler *sockHandler;
  NetListenSocket *localServer;
};

#define sNetHandler NetHandler::instance()

#endif // __BZAUTHD_NETHANDLER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
