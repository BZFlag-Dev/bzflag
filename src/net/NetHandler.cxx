/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "NetHandler.h"

#include "network.h"

namespace {
  typedef std::vector<NetworkDataLogCallback*> LogCallbacks;

  LogCallbacks& logCallbacks()
  {
    static LogCallbacks myCallbacks;
    return myCallbacks;
  }
}

void NetHandler::addNetworkLogCallback(NetworkDataLogCallback * cb )
{
  if (cb)
    logCallbacks().push_back(cb);
}

void NetHandler::removeNetworkLogCallback(NetworkDataLogCallback * cb )
{
  for (LogCallbacks::iterator itr = logCallbacks().begin(); itr != logCallbacks().end(); ++itr) {
    if ( *itr == cb ) {
      logCallbacks().erase(itr);
      break;
    }
  }
}

void NetHandler::callNetworkDataLog ( bool send, bool udp,  const unsigned char *data, size_t size, void* param )
{
  for (LogCallbacks::iterator cb = logCallbacks().begin();
       cb != logCallbacks().end(); ++cb) {
      (*cb)->networkDataLog(send, udp, data, size, param);
  }
}

// system headers
#include <errno.h>

#include "bzfsAPI.h"

// Are these size/limits reasonable?
const int udpBufSize = 128*1024;
const int tcpBufLimit = 64*1024;

// ------ Data ------
NetHandler::NetConnections NetHandler::netConnections;

SOCKET NetHandler::udpSocket = INVALID_SOCKET;

char NetHandler::udpmsg[MaxPacketLen];
int  NetHandler::udpLen  = 0;
int  NetHandler::udpRead = 0;

struct sockaddr_in NetHandler::lastUDPRxaddr;

bool NetHandler::pendingUDP = false;
TimeKeeper NetHandler::now = TimeKeeper::getCurrent();

//FIXME.. this belongs in network with the rest of the socket abstractions
void setNoDelay(SOCKET fd)
{
  // turn off TCP delay (collection).  we want packets sent immediately.
#if defined(_WIN32)
  BOOL on = TRUE;
#else
  int on = 1;
#endif
  struct protoent *p = getprotobyname("tcp");
  if (p && setsockopt(fd, p->p_proto, TCP_NODELAY, (SSOType)&on, sizeof(on)) < 0) {
    nerror("enabling TCP_NODELAY");
  }
}

bool NetHandler::initHandlers(struct sockaddr_in addr) 
{
  // udp socket
  int n;
  // we open a udp socket on the same port if alsoUDP
  if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    nerror("couldn't make udp connect socket");
    return false;
  }

  // increase send/rcv buffer size
  n = setsockopt(udpSocket, SOL_SOCKET, SO_SNDBUF, (SSOType) &udpBufSize,
		 sizeof(int));
  if (n < 0) {
    nerror("couldn't increase udp send buffer size");
    ::close(udpSocket);
    udpSocket = INVALID_SOCKET;
    return false;
  }
  n = setsockopt(udpSocket, SOL_SOCKET, SO_RCVBUF, (SSOType) &udpBufSize,
		 sizeof(int));
  if (n < 0) {
    nerror("couldn't increase udp receive buffer size");
    ::close(udpSocket);
    udpSocket = INVALID_SOCKET;
    return false;
  }
  if (bind(udpSocket, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
    nerror("couldn't bind udp listen port");
    ::close(udpSocket);
    udpSocket = INVALID_SOCKET;
    return false;
  }
  // don't buffer info, send it immediately
  BzfNetwork::setNonBlocking(udpSocket);

  return true;
}


// ------ Iterators ------
void NetHandler::setFd(fd_set *read_set, fd_set *write_set, int &maxFile) 
{
  NetConnections::const_iterator it;
  for (it = netConnections.begin(); it != netConnections.end(); it++) {
    NetHandler *handler = *it;
    if (!handler->closed) {
#if !defined(USE_THREADS) || !defined(HAVE_SDL)
      FD_SET(handler->fd, read_set);
#endif
      if (handler->outmsgSize > 0) {
	FD_SET(handler->fd, write_set);
      }
      if (handler->fd > maxFile) {
	maxFile = handler->fd;
      }
    }
  }

  FD_SET(udpSocket, read_set);
  if (udpSocket > maxFile) {
    maxFile = udpSocket;
  }

  for (it = netConnections.begin(); it != netConnections.end(); it++) {
    NetHandler *handler = *it;
    handler->ares.setFd(read_set, write_set, maxFile);
  }
}

