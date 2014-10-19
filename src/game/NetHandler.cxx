/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
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
#include "NetHandler.h"
// #include "MsgStrings.h"

// system headers
#include <errno.h>

#include "bzfsAPI.h"

const int udpBufSize = 128000;

std::vector<NetworkDataLogCallback*> logCallbacks;

void addNetworkLogCallback(NetworkDataLogCallback * cb )
{
  if (cb)
    logCallbacks.push_back(cb);
}

void removeNetworkLogCallback(NetworkDataLogCallback * cb )
{
  for ( unsigned int i = 0; i < (unsigned int)logCallbacks.size(); i++)
  {
    if ( logCallbacks[i] == cb )
    {
      logCallbacks.erase(logCallbacks.begin()+i);
      return;
    }
  }
}

void callNetworkDataLog ( bool send, bool udp,  const unsigned char *data, unsigned int size, void *param = NULL )
{
  for ( unsigned int i = 0; i < (unsigned int)logCallbacks.size(); i++)
    logCallbacks[i]->networkDataLog(send,udp,data,size,param);
}

bool NetHandler::pendingUDP = false;
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
    if (player && !player->closed) {
      FD_SET((unsigned int)player->fd, read_set);
      if (player->outmsgSize > 0) {
	FD_SET((unsigned int)player->fd, write_set);
      }
      if (player->fd > maxFile) {
	maxFile = player->fd;
      }
    }
  }

  FD_SET((unsigned int)udpSocket, read_set);
  if (udpSocket > maxFile) {
    maxFile = udpSocket;
  }

  for (int i = 0; i < maxHandlers; i++) {
    NetHandler *player = netPlayer[i];
    if (player)
    {
      if (player->ares)
	player->ares->setFd(read_set, write_set, maxFile);
    }
  }
}

int NetHandler::getUdpSocket() {
  return udpSocket;
}

