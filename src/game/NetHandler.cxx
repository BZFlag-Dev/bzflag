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

/* interface header */
#include "NetHandler.h"

NetHandler::NetHandler(PlayerInfo* _info, const struct sockaddr_in &clientAddr,
		       int _playerIndex, int _fd)
  : info(_info), playerIndex(_playerIndex), fd(_fd),
    outmsgOffset(0), outmsgSize(0), outmsgCapacity(0), outmsg(NULL),
    toBeKicked(false) {
  // store address information for player
  AddrLen addr_len = sizeof(clientAddr);
  memcpy(&uaddr, &clientAddr, addr_len);
  peer = Address(uaddr);

  // update player state
  time = TimeKeeper::getCurrent();
#ifdef NETWORK_STATS
  int i;
  struct MessageCount *statMsg;
  int direction;

  for (direction = 0; direction <= 1; direction++) {
    statMsg = msg[direction];
    for (i = 0; i < MessageTypes && statMsg[i].code != 0; i++) {
      statMsg[i].count = 0;
      statMsg[i].code = 0;
    }
    msgBytes[direction] = 0;
    perSecondTime[direction] = time;
    perSecondCurrentMsg[direction] = 0;
    perSecondMaxMsg[direction] = 0;
    perSecondCurrentBytes[direction] = 0;
    perSecondMaxBytes[direction] = 0;
  }
#endif
}

NetHandler::~NetHandler() {
  // shutdown TCP socket
  shutdown(fd, 2);
  close(fd);

  delete[] outmsg;
}

void NetHandler::fdSet(fd_set *read_set, fd_set *write_set, int &maxFile) {
  _FD_SET(fd, read_set);

  if (outmsgSize > 0)
    _FD_SET(fd, write_set);
  if (fd > maxFile)
    maxFile = fd;
}

int NetHandler::fdIsSet(fd_set *set) {
  return FD_ISSET(fd, set);
}

int NetHandler::send(const void *buffer, size_t length) {

  int n = ::send(fd, (const char *)buffer, length, 0);
  if (n >= 0) 
    return n;

  // get error code
  const int err = getErrno();

  // if socket is closed then give up
  if (err == ECONNRESET || err == EPIPE) {
    return -1;
  }

  // just try again later if it's one of these errors
  if (err != EAGAIN && err != EINTR) {
    // dump other errors and remove the player
    nerror("error on write");
    toBeKicked = true;
    toBeKickedReason = "Write error";
  }
  return 0;
}

void NetHandler::setUdpOut() {
  udpout = true;
  DEBUG2("Player %s [%d] outbound UDP up\n", info->getCallSign(), playerIndex);
}

bool NetHandler::setUdpIn(struct sockaddr_in &_uaddr) {
  if (udpin)
    return false;

  bool same = !memcmp(&uaddr.sin_addr,
		      &_uaddr.sin_addr,
		      sizeof(uaddr.sin_addr));
  if (same) {
    DEBUG2("Player %s [%d] inbound UDP up %s:%d actual %d\n",
	   info->getCallSign(), playerIndex, inet_ntoa(uaddr.sin_addr),
	   ntohs(uaddr.sin_port), ntohs(_uaddr.sin_port));
    if (_uaddr.sin_port) {
      uaddr.sin_port = _uaddr.sin_port;
      udpin = true;
    }
  } else {
    DEBUG2
      ("Player %s [%d] inbound UDP rejected %s:%d different IP than %s:%d\n",
       info->getCallSign(), playerIndex, inet_ntoa(uaddr.sin_addr),
       ntohs(uaddr.sin_port), inet_ntoa(_uaddr.sin_addr),
       ntohs(_uaddr.sin_port));
  }
  return same;
}

int NetHandler::bufferedSend(const void *buffer, size_t length) {
  // try flushing buffered data
  if (outmsgSize != 0) {
    const int n = send(outmsg + outmsgOffset, outmsgSize);
    if (n == -1) {
      return -1;
    }
    if (n > 0) {
      outmsgOffset += n;
      outmsgSize   -= n;
    }
  }
  // if the buffer is empty try writing the data immediately
  if ((outmsgSize == 0) && length > 0) {
    const int n = send(buffer, length);
    if (n == -1) {
      return -1;
    }
    if (n > 0) {
      buffer  = (void*)(((const char*)buffer) + n);
      length -= n;
    }
  }
  // write leftover data to the buffer
  if (length > 0) {
    // is there enough room in buffer?
    if (outmsgCapacity < outmsgSize + (int)length) {
      // double capacity until it's big enough
      int newCapacity = (outmsgCapacity == 0) ? 512 : outmsgCapacity;
      while (newCapacity < outmsgSize + (int)length)
	newCapacity <<= 1;

      // if the buffer is getting too big then drop the player.  chances
      // are the network is down or too unreliable to that player.
      // FIXME -- is 20kB too big?  too small?
      if (newCapacity >= 20 * 1024) {
	DEBUG2("Player %s [%d] drop, unresponsive with %d bytes queued\n",
	       info->getCallSign(), playerIndex, outmsgSize + length);
	toBeKicked = true;
	toBeKickedReason = "send queue too big";
	return 0;
      }

      // allocate memory
      char *newbuf = new char[newCapacity];

      // copy old data over
      memmove(newbuf, outmsg + outmsgOffset, outmsgSize);

      // cutover
      delete[] outmsg;
      outmsg	       = newbuf;
      outmsgOffset   = 0;
      outmsgCapacity = newCapacity;
    }

    // if we can't fit new data at the end of the buffer then move existing
    // data to head of buffer
    // FIXME -- use a ring buffer to avoid moving memory
    if (outmsgOffset + outmsgSize + (int)length > outmsgCapacity) {
      memmove(outmsg, outmsg + outmsgOffset, outmsgSize);
      outmsgOffset = 0;
    }

    // append data
    memmove(outmsg + outmsgOffset + outmsgSize, buffer, length);
    outmsgSize += length;
  }
  return 0;
}

