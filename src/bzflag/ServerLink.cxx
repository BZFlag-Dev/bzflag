/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#if defined(_WIN32)
  #pragma warning(disable: 4786)
#endif

#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>

#include "common.h"
#include "ServerLink.h"
#include "Pack.h"
#include "LocalPlayer.h"
#include "ErrorHandler.h"
#include "network.h"

// invoke persistent rebuilding for current version dates
#include "version.h"

#if !defined(_WIN32)
#include <unistd.h>
#include <errno.h>
#endif

#include "bzsignal.h"

#define UDEBUG if (UDEBUGMSG) printf
#define UDEBUGMSG false

#if defined(DEBUG)
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

#if defined(_WIN32)
DWORD ThreadID;		// Thread ID
HANDLE hConnected;	// "Connected" event
HANDLE hThread;		// Connection thread

typedef struct {
  int query;
  CNCTType* addr;
  int saddr;
} TConnect;

TConnect conn;

DWORD WINAPI ThreadConnect(LPVOID params)
{
  TConnect *conn = (TConnect*)params;
  if(connect(conn->query, conn->addr, conn->saddr) >= 0) {
    SetEvent(hConnected); // Connect successful
  }
  ExitThread(0);
  return 0;
}

#endif // !defined(_WIN32)

// FIXME -- packet recording
#include <stdio.h>
#include <stdlib.h>
#include "TimeKeeper.h"
FILE* packetStream = NULL;
TimeKeeper packetStartTime;
static const unsigned long serverPacket = 1;
static const unsigned long endPacket = 0;

ServerLink*		ServerLink::server = NULL;

