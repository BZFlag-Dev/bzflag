/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <string.h>
#include "PlayerLink.h"
#include "ServerLink.h"
#include "Pack.h"
#include "Player.h"
#include "ShotPath.h"
#include "network.h"
#include "ErrorHandler.h"
#include "Ping.h"

#if defined(ALPHA_RELEASE)
#define NETWORK_STATS
#endif

#if defined(NETWORK_STATS)
#include "bzfio.h"
#include "TimeKeeper.h"
static TimeKeeper	startTime;
static uint32_t		bytesSent;
static uint32_t		bytesReceived;
static uint32_t		packetsSent;
static uint32_t		packetsReceived;
#endif

// FIXME -- packet recording
#include <stdio.h>
#include "TimeKeeper.h"
extern FILE* packetStream;
extern TimeKeeper packetStartTime;
static const unsigned long playerPacket = 2;

PlayerLink*		PlayerLink::multicast = NULL;

PlayerLink::PlayerLink(const Address& multicastAddress, int port,
					int _ttl, const char* net_interface) :
					ttl(_ttl),
					useRelay(False),
					relay(NULL)
{
  fdIn = openMulticast(multicastAddress, port, NULL,
					ttl, net_interface, "r", &inAddr);
  fdOut = openMulticast(multicastAddress, port, NULL,
					ttl, net_interface, "w", &outAddr);
  if (fdIn == -1 || fdOut == -1) {
    closeMulticast(fdIn);
    closeMulticast(fdOut);
    state = SocketError;
    fdIn = -1;
    fdOut = -1;
  }
  else {
    state = Okay;
#if defined(NETWORK_STATS)
    bytesSent = 0;
    bytesReceived = 0;
    packetsSent = 0;
    packetsReceived = 0;
    startTime = TimeKeeper::getCurrent();
#endif
  }
}

PlayerLink::~PlayerLink()
{
  if (state == Okay) {
    closeMulticast(fdIn);
    closeMulticast(fdOut);
  }

#if defined(NETWORK_STATS)
  const float dt = TimeKeeper::getCurrent() - startTime;
  cerr << "Player network statistics:" << endl;
  cerr << "  elapsed time    : " << dt << endl;
  cerr << "  bytes sent      : " << bytesSent << " (" <<
		(float)bytesSent / dt << "/sec)" << endl;
  cerr << "  packets sent    : " << packetsSent << " (" <<
		(float)packetsSent / dt << "/sec)" << endl;
  if (packetsSent != 0)
    cerr << "  bytes/packet    : " <<
		(float)bytesSent / (float)packetsSent << endl;
  cerr << "  bytes received  : " << bytesReceived << " (" <<
		(float)bytesReceived / dt << "/sec)" << endl;
  cerr << "  packets received: " << packetsReceived << " (" <<
		(float)packetsReceived / dt << "/sec)" << endl;
  if (packetsReceived != 0)
    cerr << "  bytes/packet    : " <<
		(float)bytesReceived / (float)packetsReceived << endl;
#endif
}

void			PlayerLink::setUseRelay()
{
  useRelay = True;
}

void			PlayerLink::setRelay(ServerLink* _relay)
{
  // only use it if we've been instructed to do so
  if (!useRelay) return;

  // no going back once we set the relay
  if (state == Okay) {
    closeMulticast(fdIn);
    closeMulticast(fdOut);
    fdIn = -1;
    fdOut = -1;
  }

  // set the relay.  we're in error state if no relay or relay in error.
  relay = _relay;
  if (relay && relay->getState() == ServerLink::Okay)
    state = ServerRelay;
  else
    state = SocketError;
}

void			PlayerLink::setTTL(int _ttl)
{
  ttl = _ttl;
  if (state == Okay)
    setMulticastTTL(fdOut, ttl);
}

void			PlayerLink::send(uint16_t code, uint16_t len,
							const void* msg)
{
  if (state == Okay) {
    char msgbuf[MaxPacketLen];
    void* buf = msgbuf;
    buf = nboPackUShort(buf, len);
    buf = nboPackUShort(buf, code);
    if (msg && len != 0) buf = nboPackString(buf, msg, len);
    sendMulticast(fdOut, msgbuf, len + 4, &outAddr);
#if defined(NETWORK_STATS)
    bytesSent += len + 4;
    packetsSent++;
#endif
  }

  else if (state == ServerRelay) {
    relay->send(code, len, msg);
  }
}

int			PlayerLink::read(uint16_t& code, uint16_t& len,
						void* msg, int blockTime)
{
  code = MsgNull;
  len = 0;

  if (state == Okay) {
    // block for specified period.  default is no blocking (polling)
    struct timeval timeout;
    timeout.tv_sec = blockTime / 1000;
    timeout.tv_usec = blockTime - 1000 * timeout.tv_sec;

    // check for messages
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(fdIn, &read_set);
    int nfound = select(fdIn+1, (fd_set*)&read_set, NULL, NULL,
			(struct timeval*)(blockTime < 0 ? NULL : &timeout));
    if (nfound == 0) return 0;
    if (nfound < 0) return -1;

    // get packet header
    char buffer[MaxPacketLen];
    int msglen = recvMulticast(fdIn, buffer, MaxPacketLen, NULL);
    if (msglen < 4) {
      printError("incomplete read of player message header");
      return -2;
    }

    // unpack header
    void* buf = buffer;
    buf = nboUnpackUShort(buf, len);
    buf = nboUnpackUShort(buf, code);

    // if it's a ping packet then just throw it out.  this can happen
    // when a player broadcasts looking for a server.
    if (code == PingCodeOldReply ||
	code == PingCodeOldRequest ||
	code == PingCodeReply ||
	code == PingCodeRequest)
      return 0;

#if defined(NETWORK_STATS)
    bytesReceived += msglen;
    packetsReceived++;
#endif

    // copy message
    if (int(len) != msglen - 4) {
      printError("incomplete read of player message body");
      return -2;
    }
    memcpy(msg, buf, int(len));

// FIXME -- packet recording
if (packetStream) {
  long dt = (long)((TimeKeeper::getCurrent() - packetStartTime) * 10000.0f);
  fwrite(&playerPacket, sizeof(playerPacket), 1, packetStream);
  fwrite(&dt, sizeof(dt), 1, packetStream);
  fwrite(buffer, msglen, 1, packetStream);
}
    return 1;
  }

  if (state == ServerRelay)
    return relay->read(code, len, msg, blockTime);

  return -1;
}

void			PlayerLink::sendPlayerUpdate(const Player* player)
{
  if (state == SocketError) return;
  char msg[PlayerUpdatePLen];
  void* buf = msg;
  buf = player->getId().pack(buf);
  buf = player->pack(buf);
  send(MsgPlayerUpdate, sizeof(msg), msg);
}

PlayerLink*		PlayerLink::getMulticast() // const
{
  return multicast;
}

void			PlayerLink::setMulticast(PlayerLink* _multicast)
{
  multicast = _multicast;
}