// ------ UDP ------
bool NetHandler::isUdpFdSet(fd_set *read_set) 
{
  return (FD_ISSET(udpSocket, read_set));
}

SOCKET NetHandler::getUdpSocket() 
{
  return udpSocket;
}

int NetHandler::udpReceive(char *buffer, struct sockaddr_in *uaddr_,
			   NetHandler*& netHandler) 
{
  AddrLen recvlen = sizeof(*uaddr_);
  uint16_t len;
  uint16_t code;

  netHandler = NULL;

  if (udpLen == udpRead) {
    udpRead = 0;
    udpLen  = recvfrom(udpSocket, udpmsg, MaxPacketLen, 0,
		       (struct sockaddr *) &lastUDPRxaddr, &recvlen);
    // Error receiving data (or no data)
    if (udpLen < 0)
      return -1;
    logDebugMessage(4,"uread() len %d from %s:%d on %i\n",
	   udpLen,
	   inet_ntoa(lastUDPRxaddr.sin_addr),
	   ntohs(lastUDPRxaddr.sin_port),
	   udpSocket);
  }
  if ((udpLen - udpRead) < 4) {
    // No space for header :-(
    udpLen  = 0;
    udpRead = 0;
    return -1;
  }

  // read head
  void *buf = udpmsg + udpRead;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
  if ((udpLen - udpRead) < len + 4) {
    // No space for data :-(
    udpLen  = 0;
    udpRead = 0;
    return -1;
  }
  // copy the whole bunch of data
  memcpy(buffer, udpmsg + udpRead, len + 4);
  udpRead += len + 4;
  // copy the source identification
  memcpy(uaddr_, &lastUDPRxaddr, recvlen);

  if (len == 2 && code == MsgPingCodeRequest)
    // Ping code request
    return 0;
  
  if (len == 1 && code == MsgEchoRequest) //Same sorta thing
    return 0;

  NetConnections::const_iterator it;
  for (it = netConnections.begin(); it != netConnections.end(); it++)
    if ((*it)->isMyUdpAddrPort(*uaddr_, true)) {
      netHandler = *it;
      break;
    }

  if (!netHandler) {
    if ((len == 1) && (code == MsgUDPLinkRequest)) {
      return 0;
    }
    // no match, discard packet
    logDebugMessage(2,"uread() discard packet! %s:%d choices p(l) h:p",
	   inet_ntoa(uaddr_->sin_addr), ntohs(uaddr_->sin_port));
    for (it = netConnections.begin(); it != netConnections.end(); it++)
      if (!(*it)->closed) {
	logDebugMessage(3,"(%d-%d) %s:%d", (*it)->udpin,
	       (*it)->udpout,
	       inet_ntoa((*it)->uaddr.sin_addr),
	       ntohs((*it)->uaddr.sin_port));
    }
    logDebugMessage(2,"\n");
    return -1;
  }
#ifdef NETWORK_STATS
  netHandler->countMessage(code, len, 0);
#endif

  callNetworkDataLog(false,true,(unsigned char*)buf,len,netHandler);
 
  if (code == MsgUDPLinkEstablished) {
    netHandler->udpout = true;
  }
  return 0;
}

void NetHandler::flushAllUDP() 
{
  NetConnections::const_iterator it;
  for (it = netConnections.begin(); it != netConnections.end(); it++)
    if (!(*it)->closed)
      (*it)->flushUDP();
  pendingUDP = false;
}

// ------ DNS ------
void NetHandler::checkDNS(fd_set *read_set, fd_set *write_set) 
{
  std::list<NetHandler*>::const_iterator it;
  for (it = netConnections.begin(); it != netConnections.end(); it++)
    (*it)->ares.process(read_set, write_set);
}

void NetHandler::setCurrentTime(TimeKeeper tm)
{
  now = tm;
}