ServerLink::ServerLink(const Address& serverAddress, int port, int) :
				state(SocketError),	// assume failure
				fd(-1)			// assume failure
{
  int i;
  char cServerVersion[128];

  struct protoent* p;
#if defined(_WIN32)
  BOOL off = FALSE;
#else
  int off = 0;
#endif

  // standard server has no special abilities;
  server_abilities = Nothing;

  // queue is empty

  urecvfd = -1;

  ulinkup = false;

  // initialize version to a bogus number
  strcpy(version, "BZFS0000");

  // open connection to server.  first connect to given port.
  // don't wait too long.
  int query = socket(AF_INET, SOCK_STREAM, 0);
  if (query < 0) return;

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = serverAddress;

  UDEBUG("Remote %s\n", inet_ntoa(addr.sin_addr));

  // for UDP, used later
  memcpy((unsigned char *)&usendaddr,(unsigned char *)&addr, sizeof(addr));

#if !defined(_WIN32)
  const bool okay = true;
  int fdMax = query;
  if (BzfNetwork::setNonBlocking(query) < 0) {
    close(query);
    return;
  }
  struct timeval timeout;
  fd_set write_set;
  fd_set read_set;
  int nfound;
  if (connect(query, (CNCTType*)&addr, sizeof(addr)) < 0) {
    if (getErrno() != EINPROGRESS) {
      close(query);
      return;
    }
    FD_ZERO(&write_set);
    FD_SET(query, &write_set);
    timeout.tv_sec = long(5);
    timeout.tv_usec = 0;
    nfound = select(fdMax + 1, NULL, (fd_set*)&write_set, NULL, &timeout);
    if (nfound <= 0) {
      close(query);
      return;
    }
    int       connectError;
    socklen_t errorLen = sizeof(int);
    if (getsockopt(query, SOL_SOCKET, SO_ERROR, &connectError, &errorLen)
	< 0) {
      close(query);
      return;
    }
    if (connectError != 0) {
      close(query);
      return;
    }
  }
#else // Connection timeout for Windows

  // Initialize structure
  conn.query = query;
  conn.addr = (CNCTType*)&addr;
  conn.saddr = sizeof(addr);

  // Create event
  hConnected = CreateEvent(NULL, FALSE, FALSE, "Connected Event");

  hThread=CreateThread(NULL, 0, ThreadConnect, &conn, 0, &ThreadID);
  const bool okay = (WaitForSingleObject(hConnected, 5000) == WAIT_OBJECT_0);
  if(!okay)
    TerminateThread(hThread ,1);

  // Do some cleanup
  CloseHandle(hConnected);
  CloseHandle(hThread);

#endif // !defined(_WIN32)
  if (!okay)
    goto done;

  // get server version and verify
#if !defined(_WIN32)
  FD_ZERO(&read_set);
  FD_SET(query, &read_set);
  timeout.tv_sec = long(5);
  timeout.tv_usec = 0;
  nfound = select(fdMax + 1, (fd_set*)&read_set, NULL, NULL, &timeout);
  if (nfound <= 0) {
    close(query);
    return;
  }
#endif // !defined(_WIN32)
  i = recv(query, (char*)version, 8, 0);
  if (i < 8)
    goto done;

  sprintf(cServerVersion,"Server version: '%8s'",version);
  printError(cServerVersion);

  // FIXME is it ok to try UDP always?
  server_abilities |= CanDoUDP;
  if (strcmp(version, getServerVersion()) != 0) {
    state = BadVersion;
    goto done;
  }

  // read local player's id
#if !defined(_WIN32)
  FD_ZERO(&read_set);
  FD_SET(query, &read_set);
  timeout.tv_sec = long(5);
  timeout.tv_usec = 0;
  nfound = select(fdMax + 1, (fd_set*)&read_set, NULL, NULL, &timeout);
  if (nfound <= 0) {
    close(query);
    return;
  }
#endif // !defined(_WIN32)
  i = recv(query, (char *) &id, sizeof(id), 0);
  if (i < (int) sizeof(id))
    return;
  if (id == 0xff) {
    state = Rejected;
    goto done;
  }

#if !defined(_WIN32)
  if (BzfNetwork::setBlocking(query) < 0) {
    close(query);
    return;
  }
#endif // !defined(_WIN32)

  fd = query;

  // turn on TCP no delay
  p = getprotobyname("tcp");
  if (p)
    setsockopt(fd, p->p_proto, TCP_NODELAY, (SSOType)&off, sizeof(off));  // changed

  state = Okay;
#if defined(NETWORK_STATS)
  bytesSent = 0;
  bytesReceived = 0;
  packetsSent = 0;
  packetsReceived = 0;
#endif

  // FIXME -- packet recording
  if (getenv("BZFLAGSAVE")) {
    packetStream = fopen(getenv("BZFLAGSAVE"), "w");
    packetStartTime = TimeKeeper::getCurrent();
  }
  return;

done:
  close(query);
}

ServerLink::~ServerLink()
{
  if (state != Okay) return;
  shutdown(fd, 2);
  close(fd);

  if (urecvfd >= 0)
    close(urecvfd);

  urecvfd = -1;
  ulinkup = false;

// FIXME -- packet recording
if (packetStream) {
  long dt = (long)((TimeKeeper::getCurrent() - packetStartTime) * 10000.0f);
  fwrite(&endPacket, sizeof(endPacket), 1, packetStream);
  fwrite(&dt, sizeof(dt), 1, packetStream);
  fclose(packetStream);
}

#if defined(NETWORK_STATS)
  const float dt = TimeKeeper::getCurrent() - startTime;
  std::cerr << "Server network statistics:" << std::endl;
  std::cerr << "  elapsed time    : " << dt << std::endl;
  std::cerr << "  bytes sent      : " << bytesSent << " (" <<
		(float)bytesSent / dt << "/sec)" << std::endl;
  std::cerr << "  packets sent    : " << packetsSent << " (" <<
		(float)packetsSent / dt << "/sec)" << std::endl;
  if (packetsSent != 0)
    std::cerr << "  bytes/packet    : " <<
		(float)bytesSent / (float)packetsSent << std::endl;
  std::cerr << "  bytes received  : " << bytesReceived << " (" <<
		(float)bytesReceived / dt << "/sec)" << std::endl;
  std::cerr << "  packets received: " << packetsReceived << " (" <<
		(float)packetsReceived / dt << "/sec)" << std::endl;
  if (packetsReceived != 0)
    std::cerr << "  bytes/packet    : " <<
		(float)bytesReceived / (float)packetsReceived << std::endl;
#endif
}

