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

// system headers
#include <errno.h>

const int udpBufSize = 128000;

bool NetHandler::initHandlers(struct sockaddr_in addr) {
  // udp socket
  int n;
  // we open a udp socket on the same port if alsoUDP
  if ((udpSocket = (int) socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      nerror("couldn't make udp connect socket");
      return false;
  }

  // increase send/rcv buffer size
  n = setsockopt(udpSocket, SOL_SOCKET, SO_SNDBUF, (SSOType) &udpBufSize,
		 sizeof(int));
  if (n < 0) {
    nerror("couldn't increase udp send buffer size");
    close(udpSocket);
    return false;
  }
  n = setsockopt(udpSocket, SOL_SOCKET, SO_RCVBUF, (SSOType) &udpBufSize,
		 sizeof(int));
  if (n < 0) {
      nerror("couldn't increase udp receive buffer size");
      close(udpSocket);
      return false;
  }
  if (bind(udpSocket, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
      nerror("couldn't bind udp listen port");
      close(udpSocket);
      return false;
  }
  // don't buffer info, send it immediately
  BzfNetwork::setNonBlocking(udpSocket);

#ifdef HAVE_ADNS_H
  AdnsHandler::startupResolver();
#endif

  return true;
}

void NetHandler::destroyHandlers() {
  for (int i = 0; i < maxHandlers; i++) {
    if (netPlayer[i])
      delete netPlayer[i];
  }
}

void NetHandler::setFd(fd_set *read_set, fd_set *write_set, int &maxFile) {
  for (int i = 0; i < maxHandlers; i++) {
    NetHandler *player = netPlayer[i];
    if (player) {
      _FD_SET(player->fd, read_set);
      if (player->outmsgSize > 0)
	_FD_SET(player->fd, write_set);
      if (player->fd > maxFile)
	maxFile = player->fd;
    }
  }
  _FD_SET(udpSocket, read_set);
  if (udpSocket > maxFile)
    maxFile = udpSocket;
}

int NetHandler::getUdpSocket() {
  return udpSocket;
}

int NetHandler::udpReceive(char *buffer, struct sockaddr_in *uaddr,
			   bool &udpLinkRequest) {
  AddrLen recvlen = sizeof(*uaddr);
  int n;
  int id;
  uint16_t len;
  uint16_t code;
  while (true) {
    n = recvfrom(udpSocket, buffer, MaxPacketLen, 0, (struct sockaddr *) uaddr,
		 &recvlen);
    if ((n < 0) || (n >= 4))
      break;
  }
  // Error receiving data (or no data)
  if (n < 0)
    return -1;

  // read head
  void *buf = buffer;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
  if (n == 6 && len == 2 && code == MsgPingCodeRequest)
    // Ping code request
    return -2;

  id = -1;
  int pi;
  udpLinkRequest = false;
  for (pi = 0; pi < maxHandlers; pi++)
    if (netPlayer[pi] && netPlayer[pi]->isMyUdpAddrPort(*uaddr)) {
      id = pi;
      break;
    }
  if (id == -1 && (len == 1) && (code == MsgUDPLinkRequest)) {
    // It is a UDP lInk Request ... try to match it
    uint8_t index;
    buf = nboUnpackUByte(buf, index);
    if ((index < maxHandlers) && netPlayer[index] && !netPlayer[index]->udpin)
      if (!memcmp(&netPlayer[index]->uaddr.sin_addr, &uaddr->sin_addr,
		  sizeof(uaddr->sin_addr))) {
	id = index;
	if (uaddr->sin_port)
	  netPlayer[index]->uaddr.sin_port = uaddr->sin_port;
	netPlayer[index]->udpin = true;
	udpLinkRequest = true;
	DEBUG2("Player %s [%d] inbound UDP up %s:%d actual %d\n",
	       netPlayer[index]->info->getCallSign(), index,
	       inet_ntoa(uaddr->sin_addr),
	       ntohs(netPlayer[index]->uaddr.sin_port),
	       ntohs(uaddr->sin_port));
      } else {
	DEBUG2
	  ("Player %s [%d] inbound UDP rejected %s:%d different IP \
than %s:%d\n",
	   netPlayer[index]->info->getCallSign(), index,
	   inet_ntoa(netPlayer[index]->uaddr.sin_addr),
	   ntohs(netPlayer[index]->uaddr.sin_port),
	   inet_ntoa(uaddr->sin_addr), ntohs(uaddr->sin_port));
      }
  }

  if (id == -1) {
    // no match, discard packet
    DEBUG2("uread() discard packet! %s:%d choices p(l) h:p",
	   inet_ntoa(uaddr->sin_addr), ntohs(uaddr->sin_port));
    for (pi = 0; pi < maxHandlers; pi++) {
      if (netPlayer[pi])
	DEBUG3(" %d(%d-%d) %s:%d", pi, netPlayer[pi]->udpin,
	       netPlayer[pi]->udpout,
	       inet_ntoa(netPlayer[pi]->uaddr.sin_addr),
	       ntohs(netPlayer[pi]->uaddr.sin_port));
    }
    DEBUG2("\n");
  } else {
    DEBUG4("Player %s [%d] uread() %s:%d len %d from %s:%d on %i\n",
	   netPlayer[id]->info->getCallSign(), id,
	   inet_ntoa(netPlayer[id]->uaddr.sin_addr),
	   ntohs(netPlayer[id]->uaddr.sin_port), n,
	   inet_ntoa(uaddr->sin_addr), ntohs(uaddr->sin_port),
	   udpSocket);
#ifdef NETWORK_STATS
    netPlayer[id]->countMessage(code, len, 0);
#endif
    if (code == MsgUDPLinkEstablished) {
      netPlayer[id]->udpout = true;
      DEBUG2("Player %s [%d] outbound UDP up\n",
	     netPlayer[id]->info->getCallSign(), id);
    }
  }
  return id;
}

bool NetHandler::isUdpFdSet(fd_set *read_set) {
  if (FD_ISSET(udpSocket, read_set)) {
    return true;
  }
  return false;
}

int NetHandler::udpSocket = -1;
NetHandler *NetHandler::netPlayer[maxHandlers] = {NULL};

NetHandler::NetHandler(PlayerInfo* _info, const struct sockaddr_in &clientAddr,
		       int _playerIndex, int _fd)
  : info(_info), playerIndex(_playerIndex), fd(_fd), tcplen(0), closed(false),
    outmsgOffset(0), outmsgSize(0), outmsgCapacity(0), outmsg(NULL),
    udpin(false), udpout(false), toBeKicked(false) {
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
    for (i = 0; i < MessageTypes; i++) {
      statMsg[i].count = 0;
      statMsg[i].code = 0;
      statMsg[i].maxSize = 0;
    }
    msgBytes[direction] = 0;
    perSecondTime[direction] = time;
    perSecondCurrentMsg[direction] = 0;
    perSecondMaxMsg[direction] = 0;
    perSecondCurrentBytes[direction] = 0;
    perSecondMaxBytes[direction] = 0;
  }
#endif
  if (!netPlayer[playerIndex])
    netPlayer[playerIndex] = this;
#ifdef HAVE_ADNS_H
  adns = new AdnsHandler(_playerIndex, (struct sockaddr *) &clientAddr);
#endif
}

NetHandler::~NetHandler() {
#ifdef NETWORK_STATS
  if (info->isPlaying())
    dumpMessageStats();
#endif
  // shutdown TCP socket
  shutdown(fd, 2);
  close(fd);

  delete[] outmsg;

  if (netPlayer[playerIndex] == this)
    netPlayer[playerIndex] = NULL;

#ifdef HAVE_ADNS_H
  delete adns;
#endif 
}

bool NetHandler::exists(int _playerIndex) {
  if (_playerIndex < 0)
    return false;
  if (_playerIndex >= maxHandlers)
    return false;
  return netPlayer[_playerIndex] != NULL;
}

NetHandler *NetHandler::getHandler(int _playerIndex) {
  if (_playerIndex < 0)
    return NULL;
  if (_playerIndex >= maxHandlers)
    return NULL;
  return netPlayer[_playerIndex];
}

bool NetHandler::isFdSet(fd_set *set) {
  if (FD_ISSET(fd, set)) {
    return true;
  }
  return false;
}

int NetHandler::send(const void *buffer, size_t length) {

  int n = ::send(fd, (const char *)buffer, (int)length, 0);
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
    outmsgSize += (int)length;
  }
  return 0;
}

