/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
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
#include <sys/types.h>
#include <ctype.h>
#include "ServerLink.h"
#include "Pack.h"
#include "LocalPlayer.h"
#include "network.h"
#if !defined(_WIN32)
#include <unistd.h>
#include <signal.h>
#endif

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

#if !defined(_WIN32)

static void		timeout(int)
{
  signal(SIGALRM, SIG_IGN);
  alarm(0);
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

ServerLink::ServerLink(const Address& serverAddress, int port, int number) :
				state(SocketError),	// assume failure
				fd(-1)			// assume failure
{
  int i;
  struct protoent* p;
#if defined(_WIN32)
  BOOL on = TRUE;
#else
  int on = 1;
#endif

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
#if !defined(_WIN32)
  signal(SIGALRM, SIG_PF(timeout));
  alarm(5);
#endif // !defined(_WIN32)
  const boolean okay = (connect(query, (CNCTType*)&addr, sizeof(addr)) >= 0);
#if !defined(_WIN32)
  alarm(0);
  signal(SIGALRM, SIG_IGN);
#endif // !defined(_WIN32)
  if (!okay) goto done;

  // get server version and verify (last digit in version is ignored)
  i = recv(query, (char*)version, 8, 0);
  if (i < 8) goto done;
  if (strncmp(version, ServerVersion, 7) != 0) {
    state = BadVersion;
    goto done;
  }

  // get reconnect port
  i = recv(query, (char*)&addr.sin_port, sizeof(addr.sin_port), 0);
  if (i < (int)sizeof(addr.sin_port)) goto done;
  if (addr.sin_port == htons(0)) {
    state = Rejected;
    goto done;
  }

  // setup local player's id
  id.serverHost = serverAddress;
  id.port = addr.sin_port;
  id.number = htons(number);

  // reconnect at new port
  addr.sin_family = AF_INET;
  addr.sin_addr = serverAddress;
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) goto done;
  if (connect(fd, (CNCTType*)&addr, sizeof(addr)) < 0) {
    close(fd);
    fd = -1;
    goto done;
  }

/* not necessary on sgi
  int len = 8192;
  setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));
*/

  // turn on TCP no delay
  p = getprotobyname("tcp");
  if (p)
    setsockopt(fd, p->p_proto, TCP_NODELAY, (SSOType)&on, sizeof(on));

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

done:
  close(query);
}

ServerLink::~ServerLink()
{
  if (state != Okay) return;
  shutdown(fd, 2);
  close(fd);

// FIXME -- packet recording
if (packetStream) {
  long dt = (long)((TimeKeeper::getCurrent() - packetStartTime) * 10000.0f);
  fwrite(&endPacket, sizeof(endPacket), 1, packetStream);
  fwrite(&dt, sizeof(dt), 1, packetStream);
  fclose(packetStream);
}

#if defined(NETWORK_STATS)
  const float dt = TimeKeeper::getCurrent() - startTime;
  cerr << "Server network statistics:" << endl;
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
  if (state != Okay) return;
  char msgbuf[MaxPacketLen];
  void* buf = msgbuf;
  buf = nboPackUShort(buf, len);
  buf = nboPackUShort(buf, code);
  if (msg && len != 0) buf = nboPackString(buf, msg, len);
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

  // FIXME -- don't really want to take the chance of waiting forever
  // on the remaining select() calls, but if the server and network
  // haven't been hosed then the data will get here soon.  And if the
  // server or network is down then we don't really care anyway.

  // get packet header -- keep trying until we get 4 bytes or an error
  char headerBuffer[4];
  int rlen = recv(fd, (char*)headerBuffer, 4, 0);
  int tlen = rlen;
  while (rlen >= 1 && tlen < 4) {
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
  buf = id.pack(buf);
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
  buf = nboPackFloat(buf, position[0]);
  buf = nboPackFloat(buf, position[1]);
  buf = nboPackFloat(buf, position[2]);
  send(MsgDropFlag, sizeof(msg), msg);
}

void			ServerLink::sendKilled(const PlayerId& killer,
								int shotId)
{
  char msg[PlayerIdPLen + 2];
  void* buf = msg;
  buf = killer.pack(buf);
  buf = nboPackShort(buf, int16_t(shotId));
  send(MsgKilled, sizeof(msg), msg);
}

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
  buf = source.pack(buf);
  buf = nboPackShort(buf, int16_t(shotId));
  buf = nboPackUShort(buf, uint16_t(reason));
  send(MsgShotEnd, sizeof(msg), msg);
}

void			ServerLink::sendAlive(const float* pos,
						const float* forward)
{
  char msg[24];
  void* buf = msg;
  buf = nboPackFloat(buf, pos[0]);
  buf = nboPackFloat(buf, pos[1]);
  buf = nboPackFloat(buf, pos[2]);
  buf = nboPackFloat(buf, forward[0]);
  buf = nboPackFloat(buf, forward[1]);
  buf = nboPackFloat(buf, forward[2]);
  send(MsgAlive, sizeof(msg), msg);
}

void			ServerLink::sendTeleport(int from, int to)
{
  char msg[4];
  void* buf = msg;
  buf = nboPackUShort(buf, uint16_t(from));
  buf = nboPackUShort(buf, uint16_t(to));
  send(MsgTeleport, sizeof(msg), msg);
}

void			ServerLink::sendNewScore(int wins, int losses)
{
  char msg[4];
  void* buf = msg;
  buf = nboPackUShort(buf, uint16_t(wins));
  buf = nboPackUShort(buf, uint16_t(losses));
  send(MsgScore, sizeof(msg), msg);
}