ServerLink*		ServerLink::getServer() // const
{
  return server;
}

void			ServerLink::setServer(ServerLink* _server)
{
  server = _server;
}

void			ServerLink::send(uint16_t code, uint16_t len,
							const void* msg)
{
  bool needForSpeed=false;
  if (state != Okay) return;
  char msgbuf[MaxPacketLen];
  void* buf = msgbuf;
  buf = nboPackUShort(buf, len);
  buf = nboPackUShort(buf, code);
  if (msg && len != 0) buf = nboPackString(buf, msg, len);

  if ((urecvfd>=0) && ulinkup ) {
    switch (code) {
      case MsgShotBegin:
      case MsgShotEnd:
      case MsgPlayerUpdate:
      case MsgGMUpdate:
      case MsgAudio:
      case MsgVideo:
      case MsgUDPLinkRequest:
      case MsgUDPLinkEstablished:
	needForSpeed=true;
	break;
    }
  }
  // MsgUDPLinkRequest always goes udp
  if (code == MsgUDPLinkRequest)
    needForSpeed=true;

  if (needForSpeed) {
#ifdef TESTLINK
    if ((random()%TESTQUALTIY) != 0)
#endif
    sendto(urecvfd, (const char *)msgbuf, (char*)buf - msgbuf, 0, &usendaddr, sizeof(usendaddr));
    // we don't care about errors yet
    return;
  }

  int r = ::send(fd, (const char*)msgbuf, len + 4, 0);
  (void)r; // silence g++
#if defined(_WIN32)
  if (r == SOCKET_ERROR) {
    const int e = WSAGetLastError();
    if (e == WSAENETRESET || e == WSAECONNABORTED ||
	e == WSAECONNRESET || e == WSAETIMEDOUT)
      state = Hungup;
    r = 0;
  }
#endif

#if defined(NETWORK_STATS)
  bytesSent += r;
  packetsSent++;
#endif
}

int			ServerLink::read(uint16_t& code, uint16_t& len,
						void* msg, int blockTime)
{

  code = MsgNull;
  len = 0;

  if (state != Okay) return -1;

  if ((urecvfd >= 0) /* && ulinkup */) {
    int n;

    AddrLen recvlen = sizeof(urecvaddr);
    unsigned char ubuf[MaxPacketLen];
    n = recvfrom(urecvfd, (char *)ubuf, MaxPacketLen, 0, &urecvaddr, (socklen_t*) &recvlen);
    if (n>0) {
      // unpack header and get message
      void* buf = ubuf;
      buf = nboUnpackUShort(buf, len);
      buf = nboUnpackUShort(buf, code);
      UDEBUG("<** UDP Packet Code %x Len %x\n",code, len);
      if (len > MaxPacketLen)
	return -1;
      memcpy((char *)msg,(char *)buf, len);
      return 1;
    }
    if (UDEBUGMSG) printError("Fallback to normal TCP receive");
    len = 0;
    code = MsgNull;

    blockTime = 0;
  }

  // block for specified period.  default is no blocking (polling)
  struct timeval timeout;
  timeout.tv_sec = blockTime / 1000;
  timeout.tv_usec = blockTime - 1000 * timeout.tv_sec;

  // only check server
  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(fd, &read_set);
  int nfound = select(fd+1, (fd_set*)&read_set, NULL, NULL,
			(struct timeval*)(blockTime >= 0 ? &timeout : NULL));
  if (nfound == 0) return 0;
  if (nfound < 0) return -1;

  // printError("<** TCP Packet Code Received %d", time(0));
  // FIXME -- don't really want to take the chance of waiting forever
  // on the remaining select() calls, but if the server and network
  // haven't been hosed then the data will get here soon.  And if the
  // server or network is down then we don't really care anyway.

  // get packet header -- keep trying until we get 4 bytes or an error
  char headerBuffer[4];


  int rlen = 0;
  rlen = recv(fd, (char*)headerBuffer, 4, 0);

  int tlen = rlen;
  while (rlen >= 1 && tlen < 4) {
    printError("ServerLink::read() loop");
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    nfound = select(fd+1, (fd_set*)&read_set, NULL, NULL, NULL);
    if (nfound == 0) continue;
    if (nfound < 0) return -1;
    rlen = recv(fd, (char*)headerBuffer + tlen, 4 - tlen, 0);
    if (rlen >= 0) tlen += rlen;
  }
  if (tlen < 4) return -1;
#if defined(NETWORK_STATS)
  bytesReceived += 4;
  packetsReceived++;
#endif

  // unpack header and get message
  void* buf = headerBuffer;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);

  //printError("Code is %02x",code);
  if (len > MaxPacketLen)
    return -1;
  if (len > 0)
    rlen = recv(fd, (char*)msg, int(len), 0);
  else
    rlen = 0;
