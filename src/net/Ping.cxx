/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "Ping.h"

/* system implementation headers */
#include <string.h>
#include <math.h>
#include <ctype.h>

/* common implementation headers */
#include "global.h"
#include "Protocol.h"
#include "TimeKeeper.h"
#include "bzfio.h"

// incessint rebuilding for current versioning
#include "version.h"


//
// PingPacket
//

const int		PingPacket::PacketSize = ServerIdPLen + 52;

PingPacket::PingPacket() : gameType(eTeamFFA), gameOptions(0),
				maxShots(1),
				shakeWins(0),
				shakeTimeout(0),
				maxPlayerScore(0),
				maxTeamScore(0),
				maxTime(0),
				maxPlayers(1),
				rogueCount(0),
				rogueMax(1),
				redCount(0),
				redMax(1),
				greenCount(0),
				greenMax(1),
				blueCount(0),
				blueMax(1),
				purpleCount(0),
				purpleMax(1),
				observerCount(0),
				observerMax(1)
{
  // do nothing
}

PingPacket::~PingPacket()
{
  // do nothing
}

bool			PingPacket::read(int fd, struct sockaddr_in* addr)
{
  char buffer[PacketSize], serverVersion[9];
  uint16_t len, code;

  // get packet
  int n = recvBroadcast(fd, buffer, sizeof(buffer), addr);
  if (n < 4)
    return false;

  // decode header
  void* buf = buffer;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);

  // make sure we got the rest of the message
  if (len != n - 4)
    return false;

  // check that it's a reply
  if (code != MsgPingCodeReply)
    return false;

  // unpack body of reply
  buf = unpack(buf, serverVersion);

  // compare protocol version against my protocol version.
  return (strncmp(serverVersion, getServerVersion(), 8) == 0);
}

bool			PingPacket::waitForReply(int fd,
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
    const float timeLeft = float(blockTime - (currentTime - startTime));
    struct timeval timeout;
    timeout.tv_sec = long(floorf(timeLeft));
    timeout.tv_usec = long(1.0e+6f * (timeLeft - floorf(timeLeft)));

    // wait for input
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET((unsigned int)fd, &read_set);
    int nfound = select(fd+1, (fd_set*)&read_set, NULL, NULL, &timeout);

    // if got a message read it.  if a ping packet and from right
    // sender then return success.
    if (nfound < 0)
      return false;
    if (nfound > 0 && read(fd, NULL))
      if (sourceAddr == from)
	return true;

    currentTime = TimeKeeper::getCurrent();
  } while (currentTime - startTime < blockTime);
  return false;
}

bool			PingPacket::write(int fd,
					const struct sockaddr_in* addr) const
{
  char buffer[PacketSize] = {0};
  void* buf = buffer;
  buf = nboPackUShort(buf, PacketSize - 4);
  buf = nboPackUShort(buf, MsgPingCodeReply);
  buf = pack(buf, getServerVersion());
  return sendBroadcast(fd, buffer, sizeof(buffer), addr) == sizeof(buffer);
}

bool			PingPacket::isRequest(int fd,
				struct sockaddr_in* addr)
{
  if (fd < 0) return false;
  char buffer[6];
  void *msg = buffer;
  uint16_t len, code;
  int size = recvBroadcast(fd, buffer, sizeof(buffer), addr);
  if (size < 2) return false;
  msg = nboUnpackUShort(msg, len);
  msg = nboUnpackUShort(msg, code);
  return code == MsgPingCodeRequest;
}

bool			PingPacket::sendRequest(int fd,
					const struct sockaddr_in* addr)
{
  if (fd < 0 || !addr) return false;
  char buffer[6];
  void *msg = buffer;
  msg = nboPackUShort(msg, 2);
  msg = nboPackUShort(msg, MsgPingCodeRequest);
  msg = nboPackUShort(msg, (uint16_t) 0);
  return sendBroadcast(fd, buffer, sizeof(buffer), addr) == sizeof(buffer);
}