void NetHandler::closing()
{
  closed = true;
}

int NetHandler::pwrite(const void *b, int l) {

  if (l == 0) {
    return 0;
  }
  
  if (closed)
    return 0;

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
    case MsgPlayerUpdateSmall:
    case MsgGMUpdate:
    case MsgLagPing:
      udpSend(b, l);
      return 0;
    }
  }

  // always sent MsgUDPLinkRequest over udp with udpSend
  if (code == MsgUDPLinkRequest) {
    udpSend(b, l);
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

RxStatus NetHandler::tcpReceive() {
  // read more data into player's message buffer

  // read header if we don't have it yet
  RxStatus e = receive(4);
  if (e != ReadAll)
    // if header not ready yet then skip the read of the body
    return e;

  // read body if we don't have it yet
  uint16_t len, code;
  void *buf = tcpmsg;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
  if (len > MaxPacketLen) {
    DEBUG1("Player [%d] sent huge packet length (len=%d), possible attack\n",
	   playerIndex, len);
    return ReadHuge;
  }
  e = receive(4 + (int) len);
  if (e != ReadAll)
    // if body not ready yet then skip the command handling
    return e;

  // clear out message
  tcplen = 0;
#ifdef NETWORK_STATS
  countMessage(code, len, 0);
#endif
  if (code == MsgUDPLinkEstablished) {
    udpout = true;
    DEBUG2("Player %s [%d] outbound UDP up\n", info->getCallSign(),
	   playerIndex);
  }
  return e;
}

RxStatus NetHandler::receive(size_t length) {
  RxStatus returnValue;
  if ((int)length <= tcplen)
    return ReadAll;
  int size = recv(fd, tcpmsg + tcplen, (int)length - tcplen, 0);
  if (size > 0) {
    tcplen += size;
    if (tcplen == (int)length)
      returnValue = ReadAll;
    else
      returnValue = ReadPart;
  } else if (size < 0) {
    // handle errors
    // get error code
    const int err = getErrno();

    // ignore if it's one of these errors
    if (err == EAGAIN || err == EINTR)
      returnValue = ReadPart;

    // if socket is closed then give up
    if (err == ECONNRESET || err == EPIPE) {
      returnValue = ReadReset;
    } else {
      returnValue = ReadError;
    }
  } else { // if (size == 0)
    returnValue = ReadDiscon;
  }
  return returnValue;
};

void *NetHandler::getTcpBuffer() {
  return tcpmsg;
};

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

void NetHandler::udpSend(const void *b, size_t l) {
#ifdef TESTLINK
  if ((random()%LINKQUALITY) == 0) {
    DEBUG1("Drop Packet due to Test\n");
    return;
  }
#endif
  sendto(udpSocket, (const char *)b, (int)l, 0, (struct sockaddr*)&uaddr,
	 sizeof(uaddr));
}

bool NetHandler::isMyUdpAddrPort(struct sockaddr_in _uaddr) {
  return udpin && (uaddr.sin_port == _uaddr.sin_port) &&
    (memcmp(&uaddr.sin_addr, &_uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0);
}

void NetHandler::getPlayerList(char *list) {
  sprintf(list, "[%d]%-16s: %s%s%s%s%s%s", playerIndex, info->getCallSign(),
	  peer.getDotNotation().c_str(),
#ifdef HAVE_ADNS_H
	  adns->getHostname() ? " (" : "",
	  adns->getHostname() ? adns->getHostname() : "",
	  adns->getHostname() ? ")" : "",
#else
	  "", "", "",
#endif
	  udpin ? " udp" : "",
	  udpout ? "+" : "");
}; 

const char *NetHandler::getTargetIP() {
  return peer.getDotNotation().c_str();
}

int NetHandler::sizeOfIP() {
   return peer.getIPVersion() == 4 ? 8 : 20; // 8 for IPv4, 20 for IPv6
}

void *NetHandler::packAdminInfo(void *buf) {
  buf = peer.pack(buf);
  return buf;
}

int NetHandler::whoIsAtIP(const std::string& IP) {
  int position = -1;
  NetHandler *player;
  for (int v = 0; v < maxHandlers; v++) {
    player = netPlayer[v];
    if (player && !strcmp(player->peer.getDotNotation().c_str(), IP.c_str())) {
      position = v;
      break;
    }
  }
  return position;
}

in_addr NetHandler::getIPAddress() {
  return uaddr.sin_addr;
}

void NetHandler::updateHandlers() {
#ifdef HAVE_ADNS_H
  for (int h = 0; h < maxHandlers; h++)
    if (netPlayer[h])
      netPlayer[h]->adns->checkDNSResolution();
#endif
}

const char *NetHandler::getHostname() {
#ifdef HAVE_ADNS_H
  return adns->getHostname();
#else
  return NULL;
#endif
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
