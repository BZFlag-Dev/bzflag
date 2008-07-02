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
  {"MSG_HANDSHAKE", NullHandler},
  {"CMSG_AUTH_REQUEST", NullHandler},
  {"DMSG_AUTH_FAIL", NullHandler},
  {"DMSG_AUTH_CHALLENGE", NullHandler},
  {"CMSG_AUTH_RESPONSE", NullHandler},
  {"DMSG_AUTH_SUCCESS", NullHandler},
  {"CMSG_REGISTER_GET_FORM", NullHandler},
  {"DMSG_REGISTER_FAIL", NullHandler},
  {"DMSG_REGISTER_SEND_FORM", NullHandler},
  {"CMSG_REGISTER_REQUEST", NullHandler},
  {"DMSG_REGISTER_CHALLENGE", NullHandler},
  {"CMSG_REGISTER_RESPONSE", NullHandler},
  {"DMSG_REGISTER_SUCCESS", NullHandler},
  {"SMSG_TOKEN_VALIDATE", NullHandler},
  {"DMSG_TOKEN_VALIDATE", NullHandler}
};

class TCPServerListener : public TCPServerDataPendingListener
{
public:
  TCPServerListener()
  {
  }

  bool connect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer )
  {
    return true;
  }

  void pending ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, unsigned int count )
  {
    tvPacketList& packets = peer->getPackets();
    for(tvPacketList::iterator itr = packets.begin(); itr != packets.end(); ++itr)
    {
      unsigned int len;
      unsigned char * data = (*itr).get(len);
    }
    peer->flushPackets();
  }

  void disconnect ( TCPServerConnection *connection, TCPServerConnectedPeer *peer, bool forced = false )
  {

  }
private:
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