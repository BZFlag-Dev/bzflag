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

#ifndef __BZAUTHD_SOCKET_H__
#define __BZAUTHD_SOCKET_H__

#include "Singleton.h"
#include <string.h>
#include <string>
#include <map>

#include "net.h"

#define MAX_PACKET_SIZE 4096

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class ConnectSocket;

class PacketHandlerBase
{
public:
  PacketHandlerBase(ConnectSocket *socket) : m_socket(socket) {}
  virtual ~PacketHandlerBase() {}
protected:
  ConnectSocket *m_socket;
};

/** Packet class for both sent and received packets */
class Packet
{
public:
  Packet(uint16_t opcode, uint8_t *data, size_t size) { init(data, size, opcode); }
  Packet(uint16_t opcode, size_t size = 1024) { init(size, opcode); }
  Packet(Packet & packet) { init(packet.m_data, packet.m_size, packet.m_opcode); }

  ~Packet() { free(m_data); }

  template < class T >
  Packet& operator >> (T &x)
  {
    if(m_rpoz + sizeof(T) <= m_size)
    {
      x = *(T *)(m_data + m_rpoz);
      m_rpoz += sizeof(T);
    }
    else
      m_rpoz = m_size + 1;

    return (*this);
  }

  template < class T >
  Packet &operator << (const T &x)
  {
    append((const uint8_t*)&x, sizeof(T));
    return (*this);
  }

  Packet &operator << (const uint8_t * &x)
  {
    // get string length and protect against overflow
    size_t len = 0;
    // TODO: replace with a constant
    while(len < 4096) if(!x[len++]) break;
    
    append(x, len);
    return (*this);
  }

  template <>
  Packet &operator << (const std::string &x)
  {
    append((const uint8_t*)x.c_str(), x.size());
    return (*this);
  }

  void append(const uint8_t *x, size_t size)
  {
    while(m_wpoz + size >= m_size)
    {
      m_data = (uint8_t*)realloc((void*)m_data, 2*m_size);
      m_size *= 2;
    }

    memcpy(m_data + m_wpoz, x, size);
    m_wpoz += size;
  }

  bool read(uint8_t *x, size_t size)
  {
    if(m_rpoz + size > m_size)
    {
      m_rpoz = m_size + 1;
      return false;
    }

    memcpy(x, m_data + m_rpoz, size);
    m_rpoz += size;
    return true;
  }

  // read a string of length at most buf_size (including the terminating '\0')
  bool read_string(uint8_t *x, size_t buf_size)
  {
    for(size_t i = m_rpoz; i < std::min(m_rpoz + buf_size, m_size); i++)
    {
      x[i-m_rpoz] = m_data[i];
      if(m_data[i] == '\0')
      {
        m_rpoz = i + 1;
        return true;
      }
    }

    m_rpoz = m_size + 1;
    return false;
  }


  operator bool() const { return m_rpoz <= m_size; }

  size_t getLength() const { return m_wpoz; }
  uint16_t getOpcode() const { return m_opcode; }
  const uint8_t * getData() const { return (const uint8_t *)m_data; }

protected:
  void init(size_t size, uint16_t opcode)
  {
    m_data = (uint8_t*)malloc(size);
    m_size = std::max(size, (size_t)1);
    m_rpoz = 0;
    m_wpoz = 0;
    m_opcode = opcode;
  }

  void init(uint8_t *data, size_t size, uint16_t opcode)
  {
    init(size, opcode);
    memcpy(m_data, data, size);
    m_wpoz = size;
  }

  uint16_t m_opcode;
  uint8_t *m_data;
  size_t m_size;
  size_t m_rpoz;
  size_t m_wpoz;
};

typedef enum
{
  eTCPNoError = 0,
  eTCPNotInit,
  eTCPTimeout,
  eTCPBadAddress,
  eTCPBadPort,
  eTCPConnectionFailed,
  eTCPSocketNFG,
  eTCPInitFailed,
  eTCPSelectFailed,
  eTCPDataNFG,
  eTCPUnknownError
}teTCPError;

class SocketHandler;

/** Abstract base class for the sockets */
class Socket
{
public:
  friend class SocketHandler;
  Socket(SocketHandler *h, const TCPsocket &s) : socket(s), sockHandler(h) {}
  Socket(SocketHandler *h) : socket(NULL), sockHandler(h) {}
  virtual ~Socket() {}
  uint16_t getPort() const { return serverIP.port; }
  virtual void disconnect() = 0;
  TCPsocket &getSocket() { return socket; }

  virtual void onDisconnect() = 0;
protected:
  virtual bool update(PacketHandlerBase *& handler) = 0;
  IPaddress serverIP;
  TCPsocket socket;
  SocketHandler *sockHandler;
};

/** Socket for both outgoing and incoming connections */
class ConnectSocket : public Socket
{
public:
  ConnectSocket(SocketHandler *h, const TCPsocket &s);
  ConnectSocket(SocketHandler *h);

  Packet * readData();
  teTCPError sendData(Packet &packet);

  teTCPError connect(std::string server_and_port);
  teTCPError connect(std::string server, uint16_t port);

  /* Set/get the connected state
   * The socket will only be really disconnected
   * when removed from the handler.
   * That can happen if the connected state is changed is changed
   * during an update or if the RemoveSocket is called explicitly
   */
  void disconnect();
  bool isConnected() { return connected; }

  virtual void onReadData(PacketHandlerBase *&handler, Packet &packet) = 0;
private:
  bool update(PacketHandlerBase *& handler);
  void initRead();
  uint8_t buffer[MAX_PACKET_SIZE];
  uint16_t poz;
  uint16_t remainingHeader;
  uint16_t remainingData;
  bool connected;
};

/** Socket that listens for incoming connections */
class ListenSocket : public Socket
{
public:
  ListenSocket(SocketHandler *h) : Socket(h) {}
  ~ListenSocket() { disconnect(); }
  teTCPError listen(uint16_t port);

  void disconnect();
  
  virtual ConnectSocket* onConnect(TCPsocket &socket) = 0;

  void onDisconnect() {}
private:
  bool update(PacketHandlerBase *&);
};

/** SocketHandler updates and manages all of the sockets */
class SocketHandler
{
public:
  SocketHandler() : socketSet(NULL), is_init(false) {}
  ~SocketHandler();
  static bool global_init();
  teTCPError initialize(uint32_t connections);
  void update();
  void addSocket(Socket *socket);
  void removeSocket(Socket *socket);

  // accept at most this many sockets
  uint32_t getMaxConnections () const { return maxUsers; }
  bool isInitialized() const { return is_init; }
private:
  net_SocketSet socketSet;
  typedef std::map<Socket *, PacketHandlerBase *> SocketMapType;
  SocketMapType socketMap;
  uint32_t maxUsers;
  bool is_init;

  void removeSocket(SocketMapType::iterator &itr);
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