void*			PingPacket::unpack(void* buf, char* version)
{
  buf = nboUnpackString(buf, version, 8);
  buf = serverId.unpack(buf);
  buf = sourceAddr.unpack(buf);
  buf = nboUnpackUShort(buf, gameType);
  buf = nboUnpackUShort(buf, gameOptions);
  buf = nboUnpackUShort(buf, maxShots);
  buf = nboUnpackUShort(buf, shakeWins);
  buf = nboUnpackUShort(buf, shakeTimeout);
  buf = nboUnpackUShort(buf, maxPlayerScore);
  buf = nboUnpackUShort(buf, maxTeamScore);
  buf = nboUnpackUShort(buf, maxTime);
  buf = nboUnpackUByte(buf, maxPlayers);
  buf = nboUnpackUByte(buf, rogueCount);
  buf = nboUnpackUByte(buf, rogueMax);
  buf = nboUnpackUByte(buf, redCount);
  buf = nboUnpackUByte(buf, redMax);
  buf = nboUnpackUByte(buf, greenCount);
  buf = nboUnpackUByte(buf, greenMax);
  buf = nboUnpackUByte(buf, blueCount);
  buf = nboUnpackUByte(buf, blueMax);
  buf = nboUnpackUByte(buf, purpleCount);
  buf = nboUnpackUByte(buf, purpleMax);
  buf = nboUnpackUByte(buf, observerCount);
  buf = nboUnpackUByte(buf, observerMax);
  return buf;
}

void*			PingPacket::pack(void* buf, const char* version) const
{
  buf = nboPackString(buf, version, 8);
  buf = serverId.pack(buf);
  buf = sourceAddr.pack(buf);
  buf = nboPackUShort(buf, gameType);
  buf = nboPackUShort(buf, gameOptions);
  buf = nboPackUShort(buf, maxShots);
  buf = nboPackUShort(buf, shakeWins);
  buf = nboPackUShort(buf, shakeTimeout);	// 1/10ths of second
  buf = nboPackUShort(buf, maxPlayerScore);
  buf = nboPackUShort(buf, maxTeamScore);
  buf = nboPackUShort(buf, maxTime);
  buf = nboPackUByte(buf, maxPlayers);
  buf = nboPackUByte(buf, rogueCount);
  buf = nboPackUByte(buf, rogueMax);
  buf = nboPackUByte(buf, redCount);
  buf = nboPackUByte(buf, redMax);
  buf = nboPackUByte(buf, greenCount);
  buf = nboPackUByte(buf, greenMax);
  buf = nboPackUByte(buf, blueCount);
  buf = nboPackUByte(buf, blueMax);
  buf = nboPackUByte(buf, purpleCount);
  buf = nboPackUByte(buf, purpleMax);
  buf = nboPackUByte(buf, observerCount);
  buf = nboPackUByte(buf, observerMax);
  return buf;
}

void			PingPacket::packHex(char* buf) const
{
  buf = packHex16(buf, gameType);
  buf = packHex16(buf, gameOptions);
  buf = packHex16(buf, maxShots);
  buf = packHex16(buf, shakeWins);
  buf = packHex16(buf, shakeTimeout);
  buf = packHex16(buf, maxPlayerScore);
  buf = packHex16(buf, maxTeamScore);
  buf = packHex16(buf, maxTime);
  buf = packHex8(buf, maxPlayers);
  buf = packHex8(buf, rogueCount);
  buf = packHex8(buf, rogueMax);
  buf = packHex8(buf, redCount);
  buf = packHex8(buf, redMax);
  buf = packHex8(buf, greenCount);
  buf = packHex8(buf, greenMax);
  buf = packHex8(buf, blueCount);
  buf = packHex8(buf, blueMax);
  buf = packHex8(buf, purpleCount);
  buf = packHex8(buf, purpleMax);
  buf = packHex8(buf, observerCount);
  buf = packHex8(buf, observerMax);
  *buf = 0;
}