// Class (instance) methods

NetHandler::NetHandler(const struct sockaddr_in &clientAddr, int _fd)
  : uaddr(clientAddr)
  , fd(_fd)
  , peer(clientAddr)
  , udpLen2(0)
  , tcplen(0)
  , closed(false)
  , outmsgOffset(0)
  , outmsgSize(0)
  , outmsgCapacity(0)
  , outmsg(NULL)
  , udpOutputLen(0)
  , udpin(false)
  , udpout(false)
#ifdef NETWORK_STATS
    // note that 'time' isn't intrinsically network stats? but that's
    // what the legacy code did
  , time(now)
  , messageExchanged(false)
#endif
  , extraInfo(0)
{
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
  netConnections.push_back(this);
  ares.queryHostname((struct sockaddr *) &clientAddr);
}

NetHandler::NetHandler(ServerId const& serverAddr)
  : uaddr(serverAddr)
  , fd(INVALID_SOCKET)
  , peer(serverAddr)
  , udpLen2(0)
  , tcplen(0)
  , closed(false)
  , outmsgOffset(0)
  , outmsgSize(0)
  , outmsgCapacity(0)
  , outmsg(NULL)
  , udpOutputLen(0)
  , udpin(false)
  , udpout(false)
#ifdef NETWORK_STATS
    // note that 'time' isn't intrinsically network stats? but that's
    // what the legacy code did
  , time(now)
  , messageExchanged(false)
#endif
  , extraInfo(0)
{
  // Create a socket and bind connect it to the requested server.
  // Note that this will block until the connection is made (or
  // refused) but that should be acceptable in this case.
  fd = BzfNetwork::connect(serverAddr);
  
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
  netConnections.push_back(this);
  ares.queryHostname((struct sockaddr *) &serverAddr);
}

NetHandler::~NetHandler() {
  // Just in case, close it so callbacks get triggered
  closing("NetHandler dtor");

#ifdef NETWORK_STATS
  dumpMessageStats();
#endif

  // shutdown TCP socket
  ::shutdown(fd, SHUT_RDWR);
  ::close(fd);

  delete[] outmsg;

  netConnections.remove(this);
}

void NetHandler::setExtraInfo(void* info)
{
  extraInfo = info;
}

void* NetHandler::getExtraInfo() const
{
  return extraInfo;
}

void NetHandler::addCloseCallback(NetHandlerCloseCallback* cb)
{
  closeCallbacks.push_back(cb);
}

void NetHandler::removeCloseCallback(NetHandlerCloseCallback* cb)
{
  closeCallbacks.remove(cb);
}

bool NetHandler::hasData() const
{
  return (udpLen2 > 0) || (tcplen > 0);
}

NetHandler::Status NetHandler::getMsg(FramingFunction framer, char* buf, size_t& len, bool& isUDP)
{
  Status result(NoMsg);
  isUDP = false;

  // First try to get a UDP message
  size_t udplen(udpLen2);
  size_t dstlen(len);
  result = framer( udpmsg2, udplen, buf, dstlen);

  if (result == GoodMsg) {
    len = dstlen;
    udplen -= len;
    memmove(udpmsg2, udpmsg2 + len, udplen);
    udpLen2 = udplen;
    isUDP = true;
  } else if (result == EMsgTooLarge) {
    udpLen2 = 0;
  }

  // If necessary, try to get a TCP message
  if (result != GoodMsg) {
    result = framer( tcpmsg, tcplen, buf, len);

    if (result == GoodMsg) {
      isUDP = false;
      memmove(tcpmsg, tcpmsg + len, tcplen - len);
      tcplen -= len;
    } else if (result == EMsgTooLarge) {
      tcplen = 0;
    }
  }

//   // Disposition the resulting message
//   if (result == GoodMsg) {
//     // Log the data
//     callNetworkDataLog(false, isUDP, reinterpret_cast<unsigned char*>(buf), len, this);
//   }

  // All done
  return result;
}


bool NetHandler::isFdSet(fd_set *set)
{
  return (FD_ISSET(fd, set));
}