int NetHandler::udpReceive(char *buffer, struct sockaddr_in *uaddr,
			   bool &udpLinkRequest) {
  AddrLen recvlen = sizeof(*uaddr);
  int n;
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
  const void *buf = buffer;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
//  if (code != MsgPlayerUpdateSmall && code != MsgPlayerUpdate)
//    logDebugMessage(1,"rcvd %s len %d\n",MsgStrings::strMsgCode(code),len);
  if (n == 6 && len == 2 && code == MsgPingCodeRequest)
    // Ping code request
    return -2;

  int id(-1);	  // player index of the matched player
  int pi;
  udpLinkRequest = false;
  for (pi = 0; pi < maxHandlers; pi++)
    if (netPlayer[pi] && !netPlayer[pi]->closed
	&& netPlayer[pi]->isMyUdpAddrPort(*uaddr)) {
      id = pi;
      break;
    }
  if (id == -1 && (len == 1) && (code == MsgUDPLinkRequest)) {
    // It is a UDP lInk Request ... try to match it
    uint8_t index;
    buf = nboUnpackUByte(buf, index);
    if ((index < maxHandlers) && netPlayer[index] && !netPlayer[index]->closed
	&& !netPlayer[index]->udpin) {
      if (!memcmp(&netPlayer[index]->uaddr.sin_addr, &uaddr->sin_addr,
		  sizeof(uaddr->sin_addr))) {
	id = index;
	if (uaddr->sin_port)
	  netPlayer[index]->uaddr.sin_port = uaddr->sin_port;
	netPlayer[index]->udpin = true;
	udpLinkRequest = true;
	logDebugMessage(2,"Player slot %d inbound UDP up %s:%d actual %d\n",
	       index,
	       inet_ntoa(uaddr->sin_addr),
	       ntohs(netPlayer[index]->uaddr.sin_port),
	       ntohs(uaddr->sin_port));
      } else {
	logDebugMessage(2,"Player slot %d inbound UDP rejected %s:%d different IP \
than %s:%d\n",
	   index,
	   inet_ntoa(netPlayer[index]->uaddr.sin_addr),
	   ntohs(netPlayer[index]->uaddr.sin_port),
	   inet_ntoa(uaddr->sin_addr), ntohs(uaddr->sin_port));
      }
    }
  }

  if (id == -1) {
    // no match, discard packet
    logDebugMessage(2,"uread() discard packet! %s:%d choices p(l) h:p",
	   inet_ntoa(uaddr->sin_addr), ntohs(uaddr->sin_port));
    for (pi = 0; pi < maxHandlers; pi++) {
      if (netPlayer[pi] && !netPlayer[pi]->closed)
	logDebugMessage(3," %d(%d-%d) %s:%d", pi, netPlayer[pi]->udpin,
	       netPlayer[pi]->udpout,
	       inet_ntoa(netPlayer[pi]->uaddr.sin_addr),
	       ntohs(netPlayer[pi]->uaddr.sin_port));
    }
    logDebugMessage(2,"\n");
  } else {
    logDebugMessage(4,"Player slot %d uread() %s:%d len %d from %s:%d on %i\n",
	   id,
	   inet_ntoa(netPlayer[id]->uaddr.sin_addr),
	   ntohs(netPlayer[id]->uaddr.sin_port), n,
	   inet_ntoa(uaddr->sin_addr), ntohs(uaddr->sin_port),
	   udpSocket);
#ifdef NETWORK_STATS
    netPlayer[id]->countMessage(code, len, 0);
#endif

    callNetworkDataLog (false, true, (const unsigned char*)buf,len,netPlayer[id]);

    if (code == MsgUDPLinkEstablished) {
      netPlayer[id]->udpout = true;
      logDebugMessage(2,"Player %d outbound UDP up\n", id);
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

void NetHandler::checkDNS(fd_set *read_set, fd_set *write_set) {
  for (int i = 0; i < maxHandlers; i++) {
    NetHandler *player = netPlayer[i];
    if (player)
    {
      if (player->ares)
	player->ares->process(read_set, write_set);
    }
  }
}

int NetHandler::udpSocket = -1;
NetHandler *NetHandler::netPlayer[maxHandlers] = {NULL};

NetHandler::NetHandler(PlayerInfo* _info, const struct sockaddr_in &clientAddr,
		       int _playerIndex, int _fd)
  : ares(new AresHandler(_playerIndex)), info(_info), playerIndex(_playerIndex), fd(_fd),
    tcplen(0), closed(false),
    outmsgOffset(0), outmsgSize(0), outmsgCapacity(0), outmsg(0),
    udpOutputLen(0), udpin(false), udpout(false), toBeKicked(false),
    time(_info->now)
{
  // store address information for player
  AddrLen addr_len( sizeof(clientAddr) );
  memcpy(&uaddr, &clientAddr, addr_len);
  peer = uaddr;

  // update player state
#ifdef NETWORK_STATS

  // initialize the inbound/outbound counters to zero
  msgBytes[0] = 0;
  perSecondTime[0] = time;
  perSecondCurrentMsg[0] = 0;
  perSecondMaxMsg[0] = 0;
  perSecondCurrentBytes[0] = 0;
  perSecondMaxBytes[0] = 0;

  msgBytes[1] = 0;
  perSecondTime[1] = time;
  perSecondCurrentMsg[1] = 0;
  perSecondMaxMsg[1] = 0;
  perSecondCurrentBytes[1] = 0;
  perSecondMaxBytes[1] = 0;

#endif
  if (!netPlayer[playerIndex])
    netPlayer[playerIndex] = this;
  ares->queryHostname((const struct sockaddr *) &clientAddr);
}

NetHandler::NetHandler(const struct sockaddr_in &_clientAddr, int _fd)
  : ares(0), info(0), playerIndex(-1), fd(_fd),
    tcplen(0), closed(false),
    outmsgOffset(0), outmsgSize(0), outmsgCapacity(0), outmsg(0),
    udpOutputLen(0), udpin(false), udpout(false), toBeKicked(false),
    time()
{
  // store address information for player
  AddrLen addr_len = sizeof(_clientAddr);
  memcpy(&uaddr, &_clientAddr, addr_len);
  peer = Address(uaddr);

#ifdef NETWORK_STATS

  // initialize the inbound/outbound counters to zero
  msgBytes[0] = 0;
  perSecondTime[0] = time;
  perSecondCurrentMsg[0] = 0;
  perSecondMaxMsg[0] = 0;
  perSecondCurrentBytes[0] = 0;
  perSecondMaxBytes[0] = 0;

  msgBytes[1] = 0;
  perSecondTime[1] = time;
  perSecondCurrentMsg[1] = 0;
  perSecondMaxMsg[1] = 0;
  perSecondCurrentBytes[1] = 0;
  perSecondMaxBytes[1] = 0;

#endif
}

void NetHandler::setPlayer ( PlayerInfo* p, int index )
{
  ares.reset(new AresHandler(index));

  playerIndex = index;
  info = p;

  if (!netPlayer[playerIndex])
    netPlayer[playerIndex] = this;
  ares->queryHostname((struct sockaddr *) &uaddr);
}

NetHandler::~NetHandler() {
#ifdef NETWORK_STATS
  if (info && info->isPlaying())
    dumpMessageStats();
#endif
  // shutdown TCP socket
  shutdown(fd, SHUT_RDWR);
  close(fd);

  delete[] outmsg;

  if (netPlayer[playerIndex] == this)
    netPlayer[playerIndex] = NULL;
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
    toBeKicked = true;
    if (err != EAGAIN ) {
      nerror("error on write EAGAIN");
      toBeKickedReason = "Write error EAGAIN";
    }
    else {
      nerror("error on write EINTR");
      toBeKickedReason = "Write error EINTR";
    }
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
      buffer  = (const void*)(((const char*)buffer) + n);
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
	if (info != NULL && playerIndex >= 0) {
	  logDebugMessage(2,"Player %s [%d] drop, unresponsive with %d bytes queued\n",
		info->getCallSign(), playerIndex, outmsgSize + length);
	}
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

  const void *buf = b;
  uint16_t len, code;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
//  if (code != MsgPlayerUpdateSmall && code != MsgPlayerUpdate && code != MsgGameTime)
//    logDebugMessage(1,"send %s len %d\n",MsgStrings::strMsgCode(code),len);
#ifdef NETWORK_STATS
  countMessage(code, len, 1);
#endif

  bool useUDP = false;

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
    case MsgGameTime:
		useUDP = true;
		break;
    }
  }

  callNetworkDataLog (true, useUDP, (const unsigned char*)b,len,this);

  // always sent UDP messages and MsgUDPLinkRequest over udp with udpSend
  if (useUDP || code == MsgUDPLinkRequest) {
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
  // read header if we don't have it yet
  RxStatus e = receive(4);
  if (e != ReadAll) {
    // if header not ready yet then skip the read of the body
    return e;
  }
  
  // read body if we don't have it yet
  uint16_t len, code;
  const void *buf = tcpmsg;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
//  logDebugMessage(1,"rcvd %s len %d\n",MsgStrings::strMsgCode(code),len);
  if (len > MaxPacketLen) {
    logDebugMessage(1,"Player [%d] sent huge packet length (len=%d), possible attack\n",
	   playerIndex, len);
    return ReadHuge;
  }
  // We haven't accounted for the header yet, so only ask receive() to get len (the payload) more bytes.
  e = receive(4 + (int) len);
  if (e != ReadAll) {
    // if body not ready yet then skip the command handling
    return e;
  }

  // clear out message
  tcplen = 0;
#ifdef NETWORK_STATS
  countMessage(code, len, 0);
#endif

  callNetworkDataLog (false, false, (const unsigned char*)buf,len,this);

  if (code == MsgUDPLinkEstablished) {
    udpout = true;
    logDebugMessage(2,"Player %s [%d] outbound UDP up\n", info->getCallSign(),
	   playerIndex);
  }
  return ReadAll;
}

RxStatus NetHandler::receive(size_t length, bool *retry) {
  RxStatus returnValue(ReadError);
  
  if (retry)
    *retry = false;

  // Degenerate case, becase a closed socket should not be sending data, but be paranoid and test for it anyway
  if (closed) return returnValue;
  
  if ((int)length <= tcplen) return ReadAll;
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
    if (err == EAGAIN || err == EINTR){
      if (retry)
	*retry = true;
      returnValue = ReadPart;
    } else if (err == ECONNRESET || err == EPIPE) {
      // if socket is closed then give up
      returnValue = ReadReset;
    } else {
      returnValue = ReadError;
    }
  } else { // if (size == 0)
    returnValue = ReadDiscon;
  }
  return returnValue;
}

void *NetHandler::getTcpBuffer() {
  return tcpmsg;
}

void NetHandler::flushUDP()
{
  if (udpOutputLen) {
    sendto(udpSocket, udpOutputBuffer, udpOutputLen, 0,
	   (struct sockaddr*)&uaddr, sizeof(uaddr));
    udpOutputLen = 0;
  }
}

void NetHandler::flushAllUDP() {
  for (int i = 0; i < maxHandlers; i++) {
    if (netPlayer[i] && !netPlayer[i]->closed)
      netPlayer[i]->flushUDP();
  }
  pendingUDP = false;
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

  // see if we've received a message of this type yet for this direction
  MessageCountMap::iterator i = msg[direction].find(code);

  if (i == msg[direction].end()) {
    // if not found, initialize stats
    struct MessageCount message;

    message.count = 1;
    message.maxSize = len;
    msg[direction][code] = message;    // struct copy

  } else {

    i->second.count++;
    if (i->second.maxSize < len) {
      i->second.maxSize = len;
    }
  }

  if (info->now - perSecondTime[direction] < 1.0f) {
    perSecondCurrentMsg[direction]++;
    perSecondCurrentBytes[direction] += len;
  } else {
    perSecondTime[direction] = info->now;
    if (perSecondMaxMsg[direction] < perSecondCurrentMsg[direction])
      perSecondMaxMsg[direction] = perSecondCurrentMsg[direction];
    if (perSecondMaxBytes[direction] < perSecondCurrentBytes[direction])
      perSecondMaxBytes[direction] = perSecondCurrentBytes[direction];
    perSecondCurrentMsg[direction] = 0;
    perSecondCurrentBytes[direction] = 0;
  }
}

void NetHandler::dumpMessageStats() {
  int total;
  int direction;

  logDebugMessage(1,"Player connect time: %f\n", info->now - time);

  for (direction = 0; direction <= 1; direction++) {
    total = 0;
    logDebugMessage(1,"Player messages %s:", direction ? "out" : "in");

    for (MessageCountMap::iterator i = msg[direction].begin();
	 i != msg[direction].end(); ++i) {
      logDebugMessage(1," %c%c:%u(%u)", i->first >> 8, i->first & 0xff,
	     i->second.count, i->second.maxSize);
      total += i->second.count;
    }

    logDebugMessage(1," total:%u(%u) ", total, msgBytes[direction]);
    logDebugMessage(1,"max msgs/bytes per second: %u/%u\n",
	perSecondMaxMsg[direction],
	perSecondMaxBytes[direction]);
  }
  fflush(stdout);
}
#endif

void NetHandler::udpSend(const void *b, size_t l) {
#ifdef TESTLINK
  if ((random()%LINKQUALITY) == 0) {
    logDebugMessage(1,"Drop Packet due to Test\n");
    return;
  }
#endif

  // setting sizeLimit to -1 will disable udp-buffering
  const int sizeLimit = (int)MaxPacketLen - 4;

  // If the new data does not fit into the buffer, send the buffer
  if (udpOutputLen && (udpOutputLen + l > (int)MaxPacketLen)) {
    sendto(udpSocket, udpOutputBuffer, udpOutputLen, 0,
	   (struct sockaddr*)&uaddr, sizeof(uaddr));
    udpOutputLen = 0;
  }
  // If nothing is buffered and new data will mostly fill it, send
  // without copying
  if (!udpOutputLen && ((int)l > sizeLimit)) {
    sendto(udpSocket, (const char *)b, (int)l, 0, (struct sockaddr*)&uaddr,
	   sizeof(uaddr));
  } else {
    // Buffer new data
    memcpy(&udpOutputBuffer[udpOutputLen], (const char *)b, l);
    udpOutputLen += (int)l;
    // Send buffer if is almost full
    if (udpOutputLen > sizeLimit) {
      sendto(udpSocket, udpOutputBuffer, udpOutputLen, 0,
	     (struct sockaddr*)&uaddr, sizeof(uaddr));
      udpOutputLen = 0;
    }
  }
  if (udpOutputLen)
    pendingUDP = true;
}

bool NetHandler::isMyUdpAddrPort(struct sockaddr_in _uaddr) {
  return udpin && (uaddr.sin_port == _uaddr.sin_port) &&
    (memcmp(&uaddr.sin_addr, &_uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0);
}

const std::string NetHandler::getPlayerHostInfo() {
  return TextUtils::format("%s%s%s%s%s%s",
	  peer.getDotNotation().c_str(),
	  getHostname() ? " (" : "",
	  getHostname() ? getHostname() : "",
	  getHostname() ? ")" : "",
	  udpin ? " udp" : "",
	  udpout ? "+" : "");
}

const char* NetHandler::getTargetIP() {
  /* peer->getDotNotation returns a temp variable that is not safe
   * to pass around.  we keep a copy in allocated memory for safety.
   */
  if (dotNotation.size() == 0)
    dotNotation = peer.getDotNotation();
  return dotNotation.c_str();
}

int NetHandler::sizeOfIP() {
  // IPv4 is 1 byte for type and 4 bytes for IP = 5
  // IPv6 is 1 byte for type and 16 bytes for IP = 17
  return peer.getIPVersion() == 4 ? 5 : 17;
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
    // FIXME: this is broken for IPv6
    // there can be multiple ascii formats for the same address
    if (player && !player->closed
	&& !strcmp(player->peer.getDotNotation().c_str(), IP.c_str())) {
      position = v;
      break;
    }
  }
  return position;
}

in_addr NetHandler::getIPAddress() {
  return uaddr.sin_addr;
}

const char *NetHandler::getHostname() {
  if (!ares)
    return NULL;
  return ares->getHostname();
}

bool NetHandler::reverseDNSDone()
{
  if (!ares)
    return false;

  AresHandler::ResolutionStatus status = ares->getStatus();
  return (status == AresHandler::Failed)
    || (status == AresHandler::HbASucceeded);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
