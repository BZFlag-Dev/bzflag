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
#include <math.h>
#include <ctype.h>
#include "global.h"
#include "Ping.h"
#include "Protocol.h"
#include "TimeKeeper.h"

//
// PingPacket
//

const int		PingPacket::PacketSize = PlayerIdPLen + 52;

PingPacket::PingPacket() : gameStyle(PlainGameStyle),
				maxPlayers(1),
				maxShots(1),
				rogueCount(0),
				redCount(0),
				greenCount(0),
				blueCount(0),
				purpleCount(0),
				rogueMax(1),
				redMax(1),
				greenMax(1),
				blueMax(1),
				purpleMax(1),
				shakeWins(0),
				shakeTimeout(0),
				maxPlayerScore(0),
				maxTeamScore(0),
				maxTime(0)
{
  // do nothing
}

PingPacket::~PingPacket()
{
  // do nothing
}

boolean			PingPacket::read(int fd, struct sockaddr_in* addr)
{
  char buffer[PacketSize], serverVersion[9];
  uint16_t len, code;

  if (recvMulticast(fd, buffer, sizeof(buffer), addr) != sizeof(buffer))
    return False;				// bad packet/different version

  void* buf = buffer;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
  if (code != PingCodeReply)
    return False;				// not a reply

  // get server version and compare against my version.  ignore
  // last character of version number.  that's only used to indicate
  // compatible client-side changes.
  buf = nboUnpackString(buf, serverVersion, 8);
  if (strncmp(serverVersion, ServerVersion, 7))
    return False;				// different version

  buf = serverId.unpack(buf);
  buf = sourceAddr.unpack(buf);
  buf = nboUnpackUShort(buf, gameStyle);
  buf = nboUnpackUShort(buf, maxPlayers);
  buf = nboUnpackUShort(buf, maxShots);
  buf = nboUnpackUShort(buf, rogueCount);
  buf = nboUnpackUShort(buf, redCount);
  buf = nboUnpackUShort(buf, greenCount);
  buf = nboUnpackUShort(buf, blueCount);
  buf = nboUnpackUShort(buf, purpleCount);
  buf = nboUnpackUShort(buf, rogueMax);
  buf = nboUnpackUShort(buf, redMax);
  buf = nboUnpackUShort(buf, greenMax);
  buf = nboUnpackUShort(buf, blueMax);
  buf = nboUnpackUShort(buf, purpleMax);
  buf = nboUnpackUShort(buf, shakeWins);
  buf = nboUnpackUShort(buf, shakeTimeout);
  buf = nboUnpackUShort(buf, maxPlayerScore);
  buf = nboUnpackUShort(buf, maxTeamScore);
  buf = nboUnpackUShort(buf, maxTime);
  return True;
}

boolean			PingPacket::waitForReply(int fd,
				const Address& from,
				int millisecondsToBlock)
{
  // block waiting on input.  if the incoming message is not from the
  // indicated source then ignore it and go back to waiting.  if the
  // incoming message is not a ping then ignore it and go back to waiting.
  float blockTime = 0.0001f * (float)millisecondsToBlock;
  TimeKeeper startTime = TimeKeeper::getCurrent();
  TimeKeeper currentTime = startTime;
  do {
    // prepare timeout
    const float timeLeft = blockTime - (currentTime - startTime);
    struct timeval timeout;
    timeout.tv_sec = long(floorf(timeLeft));
    timeout.tv_usec = long(1.0e+6f * (timeLeft - floorf(timeLeft)));

    // wait for input
    fd_set read_set;
    FD_ZERO(&read_set);
// turn off == signed/unsigned mismatch on win32
#if defined(_WIN32)
#pragma warning(disable: 4018)
#endif
    FD_SET(fd, &read_set);
#if defined(_WIN32)
#pragma warning(default: 4018)
#endif
    int nfound = select(fd+1, (fd_set*)&read_set, NULL, NULL, &timeout);

    // if got a message read it.  if a ping packet and from right
    // sender then return success.
    if (nfound < 0)
      return False;
    if (nfound > 0 && read(fd, NULL))
      if (sourceAddr == from)
	return True;

    currentTime = TimeKeeper::getCurrent();
  } while (currentTime - startTime < blockTime);
  return False;
}

boolean			PingPacket::write(int fd,
					const struct sockaddr_in* addr) const
{
  char buffer[PacketSize];
  void* buf = buffer;
  buf = nboPackUShort(buf, PacketSize);
  buf = nboPackUShort(buf, PingCodeReply);
  buf = nboPackString(buf, ServerVersion, 8);
  buf = serverId.pack(buf);
  buf = sourceAddr.pack(buf);
  buf = nboPackUShort(buf, gameStyle);
  buf = nboPackUShort(buf, maxPlayers);
  buf = nboPackUShort(buf, maxShots);
  buf = nboPackUShort(buf, rogueCount);
  buf = nboPackUShort(buf, redCount);
  buf = nboPackUShort(buf, greenCount);
  buf = nboPackUShort(buf, blueCount);
  buf = nboPackUShort(buf, purpleCount);
  buf = nboPackUShort(buf, rogueMax);
  buf = nboPackUShort(buf, redMax);
  buf = nboPackUShort(buf, greenMax);
  buf = nboPackUShort(buf, blueMax);
  buf = nboPackUShort(buf, purpleMax);
  buf = nboPackUShort(buf, shakeWins);
  buf = nboPackUShort(buf, shakeTimeout);	// 1/10ths of second
  buf = nboPackUShort(buf, maxPlayerScore);
  buf = nboPackUShort(buf, maxTeamScore);
  buf = nboPackUShort(buf, maxTime);
  return sendMulticast(fd, buffer, sizeof(buffer), addr) == sizeof(buffer);
}

boolean			PingPacket::isRequest(int fd,
				struct sockaddr_in* addr, int* minReplyTTL)
{
  if (fd < 0) return False;
  char buffer[6];
  void *msg = buffer;
  uint16_t len, code;
  int size = recvMulticast(fd, buffer, sizeof(buffer), addr);
  if (size < 2) return False;
  msg = nboUnpackUShort(msg, len);
  msg = nboUnpackUShort(msg, code);
  if (minReplyTTL && size == 6 && len == 2 && code == PingCodeRequest) {
    uint16_t ttl;
    msg = nboUnpackUShort(msg, ttl);
    *minReplyTTL = (int)ttl;
  }
  else if (minReplyTTL) {
    *minReplyTTL = 0;
  }
  return code == PingCodeRequest || code == PingCodeOldRequest;
}

boolean			PingPacket::sendRequest(int fd,
					const struct sockaddr_in* addr,
					int replyMinTTL)
{
  if (fd < 0 || !addr) return False;
  char buffer[6];
  void *msg = buffer;
  msg = nboPackUShort(msg, 2);
  msg = nboPackUShort(msg, PingCodeRequest);
  msg = nboPackUShort(msg, (uint16_t)replyMinTTL);
  return sendMulticast(fd, buffer, sizeof(buffer), addr) == sizeof(buffer);
}