int NetHandler::pwrite(const void *b, int l, int udpSocket) {

  if (l == 0) {
    return 0;
  }
  
  void *buf = (void *)b;
  uint16_t len, code;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
#ifdef NETWORK_STATS
  countMessage(code, len, 1);
#endif

  // Check if UDP Link is used instead of TCP, if so jump into udpSend
  if (udpout) {
    // only send bulk messages by UDP
    switch (code) {
    case MsgShotBegin:
    case MsgShotEnd:
    case MsgPlayerUpdate:
    case MsgGMUpdate:
    case MsgLagPing:
      udpSend(udpSocket, b, l);
      return 0;
    }
  }

  // always sent MsgUDPLinkRequest over udp with udpSend
  if (code == MsgUDPLinkRequest) {
    udpSend(udpSocket, b, l);
    return 0;
  }

  return bufferedSend(b, l);
}

int NetHandler::pflush(fd_set *set) {
  if (FD_ISSET(fd, set))
    return bufferedSend(NULL, 0);
  else
    return 0;
}

std::string NetHandler::reasonToKick() {
  std::string reason;
  if (toBeKicked) {
    reason = toBeKickedReason;
  }
  toBeKicked = false;
  return reason;
}

#ifdef NETWORK_STATS
void NetHandler::countMessage(uint16_t code, int len, int direction) {
  // add length of type and length
  len += 4;

  msgBytes[direction] += len;

  int i;
  struct MessageCount *msg1;
  msg1 = msg[direction];
  for (i = 0; i < MessageTypes && msg1[i].code != 0; i++)
    if (msg1[i].code == code)
      break;
  msg1[i].code = code;
  if (msg1[i].maxSize < len)
    msg1[i].maxSize = len;
  msg1[i].count++;

  TimeKeeper now = TimeKeeper::getCurrent();
  if (now - perSecondTime[direction] < 1.0f) {
    perSecondCurrentMsg[direction]++;
    perSecondCurrentBytes[direction] += len;
  } else {
    perSecondTime[direction] = now;
    if (perSecondMaxMsg[direction] < perSecondCurrentMsg[direction])
      perSecondMaxMsg[direction] = perSecondCurrentMsg[direction];
    if (perSecondMaxBytes[direction] < perSecondCurrentBytes[direction])
      perSecondMaxBytes[direction] = perSecondCurrentBytes[direction];
    perSecondCurrentMsg[direction] = 0;
    perSecondCurrentBytes[direction] = 0;
  }
}

void NetHandler::dumpMessageStats() {
  int i;
  struct MessageCount *msgStats;
  int total;
  int direction;

  DEBUG1("Player connect time: %f\n", TimeKeeper::getCurrent() - time);
  for (direction = 0; direction <= 1; direction++) {
    total = 0;
    DEBUG1("Player messages %s:", direction ? "out" : "in");
    msgStats = msg[direction];
    for (i = 0; i < MessageTypes && msgStats[i].code != 0; i++) {
      DEBUG1(" %c%c:%u(%u)", msgStats[i].code >> 8, msgStats[i].code & 0xff,
	     msgStats[i].count, msgStats[i].maxSize);
      total += msgStats[i].count;
    }
    DEBUG1(" total:%u(%u) ", total, msgBytes[direction]);
    DEBUG1("max msgs/bytes per second: %u/%u\n",
	perSecondMaxMsg[direction],
	perSecondMaxBytes[direction]);
  }
  fflush(stdout);
}
#endif

void NetHandler::udpSend(int udpSocket, const void *b, size_t l) {
#ifdef TESTLINK
  if ((random()%LINKQUALITY) == 0) {
    DEBUG1("Drop Packet due to Test\n");
    return;
  }
#endif
  sendto(udpSocket, (const char *)b, l, 0, (struct sockaddr*)&uaddr,
	 sizeof(uaddr));
}

bool NetHandler::isMyUdpAddrPort(struct sockaddr_in &_uaddr) {
  return udpin && (uaddr.sin_port == _uaddr.sin_port) &&
    (memcmp(&uaddr.sin_addr, &_uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0);
}

void NetHandler::UdpInfo() {
  DEBUG3(" %d(%d-%d) %s:%d", playerIndex, udpin, udpout,
	 inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
}

void NetHandler::debugUdpRead(int n,
			      struct sockaddr_in &_uaddr, int udpSocket) {
  DEBUG4("Player %s [%d] uread() %s:%d len %d from %s:%d on %i\n",
	 info->getCallSign(), playerIndex, inet_ntoa(uaddr.sin_addr),
	 ntohs(uaddr.sin_port), n, inet_ntoa(_uaddr.sin_addr),
	 ntohs(_uaddr.sin_port), udpSocket);
}

void NetHandler::getPlayerList(char *list) {
  sprintf(list, "[%d]%-16s: %s%s%s%s%s%s", playerIndex, info->getCallSign(),
	  peer.getDotNotation().c_str(),
#ifdef HAVE_ADNS_H
	  info->getHostname() ? " (" : "",
	  info->getHostname() ? info->getHostname() : "",
	  info->getHostname() ? ")" : "",
#else
	  "", "", "",
#endif
	  udpin ? " udp" : "",
	  udpout ? "+" : "");
}; 

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