void			PingPacket::unpackHex(char* buf)
{
  buf = unpackHex16(buf, gameType);
  buf = unpackHex16(buf, gameOptions);
  buf = unpackHex16(buf, maxShots);
  buf = unpackHex16(buf, shakeWins);
  buf = unpackHex16(buf, shakeTimeout);
  buf = unpackHex16(buf, maxPlayerScore);
  buf = unpackHex16(buf, maxTeamScore);
  buf = unpackHex16(buf, maxTime);
  buf = unpackHex8(buf, maxPlayers);
  buf = unpackHex8(buf, rogueCount);
  buf = unpackHex8(buf, rogueMax);
  buf = unpackHex8(buf, redCount);
  buf = unpackHex8(buf, redMax);
  buf = unpackHex8(buf, greenCount);
  buf = unpackHex8(buf, greenMax);
  buf = unpackHex8(buf, blueCount);
  buf = unpackHex8(buf, blueMax);
  buf = unpackHex8(buf, purpleCount);
  buf = unpackHex8(buf, purpleMax);
  buf = unpackHex8(buf, observerCount);
  buf = unpackHex8(buf, observerMax);
}

int			PingPacket::hex2bin(char d)
{
  switch (d) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'A':
    case 'a': return 10;
    case 'B':
    case 'b': return 11;
    case 'C':
    case 'c': return 12;
    case 'D':
    case 'd': return 13;
    case 'E':
    case 'e': return 14;
    case 'F':
    case 'f': return 15;
  }
  return 0;
}

char			PingPacket::bin2hex(int d)
{
  static const char* digit = "0123456789abcdef";
  return digit[d];
}

char*			PingPacket::packHex16(char* buf, uint16_t v)
{
  *buf++ = bin2hex((v >> 12) & 0xf);
  *buf++ = bin2hex((v >>  8) & 0xf);
  *buf++ = bin2hex((v >>  4) & 0xf);
  *buf++ = bin2hex( v	& 0xf);
  return buf;
}

char*			PingPacket::unpackHex16(char* buf, uint16_t& v)
{
  uint16_t d = 0;
  d = (d << 4) | hex2bin(*buf++);
  d = (d << 4) | hex2bin(*buf++);
  d = (d << 4) | hex2bin(*buf++);
  d = (d << 4) | hex2bin(*buf++);
  v = d;
  return buf;
}

char*			PingPacket::packHex8(char* buf, uint8_t v)
{
  *buf++ = bin2hex((v >>  4) & 0xf);
  *buf++ = bin2hex( v	& 0xf);
  return buf;
}

char*			PingPacket::unpackHex8(char* buf, uint8_t& v)
{
  uint16_t d = 0;
  d = (d << 4) | hex2bin(*buf++);
  d = (d << 4) | hex2bin(*buf++);
  v = (uint8_t)d;
  return buf;
}

void			 PingPacket::zeroPlayerCounts()
{
  rogueCount = 0;
  redCount = 0;
  greenCount = 0;
  blueCount = 0;
  purpleCount = 0;
  observerCount = 0;
}

// serialize packet to file -- note lack of error checking
// must write to a binary file if we use plain "pack"
void			 PingPacket::writeToFile (std::ostream& out) const
{
  if (!out) return;

  char buffer[PingPacket::PacketSize];
  void* buf = buffer;
  buf = nboPackUShort(buf, PingPacket::PacketSize - 4);
  buf = nboPackUShort(buf, MsgPingCodeReply);
  buf = pack(buf, getServerVersion());
  out.write(buffer,sizeof(buffer));
}

// de serialize packet from file
// must read from a binary file if we use plain "unpack"
bool			 PingPacket::readFromFile(std::istream& in)
{
  if (!in) return false;

  char buffer[PingPacket::PacketSize], serverVersion[9];
  uint16_t len, code;

  // get packet
  in.read(buffer, sizeof(buffer));
  if ((size_t)in.gcount() < sizeof(buffer)){
    return false;
  }

  // decode header
  void* buf = buffer;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);

  // make sure we got the rest of the message
  if (len != in.gcount() - 4){
    return false;
  }

  // unpack body of reply
  buf = unpack(buf, serverVersion);

  // compare protocol version against my protocol version.
  return (strncmp(serverVersion, getServerVersion(), 8) == 0);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