#if defined(NETWORK_STATS)
  if (rlen >= 0) bytesReceived += rlen;
#endif
  if (rlen == int(len)) goto success;	// got whole thing

  // keep reading until we get the whole message
  tlen = rlen;
  while (rlen >= 1 && tlen < int(len)) {
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    nfound = select(fd+1, (fd_set*)&read_set, 0, 0, NULL);
    if (nfound == 0) continue;
    if (nfound < 0) return -1;
    rlen = recv(fd, (char*)msg + tlen, int(len) - tlen, 0);
    if (rlen >= 0) tlen += rlen;
#if defined(NETWORK_STATS)
  if (rlen >= 0) bytesReceived += rlen;
#endif
  }
  if (tlen < int(len)) return -1;

success:
// FIXME -- packet recording
if (packetStream) {
  long dt = (long)((TimeKeeper::getCurrent() - packetStartTime) * 10000.0f);
  fwrite(&serverPacket, sizeof(serverPacket), 1, packetStream);
  fwrite(&dt, sizeof(dt), 1, packetStream);
  fwrite(headerBuffer, 4, 1, packetStream);
  fwrite(msg, len, 1, packetStream);
}
  return 1;
}

void			ServerLink::sendEnter(PlayerType type,
						TeamColor team,
						const char* name,
						const char* email)
{
  if (state != Okay) return;
  char msg[PlayerIdPLen + 4 + CallSignLen + EmailLen];
  ::memset(msg, 0, sizeof(msg));
  void* buf = msg;
  buf = nboPackUShort(buf, uint16_t(type));
  buf = nboPackUShort(buf, uint16_t(team));
  ::memcpy(buf, name, ::strlen(name));
  buf = (void*)((char*)buf + CallSignLen);
  ::memcpy(buf, email, ::strlen(email));
  buf = (void*)((char*)buf + EmailLen);
  send(MsgEnter, sizeof(msg), msg);
}

void			ServerLink::sendCaptureFlag(TeamColor team)
{
  char msg[2];
  nboPackUShort(msg, uint16_t(team));
  send(MsgCaptureFlag, sizeof(msg), msg);
}

void			ServerLink::sendGrabFlag(int flagIndex)
{
  char msg[2];
  nboPackUShort(msg, uint16_t(flagIndex));
  send(MsgGrabFlag, sizeof(msg), msg);
}

void			ServerLink::sendDropFlag(const float* position)
{
  char msg[12];
  void* buf = msg;
  buf = nboPackVector(buf, position);
  send(MsgDropFlag, sizeof(msg), msg);
}

void			ServerLink::sendKilled(const PlayerId& killer, int reason,
								int shotId)
{
  char msg[PlayerIdPLen + 4];
  void* buf = msg;
  buf = nboPackUByte(buf, killer);
  buf = nboPackUShort(buf, int16_t(reason));
  buf = nboPackShort(buf, int16_t(shotId));
  send(MsgKilled, sizeof(msg), msg);
}