void NetHandler::flushUDP()
{
  if (udpOutputLen) {
    sendto(udpSocket, udpOutputBuffer, udpOutputLen, 0,
	   (struct sockaddr*)&uaddr, sizeof(uaddr));

    // Log the data
    callNetworkDataLog(true, true, (unsigned char const*)udpOutputBuffer, udpOutputLen, this);

    udpOutputLen = 0;
  }
}

int NetHandler::pwrite(const void *b, int l) 
{

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
    case MsgWhatTimeIsIt:
      useUDP = true;
	  break;
    }
  }

  // always sent MsgUDPLinkRequest over udp with udpSend
  if (useUDP || code == MsgUDPLinkRequest) {
    udpSend(b, l);
    return 0;
  }

  return bufferedSend(b, l);
}

int NetHandler::pflush() {
  return bufferedSend(NULL, 0);
}

void NetHandler::getPlayerList(char* list, size_t listSize) 
{
  snprintf(list, listSize, "%s%s%s%s%s%s",
	  peer.getDotNotation().c_str(),
	  getHostname() ? " (" : "",
	  getHostname() ? getHostname() : "",
	  getHostname() ? ")" : "",
	  udpin ? " udp" : "",
	  udpout ? "+" : "");
}

const char* NetHandler::getTargetIP() 
{
  /* peer->getDotNotation returns a temp variable that is not safe
   * to pass around.  we keep a copy in allocated memory for safety.
   */
  if (dotNotation.size() == 0)
    dotNotation = peer.getDotNotation();
  return dotNotation.c_str();
}

int NetHandler::sizeOfIP() 
{
  // IPv4 is 1 byte for type and 4 bytes for IP = 5
  // IPv6 is 1 byte for type and 16 bytes for IP = 17
  return peer.getIPVersion() == 4 ? 5 : 17;
}

void* NetHandler::packAdminInfo(void *buf) 
{
  buf = peer.pack(buf);
  return buf;
}

in_addr NetHandler::getIPAddress() 
{
  return uaddr.sin_addr;
}

const char* NetHandler::getHostname() 
{
  return ares.getHostname();
}

bool NetHandler::reverseDNSDone()
{
  AresHandler::ResolutionStatus status = ares.getStatus();
  return (status == AresHandler::Failed)
    || (status == AresHandler::HbASucceeded);
}

void NetHandler::closing(char const* reason)
{
//   std::cout << "NetHandler::closing(" << reason << ")\n";

  if ( !closed ) {
    closed = true;

    // Flush any pending writes
    flushAllUDP();
    pflush();

    // Let the application logic clean up after itself

    // NOTE: Because after using each callback we remove it from the
    // list, this special form of loop construct is used. It is not
    // susceptible to crashes arising from using invalidated
    // iterators, and it doesn't require fancy iterator management.
    CloseCallbacks::const_iterator itr;
    while ( (itr = closeCallbacks.begin()) != closeCallbacks.end() ) {
      (*itr)->callback(this, reason);

      // Because close is an irreversable operation (leading to
      // destruction), go ahead and unregister the the callback and
      // call its dispose() method
      NetHandlerCloseCallback* cb = *itr;
      removeCloseCallback(cb);
      cb->dispose();
    }
  } else {
//     std::cout << "......but it was already closed\n";
  }
}

void NetHandler::setUDPin(struct sockaddr_in *_uaddr)
{
  if (_uaddr->sin_port)
    uaddr.sin_port = _uaddr->sin_port;
  udpin = true;
}

bool NetHandler::isMyUdpAddrPort(struct sockaddr_in _uaddr,
				 bool checkPort) 
{
  if (closed)
    return false;

  if (checkPort != udpin)
    return false;

  if (memcmp(&uaddr.sin_addr, &_uaddr.sin_addr, sizeof(uaddr.sin_addr)))
    return false;

  if (!checkPort)
    return true;

  if (uaddr.sin_port == _uaddr.sin_port)
    return true;

  return false;
}

