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
#include "../tcp-net/include/TCPConnection.h"
#include "Config.h"
#include "Log.h"

INSTANTIATE_SINGLETON(NetHandler);

OpcodeEntry opcodeTable[NUM_OPCODES] = {
  {"MSG_HANDSHAKE",             &PacketHandler::handleHandshake         },
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

bool HandleNull(Packet &packet)
{
  return true;
}

bool HandleHandshake(Packet &packet)
{
  return true;
}

class TCPServerListener : public TCPServerDataPendingListener
{
public:
  TCPServerListener()
  {
  }

  bool connect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer )
  {
    handlerMap[peer] = new PacketHandler;
    return true;
  }

  void pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, unsigned int count )
  {
    tvPacketList& packets = peer->getPackets();
    for(tvPacketList::iterator itr = packets.begin(); itr != packets.end(); ++itr)
    {
      uint16 opcode = (uint16)(*itr).first;
      size_t len;
      uint8* data = (uint8*)(*itr).second.get((unsigned int&)len);
      if(opcode >= NUM_OPCODES)
        sLog.outError("Unknown opcode %d", opcode);
      else
      {
        sLog.outLog("received %s", opcodeTable[opcode].name);
        Packet packet(data, len);
        PacketHandler *handler = handlerMap[peer];
        if(handler)
          (handler->*opcodeTable[opcode].handler)(packet);
        else
          sLog.outError("peer not found\n");
      }
    }
    peer->flushPackets();
  }

  void disconnect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, bool forced = false )
  {
    HandlerMapType::iterator itr = handlerMap.find(peer);
    if(itr != handlerMap.end())
    {
      delete itr->second;
      handlerMap.erase(itr);
    }
    else
      sLog.outError("peer not found\n");
  }

private:
  // TODO: rewrite all of this so that the packet handlers are stored in the peer class
  typedef std::map<TCPServerConnectedPeer*, PacketHandler*> HandlerMapType;
  HandlerMapType handlerMap;
};

NetHandler::NetHandler()
{
  localServer = new TCPServerConnection;
  tcpListener = new TCPServerListener;
  // init the net library
  TCPConnection::instance();
}

bool NetHandler::initialize()
{
  uint32 listenPort = sConfig.getIntValue(CONFIG_LOCALPORT);

  localServer->addListener(tcpListener);
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
  localServer->disconnect();
  delete localServer;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8