#ifndef BUILDING_BZADMIN
void			ServerLink::sendPlayerUpdate(Player* player)
{
  char msg[PlayerUpdatePLen];
  const float timeStamp = TimeKeeper::getCurrent() - TimeKeeper::getNullTime();
  void* buf = msg;
  buf = nboPackFloat(buf, timeStamp);
  buf = nboPackUByte(buf, player->getId());
  buf = player->pack(buf);
  send(MsgPlayerUpdate, sizeof(msg), msg);
}
#endif

void			ServerLink::sendBeginShot(const FiringInfo& info)
{
  char msg[FiringInfoPLen];
  void* buf = msg;
  buf = info.pack(buf);
  send(MsgShotBegin, sizeof(msg), msg);
}

void			ServerLink::sendEndShot(const PlayerId& source,
							int shotId, int reason)
{
  char msg[PlayerIdPLen + 4];
  void* buf = msg;
  buf = nboPackUByte(buf, source);
  buf = nboPackShort(buf, int16_t(shotId));
  buf = nboPackUShort(buf, uint16_t(reason));
  send(MsgShotEnd, sizeof(msg), msg);
}

void			ServerLink::sendAlive()
{
  send(MsgAlive, 0, NULL);
}

void			ServerLink::sendTeleport(int from, int to)
{
  char msg[4];
  void* buf = msg;
  buf = nboPackUShort(buf, uint16_t(from));
  buf = nboPackUShort(buf, uint16_t(to));
  send(MsgTeleport, sizeof(msg), msg);
}

void			ServerLink::sendTransferFlag(const PlayerId& from, const PlayerId& to)
{
  char msg[PlayerIdPLen*2];
  void* buf = msg;
  buf = nboPackUByte(buf, from);
  buf = nboPackUByte(buf, to);
  send(MsgTransferFlag, sizeof(msg), msg);
}

void			ServerLink::sendNewRabbit()
{
  send(MsgNewRabbit, 0, NULL);
}

void			ServerLink::sendPaused(bool paused)
{
  uint8_t p = paused;
  send(MsgPause, 1, &p);
}

void			ServerLink::sendUDPlinkRequest()
{
  if ((server_abilities & CanDoUDP) != CanDoUDP)
    return; // server does not support udp (future bzfls test)

  char msg[1];
  unsigned short localPort;
  void* buf = msg;

  struct sockaddr_in serv_addr;

  if ((urecvfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    return; // we cannot comply
  }
#if 1
  AddrLen addr_len = sizeof(serv_addr);
  if (getsockname(fd, (struct sockaddr*)&serv_addr, (socklen_t*) &addr_len) < 0) {
    printError("Error: getsockname() failed, cannot get TCP port?");
    return;
  }
  if (bind(urecvfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
    printError("Error: getsockname() failed, cannot get TCP port?");
    return;  // we cannot get udp connection, bail out
  }

#else
  // TODO if nobody complains kill this old port 17200 code
  for (int port=17200; port < 65000; port++) {
    ::memset((unsigned char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    if (bind(urecvfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == 0) {
      break;
    }
  }
#endif
  localPort = ntohs(serv_addr.sin_port);
  memcpy((char *)&urecvaddr,(char *)&serv_addr, sizeof(serv_addr));

  std::vector<std::string> args;
  char lps[10];
  sprintf(lps, "%d", localPort);
  args.push_back(lps);
  printError("Network: Created local UDP downlink port {1}", &args);

  buf = nboPackUByte(buf, id);

  if (BzfNetwork::setNonBlocking(urecvfd) < 0) {
    printError("Error: Unable to set NonBlocking for UDP receive socket");
  }

  send(MsgUDPLinkRequest, sizeof(msg), msg);
}

// heard back from server that we can send udp
void			ServerLink::enableOutboundUDP()
{
  ulinkup = true;
  printError("Server got our UDP, using UDP to server");
}
// confirm that server can send us UDP
void			ServerLink::confirmIncomingUDP()
{
  // This is really a hack. enableOutboundUDP will be setting this
  // but frequently the udp handshake will finish first so might as
  // well start with udp as soon as we can
  ulinkup = true;

  printError("Got server's UDP packet back, server using UDP");
  send(MsgUDPLinkEstablished, 0, NULL);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