int NetHandler::send(const void *buffer, size_t length) 
{

  callNetworkDataLog(true, false, (unsigned char*)buffer, length, this);
  int n = ::send(fd, (const char *)buffer, (int)length, 0);
  if (n >= 0)
    return n;

  // get error code
  const int err( getErrno() );

  switch (err) {
    // These codes are "normal" and acceptable
  case EAGAIN:			// no data on a non-blocking socket
  case EINTR:			// interrupt before read
    break;
    // These codes represent a catastrophic connection error
  case ECONNRESET:		// connection closed when reading
  case EPIPE:
    // Broken connection. This guy is history
    closing("ECONNRESET/EPIPE");
    return -1;
    break;
    //Pretty much any other error is fatal as well. Keep this
    //separated, in case more granularity of reporting is desired at
    //a later date.
  case EBADF:			// bad file descriptor
  case ENOTCONN:		// unconnected socket (SNH)
  case ENOTSOCK:		// not a socket
  case ETIMEDOUT:		// operation timed out
  default:
    nerror("error on write");
    closing("Write error");
  }

  return 0;
}

int NetHandler::bufferedSend(const void *buffer, size_t length) 
{
  // try flushing buffered data
  if (outmsgSize != 0) {
    const int n = send(outmsg + outmsgOffset, outmsgSize);
    if (n == -1) {
      netConnections.remove(this);
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
      netConnections.remove(this);
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
      if (newCapacity >= tcpBufLimit) {
	netConnections.remove(this);
	return -2;
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

RxStatus NetHandler::receive(size_t length) 
{
  RxStatus returnValue;
  if (length <= tcplen)
    return ReadAll;
  int size = recv(fd, tcpmsg + tcplen, (int)length - tcplen, 0);
  if (size > 0) {

    // Log the data
    callNetworkDataLog(false, false, (unsigned char const*)tcpmsg+tcplen, size, this);

    tcplen += size;
    if (tcplen == length)
      returnValue = ReadAll;
    else
      returnValue = ReadPart;
  } else {
    // handle errors
    // get error code
    const int err = getErrno();

    // ignore if it's one of these errors
    if (err == EAGAIN || err == EINTR)
      returnValue = ReadPart;
    else if (err == ECONNRESET || err == EPIPE) {
      // if socket is closed then give up
      netConnections.remove(this);
      returnValue = ReadReset;
    } else {
      netConnections.remove(this);
      if (size == 0) {
	returnValue = ReadDiscon;
      } else {
	returnValue = ReadError;
      }
    }
  }
  return returnValue;
}

void NetHandler::tcpRead()
{
  // If this connection is marked as closed, don't bother reading anything else.
  if (closed) return;

  // This should only be called when data is available. Nevertheless,
  // the socket was set to non-blocking, so this should be safe even
  // if data isn't available
  size_t available = sizeof(tcpmsg) - tcplen;
  char* buf = tcpmsg + tcplen;

  ssize_t nRead = recv(fd, buf, available, 0);

  // On a good read, increment the size of the buffer
  if (nRead > 0) {
    // Log the data
    callNetworkDataLog(false, false, (unsigned char const*)buf, nRead, this);

    tcplen += nRead;
  } else if (nRead == 0) {
    // This value is only when the peer has gracefully closed the
    // connection with no data outstanding. In any event, we are done
    // with him.
    closing("Disconnected");
  } else {
    const int err( getErrno() );

    switch (err) {
      // These codes are "normal" and acceptable
    case EAGAIN:		// no data on a non-blocking socket
    case EINTR:			// interrupt before read
      break;
      // These codes represent a catastrophic connection error
    case ECONNRESET:		// connection closed when reading
    case EPIPE:
      // Broken connection. This guy is history
      closing("ECONNRESET/EPIPE");
      break;
      //Pretty much any other error is fatal as well. Keep this
      //separated, in case more granularity of reporting is desired at
      //a later date.
    case EBADF:			// bad file descriptor
    case ENOTCONN:		// unconnected socket (SNH)
    case ENOTSOCK:		// not a socket
    case ETIMEDOUT:		// operation timed out
    default:
      nerror("error on read");
      closing("Read error");
    }
  }
}

void NetHandler::udpSend(const void *b, size_t l) 
{
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

    // Log the data
    callNetworkDataLog(true, true, (unsigned char const*)udpOutputBuffer, udpOutputLen, this);

    udpOutputLen = 0;
  }
  // If nothing is buffered and new data will mostly fill it, send
  // without copying
  if (!udpOutputLen && ((int)l > sizeLimit)) {
    sendto(udpSocket, (const char *)b, (int)l, 0, (struct sockaddr*)&uaddr,
	   sizeof(uaddr));

    // Log the data
    callNetworkDataLog(true, true, (unsigned char const*)b, l, this);

  } else {
    // Buffer new data
    memcpy(&udpOutputBuffer[udpOutputLen], (const char *)b, l);
    udpOutputLen += (int)l;
    // Send buffer if is almost full
    if (udpOutputLen > sizeLimit) {
      sendto(udpSocket, udpOutputBuffer, udpOutputLen, 0,
	     (struct sockaddr*)&uaddr, sizeof(uaddr));

      // Log the data
      callNetworkDataLog(true, true, (unsigned char const*)udpOutputBuffer, udpOutputLen, this);

      udpOutputLen = 0;
    }
  }
  if (udpOutputLen)
    pendingUDP = true;
}

#ifdef NETWORK_STATS
void NetHandler::countMessage(uint16_t code, int len, int direction) 
{

  messageExchanged = true;

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

void NetHandler::dumpMessageStats() 
{
  int total;
  int direction;

  if (!messageExchanged)
    return;

  logDebugMessage(1,"Player connect time: %f\n", now - time);

  for (direction = 0; direction <= 1; direction++) {
    total = 0;
    logDebugMessage(1,"Player messages %s:", direction ? "out" : "in");

    for (MessageCountMap::iterator i = msg[direction].begin();
	 i != msg[direction].end(); i++) {
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

//=======================================================================

NetListener::NetListener()
{
  listenSocket = INVALID_SOCKET;
  maxFileDescriptors = 0;
  toRead = 0;
}

NetListener::~NetListener()
{

}

bool NetListener::listen (  Address const& serverAddress, unsigned short port, bool fallback )
{
  // init addr:port structure
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr = serverAddress;
  addr.sin_port = htons(port);

  // open well known service port
  listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket == INVALID_SOCKET) {
    nerror("couldn't make connect socket");
    return false;
  }

#ifdef SO_REUSEADDR
#  if defined(_WIN32)
  const BOOL optOn = TRUE;
  BOOL opt = optOn;
#  else
  const int optOn = 1;
  int opt = optOn;
#  endif

  /* set reuse address */
  opt = optOn;
  if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (SSOType)&opt, sizeof(opt)) < 0) {
    fail("serverStart: setsockopt SO_REUSEADDR");
    return false;
  }
#endif

  if (bind(listenSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
    if (fallback) {
      return this->listen(serverAddress, 0, false);
    } else {
      fail("couldn't bind connect socket");
      return false;
    }
  }

  if (::listen(listenSocket, 5) == -1) 
  {
    fail("couln't make connect socket queue");
    return false;
  }

  return true;
}

sockaddr_in NetListener::address()
{
  // init addr:port structure
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  if (listenSocket != INVALID_SOCKET) {
    AddrLen addrLen = sizeof(addr);
    getsockname(listenSocket, (struct sockaddr*)&addr, &addrLen);
  }
  return addr;
}

void NetListener::shutdown()
{
  if (listenSocket != INVALID_SOCKET) {
    ::shutdown(listenSocket, SHUT_RDWR);
    ::close(listenSocket);
    listenSocket = INVALID_SOCKET;
  }
}

int NetListener::update ( float waitTime )
{
  // If there isn't a listen socket, there can't be any connections,
  // so the rest of this is pointless
  if (listenSocket == INVALID_SOCKET)
    return -1;

  toRead = 0;

  // prepare select set
  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  NetHandler::setFd(&read_set, &write_set, maxFileDescriptors);

  // always listen for connections
  FD_SET((unsigned int)listenSocket, &read_set);
  if (listenSocket > maxFileDescriptors)
    maxFileDescriptors = listenSocket;

 // GameKeeper::Player::freeTCPMutex();
  struct timeval timeout;
  timeout.tv_sec = long(floorf(waitTime));
  timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));

  toRead = select(maxFileDescriptors+1, (fd_set*)&read_set, (fd_set*)&write_set, 0, &timeout);

  return toRead;
}

void NetListener::process()
{
  if (toRead < 0)
    return;

  // any new clients
  if (FD_ISSET(listenSocket, &read_set))
    accept();

  // Read the UDP socket if required. Since there is no connection,
  // there isn't necessarily a NetHandler associated with the sender,
  // but that will get sorted out.
  if (NetHandler::isUdpFdSet(&read_set)) {
    for (UdpReadCallbacks::const_iterator itr = udpReadCallbacks.begin();
	 itr != udpReadCallbacks.end();
	 ++itr) {
      (*itr)->callback();
    }
  }

  // Handle the DNS requests
  NetHandler::checkDNS(&read_set, &write_set);

  // Check each NetHandler to see if we need to read its TCP socket.
  // Invoke the required callbacks on the NetHandlers with data available
  for (NetHandler::NetConnections::const_iterator handler = NetHandler::netConnections.begin();
       handler != NetHandler::netConnections.end(); ++handler) {
    // A handler that has been closed does not need to be handled here
    if ( (*handler)->closed ) continue;

    // If there is data to read, please do so
    if ( (*handler)->isFdSet(&read_set) ) {
      (*handler)->tcpRead();
    }

    for (DataPendingCallbacks::const_iterator itr = dataPendingCallbacks.begin();
	 itr != dataPendingCallbacks.end(); ++itr) {
      bool anyPending(false);
      // If there is data pending in the buffers, invoke the callbacks
      // to consume it
      if ( (*handler)->hasData() ) {
	(*itr)->pendingRead( *handler );
	anyPending = true;
      }
      if ((*handler)->isFdSet(&write_set)) {
	(*itr)->pendingWrite(*handler);
	anyPending = true;
      }
      if (! anyPending) {
	(*itr)->idle(*handler);
      }
    }
  }
  toRead = 0;
}

void NetListener::accept()
{
  struct sockaddr_in clientAddr;
  AddrLen addr_len = sizeof(clientAddr);
  SOCKET fd = ::accept(listenSocket, (struct sockaddr*)&clientAddr, &addr_len);
  if (fd == INVALID_SOCKET)
  {
    nerror("accepting on wks");
    return;
  }
  // don't buffer info, send it immediately
  setNoDelay(fd);
  BzfNetwork::setNonBlocking(fd);

  int keepalive = 1, n;
  n = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (SSOType)&keepalive, sizeof(int));

  if (n < 0)
    nerror("couldn't set keepalive");

  // they aren't a player yet till they send us the connection string
  NetHandler* peer = new NetHandler(clientAddr, fd);

  bool kill = true;

  for (NewNetworkConnectionCallbacks::const_iterator itr = newConnectionCallbacks.begin();
       itr != newConnectionCallbacks.end(); 
       ++itr) {
    if ( (*itr)->accept(peer, fd) )
      kill = false;
  }

  if ( kill )
    delete(peer);
  else
  {
    FD_SET(fd, &read_set);
  }
}

void NetListener::addNewConnectionCallback ( NewNetworkConnectionCallback *handler )
{
  if (handler)
    newConnectionCallbacks.insert(handler);
}

void NetListener::removeNewConnectionCallback  ( NewNetworkConnectionCallback *handler )
{
  newConnectionCallbacks.erase(handler);
}

void NetListener::addDataPendingCallback( NetworkDataPendingCallback *handler )
{
  if (handler)
    dataPendingCallbacks.insert(handler);
}

void NetListener::removeDataPendingCallback( NetworkDataPendingCallback *handler )
{
  dataPendingCallbacks.erase(handler);
}

void NetListener::addUDPReadCallback ( NetworkUDPReadCallback* handler )
{
  if (handler)
    udpReadCallbacks.insert(handler);
}

void NetListener::removeUDPReadCallback  ( NetworkUDPReadCallback* handler )
{
  udpReadCallbacks.erase(handler);
}

void NetListener::fail(char const* reason)
{
  nerror(reason);
  ::close(listenSocket);
  listenSocket = INVALID_SOCKET;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
