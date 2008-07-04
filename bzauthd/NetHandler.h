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

#include "Singleton.h"
#include <string.h>
#include <string>

class TCPServerConnection;
class TCPServerListener;

enum Opcodes
{
  MSG_HANDSHAKE             = 0,
  CMSG_AUTH_REQUEST         = 1,
  DMSG_AUTH_FAIL            = 2,
  DMSG_AUTH_CHALLENGE       = 3,
  CMSG_AUTH_RESPONSE        = 4,
  DMSG_AUTH_SUCCESS         = 5,
  CMSG_REGISTER_GET_FORM    = 6,
  DMSG_REGISTER_FAIL        = 7,
  DMSG_REGISTER_SEND_FORM   = 8,
  CMSG_REGISTER_REQUEST     = 9,
  DMSG_REGISTER_CHALLENGE   = 10,
  CMSG_REGISTER_RESPONSE    = 11,
  DMSG_REGISTER_SUCCESS     = 12,
  SMSG_TOKEN_VALIDATE       = 13,
  DMSG_TOKEN_VALIDATE       = 14,
  NUM_OPCODES
};

enum SessionTypes
{
  SESSION_INIT = 0,
  SESSION_AUTH = 1,
  SESSION_REG = 2,
  SESSION_TOKEN = 3,
  NUM_SESSION_TYPES
};

enum PeerType
{
  PEER_ANY = 0,
  PEER_CLIENT = 1,
  PEER_SERVER = 2,
  PEER_DAEMON = 3,
  NUM_PEER_TYPES
};

class Peer
{
public:
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

class Packet;

class PacketHandler
{
public:
  PacketHandler() : m_peer(NULL), m_authSession(NULL), m_regSession(NULL) {}
  ~PacketHandler() { delete m_peer; delete m_authSession; delete m_regSession; }

  bool handleNull(Packet &packet);
  bool handleInvalid(Packet &packet);
  bool handleHandshake(Packet &packet);
  bool handleAuthRequest(Packet &packet);
  bool handleRegisterGetForm(Packet &packet);
  bool handleRegisterRequest(Packet &packet);
  bool handleAuthResponse(Packet &packet);
private:
  Peer *m_peer;
  AuthSession *m_authSession;
  RegisterSession *m_regSession;
};

typedef bool (PacketHandler::*PHFunc)(Packet &packet);

struct OpcodeEntry
{
  const char * name;
  PHFunc handler;
};

extern OpcodeEntry opcodeTable[NUM_OPCODES];

class Packet
{
public:
  Packet(uint16 opcode, uint8 *data, size_t size) { init(data, size, opcode); }
  Packet(uint16 opcode, size_t size = 1024) { init(size, opcode); }
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
    append((const uint8*)&x, sizeof(T));
    return (*this);
  }

  Packet &operator << (const uint8 * &x)
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
    append((const uint8*)x.c_str(), x.size());
    return (*this);
  }

  void append(const uint8 *x, size_t size)
  {
    if(m_wpoz + size >= m_size)
    {
      m_data = (uint8*)realloc((void*)m_data, 2*m_size);
      m_size *= 2;
    }

    memcpy(m_data + m_wpoz, x, size);
    m_wpoz += size;
  }

  bool read(uint8 *x, size_t size)
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

  operator bool() const { return m_rpoz <= m_size; }

  uint16 getOpcode() const { return m_opcode; }
  const char *getOpcodeName() const { return opcodeTable[getOpcode()].name; }

protected:
  void init(size_t size, uint16 opcode)
  {
    m_data = (uint8*)malloc(size);
    m_size = size;
    m_rpoz = 0;
    m_wpoz = 0;
    m_opcode = opcode;
  }

  void init(uint8 *data, size_t size, uint16 opcode)
  {
    init(size, opcode);
    memcpy(m_data, data, size);
    m_wpoz = size;
  }

  uint16 m_opcode;
  uint8 *m_data;
  size_t m_size;
  size_t m_rpoz;
  size_t m_wpoz;
};

class NetHandler : public Singleton<NetHandler>
{
public:
  NetHandler();
  ~NetHandler();
  bool initialize();
  void update();
private:
  TCPServerConnection *localServer;
  TCPServerListener *tcpListener;
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