/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "bzfs.h"

const int udpBufSize = 128000;
bool    gotWorld = false;

// every ListServerReAddTime server add ourself to the list
// server again.  this is in case the list server has reset
// or dropped us for some reason.
static const float ListServerReAddTime = 30.0f * 60.0f;

static const float FlagHalfLife = 45.0f;
// do NOT change
int NotConnected = -1;
int InvalidPlayer = -1;

float speedTolerance = 1.125f;

#define MAX_FLAG_HISTORY (10)

struct ListServerLink {
    Address address;
    int port;
    int socket;
    enum MessageType {NONE,ADD,REMOVE} nextMessageType;
    std::string hostname;
    std::string pathname;
    enum Phase {CONNECTING, WRITTEN} phase;
};

// Command Line Options
CmdLineOptions *clOptions;

// server address to listen on
static Address serverAddress;
// well known service socket
static int wksSocket;
// udpSocket should also be on serverAddress
static int udpSocket;
bool handlePings = true;
static PingPacket pingReply;
// highest fd used
static int maxFileDescriptor;
// players list FIXME should be resized based on maxPlayers
PlayerInfo player[MaxPlayers];
// team info
TeamInfo team[NumTeams];
// num flags in flag list
int numFlags;
static int numFlagsInAir;
// types of extra flags allowed
std::vector<FlagType*> allowedFlags;
bool done = false;
// true if hit time/score limit
bool gameOver = true;
static int exitCode = 0;
// "real" players, i.e. do not count observers
uint16_t maxRealPlayers = MaxPlayers;
// players + observers
uint16_t maxPlayers = MaxPlayers;
// highest active id
uint16_t curMaxPlayers = 0;
int debugLevel = 0;

#ifdef HAVE_ADNS_H
adns_state adnsState;
#endif

static float maxWorldHeight = 0.0f;

static char hexDigest[50];

#ifdef TIMELIMIT
TimeKeeper gameStartTime;
bool countdownActive = false;
#endif
static TimeKeeper listServerLastAddTime;
static ListServerLink listServerLink;
static int listServerLinksCount = 0;

static WorldInfo *world = NULL;
static char *worldDatabase = NULL;
static uint32_t worldDatabaseSize = 0;

BasesList bases;

// FIXME - define a well-known constant for a null playerid in address.h?
// might be handy in other players, too.
// Client does not check for rabbit to be 255, but it still works
// because 255 should be > curMaxPlayers and thus no matchign player will
// be found.
static uint8_t rabbitIndex = NoPlayer;

WorldWeapons  wWeapons;

static TimeKeeper lastWorldParmChange;

// city geometry
static const float	AvenueSize =	2.0f * BoxBase;	// meters


void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message, bool fullBuffer=false);


static void getSpawnLocation( int playerId, float* pos, float *azimuth);

void removePlayer(int playerIndex, const char *reason, bool notify=true);
void resetFlag(int flagIndex);
static void dropFlag(int playerIndex, float pos[3]);


// util functions
int getPlayerIDByRegName(const std::string &regName)
{
  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].regName == regName)
      return i;
  }
  return -1;
}


bool hasPerm(int playerIndex, PlayerAccessInfo::AccessPerm right)
{
  return player[playerIndex].Admin || hasPerm(player[playerIndex].accessInfo, right);
}


// write an UDP packet down the link to the client
static int puwrite(int playerIndex, const void *b, int l)
{
  PlayerInfo& p = player[playerIndex];

#ifdef TESTLINK
  if ((random()%LINKQUALITY) == 0) {
    DEBUG1("Drop Packet due to Test\n");
    return 0;
  }
#endif
  return sendto(udpSocket, (const char *)b, l, 0, (struct sockaddr*)&p.uaddr, sizeof(p.uaddr));
}


static int prealwrite(int playerIndex, const void *b, int l)
{
  PlayerInfo& p = player[playerIndex];
  assert(p.fd != NotConnected && l > 0);

  // write as much data from buffer as we can in one send()
  const int n = send(p.fd, (const char *)b, l, 0);

  // handle errors
  if (n < 0) {
    // get error code
    const int err = getErrno();

    // just try again later if it's one of these errors
    if (err == EAGAIN || err == EINTR)
      return -1;

    // if socket is closed then give up
    if (err == ECONNRESET || err == EPIPE) {
      removePlayer(playerIndex, "ECONNRESET/EPIPE", false);
      return -1;
    }

    // dump other errors and remove the player
    nerror("error on write");
    player[playerIndex].toBeKicked = true;
    player[playerIndex].toBeKickedReason = "Write error";
    return -1;
  }

  return n;
}


// try to write stuff from the output buffer
static void pflush(int playerIndex)
{
  PlayerInfo& p = player[playerIndex];
  if (p.fd == NotConnected || p.outmsgSize == 0)
    return;

  const int n = prealwrite(playerIndex, p.outmsg + p.outmsgOffset, p.outmsgSize);
  if (n > 0) {
    p.outmsgOffset += n;
    p.outmsgSize   -= n;
  }
}


#ifdef NETWORK_STATS
void initPlayerMessageStats(int playerIndex)
{
  int i;
  struct MessageCount *msg;
  int direction;

  for (direction = 0; direction <= 1; direction++) {
    msg = player[playerIndex].msg[direction];
    for (i = 0; i < MessageTypes && msg[i].code != 0; i++) {
      msg[i].count = 0;
      msg[i].code = 0;
    }
    player[playerIndex].msgBytes[direction] = 0;
    player[playerIndex].perSecondTime[direction] = player[playerIndex].time;
    player[playerIndex].perSecondCurrentMsg[direction] = 0;
    player[playerIndex].perSecondMaxMsg[direction] = 0;
    player[playerIndex].perSecondCurrentBytes[direction] = 0;
    player[playerIndex].perSecondMaxBytes[direction] = 0;
  }
}


int countMessage(int playerIndex, uint16_t code, int len, int direction)
{
  int i;
  struct MessageCount *msg;

  // add length of type and length
  len += 4;
  player[playerIndex].msgBytes[direction] += len;
  msg = player[playerIndex].msg[direction];
  TimeKeeper now = TimeKeeper::getCurrent();
  for (i = 0; i < MessageTypes && msg[i].code != 0; i++)
    if (msg[i].code == code)
      break;
  msg[i].code = code;
  if (msg[i].maxSize < len)
    msg[i].maxSize = len;
  msg[i].count++;
  if (now - player[playerIndex].perSecondTime[direction] < 1.0f) {
    player[playerIndex].perSecondCurrentMsg[direction]++;
    player[playerIndex].perSecondCurrentBytes[direction] += len;
  }
  else {
    player[playerIndex].perSecondTime[direction] = now;
    if (player[playerIndex].perSecondMaxMsg[direction] <
	player[playerIndex].perSecondCurrentMsg[direction])
      player[playerIndex].perSecondMaxMsg[direction] =
	  player[playerIndex].perSecondCurrentMsg[direction];
    if (player[playerIndex].perSecondMaxBytes[direction] <
	player[playerIndex].perSecondCurrentBytes[direction])
      player[playerIndex].perSecondMaxBytes[direction] =
	  player[playerIndex].perSecondCurrentBytes[direction];
    player[playerIndex].perSecondCurrentMsg[direction] = 0;
    player[playerIndex].perSecondCurrentBytes[direction] = 0;
  }
  return (msg[i].count);
}


void dumpPlayerMessageStats(int playerIndex)
{
  int i;
  struct MessageCount *msg;
  int total;
  int direction;

  DEBUG1("Player connect time: %f\n",
      TimeKeeper::getCurrent() - player[playerIndex].time);
  for (direction = 0; direction <= 1; direction++) {
    total = 0;
    DEBUG1("Player messages %s:", direction ? "out" : "in");
    msg = player[playerIndex].msg[direction];
    for (i = 0; i < MessageTypes && msg[i].code != 0; i++) {
      DEBUG1(" %c%c:%u(%u)", msg[i].code >> 8, msg[i].code & 0xff,
	  msg[i].count, msg[i].maxSize);
      total += msg[i].count;
    }
    DEBUG1(" total:%u(%u) ", total, player[playerIndex].msgBytes[direction]);
    DEBUG1("max msgs/bytes per second: %u/%u\n",
	player[playerIndex].perSecondMaxMsg[direction],
	player[playerIndex].perSecondMaxBytes[direction]);
  }
  fflush(stdout);
}
#endif


static void pwrite(int playerIndex, const void *b, int l)
{
  PlayerInfo& p = player[playerIndex];
  if (p.fd == NotConnected || l == 0)
    return;

  void *buf = (void *)b;
  uint16_t len, code;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
#ifdef NETWORK_STATS
  countMessage(playerIndex, code, len, 1);
#endif

  // Check if UDP Link is used instead of TCP, if so jump into puwrite
  if (p.udpout) {
    // only send bulk messages by UDP
    switch (code) {
      case MsgShotBegin:
      case MsgShotEnd:
      case MsgPlayerUpdate:
      case MsgGMUpdate:
      case MsgLagPing:
	puwrite(playerIndex,b,l);
	return;
    }
  }

  // always sent MsgUDPLinkRequest over udp with puwrite
  if (code == MsgUDPLinkRequest) {
    puwrite(playerIndex,b,l);
    return;
  }

  // try flushing buffered data
  pflush(playerIndex);

  //DEBUG4("TCP write\n");
  // if the buffer is empty try writing the data immediately
  if (p.fd != NotConnected && p.outmsgSize == 0) {
    const int n = prealwrite(playerIndex, b, l);
    if (n > 0) {
      b  = (void*)(((const char*)b) + n);
      l -= n;
    }
  }

  // write leftover data to the buffer
  if (p.fd != NotConnected && l > 0) {
    // is there enough room in buffer?
    if (p.outmsgCapacity < p.outmsgSize + l) {
      // double capacity until it's big enough
      int newCapacity = (p.outmsgCapacity == 0) ? 512 : p.outmsgCapacity;
      while (newCapacity < p.outmsgSize + l)
	newCapacity <<= 1;

      // if the buffer is getting too big then drop the player.  chances
      // are the network is down or too unreliable to that player.
      // FIXME -- is 20kB too big?  too small?
      if (newCapacity >= 20 * 1024) {
	DEBUG2("Player %s [%d] drop, unresponsive with %d bytes queued\n",
	    p.callSign, playerIndex, p.outmsgSize + l);
	player[playerIndex].toBeKicked = true;
	player[playerIndex].toBeKickedReason = "send queue too big";
	return;
      }

      // allocate memory
      char *newbuf = new char[newCapacity];

      // copy old data over
      memmove(newbuf, p.outmsg + p.outmsgOffset, p.outmsgSize);

      // cutover
      delete[] p.outmsg;
      p.outmsg	       = newbuf;
      p.outmsgOffset   = 0;
      p.outmsgCapacity = newCapacity;
    }

    // if we can't fit new data at the end of the buffer then move existing
    // data to head of buffer
    // FIXME -- use a ring buffer to avoid moving memory
    if (p.outmsgOffset + p.outmsgSize + l > p.outmsgCapacity) {
      memmove(p.outmsg, p.outmsg + p.outmsgOffset, p.outmsgSize);
      p.outmsgOffset = 0;
    }

    // append data
    memmove(p.outmsg + p.outmsgOffset + p.outmsgSize, b, l);
    p.outmsgSize += l;
  }
}


static char sMsgBuf[MaxPacketLen];
char *getDirectMessageBuffer()
{
  return &sMsgBuf[2*sizeof(short)];
}


// FIXME? 4 bytes before msg must be valid memory, will get filled in with len+code
// usually, the caller gets a buffer via getDirectMessageBuffer(), but for example
// for MsgShotBegin the receiving buffer gets used directly
void directMessage(int playerIndex, uint16_t code, int len, const void *msg)
{
  if (player[playerIndex].fd == NotConnected)
    return;

  // send message to one player
  void *bufStart = (char *)msg - 2*sizeof(short);

  void *buf = bufStart;
  buf = nboPackUShort(buf, uint16_t(len));
  buf = nboPackUShort(buf, code);
  pwrite(playerIndex, bufStart, len + 4);
}


void broadcastMessage(uint16_t code, int len, const void *msg)
{
  // send message to everyone
  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i].state > PlayerInLimbo)
      directMessage(i, code, len, msg);
}


//
// global variable callback
//
static void onGlobalChanged(const std::string& msg, void*)
{
  std::string name  = msg;
  std::string value = BZDB.get(msg);
  void *bufStart = getDirectMessageBuffer();
  void *buf = nboPackUShort(bufStart, 1);
  buf = nboPackUByte(buf, name.length());
  buf = nboPackString(buf, name.c_str(), name.length());
  buf = nboPackUByte(buf, value.length());
  buf = nboPackString(buf, value.c_str(), value.length());
  broadcastMessage(MsgSetVar, (char*)buf - (char*)bufStart, bufStart);
}


static void sendUDPupdate(int playerIndex)
{
  // confirm inbound UDP with a TCP message
  directMessage(playerIndex, MsgUDPLinkEstablished, 0, getDirectMessageBuffer());
  // request/test outbound UDP with a UDP back to where we got client's packet
  directMessage(playerIndex, MsgUDPLinkRequest, 0, getDirectMessageBuffer());
}

static void createUDPcon(int t, int remote_port) {
  if (remote_port == 0)
    return;

  player[t].uaddr.sin_port = htons(remote_port);
  player[t].udpin = true;

  // init the queues
  player[t].uqueue = player[t].dqueue = NULL;
  player[t].lastRecvPacketNo = player[t].lastSendPacketNo = 0;

  // send client the message that we are ready for him
  sendUDPupdate(t);

  return;
}

static bool realPlayer(const PlayerId& id)
{
  return id<=curMaxPlayers && player[id].state>PlayerInLimbo;
}

static int lookupPlayer(const PlayerId& id)
{
  if (id == ServerPlayer)
    return id;

  if (!realPlayer(id))
    return InvalidPlayer;

  return id;
}

static int lookupFirstTeamFlag(int teamindex)
{
  for (int i = 0; i < numFlags; i++) {
    if (flag[i].flag.type->flagTeam == teamindex)
      return i;
  }
  // FIXME should never get here, throw?
  return -1;
}

static void setNoDelay(int fd)
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


// uread - interface to the UDP Receive routines
static int uread(int *playerIndex, int *nopackets, int& n,
		 unsigned char *ubuf, struct sockaddr_in &uaddr)
{
  //DEBUG4("Into UREAD\n");

  *nopackets = 0;

  PlayerInfo *pPlayerInfo;
  PlayerId pi;
  for (pi = 0, pPlayerInfo = player; pi < curMaxPlayers; pi++, pPlayerInfo++) {
    if ((pPlayerInfo->udpin) &&
	(pPlayerInfo->uaddr.sin_port == uaddr.sin_port) &&
	(memcmp(&pPlayerInfo->uaddr.sin_addr, &uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0)) {
      break;
    }
  }
  if (pi == curMaxPlayers) {
    unsigned short len, code;
    void *tmpbuf;
    tmpbuf = nboUnpackUShort(ubuf, len);
    tmpbuf = nboUnpackUShort(tmpbuf, code);
    if ((len == 1) && (code == MsgUDPLinkRequest)) {
      tmpbuf = nboUnpackUByte(tmpbuf, pi);
      if ((pi <= curMaxPlayers) && !player[pi].udpin) {
	if (memcmp(&player[pi].uaddr.sin_addr, &uaddr.sin_addr, sizeof(uaddr.sin_addr)) == 0) {
	  DEBUG2("Player %s [%d] inbound UDP up %s:%d actual %d\n",
	      player[pi].callSign, pi, inet_ntoa(player[pi].uaddr.sin_addr),
	      ntohs(player[pi].uaddr.sin_port),
	      ntohs(uaddr.sin_port));
	  createUDPcon(pi, ntohs(uaddr.sin_port));
	} else {
	  DEBUG2("Player %s [%d] inbound UDP rejected %s:%d different IP than %s:%d\n",
	      player[pi].callSign, pi, inet_ntoa(player[pi].uaddr.sin_addr),
	      ntohs(player[pi].uaddr.sin_port),
	      inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
	  pi = (PlayerId)curMaxPlayers;
	}
      } else {
	pi = (PlayerId)curMaxPlayers;
      }
    }
  }

  // get the packet
  n = recv(udpSocket, (char *)ubuf, MaxPacketLen, 0);
  if (pi == curMaxPlayers) {
    // no match, discard packet
    DEBUG2("uread() discard packet! %s:%d choices p(l) h:p", inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
    for (pi = 0, pPlayerInfo = player; pi < curMaxPlayers; pi++, pPlayerInfo++) {
      if (pPlayerInfo->fd != -1) {
	DEBUG3(" %d(%d-%d) %s:%d",
	    pi, pPlayerInfo->udpin, pPlayerInfo->udpout,
	    inet_ntoa(pPlayerInfo->uaddr.sin_addr),
	    ntohs(pPlayerInfo->uaddr.sin_port));
      }
    }
    DEBUG2("\n");
    *playerIndex = 0;
    return 0;
  }

  *playerIndex = pi;
  pPlayerInfo = &player[pi];
  DEBUG4("Player %s [%d] uread() %s:%d len %d from %s:%d on %i\n",
      pPlayerInfo->callSign, pi, inet_ntoa(pPlayerInfo->uaddr.sin_addr),
      ntohs(pPlayerInfo->uaddr.sin_port), n, inet_ntoa(uaddr.sin_addr),
      ntohs(uaddr.sin_port), udpSocket);

  if (n > 0) {
    *nopackets = 1;
    int clen = n;
    if (clen < 1024) {
      memcpy(pPlayerInfo->udpmsg,ubuf,clen);
      pPlayerInfo->udplen = clen;
    }
    return pPlayerInfo->udplen;
  }
  return 0;
}


static int pread(int playerIndex, int l)
{
  PlayerInfo& p = player[playerIndex];
  //DEBUG1("pread,playerIndex,l %i %i\n",playerIndex,l);
  if (p.fd == NotConnected || l == 0)
    return 0;

  // read more data into player's message buffer
  const int e = recv(p.fd, p.tcpmsg + p.tcplen, l, 0);

  // accumulate bytes read
  if (e > 0) {
    p.tcplen += e;
  } else if (e < 0) {
    // handle errors
    // get error code
    const int err = getErrno();

    // ignore if it's one of these errors
    if (err == EAGAIN || err == EINTR)
      return 0;

    // if socket is closed then give up
    if (err == ECONNRESET || err == EPIPE) {
      removePlayer(playerIndex, "ECONNRESET/EPIPE", false);
      return -1;
    }

    // dump other errors and remove the player
    nerror("error on read");
    removePlayer(playerIndex, "Read error", false);
    return -1;
  } else {
    // disconnected
    removePlayer(playerIndex, "Disconnected", false);
    return -1;
  }

  return e;
}


void sendFlagUpdate(int flagIndex = -1, int playerIndex = -1)
{
  void *buf, *bufStart = getDirectMessageBuffer();

  if (flagIndex != -1) {
    buf = nboPackUShort(bufStart,1);
    buf = nboPackUShort(buf, flagIndex);
    buf = flag[flagIndex].flag.pack(buf);
    if (playerIndex == -1)
      broadcastMessage(MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
    else
      directMessage(playerIndex, MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
  }
  else {
    buf = nboPackUShort(bufStart,0); //placeholder
    int cnt = 0;
    int length = sizeof(uint16_t);
    for (flagIndex = 0; flagIndex < numFlags; flagIndex++) {
	if (flag[flagIndex].flag.status != FlagNoExist) {
	  if ((length + sizeof(uint16_t) + FlagPLen) > MaxPacketLen - 2*sizeof(uint16_t)) {
	      nboPackUShort(bufStart, cnt);
	      if (playerIndex == -1)
		broadcastMessage(MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
	      else
		directMessage(playerIndex, MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
	      cnt = 0;
	      length = sizeof(uint16_t);
	      buf = nboPackUShort(bufStart,0); //placeholder
	  }

	  buf = nboPackUShort(buf, flagIndex);
	  buf = flag[flagIndex].flag.pack(buf);
	  length += sizeof(uint16_t)+FlagPLen;
	  cnt++;
	}
    }

    if (cnt > 0) {
	nboPackUShort(bufStart, cnt);
	if (playerIndex == -1)
	  broadcastMessage(MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
	else
	  directMessage(playerIndex, MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
    }
  }
}


void sendTeamUpdate(int playerIndex = -1, int teamIndex1 = -1, int teamIndex2 = -1)
{
  // If teamIndex1 is -1, send all teams
  // If teamIndex2 is -1, just send teamIndex1 team
  // else send both teamIndex1 and teamIndex2 teams

  void *buf, *bufStart = getDirectMessageBuffer();
  if (teamIndex1 == -1) {
    buf = nboPackUByte(bufStart, CtfTeams);
    for (int t = 0; t < CtfTeams; t++) {
      buf = nboPackUShort(buf, t);
      buf = team[t].team.pack(buf);
    }
  } else if (teamIndex2 == -1) {
    buf = nboPackUByte(bufStart, 1);
    buf = nboPackUShort(buf, teamIndex1);
    buf = team[teamIndex1].team.pack(buf);
  } else {
    buf = nboPackUByte(bufStart, 2);
    buf = nboPackUShort(buf, teamIndex1);
    buf = team[teamIndex1].team.pack(buf);
    buf = nboPackUShort(buf, teamIndex2);
    buf = team[teamIndex2].team.pack(buf);
  }

  if (playerIndex == -1)
    broadcastMessage(MsgTeamUpdate, (char*)buf - (char*)bufStart, bufStart);
  else
    directMessage(playerIndex, MsgTeamUpdate, (char*)buf - (char*)bufStart, bufStart);
}


static void sendPlayerUpdate(int playerIndex, int index)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  PlayerInfo *pPlayer = &player[playerIndex];
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(pPlayer->type));
  buf = nboPackUShort(buf, uint16_t(pPlayer->team));
  buf = nboPackUShort(buf, uint16_t(pPlayer->wins));
  buf = nboPackUShort(buf, uint16_t(pPlayer->losses));
  buf = nboPackUShort(buf, uint16_t(pPlayer->tks));
  buf = nboPackString(buf, pPlayer->callSign, CallSignLen);
  buf = nboPackString(buf, pPlayer->email, EmailLen);
  if (playerIndex == index) {
    // send all players info about player[playerIndex]
    for (int i = 0; i < curMaxPlayers; i++)
      if (player[i].state > PlayerInLimbo)
	directMessage(i, MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
  } else
    directMessage(index, MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
}


static void closeListServer()
{
  ListServerLink& link = listServerLink;
  if (link.socket != NotConnected) {
    close(link.socket);
    DEBUG4("Closing List server\n");
    link.socket = NotConnected;
  }
}

static void openListServer();

static void readListServer()
{
  ListServerLink& link = listServerLink;
  if (link.socket != NotConnected) {
    char    buf[256];
    recv(link.socket, buf, sizeof(buf), 0);
    closeListServer();
    if (link.nextMessageType != ListServerLink::NONE)
      // There was a pending request arrived after we write:
      // we should redo all the stuff
      openListServer();
  }
}

static void openListServer()
{
  ListServerLink& link = listServerLink;

  // start opening connection if not already doing so
  if (link.socket == NotConnected) {
    link.socket = socket(AF_INET, SOCK_STREAM, 0);
    DEBUG4("Opening List Server\n");
    if (link.socket == NotConnected) {
      return;
    }

    // set to non-blocking for connect
    if (BzfNetwork::setNonBlocking(link.socket) < 0) {
      closeListServer();
      return;
    }

    // Make our connection come from our serverAddress in case we have
    // multiple/masked IPs so the list server can verify us.
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = serverAddress;

    // assign the address to the socket
    if (bind(link.socket, (CNCTType*)&addr, sizeof(addr)) < 0) {
      closeListServer();
      return;
    }

    // connect.  this should fail with EINPROGRESS but check for
    // success just in case.
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(link.port);
    addr.sin_addr   = link.address;
    if (connect(link.socket, (CNCTType*)&addr, sizeof(addr)) < 0) {
#if defined(_WIN32)
#undef EINPROGRESS
#define EINPROGRESS EWOULDBLOCK
#endif
      if (getErrno() != EINPROGRESS) {
	nerror("connecting to list server");
	// TODO should try to lookup dns name again, but we don't have it anymore
	closeListServer();
      }
      else {
	listServerLink.phase = ListServerLink::CONNECTING;
      }
    } else {
      // shouldn't arrive here. Just in case, clean
      closeListServer();
    }
  }
}


// Sending message to list server foresee 3 stages
// 1. Connection
// 2. Writing
// 3. Reading & closing
// These phases are started via sendMessageToListServer
static void sendMessageToListServer(ListServerLink::MessageType type)
{
  // ignore if not publicizing
  if (!clOptions->publicizeServer)
    return;

  // start opening connections if not already doing so
  if (listServerLinksCount) {

    ListServerLink& link = listServerLink;

    // Open network connection only if closed
    if (link.socket == NotConnected)
      openListServer();

    // record next message to send.
    link.nextMessageType = type;
  }
}


static void sendMessageToListServerForReal()
{
  // ignore if link not connected
  ListServerLink& link = listServerLink;
  if (link.socket == NotConnected)
    return;

  char msg[4096] = "";

  if (link.nextMessageType == ListServerLink::ADD) {
    if (gameOver) {
      // pretend there are no players if the game is over.
      pingReply.rogueCount = 0;
      pingReply.redCount = 0;
      pingReply.greenCount = 0;
      pingReply.blueCount = 0;
      pingReply.purpleCount = 0;
      pingReply.observerCount = 0;
    } else {
      // update player counts in ping reply.
      pingReply.rogueCount = (uint8_t)team[0].team.size;
      pingReply.redCount = (uint8_t)team[1].team.size;
      pingReply.greenCount = (uint8_t)team[2].team.size;
      pingReply.blueCount = (uint8_t)team[3].team.size;
      pingReply.purpleCount = (uint8_t)team[4].team.size;
      pingReply.observerCount = (uint8_t)team[5].team.size;
    }

    // encode ping reply as ascii hex digits plus NULL
    char gameInfo[PingPacketHexPackedSize + 1];
    pingReply.packHex(gameInfo);

    // send ADD message (must send blank line)
    sprintf(msg, "GET %s?action=ADD&nameport=%s&version=%s&gameinfo=%s&title=%s HTTP/1.1\r\n"
      "Host: %s\r\nCache-Control: no-cache\r\n\r\n",
      link.pathname.c_str(), clOptions->publicizedAddress.c_str(),
      getServerVersion(), gameInfo,
      url_encode(clOptions->publicizedTitle).c_str(),
      link.hostname.c_str());
  }
  else if (link.nextMessageType == ListServerLink::REMOVE) {
    // send REMOVE (must send blank line)
    sprintf(msg, "GET %s?action=REMOVE&nameport=%s HTTP/1.1\r\n"
      "Host: %s\r\nCache-Control: no-cache\r\n\r\n",
      link.pathname.c_str(),
      clOptions->publicizedAddress.c_str(),
      link.hostname.c_str());
  }
  if (strlen(msg) > 0) {
    DEBUG3("%s\n",msg);
    if (send(link.socket, msg, strlen(msg), 0) == -1) {
      perror("List server send failed");
      DEBUG3("Unable to send to the list server!\n");
      closeListServer();
    } else {
      link.nextMessageType = ListServerLink::NONE;
      link.phase           = ListServerLink::WRITTEN;
    }
  } else {
    closeListServer();
  }
}


static void publicize()
{
  // hangup any previous list server sockets
  if (listServerLinksCount)
    closeListServer();

  // list server initialization
  listServerLinksCount	= 0;

  // parse the list server URL if we're publicizing ourself
  if (clOptions->publicizeServer) {
    // parse url
    std::string protocol, hostname, pathname;
    int port = 80;
    if (!BzfNetwork::parseURL(clOptions->listServerURL, protocol,
                              hostname, port, pathname))
      return;

    // ignore if not right protocol
    if (protocol != "http")
      return;

    // ignore if port is bogus
    if (port < 1 || port > 65535)
      return;

    // ignore if bad address
    Address address = Address::getHostAddress(hostname.c_str());
    if (address.isAny())
      return;

    // add to list
    listServerLink.address  = address;
    listServerLink.port     = port;
    listServerLink.socket   = NotConnected;
    listServerLink.pathname = pathname;
    listServerLink.hostname = hostname;
    listServerLinksCount++;

    // schedule message for list server
    sendMessageToListServer(ListServerLink::ADD);
    DEBUG3("Sent ADD message to list server\n");
  }
}


static bool serverStart()
{
#if defined(_WIN32)
  const BOOL optOn = TRUE;
  BOOL opt = optOn;
#else
  const int optOn = 1;
  int opt = optOn;
#endif
  maxFileDescriptor = 0;

  // init addr:port structure
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr = serverAddress;

  // look up service name and use that port if no port given on
  // command line.  if no service then use default port.
  if (!clOptions->useGivenPort) {
    struct servent *service = getservbyname("bzfs", "tcp");
    if (service) {
      clOptions->wksPort = ntohs(service->s_port);
    }
  }
  pingReply.serverId.port = addr.sin_port = htons(clOptions->wksPort);

  // open well known service port
  wksSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (wksSocket == -1) {
    nerror("couldn't make connect socket");
    return false;
  }
#ifdef SO_REUSEADDR
  /* set reuse address */
  opt = optOn;
  if (setsockopt(wksSocket, SOL_SOCKET, SO_REUSEADDR, (SSOType)&opt, sizeof(opt)) < 0) {
    nerror("serverStart: setsockopt SO_REUSEADDR");
    close(wksSocket);
    return false;
  }
#endif
  if (bind(wksSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
    if (!clOptions->useFallbackPort) {
      nerror("couldn't bind connect socket");
      close(wksSocket);
      return false;
    }

    // if we get here then try binding to any old port the system gives us
    addr.sin_port = htons(0);
    if (bind(wksSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
      nerror("couldn't bind connect socket");
      close(wksSocket);
      return false;
    }

    // fixup ping reply
    AddrLen addrLen = sizeof(addr);
    if (getsockname(wksSocket, (struct sockaddr*)&addr, &addrLen) >= 0)
      pingReply.serverId.port = addr.sin_port;

    // fixup publicized name will want it here later
    clOptions->wksPort = ntohs(addr.sin_port);
  }

  if (listen(wksSocket, 5) == -1) {
    nerror("couldn't make connect socket queue");
    close(wksSocket);
    return false;
  }
  // udp socket
  int n;
  // we open a udp socket on the same port if alsoUDP
  if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      nerror("couldn't make udp connect socket");
      return false;
  }

    // increase send/rcv buffer size
  n = setsockopt(udpSocket,SOL_SOCKET,SO_SNDBUF,(SSOType)&udpBufSize,sizeof(int));
  if (n < 0) {
      nerror("couldn't increase udp send buffer size");
      close(wksSocket);
      close(udpSocket);
      return false;
  }

  n = setsockopt(udpSocket,SOL_SOCKET,SO_RCVBUF,(SSOType)&udpBufSize,sizeof(int));
  if (n < 0) {
      nerror("couldn't increase udp receive buffer size");
      close(wksSocket);
      close(udpSocket);
      return false;
  }
  addr.sin_port = htons(clOptions->wksPort);
  if (bind(udpSocket, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
      nerror("couldn't bind udp listen port");
      close(wksSocket);
      close(udpSocket);
      return false;
  }
  // don't buffer info, send it immediately
  BzfNetwork::setNonBlocking(udpSocket);

  for (int i = 0; i < MaxPlayers; i++) {	// no connections
    player[i].fd = NotConnected;
    player[i].state = PlayerNoExist;
    player[i].outmsg = NULL;
    player[i].outmsgSize = 0;
    player[i].outmsgOffset = 0;
    player[i].outmsgCapacity = 0;
#ifdef HAVE_ADNS_H
    player[i].hostname = NULL;
    player[i].adnsQuery = NULL;
#endif
  }

  listServerLinksCount = 0;
  publicize();
  return true;
}


static void serverStop()
{
  // shut down server
  // first ignore further attempts to kill me
  bzSignal(SIGINT, SIG_IGN);
  bzSignal(SIGTERM, SIG_IGN);

  // reject attempts to talk to server
  shutdown(wksSocket, 2);
  close(wksSocket);

  // tell players to quit
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    directMessage(i, MsgSuperKill, 0, getDirectMessageBuffer());

  // close connections
  for (i = 0; i < MaxPlayers; i++)
    if (player[i].fd != NotConnected) {
      shutdown(player[i].fd, 2);
      close(player[i].fd);
      delete[] player[i].outmsg;
    }

  // now tell the list servers that we're going away.  this can
  // take some time but we don't want to wait too long.  we do
  // our own multiplexing loop and wait for a maximum of 3 seconds
  // total.
  sendMessageToListServer(ListServerLink::REMOVE);
  TimeKeeper start = TimeKeeper::getCurrent();
  if (!listServerLinksCount)
    return;
  do {
    // compute timeout
    float waitTime = 3.0f - (TimeKeeper::getCurrent() - start);
    if (waitTime <= 0.0f)
      break;
    if (listServerLink.socket == NotConnected)
      break;
    // check for list server socket connection
    int fdMax = -1;
    fd_set write_set;
    fd_set read_set;
    FD_ZERO(&write_set);
    FD_ZERO(&read_set);
    if (listServerLink.phase == ListServerLink::CONNECTING)
      FD_SET(listServerLink.socket, &write_set);
    else
      FD_SET(listServerLink.socket, &read_set);
    fdMax = listServerLink.socket;

    // wait for socket to connect or timeout
    struct timeval timeout;
    timeout.tv_sec = long(floorf(waitTime));
    timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));
    int nfound = select(fdMax + 1, (fd_set*)&read_set, (fd_set*)&write_set,
			0, &timeout);
    if (nfound == 0)
      // Time has gone, close and go
      break;
    // check for connection to list server
    if (FD_ISSET(listServerLink.socket, &write_set))
      sendMessageToListServerForReal();
    else if (FD_ISSET(listServerLink.socket, &read_set))
      readListServer();
  } while (true);

  // stop list server communication
  closeListServer();
}


static void relayPlayerPacket(int index, uint16_t len, const void *rawbuf)
{
  // relay packet to all players except origin
  for (int i = 0; i < curMaxPlayers; i++)
    if (i != index && player[i].state > PlayerInLimbo)
      pwrite(i, rawbuf, len + 4);
}


static std::istream &readToken(std::istream& input, char *buffer, int n)
{
  int c = -1;

  // skip whitespace
  while (input.good() && (c = input.get()) != -1 && isspace(c) && c != '\n')
    ;

  // read up to whitespace or n - 1 characters into buffer
  int i = 0;
  if (c != -1 && c != '\n') {
    buffer[i++] = c;
    while (input.good() && i < n - 1 && (c = input.get()) != -1 && !isspace(c))
      buffer[i++] = (char)c;
  }

  // terminate string
  buffer[i] = 0;

  // put back last character we didn't use
  if (c != -1 && isspace(c))
    input.putback(c);

  return input;
}


static bool readWorldStream(std::istream& input, const char *location, std::vector<WorldFileObject*>& wlist)
{
  int line = 1;
  char buffer[1024];
  WorldFileObject *object    = NULL;
  WorldFileObject *newObject = NULL;
  while (!input.eof())
  {
    // watch out for starting a new object when one is already in progress
    if (newObject) {
      if (object) {
	std::cout << location << '(' << line << ") : discarding incomplete object\n";
	delete object;
      }
      object = newObject;
      newObject = NULL;
    }

    // read first token but do not skip newlines
    readToken(input, buffer, sizeof(buffer));
    if (strcmp(buffer, "") == 0) {
      // ignore blank line
    }

    else if (buffer[0] == '#') {
      // ignore comment
    }

    else if (strcasecmp(buffer, "end") == 0) {
      if (object) {
	wlist.push_back(object);
	object = NULL;
      }
      else {
	std::cout << location << '(' << line << ") : unexpected \"end\" token\n";
	return false;
      }
    }

    else if (strcasecmp(buffer, "box") == 0)
      newObject = new CustomBox;

    else if (strcasecmp(buffer, "pyramid") == 0)
      newObject = new CustomPyramid();

    else if (strcasecmp(buffer, "teleporter") == 0)
      newObject = new CustomGate();

    else if (strcasecmp(buffer, "link") == 0)
      newObject = new CustomLink();

    else if (strcasecmp(buffer, "base") == 0)
      newObject = new CustomBase;

    else if (strcasecmp(buffer, "weapon") == 0)
      newObject = new CustomWeapon;

    else if (strcasecmp(buffer, "world") == 0){
		if (!gotWorld){
			newObject = new CustomWorld();
			gotWorld = true;
		}
	}
    else if (object) {
      if (!object->read(buffer, input)) {
	// unknown token
	std::cout << location << '(' << line << ") : unknown object parameter \"" << buffer << "\" - skipping\n";
	//delete object;
	//return false;
      }
    }
    else {// filling the current object
      // unknown token
      std::cout << location << '(' << line << ") : invalid object type \"" << buffer << "\" - skipping\n";
      delete object;
     // return false;
    }

    // discard remainder of line
    while (input.good() && input.peek() != '\n')
      input.get(buffer, sizeof(buffer));
    input.getline(buffer, sizeof(buffer));
    ++line;
  }

  if (object) {
    std::cout << location << '(' << line << ") : missing \"end\" token\n";
    delete object;
    return false;
  }

  return true;
}


static WorldInfo *defineWorldFromFile(const char *filename)
{
  // open file
  std::ifstream input(filename, std::ios::in);

  if (!input) {
    std::cout << "could not find bzflag world file : " << filename << std::endl;
    return NULL;
  }

  // create world object
  world = new WorldInfo;
  if (!world)
    return NULL;

  // read file
  std::vector<WorldFileObject*> list;
  if (!readWorldStream(input, filename, list)) {
    emptyWorldFileObjectList(list);
    delete world;
    return NULL;
  }

  // make walls
  float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
  float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  world->addWall(0.0f, 0.5f * worldSize, 0.0f, 1.5f * M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(0.5f * worldSize, 0.0f, 0.0f, M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(0.0f, -0.5f * worldSize, 0.0f, 0.5f * M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(-0.5f * worldSize, 0.0f, 0.0f, 0.0f, 0.5f * worldSize, wallHeight);

  // add objects
  const int n = list.size();
  for (int i = 0; i < n; ++i)
    list[i]->write(world);

  if (clOptions->gameStyle & TeamFlagGameStyle) {
    for (int i = RedTeam; i <= PurpleTeam; i++) {
      if ((clOptions->maxTeam[i] > 0) && bases.find(i) == bases.end()) {
	std::cout << "base was not defined for team " << i << ", capture the flag game style removed.\n";
	clOptions->gameStyle &= (~TeamFlagGameStyle);
	break;
      }
    }
  }
  
  // clean up
  emptyWorldFileObjectList(list);
  return world;
}


static WorldInfo *defineTeamWorld()
{
  if (!clOptions->worldFile) {
    world = new WorldInfo();
    if (!world)
      return NULL;

    const float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    const float worldfactor = worldSize / (float)DEFAULT_WORLD;
    const int actCitySize = int(clOptions->citySize * worldfactor + 0.5f);
    const float pyrBase = BZDB.eval(StateDatabase::BZDB_PYRBASE);

    // set team base and team flag safety positions
    int t;
    for (t = RedTeam; t <= PurpleTeam; t++)
      bases[t] = TeamBases((TeamColor)t, true);

    const bool haveRed    = clOptions->maxTeam[RedTeam] > 0;
    const bool haveGreen  = clOptions->maxTeam[GreenTeam] > 0;
    const bool haveBlue   = clOptions->maxTeam[BlueTeam] > 0;
    const bool havePurple = clOptions->maxTeam[PurpleTeam] > 0;

    // make walls
    const float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
    world->addWall(0.0f, 0.5f * worldSize, 0.0f, 1.5f * M_PI, 0.5f * worldSize, wallHeight);
    world->addWall(0.5f * worldSize, 0.0f, 0.0f, M_PI, 0.5f * worldSize, wallHeight);
    world->addWall(0.0f, -0.5f * worldSize, 0.0f, 0.5f * M_PI, 0.5f * worldSize, wallHeight);
    world->addWall(-0.5f * worldSize, 0.0f, 0.0f, 0.0f, 0.5f * worldSize, wallHeight);

    const float pyrHeight = BZDB.eval(StateDatabase::BZDB_PYRHEIGHT);
    // make pyramids
    if (haveRed) {
      // around red base
      const float *pos = bases[RedTeam].getBasePosition(0);
      world->addPyramid(
	  pos[0] + 0.5f * BaseSize - pyrBase,
	  pos[1] - 0.5f * BaseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * BaseSize + pyrBase,
	  pos[1] - 0.5f * BaseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * BaseSize + pyrBase,
	  pos[1] + 0.5f * BaseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * BaseSize - pyrBase,
	  pos[1] + 0.5f * BaseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
    }

    if (haveGreen) {
      // around green base
      const float *pos = bases[GreenTeam].getBasePosition(0);
      world->addPyramid(
	  pos[0] - 0.5f * BaseSize + pyrBase,
	  pos[1] - 0.5f * BaseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * BaseSize - pyrBase,
	  pos[1] - 0.5f * BaseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * BaseSize - pyrBase,
	  pos[1] + 0.5f * BaseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * BaseSize + pyrBase,
	  pos[1] + 0.5f * BaseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
    }

    if (haveBlue) {
      // around blue base
      const float *pos = bases[BlueTeam].getBasePosition(0);
      world->addPyramid(
	  pos[0] - 0.5f * BaseSize - pyrBase,
	  pos[1] + 0.5f * BaseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * BaseSize + pyrBase,
	  pos[1] + 0.5f * BaseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * BaseSize - pyrBase,
	  pos[1] + 0.5f * BaseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * BaseSize + pyrBase,
	  pos[1] + 0.5f * BaseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
    }

    if (havePurple) {
      // around purple base
      const float *pos = bases[PurpleTeam].getBasePosition(0);
      world->addPyramid(
	  pos[0] - 0.5f * BaseSize - pyrBase,
	  pos[1] - 0.5f * BaseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * BaseSize + pyrBase,
	  pos[1] - 0.5f * BaseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * BaseSize - pyrBase,
	  pos[1] - 0.5f * BaseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * BaseSize + pyrBase,
	  pos[1] - 0.5f * BaseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
    }

    // create symmetric map of random buildings for random CTF mode
    if (clOptions->randomCTF) {
      int i;
      float h = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
      const bool redGreen = haveRed || haveGreen;
      const bool bluePurple = haveBlue || havePurple;
      if (!redGreen && !bluePurple) {
	std::cerr << "need some teams, use -mp\n";
	exit(20);
      }
      const float *redPosition = bases[RedTeam].getBasePosition(0);
      const float *greenPosition = bases[GreenTeam].getBasePosition(0);
      const float *bluePosition = bases[BlueTeam].getBasePosition(0);
      const float *purplePosition = bases[PurpleTeam].getBasePosition(0);

      const int numBoxes = int((0.5 + 0.4 * bzfrand()) * actCitySize * actCitySize);
      const float boxHeight = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
      for (i = 0; i < numBoxes;) {
	if (clOptions->randomHeights)
	  h = boxHeight * (2.0f * (float)bzfrand() + 0.5f);
	float x=worldSize * ((float)bzfrand() - 0.5f);
	float y=worldSize * ((float)bzfrand() - 0.5f);
	// don't place near center and bases
	if ((redGreen &&
	     (hypotf(fabs(x-redPosition[0]),fabs(y-redPosition[1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(-x-redPosition[0]),fabs(-y-redPosition[1])) <=
	      BoxBase*4)) ||
	    (bluePurple &&
	     (hypotf(fabs(y-bluePosition[0]),fabs(-x-bluePosition[1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(-y-bluePosition[0]),fabs(x-bluePosition[1])) <=
	      BoxBase*4)) ||
	    (redGreen && bluePurple &&
	     (hypotf(fabs(x-bluePosition[0]),fabs(y-bluePosition[1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(-x-bluePosition[0]),fabs(-y-bluePosition[1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(y-redPosition[0]),fabs(-x-redPosition[1])) <=
	      BoxBase*4 ||
	      hypotf(fabs(-y-redPosition[0]),fabs(x-redPosition[1])) <=
	      BoxBase*4)) ||
	    (hypotf(fabs(x),fabs(y)) <= worldSize/12))
	  continue;

	float angle=2.0f * M_PI * (float)bzfrand();
	if (redGreen) {
	  world->addBox(x,y,0.0f, angle, BoxBase, BoxBase, h);
	  world->addBox(-x,-y,0.0f, angle, BoxBase, BoxBase, h);
	  i+=2;
	}
	if (bluePurple) {
	  world->addBox(y,-x,0.0f, angle, BoxBase, BoxBase, h);
	  world->addBox(-y,x,0.0f, angle, BoxBase, BoxBase, h);
	  i+=2;
	}
      }

      // make pyramids
      h = BZDB.eval(StateDatabase::BZDB_PYRHEIGHT);
      const int numPyrs = int((0.5 + 0.4 * bzfrand()) * actCitySize * actCitySize * 2);
      for (i = 0; i < numPyrs; i++) {
	if (clOptions->randomHeights)
	  h = pyrHeight * (2.0f * (float)bzfrand() + 0.5f);
	float x=worldSize * ((float)bzfrand() - 0.5f);
	float y=worldSize * ((float)bzfrand() - 0.5f);
	// don't place near center or bases
	if ((redGreen &&
	     (hypotf(fabs(x-redPosition[0]),fabs(y-redPosition[1])) <=
	      pyrBase*6 ||
	      hypotf(fabs(-x-redPosition[0]),fabs(-y-redPosition[1])) <=
	      pyrBase*6)) ||
	    (bluePurple &&
	     (hypotf(fabs(y-bluePosition[0]),fabs(-x-bluePosition[1])) <=
	      pyrBase*6 ||
	      hypotf(fabs(-y-bluePosition[0]),fabs(x-bluePosition[1])) <=
	      pyrBase*6)) ||
	    (redGreen && bluePurple &&
	     (hypotf(fabs(x-bluePosition[0]),fabs(y-bluePosition[1])) <=
	      pyrBase*6 ||
	      hypotf(fabs(-x-bluePosition[0]),fabs(-y-bluePosition[1])) <=
	      pyrBase*6 ||
	      hypotf(fabs(y-redPosition[0]),fabs(-x-redPosition[1])) <=
	      pyrBase*6 ||
	      hypotf(fabs(-y-redPosition[0]),fabs(x-redPosition[1])) <=
	      pyrBase*6)) ||
	    (hypotf(fabs(x),fabs(y)) <= worldSize/12))
	  continue;

	float angle=2.0f * M_PI * (float)bzfrand();
	if (redGreen) {
	  world->addPyramid(x,y, 0.0f, angle,pyrBase, pyrBase, h);
	  world->addPyramid(-x,-y, 0.0f, angle,pyrBase, pyrBase, h);
	  i+=2;
	}
	if (bluePurple) {
	  world->addPyramid(y,-x,0.0f, angle, pyrBase, pyrBase, h);
	  world->addPyramid(-y,x,0.0f, angle, pyrBase, pyrBase, h);
	  i+=2;
	}
      }

      // make teleporters
      if (clOptions->useTeleporters) {
	const int teamFactor = redGreen && bluePurple ? 4 : 2;
	const int numTeleporters = (8 + int(8 * (float)bzfrand())) / teamFactor * teamFactor;
	const int numLinks = 2 * numTeleporters / teamFactor;
	int (*linked)[2] = new int[numLinks][2];
	for (i = 0; i < numTeleporters;) {
	  const float x = (worldSize - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
	  const float y = (worldSize - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
	  const float rotation = 2.0f * M_PI * (float)bzfrand();

	  // if too close to building then try again
	  if (NOT_IN_BUILDING != world->inBuilding(NULL, x, y, 0,
						   1.75f * TeleBreadth,
						   1.0f))
	    continue;
	  // if to close to a base then try again
	  if ((redGreen &&
	       (hypotf(fabs(x-redPosition[0]),fabs(y-redPosition[1])) <=
		BaseSize*4 ||
		hypotf(fabs(x-greenPosition[0]),fabs(y-greenPosition[1])) <=
		BaseSize*4)) ||
	      (bluePurple &&
	       (hypotf(fabs(x-bluePosition[0]),fabs(y-bluePosition[1])) <=
		BaseSize*4 ||
		hypotf(fabs(x-purplePosition[0]),fabs(y-purplePosition[1])) <=
		BaseSize*4)))
	    continue;

	  linked[i/teamFactor][0] = linked[i/teamFactor][1] = 0;
	  if (redGreen) {
	    world->addTeleporter(x, y, 0.0f, rotation, 0.5f*TeleWidth,
		TeleBreadth, 2.0f*TeleHeight, TeleWidth);
	    world->addTeleporter(-x, -y, 0.0f, rotation + M_PI, 0.5f*TeleWidth,
		TeleBreadth, 2.0f*TeleHeight, TeleWidth);
	    i+=2;
	  }
	  if (bluePurple) {
	    world->addTeleporter(y, -x, 0.0f, rotation + M_PI / 2,
				 0.5f*TeleWidth, TeleBreadth, 2.0f*TeleHeight,
				 TeleWidth);
	    world->addTeleporter(-y, x, 0.0f, rotation + M_PI * 3 / 2,
				 0.5f*TeleWidth, TeleBreadth, 2.0f*TeleHeight,
				 TeleWidth);
	    i+=2;
	  }
	}

	// make teleporter links
	int numUnlinked = numLinks;
	for (i = 0; i < numLinks / 2; i++)
	  for (int j = 0; j < 2; j++) {
	    int a = (int)(numUnlinked * (float)bzfrand());
	    if (linked[i][j])
	      continue;
	    for (int k = 0, i2 = i; i2 < numLinks / 2; ++i2) {
	      for (int j2 = ((i2 == i) ? j : 0); j2 < 2; ++j2) {
		if (linked[i2][j2])
		  continue;
		if (k++ == a) {
		  world->addLink((2 * i + j) * teamFactor, (2 * i2 + j2) * teamFactor);
		  world->addLink((2 * i + j) * teamFactor + 1, (2 * i2 + j2) * teamFactor + 1);
		  if (redGreen && bluePurple) {
		    world->addLink((2 * i + j) * teamFactor + 2, (2 * i2 + j2) * teamFactor + 2);
		    world->addLink((2 * i + j) * teamFactor + 3, (2 * i2 + j2) * teamFactor + 3);
		  }
		  linked[i][j] = 1;
		  numUnlinked--;
		  if (i != i2 || j != j2) {
		    world->addLink((2 * i2 + j2) * teamFactor, (2 * i + j) * teamFactor);
		    world->addLink((2 * i2 + j2) * teamFactor + 1, (2 * i + j) * teamFactor + 1);
		    if (redGreen && bluePurple) {
		      world->addLink((2 * i2 + j2) * teamFactor + 2, (2 * i + j) * teamFactor + 2);
		      world->addLink((2 * i2 + j2) * teamFactor + 3, (2 * i + j) * teamFactor + 3);
		    }
		    linked[i2][j2] = 1;
		    numUnlinked--;
		  }
		}
	      }
	    }
	  }
	delete[] linked;
      }
    }
    else
    {
      // pyramids in center
      world->addPyramid(
	  -(BoxBase + 0.25f * AvenueSize),
	  -(BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  (BoxBase + 0.25f * AvenueSize),
	  -(BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  -(BoxBase + 0.25f * AvenueSize),
	  (BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  (BoxBase + 0.25f * AvenueSize),
	  (BoxBase + 0.25f * AvenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(0.0f, -(BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(0.0f,  (BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(-(BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid( (BoxBase + 0.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);

      // halfway out from city center
      world->addPyramid(0.0f, -(3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(0.0f,  (3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(-(3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid( (3.0f * BoxBase + 1.5f * AvenueSize), 0.0f, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      // add boxes, four at once with same height so no team has an advantage
      const float xmin = -0.5f * ((2.0f * BoxBase + AvenueSize) * (actCitySize - 1));
      const float ymin = -0.5f * ((2.0f * BoxBase + AvenueSize) * (actCitySize - 1));
      const float boxHeight = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
      for (int j = 0; j <= actCitySize/2; j++)
	for (int i = 0; i < actCitySize/2; i++)
      if (i != actCitySize/2 || j != actCitySize/2) {
	float h = boxHeight;
	if (clOptions->randomHeights)
	  h *= 2.0f * (float)bzfrand() + 0.5f;
	world->addBox(
	    xmin + float(i) * (2.0f * BoxBase + AvenueSize),
	    ymin + float(j) * (2.0f * BoxBase + AvenueSize), 0.0f,
	    clOptions->randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    BoxBase, BoxBase, h);
	world->addBox(
	    -1.0f * (xmin + float(i) * (2.0f * BoxBase + AvenueSize)),
	    -1.0f * (ymin + float(j) * (2.0f * BoxBase + AvenueSize)), 0.0f,
	    clOptions->randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    BoxBase, BoxBase, h);
	world->addBox(
	    -1.0f * (ymin + float(j) * (2.0f * BoxBase + AvenueSize)),
	    xmin + float(i) * (2.0f * BoxBase + AvenueSize), 0.0f,
	    clOptions->randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    BoxBase, BoxBase, h);
	world->addBox(
	    ymin + float(j) * (2.0f * BoxBase + AvenueSize),
	    -1.0f * (xmin + float(i) * (2.0f * BoxBase + AvenueSize)), 0.0f,
	    clOptions->randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    BoxBase, BoxBase, h);
      }
      // add teleporters
      if (clOptions->useTeleporters) {
	const float xoff = BoxBase + 0.5f * AvenueSize;
	const float yoff = BoxBase + 0.5f * AvenueSize;
	world->addTeleporter( xmin - xoff,  ymin - yoff, 0.0f, 1.25f * M_PI,
			     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter( xmin - xoff, -ymin + yoff, 0.0f, 0.75f * M_PI,
			     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter(-xmin + xoff,  ymin - yoff, 0.0f, 1.75f * M_PI,
			     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter(-xmin + xoff, -ymin + yoff, 0.0f, 0.25f * M_PI,
			     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter(-3.5f * TeleBreadth, -3.5f * TeleBreadth, 0.0f, 1.25f * M_PI,
			     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter(-3.5f * TeleBreadth,  3.5f * TeleBreadth, 0.0f, 0.75f * M_PI,
			     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter( 3.5f * TeleBreadth, -3.5f * TeleBreadth, 0.0f, 1.75f * M_PI,
			     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);
	world->addTeleporter( 3.5f * TeleBreadth,  3.5f * TeleBreadth, 0.0f, 0.25f * M_PI,
			     0.5f * TeleWidth, TeleBreadth, 2.0f * TeleHeight, TeleWidth);

	world->addLink(0, 14);
	world->addLink(1, 7);
	world->addLink(2, 12);
	world->addLink(3, 5);
	world->addLink(4, 10);
	world->addLink(5, 3);
	world->addLink(6, 8);
	world->addLink(7, 1);
	world->addLink(8, 6);
	world->addLink(9, 0);
	world->addLink(10, 4);
	world->addLink(11, 2);
	world->addLink(12, 2);
	world->addLink(13, 4);
	world->addLink(14, 0);
	world->addLink(15, 6);
      }
    }

    // get rid of unneeded bases
    for (t = RedTeam; t <= PurpleTeam; t++) {
      if (clOptions->maxTeam[t] == 0) {
        bases.erase(t);
      }
    }

    return world;
  } else {
    return defineWorldFromFile(clOptions->worldFile);
  }
}


static WorldInfo *defineRandomWorld()
{
  world = new WorldInfo();
  if (!world)
    return NULL;

  // make walls
  float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
  world->addWall(0.0f, 0.5f * worldSize, 0.0f, 1.5f * M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(0.5f * worldSize, 0.0f, 0.0f, M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(0.0f, -0.5f * worldSize, 0.0f, 0.5f * M_PI, 0.5f * worldSize, wallHeight);
  world->addWall(-0.5f * worldSize, 0.0f, 0.0f, 0.0f, 0.5f * worldSize, wallHeight);

  const float worldfactor = worldSize / (float)DEFAULT_WORLD;
  const int actCitySize = int(clOptions->citySize * worldfactor + 0.5f);
  const int numTeleporters = 8 + int(8 * (float)bzfrand() * worldfactor);
  // make boxes
  int i;
  float boxHeight = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
  float h = boxHeight;
  const int numBoxes = int((0.5f + 0.7f * bzfrand()) * actCitySize * actCitySize);
  for (i = 0; i < numBoxes; i++) {
    if (clOptions->randomHeights)
      h = boxHeight * ( 2.0f * (float)bzfrand() + 0.5f);
      world->addBox(worldSize * ((float)bzfrand() - 0.5f),
	  worldSize * ((float)bzfrand() - 0.5f),
	  0.0f, 2.0f * M_PI * (float)bzfrand(),
	  BoxBase, BoxBase, h);
  }

  // make pyramids
  float pyrHeight = BZDB.eval(StateDatabase::BZDB_PYRHEIGHT);
  float pyrBase = BZDB.eval(StateDatabase::BZDB_PYRBASE);
  h = pyrHeight;
  const int numPyrs = int((0.5f + 0.7f * bzfrand()) * actCitySize * actCitySize);
  for (i = 0; i < numPyrs; i++) {
    if (clOptions->randomHeights)
      h = pyrHeight * ( 2.0f * (float)bzfrand() + 0.5f);
      world->addPyramid(worldSize * ((float)bzfrand() - 0.5f),
	  worldSize * ((float)bzfrand() - 0.5f),
	  0.0f, 2.0f * M_PI * (float)bzfrand(),
	  pyrBase, pyrBase, h);
  }

  if (clOptions->useTeleporters) {
    // make teleporters
    int (*linked)[2] = new int[numTeleporters][2];
    for (i = 0; i < numTeleporters;) {
      const float x = (worldSize - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
      const float y = (worldSize - 4.0f * TeleBreadth) * ((float)bzfrand() - 0.5f);
      const float rotation = 2.0f * M_PI * (float)bzfrand();

      // if too close to building then try again
      if (NOT_IN_BUILDING != world->inBuilding(NULL, x, y, 0,
					       1.75f * TeleBreadth, 1.0f))
	continue;

      world->addTeleporter(x, y, 0.0f, rotation,
	  0.5f*TeleWidth, TeleBreadth, 2.0f*TeleHeight, TeleWidth);
      linked[i][0] = linked[i][1] = 0;
      i++;
    }

    // make teleporter links
    int numUnlinked = 2 * numTeleporters;
    for (i = 0; i < numTeleporters; i++)
      for (int j = 0; j < 2; j++) {
	int a = (int)(numUnlinked * (float)bzfrand());
	if (linked[i][j])
	  continue;
	for (int k = 0, i2 = i; i2 < numTeleporters; ++i2)
	  for (int j2 = ((i2 == i) ? j : 0); j2 < 2; ++j2) {
	    if (linked[i2][j2])
	      continue;
	    if (k++ == a) {
	      world->addLink(2 * i + j, 2 * i2 + j2);
	      linked[i][j] = 1;
	      numUnlinked--;
	      if (i != i2 || j != j2) {
		world->addLink(2 * i2 + j2, 2 * i + j);
		linked[i2][j2] = 1;
		numUnlinked--;
	      }
	    }
	  }
      }
    delete[] linked;
  }

  return world;
}


static bool defineWorld()
{
  // clean up old database
  if (world)
  delete world;

  if(worldDatabase)
  delete[] worldDatabase;

  // make world and add buildings
  if (clOptions->gameStyle & TeamFlagGameStyle) {
    world = defineTeamWorld();
  } else if (clOptions->worldFile) {
    world = defineWorldFromFile(clOptions->worldFile);
  } else {
    world = defineRandomWorld();
  }

  if (world == NULL)
    return false;

  maxWorldHeight = world->getMaxWorldHeight();

  int numBases = 0;
  for (BasesList::iterator it = bases.begin(); it != bases.end(); ++it)
    numBases += it->second.size();

  // package up world
  world->packDatabase();
  // now get world packaged for network transmission
  worldDatabaseSize = 4 + WorldCodeHeaderSize +
      world->getDatabaseSize() + 4 + WorldCodeEndSize;
  if (clOptions->gameStyle & TeamFlagGameStyle)
    worldDatabaseSize += numBases * (4 + WorldCodeBaseSize);

  worldDatabase = new char[worldDatabaseSize];
  // this should NOT happen but it does sometimes
  if(!worldDatabase)
    return false;
  memset( worldDatabase, 0, worldDatabaseSize );

  void *buf = worldDatabase;
  buf = nboPackUShort(buf, WorldCodeHeaderSize);
  buf = nboPackUShort(buf, WorldCodeHeader);
  buf = nboPackUShort(buf, mapVersion);
  buf = nboPackFloat(buf, BZDB.eval(StateDatabase::BZDB_WORLDSIZE));
  buf = nboPackUShort(buf, clOptions->gameStyle);
  buf = nboPackUShort(buf, maxPlayers);
  buf = nboPackUShort(buf, clOptions->maxShots);
  buf = nboPackUShort(buf, numFlags);
  buf = nboPackFloat(buf, clOptions->linearAcceleration);
  buf = nboPackFloat(buf, clOptions->angularAcceleration);
  buf = nboPackUShort(buf, clOptions->shakeTimeout);
  buf = nboPackUShort(buf, clOptions->shakeWins);
  // time-of-day will go here
  buf = nboPackUInt(buf, 0);
  if (clOptions->gameStyle & TeamFlagGameStyle) {
    for (BasesList::iterator it = bases.begin(); it != bases.end(); ++it)
      buf = it->second.pack(buf);
  }
  buf = nboPackString(buf, world->getDatabase(), world->getDatabaseSize());
  buf = nboPackUShort(buf, WorldCodeEndSize);
  buf = nboPackUShort(buf, WorldCodeEnd);

  MD5 md5;
  md5.update( (unsigned char *)worldDatabase, worldDatabaseSize );
  md5.finalize();
  if (clOptions->worldFile == NULL)
    strcpy(hexDigest,"t");
  else
    strcpy(hexDigest, "p");
  std::string digest = md5.hexdigest();
  strcat(hexDigest, digest.c_str());

  // reset other stuff
  int i;
  for (i = 0; i < NumTeams; i++) {
    team[i].team.size = 0;
    team[i].team.won = 0;
    team[i].team.lost = 0;
  }
  numFlagsInAir = 0;
  for (i = 0; i < numFlags; i++)
    resetFlag(i);

  return true;
}


static TeamColor whoseBase(float x, float y, float z)
{
  if (!(clOptions->gameStyle & TeamFlagGameStyle))
    return NoTeam;

  float highest = -1;
  int highestteam = -1;

  for (BasesList::iterator it = bases.begin(); it != bases.end(); ++it) {
    float baseZ = it->second.findBaseZ(x,y,z);
    if (baseZ > highest) {
      highest = baseZ;
      highestteam = it->second.getTeam();
    }
  }

  if(highestteam == -1)
    return NoTeam;
  else
    return TeamColor(highestteam);
}


#ifdef PRINTSCORE
static void dumpScore()
{
  int i;

  if (!clOptions->printScore)
    return;
#ifdef TIMELIMIT
  if (clOptions->timeLimit > 0.0f)
    std::cout << "#time " << clOptions->timeLimit - clOptions->timeElapsed << std::endl;
#endif
  std::cout << "#teams";
  for (i = int(RedTeam); i < NumTeams; i++)
    std::cout << ' ' << team[i].team.won << '-' << team[i].team.lost << ' ' <<
    	         Team::getName(TeamColor(i));
  std::cout << "\n#players\n";
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i].state > PlayerInLimbo)
      std::cout << player[i].wins << '-' << player[i].losses << ' ' <<
      		   player[i].callSign << std::endl;
  std::cout << "#end\n";
}
#endif


static void acceptClient()
{
  // client (not a player yet) is requesting service.
  // accept incoming connection on our well known port
  struct sockaddr_in clientAddr;
  AddrLen addr_len = sizeof(clientAddr);
  int fd = accept(wksSocket, (struct sockaddr*)&clientAddr, &addr_len);
  if (fd == -1) {
    nerror("accepting on wks");
    return;
  }
  // don't buffer info, send it immediately
  setNoDelay(fd);
  BzfNetwork::setNonBlocking(fd);

  if (!clOptions->acl.validate( clientAddr.sin_addr)) {
    close(fd);
    return;
  }

  int keepalive = 1;
  int n;
  n = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
		 (SSOType)&keepalive, sizeof(int));
  if (n < 0) {
      nerror("couldn't set keepalive");
  }
  // send server version and playerid
  char buffer[9];
  memcpy(buffer, getServerVersion(), 8);
  // send 0xff if list is full
  buffer[8] = (char)0xff;

  PlayerId playerIndex;

  // find open slot in players list
  for (playerIndex = 0; playerIndex < maxPlayers; playerIndex++)
    if (player[playerIndex].state == PlayerNoExist)
      break;

  if (playerIndex < maxPlayers) {
    DEBUG1("Player [%d] accept() from %s:%d on %i\n", playerIndex,
	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fd);

    if (playerIndex >= curMaxPlayers)
      curMaxPlayers = playerIndex+1;
  } else { // full? reject by closing socket
    DEBUG1("all slots occupied, rejecting accept() from %s:%d on %i\n",
	   inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fd);

    // send back 0xff before closing
    send(fd, (const char*)buffer, sizeof(buffer), 0);

    close(fd);
    return;
  }

  // store address information for player
  memcpy(&player[playerIndex].taddr, &clientAddr, addr_len);
  memcpy(&player[playerIndex].uaddr, &clientAddr, addr_len);

  buffer[8] = (uint8_t)playerIndex;
  send(fd, (const char*)buffer, sizeof(buffer), 0);

  // FIXME add new client server welcome packet here when client code is ready

  // update player state
  player[playerIndex].time = TimeKeeper::getCurrent();
  player[playerIndex].fd = fd;
  player[playerIndex].state = PlayerInLimbo;
  player[playerIndex].peer = Address(player[playerIndex].taddr);
  player[playerIndex].tcplen = 0;
  player[playerIndex].udplen = 0;
  assert(player[playerIndex].outmsg == NULL);
  player[playerIndex].outmsgSize = 0;
  player[playerIndex].outmsgOffset = 0;
  player[playerIndex].outmsgCapacity = 0;
  player[playerIndex].lastState.order = 0;
  player[playerIndex].paused = false;
  player[playerIndex].quellRoger = false;
  
#ifdef HAVE_ADNS_H
  if (player[playerIndex].adnsQuery) {
    adns_cancel(player[playerIndex].adnsQuery);
    player[playerIndex].adnsQuery = NULL;
  }
  // launch the asynchronous query to look up this hostname
  if (adns_submit_reverse(adnsState, (struct sockaddr *)&clientAddr, adns_r_ptr,
			  (adns_queryflags)(adns_qf_quoteok_cname|adns_qf_cname_loose),
			  0, &player[playerIndex].adnsQuery) != 0) {
    DEBUG1("Player [%d] failed to submit reverse resolve query: errno %d\n", playerIndex, getErrno());
    player[playerIndex].adnsQuery = NULL;
  } else {
    DEBUG2("Player [%d] submitted reverse resolve query\n", playerIndex);
  }
#endif

  player[playerIndex].pausedSince = TimeKeeper::getNullTime();
#ifdef NETWORK_STATS
  initPlayerMessageStats(playerIndex);
#endif

  // if game was over and this is the first player then game is on
  if (gameOver) {
    int count = 0;
    for (int i = 0; i < curMaxPlayers; i++)
      if (player[i].state >= PlayerInLimbo)
	count++;
    if (count == 1) {
      gameOver = false;
#ifdef TIMELIMIT
      gameStartTime = TimeKeeper::getCurrent();
      if (clOptions->timeLimit > 0.0f && !clOptions->timeManualStart) {
	clOptions->timeElapsed = 0.0f;
	countdownActive = true;
      }
#endif
    }
  }
}


static void respondToPing()
{
  // get and discard ping packet
  struct sockaddr_in addr;
  if (!PingPacket::isRequest(udpSocket, &addr)) return;

  // if I'm ignoring pings and the ping is not from a connected host
  // then ignore the ping.
  if (!handlePings) {
      return;
  }

  // reply with current game info on udpSocket
  pingReply.sourceAddr = Address(addr);
  pingReply.rogueCount = (uint8_t)team[0].team.size;
  pingReply.redCount = (uint8_t)team[1].team.size;
  pingReply.greenCount = (uint8_t)team[2].team.size;
  pingReply.blueCount = (uint8_t)team[3].team.size;
  pingReply.purpleCount = (uint8_t)team[4].team.size;
  pingReply.observerCount = (uint8_t)team[5].team.size;
  pingReply.write(udpSocket, &addr);
}


void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message, bool fullBuffer)
{
  // player is sending a message to a particular player, a team, or all.
  // send MsgMessage

  if (strlen(message) > (unsigned)MessageLen) {
    DEBUG1("WARNING: Network message being sent is too long! (cutoff at %d)\n", MessageLen);
  }

  // if fullBuffer=true, it means, that caller already passed a buffer
  // of size MessageLen and we can use that directly;
  // otherwise copy the message to a buffer of the correct size first
  char messagebuf[MessageLen];
  if (!fullBuffer) {
    strncpy(messagebuf,message,MessageLen);
    message=messagebuf;
  }

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUByte(buf, targetPlayer);
  buf = nboPackString(buf, message, MessageLen);

  int len = (char*)buf - (char*)bufStart;

  if (targetPlayer <= LastRealPlayer) {
    directMessage(targetPlayer, MsgMessage, len, bufStart);
    if (playerIndex <= LastRealPlayer && targetPlayer != playerIndex)
      directMessage(playerIndex, MsgMessage, len, bufStart);
  }
  // FIXME this teamcolor <-> player id conversion is in several files now
  else if (targetPlayer >= 244 && targetPlayer <= 250) {
    TeamColor team = TeamColor(250 - targetPlayer);
    // send message to all team members only
    for (int i = 0; i < curMaxPlayers; i++)
      if (player[i].state > PlayerInLimbo && player[i].team == team)
        directMessage(i, MsgMessage, len, bufStart);
  } else if (targetPlayer == AdminPlayers){
    // admin messages
    for (int i = 0; i < curMaxPlayers; i++){
      if (player[i].state > PlayerInLimbo && 
        hasPerm(i,PlayerAccessInfo::adminMessages)){
         directMessage(i, MsgMessage,len, bufStart);
      }
    }
  } else
    broadcastMessage(MsgMessage, len, bufStart);
}


static void rejectPlayer(int playerIndex, uint16_t code)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, code);
  directMessage(playerIndex, MsgReject, (char*)buf-(char*)bufStart, bufStart);
  return;
}

// Team Size is wrong at some time
// as we are fixing, look also at unconnected player slot to rub it
static void fixTeamCount() {
  int playerIndex;
  for (playerIndex = 0; playerIndex < maxPlayers; playerIndex++)
    if (player[playerIndex].fd == NotConnected)
      player[playerIndex].state = PlayerNoExist;
  int teamNum;
  for (teamNum = RogueTeam; teamNum < RabbitTeam; teamNum++)
    team[teamNum].team.size = 0;
  for (playerIndex = 0; playerIndex < maxPlayers; playerIndex++)
    if (player[playerIndex].state > PlayerInLimbo) {
      teamNum = player[playerIndex].team;
      if (teamNum == RabbitTeam)
	teamNum = RogueTeam;
      team[teamNum].team.size++;
    }
}

static void addPlayer(int playerIndex)
{
  // strip leading blanks
  char *sp = player[playerIndex].callSign, *tp = sp;
  while (*sp==' ')
    sp++;

  // strip any non-printable characters and ', and " from callsign
  do {
    if (isprint(*sp) && (*sp != '\'') && (*sp != '"'))
      *tp++ = *sp;
  } while (*++sp);
  *tp = *sp;


  // strip trailing blanks
  while (*--tp==' ') {
    *tp=0;
  }

  // don't allow empty callsign
  if (player[playerIndex].callSign[0] == '\0')
    rejectPlayer(playerIndex, RejectBadCallsign);

  // look if there is as name clash, we won't allow this
  int i;
  for (i = 0; i < curMaxPlayers; i++)
  {
    if (i == playerIndex || player[i].state <= PlayerInLimbo)
      continue;
    if (strcasecmp(player[i].callSign,player[playerIndex].callSign) == 0) {
      rejectPlayer(playerIndex, RejectRepeatCallsign);
      return;
    }
  }

  // make sure the name is not obscene/filtered
  if (clOptions->filterCallsigns) {
    DEBUG2("checking callsign: %s\n",player[playerIndex].callSign);
    bool filtered = false;
    char cs[CallSignLen];
    memcpy(cs, player[playerIndex].callSign, sizeof(char) * CallSignLen);
    filtered = clOptions->filter.filter(cs, clOptions->filterSimple);
    if (!filtered) {
      DEBUG2("checking email: %s\n",player[playerIndex].email);
      char em[EmailLen];
      memcpy(em, player[playerIndex].email, sizeof(char) * EmailLen);
      filtered = clOptions->filter.filter(em, clOptions->filterSimple);
    }
    if (filtered) {
      rejectPlayer(playerIndex, RejectBadCallsign);
      return ;
    }
  }

  TeamColor t = player[playerIndex].team;

  // count current number of players and players+observers
  int numplayers = 0;
  for (i = 0; i < int(ObserverTeam); i++)
    numplayers += team[i].team.size;
  const int numplayersobs = numplayers + team[ObserverTeam].team.size;

  // no player slots open -> try observer
  if (numplayers == maxRealPlayers) {
    t = player[playerIndex].team = ObserverTeam;
  } else {
    // automatically assign the player's team
    if ((clOptions->autoTeam && t < (int)ObserverTeam) || t == AutomaticTeam) {
      std::vector<TeamColor> minIndex;
      int sizeOfSmallestTeam = maxRealPlayers;

      for (int i = (int)RogueTeam; i < (int)ObserverTeam; i++) {
        const int teamsize = team[i].team.size;
        // if the team is not full and the smallest
        if (teamsize < clOptions->maxTeam[i] && teamsize <= sizeOfSmallestTeam) {
          if (teamsize < sizeOfSmallestTeam) {
            minIndex.clear();
            sizeOfSmallestTeam = team[i].team.size;
          }
          minIndex.push_back((TeamColor)i);
        }
      } // end iteration over teams

      // reassign the team if
      if (minIndex.size() == 0) {
        // all teams are all full, try observer
        t = player[playerIndex].team = ObserverTeam;
      } else if (minIndex.size() == 1) {
        // only one team has a slot open anyways
        t = player[playerIndex].team = minIndex[0];
      } else {
        // multiple equally unfilled teams, choose the one sucking most

        // see if the player's choice was a weak team
        bool foundTeam = false;
        for (int i = 0; i < (int) minIndex.size(); i++) {
          if (minIndex[i] == (TeamColor)t) {
            foundTeam = true;
            break;
          }
        }
        if (!foundTeam) {
          // FIXME -- should pick the team with the least average player kills
          // for now, pick random
          t = player[playerIndex].team = minIndex[rand() % minIndex.size()];
        }
      }
    }
  }

  // reject player if asks for bogus team or rogue and rogues aren't allowed
  // or if the team is full or if the server is full
  if (player[playerIndex].type != TankPlayer &&
      player[playerIndex].type != ComputerPlayer) {
    rejectPlayer(playerIndex, RejectBadType);
        return;
  } else if (t == NoTeam) {
    rejectPlayer(playerIndex, RejectBadTeam);
        return;
  } else if (t == ObserverTeam && player[playerIndex].type == ComputerPlayer) {
    rejectPlayer(playerIndex, RejectServerFull);
    return;
  } else if (numplayersobs == maxPlayers) {
    // server is full
    rejectPlayer(playerIndex, RejectServerFull);
    return;
  } else if (team[int(t)].team.size >= clOptions->maxTeam[int(t)]) {
      rejectPlayer(playerIndex, RejectTeamFull);
      return ;
  }

  player[playerIndex].wasRabbit = false;
  player[playerIndex].toBeKicked = false;
  player[playerIndex].Admin = false;
  player[playerIndex].restartOnBase = (clOptions->gameStyle & TeamFlagGameStyle) != 0;
  player[playerIndex].passwordAttempts = 0;

  player[playerIndex].regName = player[playerIndex].callSign;
  makeupper(player[playerIndex].regName);

  player[playerIndex].accessInfo.explicitAllows.reset();
  player[playerIndex].accessInfo.explicitDenys.reset();
  player[playerIndex].accessInfo.verified = false;
  player[playerIndex].accessInfo.loginTime = TimeKeeper::getCurrent();
  player[playerIndex].accessInfo.loginAttempts = 0;
  player[playerIndex].accessInfo.groups.clear();
  player[playerIndex].accessInfo.groups.push_back("DEFAULT");

  player[playerIndex].lastRecvPacketNo = 0;
  player[playerIndex].lastSendPacketNo = 0;

  player[playerIndex].uqueue = NULL;
  player[playerIndex].dqueue = NULL;

  player[playerIndex].lagavg = 0;
  player[playerIndex].lagcount = 0;
  player[playerIndex].laglastwarn = 0;
  player[playerIndex].lagwarncount = 0;
  player[playerIndex].lagalpha = 1;

  player[playerIndex].jitteravg = 0;
  player[playerIndex].jitteralpha = 1;

  player[playerIndex].lostavg = 0;
  player[playerIndex].lostalpha = 1;

  player[playerIndex].lasttimestamp = 0.0f;
  player[playerIndex].lastupdate = TimeKeeper::getCurrent();
  player[playerIndex].lastmsg	 = TimeKeeper::getCurrent();

  player[playerIndex].quellRoger = false;

  player[playerIndex].nextping = TimeKeeper::getCurrent();
  player[playerIndex].nextping += 10.0;
  player[playerIndex].pingpending = false;
  player[playerIndex].pingseqno = 0;
  player[playerIndex].pingssent = 0;

#ifdef TIMELIMIT
  player[playerIndex].playedEarly = false;
#endif

  // accept player
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  directMessage(playerIndex, MsgAccept, (char*)buf-(char*)bufStart, bufStart);

  //send SetVars
  { // scoping is mandatory
     PackVars pv(bufStart, playerIndex);
     BZDB.iterate(PackVars::packIt, &pv);
  }

  // abort if we hung up on the client
  if (player[playerIndex].fd == NotConnected)
    return;

  // player is signing on (has already connected via addClient).
  player[playerIndex].state = PlayerDead;
  player[playerIndex].flag = -1;
  player[playerIndex].wins = 0;
  player[playerIndex].losses = 0;
  player[playerIndex].tks = 0;
  // update team state and if first player on team,
  // add team's flag and reset it's score
  bool resetTeamFlag = false;
  int teamIndex = int(player[playerIndex].team);
  team[teamIndex].team.size++;
  if (team[teamIndex].team.size == 1 && Team::isColorTeam(player[playerIndex].team)) {
    team[teamIndex].team.won = 0;
    team[teamIndex].team.lost = 0;
    if (clOptions->gameStyle & int(TeamFlagGameStyle)) {
      int flagid = lookupFirstTeamFlag(teamIndex);
      if (flagid >= 0 && flag[flagid].flag.status == FlagNoExist) {
        // can't call resetFlag() here cos it'll screw up protocol for
        // player just joining, so do it later
        resetTeamFlag = true;
      }
    }
  }

  // send new player updates on each player, all existing flags, and all teams.
  // don't send robots any game info.  watch out for connection being closed
  // because of an error.
  if (player[playerIndex].type != ComputerPlayer) {
    int i;
    if (player[playerIndex].fd != NotConnected) {
      sendTeamUpdate(playerIndex);
      sendFlagUpdate(-1, playerIndex);
    }
    for (i = 0; i < curMaxPlayers && player[playerIndex].fd != NotConnected; i++)
      if (player[i].state > PlayerInLimbo && i != playerIndex)
	sendPlayerUpdate(i, playerIndex);
  }

  // if new player connection was closed (because of an error) then stop here
  if (player[playerIndex].fd == NotConnected)
    return;

  // send MsgAddPlayer to everybody -- this concludes MsgEnter response
  // to joining player
  sendPlayerUpdate(playerIndex, playerIndex);

  // send update of info for team just joined
  sendTeamUpdate(-1, teamIndex);

  // send rabbit information
  if (clOptions->gameStyle & int(RabbitChaseGameStyle)) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, rabbitIndex);
    directMessage(playerIndex, MsgNewRabbit, (char*)buf-(char*)bufStart, bufStart);
  }

#ifdef TIMELIMIT
  // send time update to new player if we're counting down
  if (countdownActive && clOptions->timeLimit > 0.0f && player[playerIndex].type != ComputerPlayer) {
    float timeLeft = clOptions->timeLimit - (TimeKeeper::getCurrent() - gameStartTime);
    if (timeLeft < 0.0f) {
      // oops
      timeLeft = 0.0f;
    }

    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUShort(bufStart, (uint16_t)(int)timeLeft);
    directMessage(playerIndex, MsgTimeUpdate, (char*)buf-(char*)bufStart, bufStart);
  }
#endif

  // again check if player was disconnected
  if (player[playerIndex].fd == NotConnected)
    return;

  // reset that flag
  if (resetTeamFlag) {
    int flagid = lookupFirstTeamFlag(teamIndex);
    if (flagid >= 0) {
      for (int n = 0; n < clOptions->numTeamFlags[teamIndex]; n++)
        resetFlag(n+flagid);
    }
  }

  fixTeamCount();

  // tell the list server the new number of players
  sendMessageToListServer(ListServerLink::ADD);

#ifdef PRINTSCORE
  dumpScore();
#endif
  char message[MessageLen];

#ifdef SERVERLOGINMSG
  sprintf(message,"BZFlag server %s, http://BZFlag.org/", getAppVersion());
  sendMessage(ServerPlayer, playerIndex, message, true);

  if (clOptions->servermsg && (strlen(clOptions->servermsg) > 0)) {

    // split the servermsg into several lines if it contains '\n'
    const char* i = clOptions->servermsg;
    const char* j;
    while ((j = strstr(i, "\\n")) != NULL) {
      unsigned int l = j - i < MessageLen - 1 ? j - i : MessageLen - 1;
      strncpy(message, i, l);
      message[l] = '\0';
      sendMessage(ServerPlayer, playerIndex, message, true);
      i = j + 2;
    }
    strncpy(message, i, MessageLen - 1);
    message[strlen(i) < MessageLen - 1 ? strlen(i) : MessageLen - 1] = '\0';
    sendMessage(ServerPlayer, playerIndex, message, true);
  }

  // look for a startup message -- from a file
  static const std::vector<std::string>* lines = clOptions->textChunker.getTextChunk("srvmsg");
  if (lines != NULL){
    for (int i = 0; i < (int)lines->size(); i ++){
      sendMessage(ServerPlayer, playerIndex, (*lines)[i].c_str());
    }
  }

  if (player[playerIndex].team == ObserverTeam)
    sendMessage(ServerPlayer, playerIndex, "You are in observer mode.");
#endif

  if (userExists(player[playerIndex].regName)) {
    // nick is in the DB send him a message to identify.
    if (hasPerm(getUserInfo(player[playerIndex].regName), PlayerAccessInfo::requireIdentify))
      sendMessage(ServerPlayer, playerIndex, "This callsign is registered.  You must identify yourself before playing.");
    else
      sendMessage(ServerPlayer, playerIndex, "This callsign is registered.");
    sendMessage(ServerPlayer, playerIndex, "Identify with /identify <your password>");
  }
}


static void addFlag(int flagIndex)
{
  if (flagIndex < 0) {
    // invalid flag
    return;
  }

  // flag in now entering game
  flag[flagIndex].flag.status = FlagComing;
  numFlagsInAir++;

  // compute drop time
  const float flightTime = 2.0f * sqrtf(-2.0f * BZDB.eval(StateDatabase::BZDB_FLAGALTITUDE) / BZDB.eval(StateDatabase::BZDB_GRAVITY));
  flag[flagIndex].flag.flightTime = 0.0f;
  flag[flagIndex].flag.flightEnd = flightTime;
  flag[flagIndex].flag.initialVelocity = -0.5f * BZDB.eval(StateDatabase::BZDB_GRAVITY) * flightTime;
  flag[flagIndex].dropDone = TimeKeeper::getCurrent();
  flag[flagIndex].dropDone += flightTime;

	// decide how sticky our flag is
	if (flag[flagIndex].flag.type->flagQuality == FlagBad)
    flag[flagIndex].flag.endurance = FlagSticky;
  else
    flag[flagIndex].flag.endurance = FlagUnstable;

  // how times will it stick around
  if ((flag[flagIndex].flag.endurance == FlagSticky)
  ||  (flag[flagIndex].flag.type == Flags::Thief))
    flag[flagIndex].grabs = 1;
  else
    flag[flagIndex].grabs = int(floor(BZDB.eval(StateDatabase::BZDB_MAXFLAGGRABS) * (float)bzfrand())) + 1;
  sendFlagUpdate(flagIndex);
}


static void randomFlag(int flagIndex)
{
  if (flagIndex < 0) {
    return;
  }

  // pick a random flag
  flag[flagIndex].flag.type = allowedFlags[(int)(allowedFlags.size() * (float)bzfrand())];
  if (flag[flagIndex].flag.type->flagQuality == FlagBad)
    flag[flagIndex].flag.endurance = FlagSticky;
  else
    flag[flagIndex].flag.endurance = FlagUnstable;
  addFlag(flagIndex);
}


void resetFlag(int flagIndex)
{
  // NOTE -- must not be called until world is defined
  assert(world != NULL);

  if (flagIndex < 0) {
    // invalid flag
    return;
  }
  float flagHeight = BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT);
  FlagInfo *pFlagInfo = &flag[flagIndex];
  // reset a flag's info
  pFlagInfo->player = -1;
  pFlagInfo->flag.status = FlagNoExist;

  // if it's a random flag, reset flag id
  if (flagIndex >= numFlags - clOptions->numExtraFlags)
    pFlagInfo->flag.type = Flags::Null;

  // reposition flag
  int teamIndex = pFlagInfo->flag.type->flagTeam;
  if ((teamIndex >= ::RedTeam) 
  &&  (teamIndex <= ::PurpleTeam)
  &&  (bases.find(teamIndex) != bases.end())) {
    TeamBases &teamBases = bases[teamIndex];
    const TeamBase &base = teamBases.getRandomBase( flagIndex );
    pFlagInfo->flag.position[0] = base.position[0];
    pFlagInfo->flag.position[1] = base.position[1];
    pFlagInfo->flag.position[2] = base.position[2] + base.size[2];
  } else {
    // random position (not in a building)
    float r = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
    if (pFlagInfo->flag.type == Flags::Obesity)
      r *= 2.0f * BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    WorldInfo::ObstacleLocation *obj;
    float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    pFlagInfo->flag.position[0] = (worldSize - BaseSize) * ((float)bzfrand() - 0.5f);
    pFlagInfo->flag.position[1] = (worldSize - BaseSize) * ((float)bzfrand() - 0.5f);
    pFlagInfo->flag.position[2] = 0.0f;
    int topmosttype = world->inBuilding(&obj,
					pFlagInfo->flag.position[0],
					pFlagInfo->flag.position[1],
					pFlagInfo->flag.position[2],
					r,
					flagHeight);
    while (topmosttype != NOT_IN_BUILDING) {
      if ((clOptions->flagsOnBuildings
	   && ((topmosttype == IN_BOX) || (topmosttype == IN_BASE)))
	  && (obj->pos[2] < (pFlagInfo->flag.position[2] + flagHeight - Epsilon))
	  && ((obj->pos[2] + obj->size[2] - Epsilon) > pFlagInfo->flag.position[2])
	  && (world->inRect(obj->pos, obj->rotation, obj->size, pFlagInfo->flag.position[0], pFlagInfo->flag.position[1], 0.0f)))
      {
	pFlagInfo->flag.position[2] = obj->pos[2] + obj->size[2];
      }
      else
      {
	pFlagInfo->flag.position[0] = (worldSize - BaseSize) * ((float)bzfrand() - 0.5f);
	pFlagInfo->flag.position[1] = (worldSize - BaseSize) * ((float)bzfrand() - 0.5f);
	pFlagInfo->flag.position[2] = 0.0f;
      }
      topmosttype = world->inBuilding(&obj,
				      pFlagInfo->flag.position[0],
				      pFlagInfo->flag.position[1],
				      pFlagInfo->flag.position[2],
				      r,
				      flagHeight);
    }
  }

  // required flags mustn't just disappear
  if (pFlagInfo->required) {
    if (pFlagInfo->flag.type->flagTeam != ::NoTeam) {
      if (team[pFlagInfo->flag.type->flagTeam].team.size == 0)
	pFlagInfo->flag.status = FlagNoExist;
      else
	pFlagInfo->flag.status = FlagOnGround;
    }
    else if (pFlagInfo->flag.type == Flags::Null)
      randomFlag(flagIndex);
    else
      addFlag(flagIndex);
  }

  sendFlagUpdate(flagIndex);
}


void zapFlag(int flagIndex)
{
  // called when a flag must just disappear -- doesn't fly
  // into air, just *poof* vanishes.
  if (flagIndex < 0) {
    // invalid flag
    return;
  }

  // see if someone had grabbed flag.  tell 'em to drop it.
  const int playerIndex = flag[flagIndex].player;
  if (playerIndex != -1) {
    flag[flagIndex].player = -1;
    flag[flagIndex].flag.status = FlagNoExist;
    player[playerIndex].flag = -1;

    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, playerIndex);
    buf = nboPackUShort(buf, uint16_t(flagIndex));
    buf = flag[flagIndex].flag.pack(buf);
    broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
    player[playerIndex].lastFlagDropTime = TimeKeeper::getCurrent();
  }

  // if flag was flying then it flies no more
  if (flag[flagIndex].flag.status == FlagInAir ||
      flag[flagIndex].flag.status == FlagComing ||
      flag[flagIndex].flag.status == FlagGoing)
    numFlagsInAir--;

  // reset flag status
  resetFlag(flagIndex);
}

// Take into account the quality of player wins/(wins+loss)
// Try to penalize winning casuality 
static float rabbitRank (PlayerInfo& player) {
  if (clOptions->rabbitSelection == RandomRabbitSelection)
    return (float)bzfrand();
  
  // otherwise do score-based ranking
  int sum = player.wins + player.losses;
  if (sum == 0)
    return 0.5;
  float average = (float)player.wins/(float)sum;
  // IIRC that is how wide is the gaussian
  float penalty = (1.0f - 0.5f / sqrt((float)sum));
  return average * penalty;
}

static void anointNewRabbit(int killerId = NoPlayer)
{
  float topRatio = -100000.0f;
  int i;
  int oldRabbit = rabbitIndex;
  rabbitIndex = NoPlayer;

  if (clOptions->rabbitSelection == KillerRabbitSelection)
    // check to see if the rabbit was just killed by someone; if so, make them the rabbit if they're still around.
    if (killerId != oldRabbit && realPlayer(killerId) && !player[killerId].paused
	&& !player[killerId].notResponding && (player[killerId].state == PlayerAlive)
	&& player[killerId].team != ObserverTeam)
      rabbitIndex = killerId;
  
  if (rabbitIndex == NoPlayer) {
    for (i = 0; i < curMaxPlayers; i++) {
      if (i != oldRabbit && !player[i].paused && !player[i].notResponding && (player[i].state == PlayerAlive) && (player[i].team != ObserverTeam)) {
	float ratio = rabbitRank(player[i]);
	if (ratio > topRatio) {
	  topRatio = ratio;
	  rabbitIndex = i;
	  DEBUG3("rabbitIndex is set to %d\n", rabbitIndex);
	}
      }
    }
  }
  if (rabbitIndex == NoPlayer) {
    for (i = 0; i < curMaxPlayers; i++) {
      if (player[i].state > PlayerInLimbo && !player[i].paused && !player[i].notResponding && player[i].team != ObserverTeam) {
	float ratio = rabbitRank(player[i]);
	if (ratio > topRatio) {
	  topRatio = ratio;
	  rabbitIndex = i;
	  DEBUG3("rabbitIndex is set again to %d\n", rabbitIndex);
	}
      }
    }
    DEBUG3("nobody, or no other than old rabbit to choose from, rabbitIndex is %d\n", rabbitIndex);
  }

  if (rabbitIndex != oldRabbit) {
    if (oldRabbit != NoPlayer) {
      player[oldRabbit].team = RogueTeam;
      player[oldRabbit].wasRabbit = true;
    }
    if (rabbitIndex != NoPlayer) {
      player[rabbitIndex].team = RabbitTeam;
      void *buf, *bufStart = getDirectMessageBuffer();
      buf = nboPackUByte(bufStart, rabbitIndex);
      broadcastMessage(MsgNewRabbit, (char*)buf-(char*)bufStart, bufStart);
    }
  }
}


static void pausePlayer(int playerIndex, bool paused)
{
  player[playerIndex].paused = paused;
  player[playerIndex].pausedSince = TimeKeeper::getCurrent();
  if (clOptions->gameStyle & int(RabbitChaseGameStyle)) {
    if (paused && (rabbitIndex == playerIndex)) {
      anointNewRabbit();
    }
    else if (!paused && (rabbitIndex == NoPlayer)) {
      anointNewRabbit();
    }
  }

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUByte(buf, paused);
  broadcastMessage(MsgPause, (char*)buf-(char*)bufStart, bufStart);
}


void removePlayer(int playerIndex, const char *reason, bool notify)
{
  // player is signing off or sent a bad packet.  since the
  // bad packet can come before MsgEnter, we must be careful
  // not to undo operations that haven't been done.
  // first shutdown connection

  // check if we are called again for a dropped player!
  if (player[playerIndex].fd == NotConnected)
    return;

  if (reason == NULL)
    reason = "";

  // status message
  DEBUG1("Player %s [%d] on %d removed: %s\n",
      player[playerIndex].callSign, playerIndex, player[playerIndex].fd, reason);

  // send a super kill to be polite
  if (notify)
    directMessage(playerIndex, MsgSuperKill, 0, getDirectMessageBuffer());

  // shutdown TCP socket
  shutdown(player[playerIndex].fd, 2);
  close(player[playerIndex].fd);
  player[playerIndex].fd = NotConnected;

  player[playerIndex].accessInfo.verified = false;
  player[playerIndex].accessInfo.loginAttempts = 0;
  player[playerIndex].regName.empty();

  player[playerIndex].uqueue = NULL;
  player[playerIndex].dqueue = NULL;
  player[playerIndex].lastRecvPacketNo = 0;
  player[playerIndex].lastSendPacketNo = 0;

  // shutdown the UDP socket
  memset(&player[playerIndex].uaddr, 0, sizeof(player[playerIndex].uaddr));

  // no UDP connection anymore
  player[playerIndex].udpin = false;
  player[playerIndex].udpout = false;
  player[playerIndex].toBeKicked = false;
  player[playerIndex].udplen = 0;

  player[playerIndex].tcplen = 0;

  player[playerIndex].callSign[0] = 0;

  if (player[playerIndex].outmsg != NULL) {
    delete[] player[playerIndex].outmsg;
    player[playerIndex].outmsg = NULL;
  }
  player[playerIndex].outmsgSize = 0;

  player[playerIndex].flagHistory.clear();

#ifdef HAVE_ADNS_H
  if (player[playerIndex].hostname) {
    free(player[playerIndex].hostname);
    player[playerIndex].hostname = NULL;
  }
#endif

  // player is outta here.  if player never joined a team then
  // don't count as a player.
  if (player[playerIndex].state == PlayerInLimbo) {
    player[playerIndex].state = PlayerNoExist;

    while ((playerIndex >= 0)
	&& (playerIndex+1 == curMaxPlayers)
	&& (player[playerIndex].state == PlayerNoExist)
	&& (player[playerIndex].fd == NotConnected))
    {
      playerIndex--;
      curMaxPlayers--;
    }
    return;
  }

  player[playerIndex].state = PlayerNoExist;

  // if there is an active poll, cancel any vote this player may have made
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");
  if ((arbiter != NULL) && (arbiter->knowsPoll())) {
    arbiter->retractVote(player[playerIndex].callSign);
  }

  if (clOptions->gameStyle & int(RabbitChaseGameStyle))
    if (playerIndex == rabbitIndex)
      anointNewRabbit();

  if (player[playerIndex].team != NoTeam) {
    int flagid = player[playerIndex].flag;
    if (flagid >= 0) {
      // do not simply zap team flag
      Flag &carriedflag = flag[flagid].flag;
      if (carriedflag.type->flagTeam != ::NoTeam) {
	dropFlag(playerIndex, player[playerIndex].lastState.pos);
      }
      else {
	zapFlag(flagid);
      }
    }

    // tell everyone player has left
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, playerIndex);
    broadcastMessage(MsgRemovePlayer, (char*)buf-(char*)bufStart, bufStart);

    // decrease team size
    int teamNum = int(player[playerIndex].team);
    --team[teamNum].team.size;

    // if last active player on team then remove team's flag if no one
    // is carrying it
    if (Team::isColorTeam(player[playerIndex].team) && team[teamNum].team.size == 0 &&
        (clOptions->gameStyle & int(TeamFlagGameStyle))) {
      int flagid = lookupFirstTeamFlag(teamNum);
      if (flagid >= 0) {
        for (int n = 0; n < clOptions->numTeamFlags[teamNum]; n++) {
          if ((flag[flagid+n].player == -1 || player[flag[flagid+n].player].team == teamNum))
	    zapFlag(flagid+n);
	}
      }
    }

    // send team update
    sendTeamUpdate(-1, teamNum);
  }

#ifdef NETWORK_STATS
  dumpPlayerMessageStats(playerIndex);
#endif

  fixTeamCount();

  // tell the list server the new number of players
  sendMessageToListServer(ListServerLink::ADD);

  while ((playerIndex >= 0)
      && (playerIndex+1 == curMaxPlayers)
      && (player[playerIndex].state == PlayerNoExist)
      && (player[playerIndex].fd == NotConnected))
  {
     playerIndex--;
     curMaxPlayers--;
  }

  // anybody left?
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i].state > PlayerInLimbo)
      break;

  // if everybody left then reset world
  if (i == curMaxPlayers) {
    if (!clOptions->worldFile) {
      bases.clear();
    }

    if (clOptions->oneGameOnly) {
      done = true;
      exitCode = 0;
    }
    else if ((!clOptions->worldFile) && (!defineWorld())) {
      done = true;
      exitCode = 1;
    }
    else {
      // republicize ourself.  this dereferences the URL chain
      // again so we'll notice any pointer change when any game
      // is over (i.e. all players have quit).
      publicize();
    }
  }
}

// are the two teams foes withthe current game style?
static bool areFoes(TeamColor team1, TeamColor team2)
{
  return team1!=team2 ||
         (team1==RogueTeam && !(clOptions->gameStyle & int(RabbitChaseGameStyle)));
}

static float enemyProximityCheck(TeamColor team, float *pos)
{
  float worstDist = 1e12f; // huge number

  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].state == PlayerAlive && areFoes(player[i].team, team)) {
      float *enemyPos = player[i].lastState.pos;
      if (fabs(enemyPos[2] - pos[2]) < 1.0f) {
        float x = enemyPos[0] - pos[0];
        float y = enemyPos[1] - pos[1];
        float distSq = x * x + y * y;
        if (distSq < worstDist)
          worstDist = distSq;
      }
    }
  }

  return sqrtf(worstDist);
}

static void getSpawnLocation(int playerId, float* spawnpos, float *azimuth)
{
  const float tankRadius = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
  const TeamColor team = player[playerId].team;
  if (player[playerId].restartOnBase &&
      (team >= RedTeam) && (team <= PurpleTeam) && 
      (bases.find(team) != bases.end())) {
    TeamBases &teamBases = bases[team];
    const TeamBase &base = teamBases.getRandomBase( (int) (bzfrand() * 100) );
    base.getRandomPosition( spawnpos[0], spawnpos[1], spawnpos[2] );
    player[playerId].restartOnBase = false;
  }
  else {
    bool onGroundOnly = (!clOptions->respawnOnBuildings) || (player[playerId].type == ComputerPlayer);
    const float size = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    WorldInfo::ObstacleLocation *building;

    // keep track of how much time we spend searching for a location
    TimeKeeper start = TimeKeeper::getCurrent();

    int inAirAttempts = 50;
    int tries = 0;
    float minProximity = size / 3.0f;
    float bestDist = -1.0f;
    float pos[3];
    bool foundspot = false;
    while (!foundspot) {
      if (clOptions->gameStyle & TeamFlagGameStyle) {
        // don't spawn close to map edges in CTF mode
        pos[0] = ((float)bzfrand() - 0.5f) * size * 0.6f;
        pos[1] = ((float)bzfrand() - 0.5f) * size * 0.6f;
      }
      else {
        pos[0] = ((float)bzfrand() - 0.5f) * (size - 2.0f * tankRadius);
        pos[1] = ((float)bzfrand() - 0.5f) * (size - 2.0f * tankRadius);
      }
      pos[2] = onGroundOnly ? 0.0f : ((float)bzfrand() * maxWorldHeight);
      tries++;

      int type = world->inBuilding(&building, pos[0], pos[1], pos[2],
                                   tankRadius, BZDBCache::tankHeight);

      if (onGroundOnly) {
        if (type == NOT_IN_BUILDING)
          foundspot = true;
      }
      else {
        if ((type == NOT_IN_BUILDING) && (pos[2] > 0.0f)) {
          pos[2] = 0.0f;
          //Find any intersection regardless of z
          type = world->inBuilding(&building, pos[0], pos[1], pos[2],
                                   tankRadius, maxWorldHeight);
        }

        // in a building? try climbing on roof until on top
        int lastType = type;
	int retriesRemaining = 100; // don't climb forever
        while (type != NOT_IN_BUILDING) {
          pos[2] = building->pos[2] + building->size[2] + 0.0001f;
          tries++;
          lastType = type;
          type = world->inBuilding(&building, pos[0], pos[1], pos[2],
                                   tankRadius, BZDBCache::tankHeight);
	  if (--retriesRemaining <= 0) {
	    DEBUG1("Warning: getSpawnLocation had to climb too many buildings\n");
	    break;
	  }
        }
        // ok, when not on top of pyramid or teleporter
        if (lastType != IN_PYRAMID  &&  lastType != IN_TELEPORTER) {
          foundspot = true;
        }
        // only try up in the sky so many times
        if (--inAirAttempts <= 0) {
          onGroundOnly = true;
        }
      }

      // check every now and then if we have already used up 10ms of time
      if (tries >= 50) {
        tries=0;
        if (TimeKeeper::getCurrent() - start > 0.01f) {
          if (bestDist < 0.0f) { // haven't found a single spot
            //Just drop the sucka in, and pray
            spawnpos[0] = pos[0];
            spawnpos[1] = pos[1];
            spawnpos[2] = maxWorldHeight;
	    DEBUG1("Warning: getSpawnLocation ran out of time, just dropping the sucker in\n");
          }
          break;
        }
      }

      // check if spot is safe enough
      if (foundspot) {
        float dist = enemyProximityCheck(team, pos);
        if (dist > bestDist) { // best so far
          bestDist = dist;
          spawnpos[0] = pos[0];
          spawnpos[1] = pos[1];
          spawnpos[2] = pos[2];
        }
        if (bestDist < minProximity) { // not good enough, keep looking
          foundspot = false;
          minProximity *= 0.99f; // relax requirements a little
        }
      }
    }
  }
  *azimuth = (float)bzfrand() * 2.0f * M_PI;
}

static void sendWorld(int playerIndex, uint32_t ptr)
{
  // send another small chunk of the world database
  assert(world != NULL && worldDatabase != NULL);
  void *buf, *bufStart = getDirectMessageBuffer();
  uint32_t size = MaxPacketLen - 2*sizeof(uint16_t) - sizeof(uint32_t), left = worldDatabaseSize - ptr;
  if (ptr >= worldDatabaseSize) {
    size = 0;
    left = 0;
  } else if (ptr + size >= worldDatabaseSize) {
      size = worldDatabaseSize - ptr;
      left = 0;
  }
  buf = nboPackUInt(bufStart, uint32_t(left));
  buf = nboPackString(buf, (char*)worldDatabase + ptr, size);
  directMessage(playerIndex, MsgGetWorld, (char*)buf-(char*)bufStart, bufStart);
}

static void sendQueryGame(int playerIndex)
{
  // much like a ping packet but leave out useless stuff (like
  // the server address, which must already be known, and the
  // server version, which was already sent).
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, pingReply.gameStyle);
  buf = nboPackUShort(buf, pingReply.maxPlayers);
  buf = nboPackUShort(buf, pingReply.maxShots);
  buf = nboPackUShort(buf, team[0].team.size);
  buf = nboPackUShort(buf, team[1].team.size);
  buf = nboPackUShort(buf, team[2].team.size);
  buf = nboPackUShort(buf, team[3].team.size);
  buf = nboPackUShort(buf, team[4].team.size);
  buf = nboPackUShort(buf, pingReply.rogueMax);
  buf = nboPackUShort(buf, pingReply.redMax);
  buf = nboPackUShort(buf, pingReply.greenMax);
  buf = nboPackUShort(buf, pingReply.blueMax);
  buf = nboPackUShort(buf, pingReply.purpleMax);
  buf = nboPackUShort(buf, pingReply.shakeWins);
  // 1/10ths of second
  buf = nboPackUShort(buf, pingReply.shakeTimeout);
  buf = nboPackUShort(buf, pingReply.maxPlayerScore);
  buf = nboPackUShort(buf, pingReply.maxTeamScore);
  buf = nboPackUShort(buf, pingReply.maxTime);

  // send it
  directMessage(playerIndex, MsgQueryGame, (char*)buf-(char*)bufStart, bufStart);
}

static void sendQueryPlayers(int playerIndex)
{
  int i, numPlayers = 0;

  // count the number of active players
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i].state > PlayerInLimbo)
      numPlayers++;

  // first send number of teams and players being sent
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, NumTeams);
  buf = nboPackUShort(buf, numPlayers);
  directMessage(playerIndex, MsgQueryPlayers, (char*)buf-(char*)bufStart, bufStart);

  // now send the teams and players
  if (player[playerIndex].fd != NotConnected)
    sendTeamUpdate(playerIndex);
  for (i = 0; i < curMaxPlayers && player[playerIndex].fd != NotConnected; i++)
    if (player[i].state > PlayerInLimbo)
      sendPlayerUpdate(i, playerIndex);
}

static void playerAlive(int playerIndex)
{
  // ignore multiple MsgAlive; also observer should not send MsgAlive; diagnostic?
  if (player[playerIndex].state != PlayerDead ||
      player[playerIndex].team == ObserverTeam)
    return;

  // make sure the user identifies themselves if required.
  if (!player[playerIndex].accessInfo.verified && userExists(player[playerIndex].regName) && hasPerm(getUserInfo(player[playerIndex].regName), PlayerAccessInfo::requireIdentify)) {
    sendMessage(ServerPlayer, playerIndex, "This callsign is registered.  You must identify yourself before playing or use a different callsign.");
    removePlayer(playerIndex, "unidentified");
    return;
  }
  
  // disallow roger from respawning if we disable roger.
  if (player[playerIndex].quellRoger) {
    sendMessage(ServerPlayer, playerIndex, "I'm sorry, we do not allow autopilot on this server.");
    removePlayer(playerIndex, "roger");
    return;
  }

  if (player[playerIndex].type == ComputerPlayer && clOptions->prohibitBots) {
    sendMessage(ServerPlayer, playerIndex, "I'm sorry, we do not allow bots on this server.");
    removePlayer(playerIndex, "ComputerPlayer");
    return;
  }

  // player is coming alive.
  player[playerIndex].state = PlayerAlive;
  player[playerIndex].flag = -1;

  // send MsgAlive
  float pos[3], fwd;
  getSpawnLocation(playerIndex, pos, &fwd);
  // update last position immediately
  player[playerIndex].lastState.pos[0] = pos[0];
  player[playerIndex].lastState.pos[1] = pos[1];
  player[playerIndex].lastState.pos[2] = pos[2];
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackVector(buf,pos);
  buf = nboPackFloat(buf,fwd);
  broadcastMessage(MsgAlive, (char*)buf-(char*)bufStart, bufStart);

  if (clOptions->gameStyle & int(RabbitChaseGameStyle)) {
    player[playerIndex].wasRabbit = false;
    if (rabbitIndex == NoPlayer) {
      anointNewRabbit();
    }
  }
}

static void checkTeamScore(int playerIndex, int teamIndex)
{
  if (clOptions->maxTeamScore == 0 || !Team::isColorTeam(TeamColor(teamIndex))) return;
  if (team[teamIndex].team.won - team[teamIndex].team.lost >= clOptions->maxTeamScore) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, playerIndex);
    buf = nboPackUShort(buf, uint16_t(teamIndex));
    broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
    gameOver = true;
  }
}

// FIXME - needs extra checks for killerIndex=ServerPlayer (world weapons)
// (was broken before); it turns out that killerIndex=-1 for world weapon?
// No need to check on victimIndex.
//   It is taken as the index of the udp table when called by incoming message
//   It is taken by killerIndex when autocalled, but only if != -1
// killer could be InvalidPlayer or a number within [0 curMaxPlayer)
static void playerKilled(int victimIndex, int killerIndex, int reason,
			int16_t shotIndex)
{
  if (!realPlayer(victimIndex))
    return;

  // aliases for convenience
  // Warning: killer should not be used when killerIndex == InvalidPlayer or ServerPlayer
  PlayerInfo *killer = realPlayer(killerIndex) ? &player[killerIndex] : 0,
             *victim = &player[victimIndex];

  // victim was already dead. keep score.
  if (victim->state != PlayerAlive) return;

  victim->state = PlayerDead;

  // killing rabbit or killing anything when I am a dead ex-rabbit is allowed
  bool teamkill = false;
  if (killer) {
    const bool rabbitinvolved = killer->wasRabbit || victim->team == RabbitTeam;
    const bool foe = areFoes(victim->team, killer->team);
    teamkill = !foe && !rabbitinvolved;
  }

  //update tk-score
  if ((victimIndex != killerIndex) && teamkill) {
     killer->tks++;
     if (killer->tks >= 3 && (clOptions->teamKillerKickRatio > 0) && // arbitrary 3
         (killer->wins == 0 ||
          killer->tks * 100 / killer->wins > clOptions->teamKillerKickRatio)) {
       char message[MessageLen];
       strcpy(message, "You have been automatically kicked for team killing" );
       sendMessage(ServerPlayer, killerIndex, message, true);
       removePlayer(killerIndex, "teamkilling");
     }
  }

  // send MsgKilled
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, victimIndex);
  buf = nboPackUByte(buf, killerIndex);
  buf = nboPackShort(buf, reason);
  buf = nboPackShort(buf, shotIndex);
  broadcastMessage(MsgKilled, (char*)buf-(char*)bufStart, bufStart);

  // zap flag player was carrying.  clients should send a drop flag
  // message before sending a killed message, so this shouldn't happen.
  int flagid = victim->flag;
  if (flagid >= 0) {
    // do not simply zap team flag
    Flag &carriedflag = flag[flagid].flag;
    if (carriedflag.type->flagTeam != ::NoTeam) {
      dropFlag(victimIndex, carriedflag.position);
    }
    else {
      zapFlag(flagid);
    }
  }

  // change the player score
  bufStart = getDirectMessageBuffer();
  victim->losses++;
  if (killer) {
    if (victimIndex != killerIndex) {
      if (teamkill) {
        if (clOptions->teamKillerDies)
          playerKilled(killerIndex, killerIndex, reason, -1);
        else
          killer->losses++;
      } else
        killer->wins++;
    }

    buf = nboPackUByte(bufStart, 2);
    buf = nboPackUByte(buf, killerIndex);
    buf = nboPackUShort(buf, killer->wins);
    buf = nboPackUShort(buf, killer->losses);
    buf = nboPackUShort(buf, killer->tks);
  }
  else {
    buf = nboPackUByte(bufStart, 1);
  }

  buf = nboPackUByte(buf, victimIndex);
  buf = nboPackUShort(buf, victim->wins);
  buf = nboPackUShort(buf, victim->losses);
  buf = nboPackUShort(buf, victim->tks);
  broadcastMessage(MsgScore, (char*)buf-(char*)bufStart, bufStart);

  // see if the player reached the score limit
  if (clOptions->maxPlayerScore != 0
      && killerIndex != InvalidPlayer
      && killerIndex != ServerPlayer
      && killer->wins - killer->losses >= clOptions->maxPlayerScore) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, killerIndex);
    buf = nboPackUShort(buf, uint16_t(NoTeam));
    broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
    gameOver = true;
  }

  if (clOptions->gameStyle & int(RabbitChaseGameStyle)) {
    if (victimIndex == rabbitIndex)
      anointNewRabbit(killerIndex);
  } else {
    // change the team scores -- rogues don't have team scores.  don't
    // change team scores for individual player's kills in capture the
    // flag mode.
    // Team score is even not used on RabbitChase
    int winningTeam = (int)NoTeam;
    if (!(clOptions->gameStyle & (TeamFlagGameStyle | RabbitChaseGameStyle))) {
      int killerTeam = -1;
      if (killer && victim->team == killer->team) {
	if (killer->team != RogueTeam)
	  if (killerIndex == victimIndex)
	    team[int(victim->team)].team.lost += 1;
	  else
	    team[int(victim->team)].team.lost += 2;
      } else {
	if (killer && killer->team != RogueTeam) {
	  winningTeam = int(killer->team);
	  team[winningTeam].team.won++;
	}
	if (victim->team != RogueTeam)
	  team[int(victim->team)].team.lost++;
	if (killer)
	  killerTeam = killer->team;
      }
      sendTeamUpdate(-1,int(victim->team), killerTeam);
    }
#ifdef PRINTSCORE
    dumpScore();
#endif
    if (winningTeam != (int)NoTeam)
      checkTeamScore(killerIndex, winningTeam);
  }
}

static void grabFlag(int playerIndex, int flagIndex)
{
  // Sanity check
  if (flagIndex < -1 || flagIndex >= numFlags)
    return;

  // player wants to take possession of flag
  if (player[playerIndex].team == ObserverTeam ||
      player[playerIndex].state != PlayerAlive ||
      player[playerIndex].flag != -1 ||
      flag[flagIndex].flag.status != FlagOnGround)
    return;

  //last Pos might be lagged by TankSpeed so include in calculation
  const float tankRadius = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
  const float tankSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);
  const float radius2 = (tankSpeed + tankRadius + BZDBCache::flagRadius) * (tankSpeed + tankRadius + BZDBCache::flagRadius);
  const float* tpos = player[playerIndex].lastState.pos;
  const float* fpos = flag[flagIndex].flag.position;
  const float delta = (tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
		      (tpos[1] - fpos[1]) * (tpos[1] - fpos[1]);

  if ((fabs(tpos[2] - fpos[2]) < 0.1f) && (delta > radius2)) {
       DEBUG2("Player %s [%d] %f %f %f tried to grab distant flag %f %f %f: distance=%f\n",
    player[playerIndex].callSign, playerIndex,
    tpos[0], tpos[1], tpos[2], fpos[0], fpos[1], fpos[2], sqrt(delta));
    return;
  }

  // okay, player can have it
  flag[flagIndex].flag.status = FlagOnTank;
  flag[flagIndex].flag.owner = playerIndex;
  flag[flagIndex].player = playerIndex;
  flag[flagIndex].numShots = 0;
  player[playerIndex].flag = flagIndex;

  // send MsgGrabFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = flag[flagIndex].flag.pack(buf);
  broadcastMessage(MsgGrabFlag, (char*)buf-(char*)bufStart, bufStart);

  std::vector<FlagType*> *pFH = &player[playerIndex].flagHistory;
  if (pFH->size() >= MAX_FLAG_HISTORY)
    pFH->erase(pFH->begin());
  pFH->push_back(flag[flagIndex].flag.type );
}

static void dropFlag(int playerIndex, float pos[3])
{
  const float size = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  if (pos[0] < -size || pos[0] > size)
    pos[0] = 0.0;
  if (pos[1] < -size || pos[1] > size)
    pos[1] = 0.0;
  if (pos[2] > maxWorldHeight)
    pos[2] = 0.0;

  assert(world != NULL);
  WorldInfo::ObstacleLocation* container;
  int topmosttype = NOT_IN_BUILDING;
  WorldInfo::ObstacleLocation* topmost = 0;
  // player wants to drop flag.  we trust that the client won't tell
  // us to drop a sticky flag until the requirements are satisfied.
  const int flagIndex = player[playerIndex].flag;
  if (flagIndex < 0)
    return;
  FlagInfo &drpFlag = flag[flagIndex];
  if (drpFlag.flag.status != FlagOnTank)
    return;
  int flagTeam = drpFlag.flag.type->flagTeam;
  bool isTeamFlag = (flagTeam != ::NoTeam);

  // okay, go ahead and drop it
  drpFlag.player = -1;
  drpFlag.numShots = 0;
  numFlagsInAir++;
	
	// limited flags should be disposed of 
	bool limited = clOptions->flagLimit[drpFlag.flag.type] != -1;
	if (limited) drpFlag.grabs = 0;

  // note: sticky/bad flags should always have grabs=1
  if (isTeamFlag || (--drpFlag.grabs > 0))
    drpFlag.flag.status = FlagInAir;
  else
    drpFlag.flag.status = FlagGoing;

  topmosttype = world->inBuilding(&container,
				  pos[0], pos[1], pos[2], 0,
				  BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT));

  // the tank is inside a building - find the roof
  if (topmosttype != NOT_IN_BUILDING) {
    topmost = container;
    int tmp;
    for (float i = container->pos[2] + container->size[2];
	 (tmp = world->inBuilding(&container,
				  pos[0], pos[1], i, 0,
				  BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT))) !=
	   NOT_IN_BUILDING; i += 0.1f) {
      topmosttype = tmp;
      topmost = container;
    }
  }

  // the tank is _not_ inside a building - find the floor
  else {
    for (float i = pos[2]; i >= 0.0f; i -= 0.1f) {
      topmosttype = world->inBuilding(&topmost,
				      pos[0], pos[1], i, 0,
				      BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT));
      if (topmosttype != NOT_IN_BUILDING)
	break;
    }
  }

  // figure out landing spot -- if flag in a Bad Place
  // when dropped, move to safety position or make it going
  TeamColor teamBase = whoseBase(pos[0], pos[1],
				 (topmosttype == NOT_IN_BUILDING ? pos[2] :
				  topmost->pos[2] + topmost->size[2] + 0.01f));

  if (drpFlag.flag.status == FlagGoing) {
    drpFlag.flag.landingPosition[0] = pos[0];
    drpFlag.flag.landingPosition[1] = pos[1];
    drpFlag.flag.landingPosition[2] = pos[2];
  }
  else if (isTeamFlag && (teamBase == flagTeam) && (topmosttype == IN_BASE)) {
    drpFlag.flag.landingPosition[0] = pos[0];
    drpFlag.flag.landingPosition[1] = pos[1];
    drpFlag.flag.landingPosition[2] = topmost->pos[2] + topmost->size[2];
  }
  else if (isTeamFlag && (teamBase != NoTeam) && (teamBase != flagTeam) && (bases.find(teamBase) != bases.end())) {
    bases[teamBase].getSafetyZone( drpFlag.flag.landingPosition[0],
                                   drpFlag.flag.landingPosition[1],
                                   drpFlag.flag.landingPosition[2] );
  }
  else if (topmosttype == NOT_IN_BUILDING) {
    drpFlag.flag.landingPosition[0] = pos[0];
    drpFlag.flag.landingPosition[1] = pos[1];
    drpFlag.flag.landingPosition[2] = 0.0f;
  }
  else if (clOptions->flagsOnBuildings && (topmosttype == IN_BOX || topmosttype == IN_BASE)) {
    drpFlag.flag.landingPosition[0] = pos[0];
    drpFlag.flag.landingPosition[1] = pos[1];
    drpFlag.flag.landingPosition[2] = topmost->pos[2] + topmost->size[2];
  }
  else if (isTeamFlag) {
    // people were cheating by dropping their flag above the nearest
    // convenient building which makes it fly all the way back to
    // your own base.  make it fly to the center of the board.
    topmosttype = world->inBuilding(&container,
				    0.0f, 0.0f, 0.0f,
				    BZDB.eval(StateDatabase::BZDB_TANKRADIUS),
				    BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT));
    if (topmosttype == NOT_IN_BUILDING) {
	drpFlag.flag.landingPosition[0] = 0.0f;
	drpFlag.flag.landingPosition[1] = 0.0f;
	drpFlag.flag.landingPosition[2] = 0.0f;
    }
    else {// oh well, whatcha gonna do?
	TeamBases &teamBases = bases[flagTeam];
	const TeamBase &base = teamBases.getRandomBase(flagIndex); 
	drpFlag.flag.landingPosition[0] = base.position[0];
	drpFlag.flag.landingPosition[1] = base.position[1];
	drpFlag.flag.landingPosition[2] = base.position[2] + base.size[2];
    }
  }
  else
    drpFlag.flag.status = FlagGoing;

  // if it is a team flag, check if there are any players left in that team -
  // if not, start the flag timeout
  if (isTeamFlag && team[drpFlag.flag.type->flagTeam].team.size == 0) {
    team[flagIndex + 1].flagTimeout = TimeKeeper::getCurrent();
    team[flagIndex + 1].flagTimeout += (float)clOptions->teamFlagTimeout;
  }

  drpFlag.flag.position[0] = drpFlag.flag.landingPosition[0];
  drpFlag.flag.position[1] = drpFlag.flag.landingPosition[1];
  drpFlag.flag.position[2] = drpFlag.flag.landingPosition[2];
  drpFlag.flag.launchPosition[0] = pos[0];
  drpFlag.flag.launchPosition[1] = pos[1];
  drpFlag.flag.launchPosition[2] = pos[2] + BZDBCache::tankHeight;

  // compute flight info -- flight time depends depends on start and end
  // altitudes and desired height above start altitude
  const float flagAltitude = BZDB.eval(StateDatabase::BZDB_FLAGALTITUDE);
  const float thrownAltitude = (drpFlag.flag.type == Flags::Shield) ?
     BZDB.eval(StateDatabase::BZDB_SHIELDFLIGHT) * flagAltitude : flagAltitude;
  const float maxAltitude = pos[2] + thrownAltitude;
  const float upTime = sqrtf(-2.0f * thrownAltitude / BZDB.eval(StateDatabase::BZDB_GRAVITY));
  const float downTime = sqrtf(-2.0f * (maxAltitude - pos[2]) / BZDB.eval(StateDatabase::BZDB_GRAVITY));
  const float flightTime = upTime + downTime;

  // set flight info
  drpFlag.dropDone = TimeKeeper::getCurrent();
  drpFlag.dropDone += flightTime;
  drpFlag.flag.flightTime = 0.0f;
  drpFlag.flag.flightEnd = flightTime;
  drpFlag.flag.initialVelocity = -BZDB.eval(StateDatabase::BZDB_GRAVITY) * upTime;

  // player no longer has flag -- send MsgDropFlag
  player[playerIndex].flag = -1;
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = drpFlag.flag.pack(buf);
  broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);

  // notify of new flag state
  sendFlagUpdate(flagIndex);

  player[playerIndex].lastFlagDropTime = TimeKeeper::getCurrent();
}

static void captureFlag(int playerIndex, TeamColor teamCaptured)
{
  // Sanity check
  if (teamCaptured < RedTeam || teamCaptured > PurpleTeam)
    return;

  // player captured a flag.  can either be enemy flag in player's own
  // team base, or player's own flag in enemy base.
  int flagIndex = int(player[playerIndex].flag);
  if (flagIndex < 0 || (flag[flagIndex].flag.type->flagTeam == ::NoTeam))
    return;

  // player no longer has flag and put flag back at it's base
  player[playerIndex].flag = -1;
  resetFlag(flagIndex);

  // send MsgCaptureFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = nboPackUShort(buf, uint16_t(teamCaptured));
  broadcastMessage(MsgCaptureFlag, (char*)buf-(char*)bufStart, bufStart);

  // everyone on losing team is dead
  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i].fd != NotConnected &&
	flag[flagIndex].flag.type->flagTeam == int(player[i].team) &&
	player[i].state >= PlayerDead) {
      player[i].state = PlayerDead;
      player[i].restartOnBase = true;
    }

  // update score (rogues can't capture flags)
  int winningTeam = (int)NoTeam;
  if (int(flag[flagIndex].flag.type->flagTeam) != int(player[playerIndex].team)) {
    // player captured enemy flag
    winningTeam = int(player[playerIndex].team);
    team[winningTeam].team.won++;
  }
  team[int(flag[flagIndex].flag.type->flagTeam)].team.lost++;
  sendTeamUpdate(-1, winningTeam, int(flag[flagIndex].flag.type->flagTeam));
#ifdef PRINTSCORE
  dumpScore();
#endif
  if (winningTeam != (int)NoTeam)
    checkTeamScore(playerIndex, winningTeam);
}

static void shotFired(int playerIndex, void *buf, int len)
{
  bool repack = false;
  const PlayerInfo &shooter = player[playerIndex];
  if (shooter.team == ObserverTeam)
    return;
  if (shooter.quellRoger) {
    return;			// don't let rogers shoot if we disallow autopilot
  }
  FiringInfo firingInfo;
  firingInfo.unpack(buf);
  const ShotUpdate &shot = firingInfo.shot;

  // verify playerId
  if (shot.player != playerIndex) {
    DEBUG2("Player %s [%d] shot playerid mismatch\n", shooter.callSign, playerIndex);
    return;
  }

  // make sure the shooter flag is a valid index to prevent segfaulting later
  if (shooter.flag < 0) {
    firingInfo.flagType = Flags::Null;
    repack = true;
  }

  // verify player flag
  if ((firingInfo.flagType != Flags::Null) && (firingInfo.flagType != flag[shooter.flag].flag.type)) {
    DEBUG2("Player %s [%d] shot flag mismatch %s %s\n", shooter.callSign,
	   playerIndex, firingInfo.flagType->flagAbbv, flag[shooter.flag].flag.type->flagAbbv);
    firingInfo.flagType = Flags::Null;
    firingInfo.shot.vel[0] = BZDB.eval(StateDatabase::BZDB_SHOTSPEED) * cos(shooter.lastState.azimuth);
    firingInfo.shot.vel[1] = BZDB.eval(StateDatabase::BZDB_SHOTSPEED) * sin(shooter.lastState.azimuth);
    firingInfo.shot.vel[2] = 0.0f;
  }

  // verify shot number
  if ((shot.id & 0xff) > clOptions->maxShots - 1) {
    DEBUG2("Player %s [%d] shot id out of range %d %d\n", shooter.callSign,
	   playerIndex,	shot.id & 0xff, clOptions->maxShots);
    return;
  }

  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  float tankSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);
  float lifetime = BZDB.eval(StateDatabase::BZDB_RELOADTIME);
  if (firingInfo.flagType == Flags::ShockWave) {
      shotSpeed = 0.0f;
      tankSpeed = 0.0f;
  }
  else if (firingInfo.flagType == Flags::Velocity) {
      tankSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
  }
  else if (firingInfo.flagType == Flags::Thief) {
      tankSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
  }
  else if ((firingInfo.flagType == Flags::Burrow) && (firingInfo.shot.pos[2] < BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT))) {
      tankSpeed *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
  }
  else {
      //If shot is different height than player, can't be sure they didn't drop V in air
      if (shooter.lastState.pos[2] != (shot.pos[2]-BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT)))
	tankSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
  }

  // FIXME, we should look at the actual TankSpeed ;-)
  shotSpeed += tankSpeed;

  // verify lifetime
  if (fabs(firingInfo.lifetime - lifetime) > Epsilon) {
    DEBUG2("Player %s [%d] shot lifetime mismatch %f %f\n", shooter.callSign,
	   playerIndex, firingInfo.lifetime, lifetime);
    return;
  }

  // verify velocity
  if (hypotf(shot.vel[0], hypotf(shot.vel[1], shot.vel[2])) > shotSpeed * 1.01f) {
    DEBUG2("Player %s [%d] shot over speed %f %f\n", shooter.callSign,
	   playerIndex, hypotf(shot.vel[0], hypotf(shot.vel[1], shot.vel[2])),
	   shotSpeed);
    return;
  }

  // verify position
  float dx = shooter.lastState.pos[0] - shot.pos[0];
  float dy = shooter.lastState.pos[1] - shot.pos[1];
  float dz = shooter.lastState.pos[2] - shot.pos[2];

  float front = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT);
  if (firingInfo.flagType == Flags::Obesity)
    front *= BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);

  float delta = dx*dx + dy*dy + dz*dz;
  if (delta > (BZDB.eval(StateDatabase::BZDB_TANKSPEED) * BZDB.eval(StateDatabase::BZDB_VELOCITYAD) + front) *
	      (BZDB.eval(StateDatabase::BZDB_TANKSPEED) * BZDB.eval(StateDatabase::BZDB_VELOCITYAD) + front)) {
    DEBUG2("Player %s [%d] shot origination %f %f %f too far from tank %f %f %f: distance=%f\n",
	    shooter.callSign, playerIndex,
	    shot.pos[0], shot.pos[1], shot.pos[2],
	    shooter.lastState.pos[0], shooter.lastState.pos[1],
	    shooter.lastState.pos[2], sqrt(delta));
    return;
  }

  // repack if changed
  if (repack)
    firingInfo.pack(buf);


  // if shooter has a flag

  char message[MessageLen];
  if (shooter.flag >= 0){

    FlagInfo & fInfo = flag[shooter.flag];
    fInfo.numShots++; // increase the # shots fired

    int limit = clOptions->flagLimit[fInfo.flag.type];
    if (limit != -1){ // if there is a limit for players flag
      int shotsLeft = limit -  fInfo.numShots;
      if (shotsLeft > 0) { //still have some shots left
	// give message each shot below 5, each 5th shot & at start
	if (shotsLeft % 5 == 0 || shotsLeft <= 3 || shotsLeft == limit-1){
	  sprintf(message,"%d shots left",shotsLeft);
	  sendMessage(ServerPlayer, playerIndex, message, true);
	}
      } else { // no shots left
	if (shotsLeft == 0 || (limit == 0 && shotsLeft < 0)){
	  // drop flag at last known position of player
	  // also handle case where limit was set to 0
	  float lastPos [3];
	  for (int i = 0; i < 3; i ++){
	    lastPos[i] = shooter.lastState.pos[i];
	  }
	  fInfo.grabs = 0; // recycle this flag now
	  dropFlag(playerIndex, lastPos);
	} else { // more shots fired than allowed
	  // do nothing for now -- could return and not allow shot
	}
      } // end no shots left
    } // end is limit
  } // end of player has flag

  broadcastMessage(MsgShotBegin, len, buf);

}

static void shotEnded(const PlayerId& id, int16_t shotIndex, uint16_t reason)
{
  // shot has ended prematurely -- send MsgShotEnd
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, id);
  buf = nboPackShort(buf, shotIndex);
  buf = nboPackUShort(buf, reason);
  broadcastMessage(MsgShotEnd, (char*)buf-(char*)bufStart, bufStart);
}



// updateLagLost: update % lost/out of order packets
// updated on lost/received LagPing echo and lost/ooo/received
// MsgPlayerUpdate
static void updateLagLost(int playerIndex, bool lost=true)
{
  PlayerInfo &pl=player[playerIndex];
  pl.lostavg = pl.lostavg*(1-pl.lostalpha) + pl.lostalpha*lost;
  pl.lostalpha = pl.lostalpha / (0.99f + pl.lostalpha);
}

// update absolute latency based on LagPing messages
static void updateLag(int playerIndex, float timepassed)
{
  PlayerInfo &pl=player[playerIndex];
  // time is smoothed exponentially using a dynamic smoothing factor
  pl.lagavg = pl.lagavg*(1-pl.lagalpha) + pl.lagalpha*timepassed;
  pl.lagalpha = pl.lagalpha / (0.9f + pl.lagalpha);
  pl.lagcount++;
  // warn players from time to time whose lag is > threshold (-lagwarn)
  if (pl.team != ObserverTeam && clOptions->lagwarnthresh > 0 && pl.lagavg > clOptions->lagwarnthresh &&
      pl.lagcount - pl.laglastwarn > 2 * pl.lagwarncount) {
    char message[MessageLen];
    sprintf(message,"*** Server Warning: your lag is too high (%d ms) ***",
        int(pl.lagavg * 1000));
    sendMessage(ServerPlayer, playerIndex, message, true);
    pl.laglastwarn = pl.lagcount;
    pl.lagwarncount++;;
    if (pl.lagwarncount++ > clOptions->maxlagwarn) {
      // drop the player
      sprintf(message,"You have been kicked due to excessive lag (you have been warned %d times).",
        clOptions->maxlagwarn);
      sendMessage(ServerPlayer, playerIndex, message, true);
      removePlayer(playerIndex, "lag");
    }
  }
  updateLagLost(playerIndex, false);
}

// updateLagJitter: update jitter based on timestamps in MsgPlayerUpdate
static void updateLagJitter(int playerIndex, float jitter)
{
  PlayerInfo &pl=player[playerIndex];
  // time is smoothed exponentially using a dynamic smoothing factor
  pl.jitteravg = pl.jitteravg*(1-pl.jitteralpha) + pl.jitteralpha*fabs(jitter);
  pl.jitteralpha = pl.jitteralpha / (0.99f + pl.jitteralpha);
  updateLagLost(playerIndex, false);
}


static void sendTeleport(int playerIndex, uint16_t from, uint16_t to)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, from);
  buf = nboPackUShort(buf, to);
  broadcastMessage(MsgTeleport, (char*)buf-(char*)bufStart, bufStart);
}


// parse player comands (messages with leading /)
static void parseCommand(const char *message, int t)
{
  if (strncmp(message + 1, "password", 8) == 0) {
    handlePasswordCmd(t, message);

  } else if (strncmp(message + 1, "set ", 4) == 0) {
    handleSetCmd(t, message);

  } else if (strncmp(message + 1, "reset", 5) == 0) {
    handleResetCmd(t, message);

  } else if (strncmp(message + 1, "shutdownserver", 8) == 0) {
    handleShutdownserverCmd(t, message);

  } else if (strncmp(message + 1, "superkill", 8) == 0) {
    handleSuperkillCmd(t, message);

  } else if (strncmp(message + 1, "gameover", 8) == 0) {
    handleGameoverCmd(t, message);

  } else if (strncmp(message + 1, "countdown", 9) == 0) {
    handleCountdownCmd(t, message);

  } else if (strncmp(message + 1, "flag ", 5) == 0) {
    handleFlagCmd(t,message);

  } else if (strncmp(message + 1, "kick", 4) == 0) {
    handleKickCmd(t, message);

  } else if (strncmp(message+1, "banlist", 7) == 0) {
    handleBanlistCmd(t, message);

  } else if (strncmp(message+1, "hostbanlist", 11) == 0) {
    handleHostBanlistCmd(t, message);

  } else if (strncmp(message+1, "ban", 3) == 0) {
    handleBanCmd(t, message);

  } else if (strncmp(message+1, "hostban", 7) == 0) {
    handleHostBanCmd(t, message);

  } else if (strncmp(message+1, "unban", 5) == 0) {
    handleUnbanCmd(t, message);

  } else if (strncmp(message+1, "hostunban", 9) == 0) {
    handleHostUnbanCmd(t, message);

  } else if (strncmp(message+1, "lagwarn",7) == 0) {
    handleLagwarnCmd(t, message);

  } else if (strncmp(message+1, "lagstats",8) == 0) {
    handleLagstatsCmd(t, message);

  } else if (strncmp(message+1, "idlestats",9) == 0) {
    handleIdlestatsCmd(t, message);

  } else if (strncmp(message+1, "flaghistory", 11 ) == 0) {
    handleFlaghistoryCmd(t, message);

  } else if (strncmp(message+1, "playerlist", 10) == 0) {
    handlePlayerlistCmd(t, message);

  } else if (strncmp(message+1, "report", 6) == 0) {
    handleReportCmd(t, message);

  } else if (strncmp(message+1, "help", 4) == 0) {
    handleHelpCmd(t, message);

  } else if (strncmp(message + 1, "identify", 8) == 0) {
    handleIdentifyCmd(t, message);

  } else if (strncmp(message + 1, "register", 8) == 0) {
    handleRegisterCmd(t, message);

  } else if (strncmp(message + 1, "ghost", 5) == 0) {
    handleGhostCmd(t, message);

  } else if (strncmp(message + 1, "deregister", 10) == 0) {
    handleDeregisterCmd(t, message);

  } else if (strncmp(message + 1, "setpass", 7) == 0) {
    handleSetpassCmd(t, message);

  } else if (strncmp(message + 1, "grouplist", 9) == 0) {
    handleGrouplistCmd(t, message);

  } else if (strncmp(message + 1, "showgroup", 9) == 0) {
    handleShowgroupCmd(t, message);

  } else if (strncmp(message + 1, "groupperms", 10) == 0) {
    handleGrouppermsCmd(t, message);

  } else if (strncmp(message + 1, "setgroup", 8) == 0) {
    handleSetgroupCmd(t, message);

  } else if (strncmp(message + 1, "removegroup", 11) == 0) {
    handleRemovegroupCmd(t, message);

  } else if (strncmp(message + 1, "reload", 6) == 0) {
    handleReloadCmd(t, message);

  } else if (strncmp(message+1, "poll",4) == 0) {
    handlePollCmd(t, message);

  } else if (strncmp(message+1, "vote", 4) == 0) {
    handleVoteCmd(t, message);

  } else if (strncmp(message+1, "veto", 4) == 0) {
    handleVetoCmd(t, message);

  } else {
    char reply[MessageLen];
    sprintf(reply, "Unknown command [%s]", message+1);
    sendMessage(ServerPlayer, t, reply, true);
  }
}

static void handleCommand(int t, uint16_t code, uint16_t len, void *rawbuf)
{
  void *buf = (void*)((char*)rawbuf + 4);
#ifdef NETWORK_STATS
  countMessage(t, code, len, 0);
#endif
  switch (code) {
    // player joining
    case MsgEnter: {
      // data: type, team, name, email
      uint16_t type;
      int16_t team;
      buf = nboUnpackUShort(buf, type);
      buf = nboUnpackShort(buf, team);
      player[t].type = PlayerType(type);
      player[t].team = TeamColor(team);
      buf = nboUnpackString(buf, player[t].callSign, CallSignLen);
      buf = nboUnpackString(buf, player[t].email, EmailLen);
      addPlayer(t);
      DEBUG1("Player %s [%d] has joined from %s:%d on %i\n",
	  player[t].callSign, t,
	  inet_ntoa(player[t].taddr.sin_addr),
	  ntohs(player[t].taddr.sin_port),
	  player[t].fd);
      break;
    }

    // player closing connection
    case MsgExit:
      // data: <none>
      removePlayer(t, "left", false);
      break;

    case MsgNegotiateFlags: {
	void *bufStart;
	FlagTypeMap::iterator it;
	FlagSet::iterator m_it;
	FlagOptionMap hasFlag;
	FlagSet missingFlags;
	int i;
	unsigned short numClientFlags = len/2;

	/* Unpack incoming message containing the list of flags our client supports */
	for (i = 0; i < numClientFlags; i++) {
		FlagType *fDesc;
		buf = FlagType::unpack(buf, fDesc);
		if (fDesc != Flags::Null)
		  hasFlag[fDesc] = true;
	}

	/* Compare them to the flags this game might need, generating a list of missing flags */
	for (it = FlagType::getFlagMap().begin();
	     it != FlagType::getFlagMap().end(); ++it) {
		if (!hasFlag[it->second]) {
		   if (clOptions->flagCount[it->second] > 0)
		     missingFlags.insert(it->second);
		   if ((clOptions->numExtraFlags > 0) && !clOptions->flagDisallowed[it->second])
		     missingFlags.insert(it->second);
		}
	}

	/* Pack a message with the list of missing flags */
	buf = bufStart = getDirectMessageBuffer();
	for (m_it = missingFlags.begin(); m_it != missingFlags.end(); ++m_it) {
	  if ((*m_it) != Flags::Null)
	    buf = (*m_it)->pack(buf);
	}
	directMessage(t, MsgNegotiateFlags, (char*)buf-(char*)bufStart, bufStart);
	break;
    }



    // player wants more of world database
    case MsgGetWorld: {
      // data: count (bytes read so far)
      uint32_t ptr;
      buf = nboUnpackUInt(buf, ptr);
      if (ptr == 0) {
	// update time of day in world database
	const uint32_t epochOffset = (uint32_t)time(NULL);
	void *epochPtr = ((char*)worldDatabase) + WorldCodeEpochOffset;
	nboPackUInt(epochPtr, epochOffset);
      }
      sendWorld(t, ptr);
      break;
    }

    case MsgWantWHash: {
      void *buf, *bufStart = getDirectMessageBuffer();
      buf = nboPackString(bufStart, hexDigest, strlen(hexDigest)+1);
      directMessage(t, MsgWantWHash, (char*)buf-(char*)bufStart, bufStart);
      break;
    }

    case MsgQueryGame:
      sendQueryGame(t);
      break;

    case MsgQueryPlayers:
      sendQueryPlayers(t);
      break;

    // player is coming alive
    case MsgAlive: {
#ifdef TIMELIMIT
      // player moved before countdown started
      if (clOptions->timeLimit>0.0f && !countdownActive)
	player[t].playedEarly = true;
#endif
      playerAlive(t);
      break;
    }

    // player declaring self destroyed
    case MsgKilled: {
      if (player[t].team == ObserverTeam)
	break;
      // data: id of killer, shot id of killer
      PlayerId killer;
      int16_t shot, reason;
      buf = nboUnpackUByte(buf, killer);
      buf = nboUnpackShort(buf, reason);
      buf = nboUnpackShort(buf, shot);

      // Sanity check on shot: Here we have the killer
      if (killer != ServerPlayer) {
	int si = (shot == -1 ? -1 : shot & 0x00FF);
	if ((si < -1) || (si >= clOptions->maxShots))
	  break;
      }
      playerKilled(t, lookupPlayer(killer), reason, shot);
      break;
    }

    // player requesting to grab flag
    case MsgGrabFlag: {
      // data: flag index
      uint16_t flag;
      buf = nboUnpackUShort(buf, flag);
      grabFlag(t, int(flag));
      break;
    }

    // player requesting to drop flag
    case MsgDropFlag: {
      // data: position of drop
      float pos[3];
      buf = nboUnpackVector(buf, pos);
      dropFlag(t, pos);
      break;
    }

    // player has captured a flag
    case MsgCaptureFlag: {
      // data: team whose territory flag was brought to
      uint16_t team;
      buf = nboUnpackUShort(buf, team);
      captureFlag(t, TeamColor(team));
      break;
    }

    // shot fired
    case MsgShotBegin:
      // Sanity check
      if (len == 39) //wow thats bad
	shotFired(t, buf, int(len));
      break;

    // shot ended prematurely
    case MsgShotEnd: {
      if (player[t].team == ObserverTeam)
	break;
      // data: shooter id, shot number, reason
      PlayerId sourcePlayer;
      int16_t shot;
      uint16_t reason;
      buf = nboUnpackUByte(buf, sourcePlayer);
      buf = nboUnpackShort(buf, shot);
      buf = nboUnpackUShort(buf, reason);
      shotEnded(sourcePlayer, shot, reason);
      break;
    }

    // player teleported
    case MsgTeleport: {
      if (player[t].team == ObserverTeam)
	break;
      uint16_t from, to;
      buf = nboUnpackUShort(buf, from);
      buf = nboUnpackUShort(buf, to);
      sendTeleport(t, from, to);
      break;
    }

    // player sending a message
    case MsgMessage: {
      player[t].lastmsg = TimeKeeper::getCurrent();
      // data: target player/team, message string
      PlayerId targetPlayer;
      char message[MessageLen];
      buf = nboUnpackUByte(buf, targetPlayer);
      buf = nboUnpackString(buf, message, sizeof(message));
      message[MessageLen - 1] = '\0';
      DEBUG1("Player %s [%d]: %s\n",player[t].callSign, t, message);
      // check for command
      if (message[0] == '/') {
	/* make commands case insensitive for user-friendlyness */
	unsigned int pos=1;
	while ((pos < strlen(message)) && (isAlphanumeric(message[pos]))) {
	  message[pos] = tolower((int)message[pos]);
	  pos++;
	}
	parseCommand(message, t);
      } else if (targetPlayer == AdminPlayers){
        //printf ("Admin message %s\n",message);
        sendMessage (t, AdminPlayers,message, true);
      }
      // check if the target player is invalid
      else if (targetPlayer < LastRealPlayer && 
               player[targetPlayer].state <= PlayerInLimbo) {
	sendMessage(ServerPlayer, t, "The player you tried to talk to does "
		    "not exist!");
      } else {
	if (clOptions->filterChat) {
	  if (clOptions->filterSimple) {
	    clOptions->filter.filter(message, true);
	  } else {
	    clOptions->filter.filter(message, false);
	  }
	}
	sendMessage(t, targetPlayer, message, true);

	if (clOptions->prohibitBots && strncmp(message, "[ROGER] Taking Controls of ", 27) == 0) {
	  sendMessage(ServerPlayer, t, "Autopilot is prohibited on this server.  Please turn it off immediately.");
	  player[t].quellRoger = true;
	} else if (player[t].quellRoger && strncmp(message, "[ROGER] Releasing Controls of ", 30) == 0) {
	  sendMessage(ServerPlayer, t, "Thank you for turning off autopilot.  Please refrain from using autopilot on this server in the future.");
	  player[t].quellRoger = false;
	}
      }
      break;
    }

    // player has transferred flag to another tank
    case MsgTransferFlag: {
	PlayerId from, to;

	buf = nboUnpackUByte(buf, from);
	buf = nboUnpackUByte(buf, to);

	// Sanity check
	if (from >= curMaxPlayers)
	  return;
	if (to >= curMaxPlayers)
	  return;

	int flagIndex = player[from].flag;
	if (flagIndex == -1)
	  return;

	zapFlag(player[to].flag);

	void *bufStart = getDirectMessageBuffer();
	void *buf = nboPackUByte(bufStart, from);
	buf = nboPackUByte(buf, to);
	buf = nboPackUShort(buf, uint16_t(flagIndex));
	flag[flagIndex].flag.owner = to;
	flag[flagIndex].player = to;
	player[to].flag = flagIndex;
	player[from].flag = -1;
	buf = flag[flagIndex].flag.pack(buf);
	broadcastMessage(MsgTransferFlag, (char*)buf - (char*)bufStart, bufStart);
	player[from].lastFlagDropTime = TimeKeeper::getCurrent();
	player[to].lastFlagDropTime = TimeKeeper::getCurrent();
	break;
    }

    case MsgUDPLinkEstablished:
      player[t].udpout = true;
      DEBUG2("Player %s [%d] outbound UDP up\n", player[t].callSign, t);
      break;

    case MsgNewRabbit: {
      if (t == rabbitIndex)
	anointNewRabbit();
      break;
    }

    case MsgPause: {
      uint8_t pause;
      nboUnpackUByte(buf, pause);
      pausePlayer(t, pause != 0);
      break;
    }

    // player is sending a Server Control Message not implemented yet
    case MsgServerControl:
      break;

    case MsgLagPing: {
      uint16_t pingseqno;
      buf = nboUnpackUShort(buf, pingseqno);
      if (pingseqno == player[t].pingseqno)
      {
	float dt = TimeKeeper::getCurrent() - player[t].lastping;
	updateLag(t, dt);
	player[t].pingpending = false;
      }
      break;
    }

    // player is sending his position/speed (bulk data)
    case MsgPlayerUpdate: {
      float timestamp;
      PlayerId id;
      PlayerState state;
      buf = nboUnpackFloat(buf, timestamp);
      buf = nboUnpackUByte(buf, id);
      buf = state.unpack(buf);
      if (t != id) {
	// Should be a Robot or a cheater
	if ((id >= curMaxPlayers) || (player[id].type != ComputerPlayer)) {
	  // FIXME - Commented out autokick occasionally being kicked
	  // out with Robot
	  // Should check why!
// 	  char message[MessageLen];
	  DEBUG1("kicking Player %s [%d] Invalid Id %s [%d]\n",
		 player[t].callSign, t, player[id].callSign, id);
// 	  strcpy(message, "Autokick: Using invalid PlayerId, don't cheat.");
// 	  sendMessage(ServerPlayer, t, message, true);
// 	  removePlayer(t, "Using invalid PlayerId");
// 	  break;
	} else
	  t = id;
      }

      // silently drop old packet
      if (state.order <= player[t].lastState.order)
	break;

      // packet got lost (or out ouf order): count
      if (state.order - player[t].lastState.order > 1)
        updateLagLost(t);

      TimeKeeper now = TimeKeeper::getCurrent();
      // don't calc jitter if more than 2 seconds between packets
      if (player[t].lasttimestamp > 0.0f && timestamp-player[t].lasttimestamp < 2.0f) {
	const float jitter = fabs(now - player[t].lastupdate - (timestamp - player[t].lasttimestamp));
	updateLagJitter(t, jitter);
      }
      player[t].lasttimestamp = timestamp;
      player[t].lastupdate = now;

      //Don't kick players up to 10 seconds after a world parm has changed,
      static const float heightFudge = 1.1f; /* 10% */
      if (now - lastWorldParmChange > 10.0f) {
	float gravity = BZDB.eval(StateDatabase::BZDB_GRAVITY);
	if (gravity < 0.0f) {
	  float maxTankHeight = maxWorldHeight + heightFudge * ((BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY)*BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY)) / (2.0f * -gravity));

	  if (state.pos[2] > maxTankHeight) {
	    DEBUG1("Kicking Player %s [%d] jumped too high [max: %f height: %f]\n", player[t].callSign, t, maxTankHeight, state.pos[2]);
	    sendMessage(ServerPlayer, t, "Autokick: Player location was too high.", true);
	    removePlayer(t, "too high");
	    break;
	  }
	}

	// make sure the player is still in the map
	// test all the map bounds + some fudge factor, just in case
	static const float positionFudge = 10.0f; /* linear distance */
	bool InBounds = true;
	float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
	if ( (state.pos[1] >= worldSize*0.5f + positionFudge) || (state.pos[1] <= -worldSize*0.5f - positionFudge)) {
	  std::cout << "y position (" << state.pos[1] << ") is out of bounds (" << worldSize * 0.5f << " + " << positionFudge << ")" << std::endl;
	  InBounds = false;
	} else if ( (state.pos[0] >= worldSize*0.5f + positionFudge) || (state.pos[0] <= -worldSize*0.5f - positionFudge)) {
	  std::cout << "x position (" << state.pos[0] << ") is out of bounds (" << worldSize * 0.5f << " + " << positionFudge << ")" << std::endl;
       	  InBounds = false;
	}

	static const float burrowFudge = 1.0f; /* linear distance */
	if (state.pos[2]<BZDB.eval(StateDatabase::BZDB_BURROWDEPTH) - burrowFudge) {
	  std::cout << "z depth (" << state.pos[2] << ") is less than burrow depth (" << BZDB.eval(StateDatabase::BZDB_BURROWDEPTH) << " - " << burrowFudge << ")" << std::endl;
	  InBounds = false;
	}

	// kick em cus they are most likely cheating or using a buggy client
	if (!InBounds)
	{
	  DEBUG1("Kicking Player %s [%d] Out of map bounds at position (%.2f,%.2f,%.2f)\n", player[t].callSign, t, state.pos[0], state.pos[1], state.pos[2]);
	  sendMessage(ServerPlayer, t, "Autokick: Player location was outside the playing area.", true);
	  removePlayer(t, "Out of map bounds");
	}

	// Speed problems occur around flag drops, so don't check for a short period of time
	// after player drops a flag. Currently 2 second, adjust as needed.

	if (TimeKeeper::getCurrent() - player[t].lastFlagDropTime >= 2.0f) {
	  // check for highspeed cheat; if inertia is enabled, skip test for now
	  if (clOptions->linearAcceleration == 0.0f) {
	    // Doesn't account for going fast backwards, or jumping/falling
	    float curPlanarSpeedSqr = state.velocity[0]*state.velocity[0] +
				      state.velocity[1]*state.velocity[1];

	    float maxPlanarSpeedSqr = BZDB.eval(StateDatabase::BZDB_TANKSPEED)*BZDB.eval(StateDatabase::BZDB_TANKSPEED);

	    bool logOnly = false;

	    // if tank is not driving cannot be sure it didn't toss (V) in flight
	    // if tank is not alive cannot be sure it didn't just toss (V)
  	    if (flag[player[t].flag].flag.type == Flags::Velocity)
	      maxPlanarSpeedSqr *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD) * BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
	    else if (flag[player[t].flag].flag.type == Flags::Thief)
	      maxPlanarSpeedSqr *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD) * BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
 	    else if ((flag[player[t].flag].flag.type == Flags::Burrow) &&
	      (player[t].lastState.pos[2] == state.pos[2]) && 
	      (player[t].lastState.velocity[2] == state.velocity[2]) &&
	      (state.pos[2] <= BZDB.eval(StateDatabase::BZDB_BURROWDEPTH)))
	      // if we have burrow and are not actively burrowing
	      // You may have burrow and still be above ground. Must check z in ground!!
 	      maxPlanarSpeedSqr *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD) * BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
	    else {
	      // If player is moving vertically, or not alive the speed checks
	      // seem to be problematic. If this happens, just log it for now,
	      // but don't actually kick
	      if ((player[t].lastState.pos[2] != state.pos[2])
	      ||  (player[t].lastState.velocity[2] != state.velocity[2])
	      ||  ((state.status & PlayerState::Alive) == 0)) {
		logOnly = true;
	      }
	    }

	    // allow a 10% tolerance level for speed if -speedtol is not sane
	    float realtol = 1.1f;
	    if (speedTolerance > 1.0f)
	      realtol = speedTolerance;
	    maxPlanarSpeedSqr *= realtol;
	    if (curPlanarSpeedSqr > maxPlanarSpeedSqr) {
	      if (logOnly) {
		DEBUG1("Logging Player %s [%d] tank too fast (tank: %f, allowed: %f){Dead or v[z] != 0}\n",
		player[t].callSign, t,
		sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
	      }
	      else {
		DEBUG1("Kicking Player %s [%d] tank too fast (tank: %f, allowed: %f)\n",
		       player[t].callSign, t,
		       sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
		sendMessage(ServerPlayer, t, "Autokick: Player tank is moving too fast.", true);
		removePlayer(t, "too fast");
	      }
	      break;
	    }
	  }
	}
      }

      player[t].lastState = state;

      // Player might already be dead and did not know it yet (e.g. teamkill)
      // do not propogate
      if (player[t].state != PlayerAlive && (state.status & short(PlayerState::Alive)))
        break;
    }

    //Fall thru
    case MsgGMUpdate:
    case MsgAudio:
    case MsgVideo:
      // observer shouldn't send bulk messages anymore, they used to when it was
      // a server-only hack; but the check does not hurt, either
      if (player[t].team == ObserverTeam)
	break;
      relayPlayerPacket(t, len, rawbuf);
      break;

    // FIXME handled inside uread, but not discarded
    case MsgUDPLinkRequest:
      break;

    // unknown msg type
    default:
      DEBUG1("Player [%d] sent unknown packet type (%x), possible attack from %s\n",
	     t,code,player[t].peer.getDotNotation().c_str());
  }
}

static void terminateServer(int /*sig*/)
{
  bzSignal(SIGINT, SIG_PF(terminateServer));
  bzSignal(SIGTERM, SIG_PF(terminateServer));
  exitCode = 0;
  done = true;
}


static std::string cmdSet(const std::string&, const CommandManager::ArgList& args)
{
  switch (args.size()) {
    case 2:
      if (BZDB.isSet(args[0])) {
	StateDatabase::Permission permission=BZDB.getPermission(args[0]);
	if ((permission == StateDatabase::ReadWrite) || (permission == StateDatabase::Locked)) {
	  BZDB.set(args[0], args[1], StateDatabase::Server);
	  lastWorldParmChange = TimeKeeper::getCurrent();
	  return args[0] + " set";
	}
	return "variable " + args[0] + " is not writeable";
      }
      else
	return "variable " + args[0] + " does not exist";
    case 1:
      if (BZDB.isSet(args[0])) {
	return args[0] + " is " + BZDB.get(args[0]);
      }
      else
	return "variable " + args[0] + " does not exist";
    default:
      return "usage: set <name> [<value>]";
  }
}

static void resetAllCallback(const std::string &name, void*)
{
  StateDatabase::Permission permission=BZDB.getPermission(name);
  if ((permission == StateDatabase::ReadWrite) || (permission == StateDatabase::Locked)) {
    BZDB.set(name, BZDB.getDefault(name), StateDatabase::Server);
  }
}

static std::string cmdReset(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() == 1) {
    if (args[0] == "*") {
      BZDB.iterate(resetAllCallback,NULL);
      return "all variables reset";
    }
    else if (BZDB.isSet(args[0])) {
      StateDatabase::Permission permission=BZDB.getPermission(args[0]);
      if ((permission == StateDatabase::ReadWrite) || (permission == StateDatabase::Locked)) {
	BZDB.set(args[0], BZDB.getDefault(args[0]), StateDatabase::Server);
	lastWorldParmChange = TimeKeeper::getCurrent();
	return args[0] + " reset";
      }
      return "variable " + args[0] + " is not writeable";
    }
    else
      return "variable " + args[0] + " does not exist";
  }
  else
    return "usage: reset <name>";
}


/** main parses command line options and then enters an event and activity
 * dependant main loop.  once inside the main loop, the server is up and
 * running and should be ready to process connections and activity.
 */
int main(int argc, char **argv)
{
  VotingArbiter *votingarbiter = (VotingArbiter *)NULL;

  setvbuf(stdout, (char *)NULL, _IOLBF, 0);
  setvbuf(stderr, (char *)NULL, _IOLBF, 0);

  int nfound;

  // check time bomb
  if (timeBombBoom()) {
    std::cerr << "This release expired on " << timeBombString() << ".\n";
    std::cerr << "Please upgrade to the latest release.\n";
    exit(0);
  }

  // print expiration date
  if (timeBombString()) {
    std::cerr << "This release will expire on " << timeBombString() << ".\n";
    std::cerr << "Version " << getAppVersion() << std::endl;
  }

  // trap some signals
  // let user kill server
  if (bzSignal(SIGINT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGINT, SIG_PF(terminateServer));
  // ditto
  bzSignal(SIGTERM, SIG_PF(terminateServer));

  // initialize
#if defined(_WIN32)
  {
    static const int major = 2, minor = 2;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
      DEBUG2("Failed to initialize winsock.  Terminating.\n");
      return 1;
    }
    if (LOBYTE(wsaData.wVersion) != major ||
	HIBYTE(wsaData.wVersion) != minor) {
      DEBUG2("Version mismatch in winsock;"
	  "  got %d.%d.  Terminating.\n",
	  (int)LOBYTE(wsaData.wVersion),
	  (int)HIBYTE(wsaData.wVersion));
      WSACleanup();
      return 1;
    }
  }
#else
  bzSignal(SIGPIPE, SIG_IGN);
#endif /* defined(_WIN32) */

  bzfsrand(time(0));

  Flags::init();

  clOptions = new CmdLineOptions();

  // set default DB entries
  for (unsigned int gi = 0; gi < numGlobalDBItems; ++gi) {
    assert(globalDBItems[gi].name != NULL);
    if (globalDBItems[gi].value != NULL) {
      BZDB.set(globalDBItems[gi].name, globalDBItems[gi].value);
      BZDB.setDefault(globalDBItems[gi].name, globalDBItems[gi].value);
    }
    BZDB.setPersistent(globalDBItems[gi].name, globalDBItems[gi].persistent);
    BZDB.setPermission(globalDBItems[gi].name, globalDBItems[gi].permission);
    BZDB.addCallback(std::string(globalDBItems[gi].name), onGlobalChanged, (void*) NULL);
  }
  CMDMGR.add("set", cmdSet, "set <name> [<value>]");
  CMDMGR.add("reset", cmdReset, "reset <name>");

  BZDBCache::init();

  // parse arguments
  parse(argc, argv, *clOptions);

  if (clOptions->bzdbVars.length() > 0) {
    DEBUG1("Loading variables from %s\n", clOptions->bzdbVars.c_str());
    bool success = CFGMGR.read(clOptions->bzdbVars);
    if (success) {
      DEBUG1("Successfully loaded variable(s)\n");
    } else {
      DEBUG1("WARNING: unable to load the variable file\n");
    }
  }

  /* load the bad word filter if it was set */
  if (clOptions->filterFilename.length() != 0) {
    if (clOptions->filterChat || clOptions->filterCallsigns) {
      if (debugLevel >= 1) {
	unsigned int count;
	DEBUG1("Loading %s\n", clOptions->filterFilename.c_str());
	count = clOptions->filter.loadFromFile(clOptions->filterFilename, true);
	DEBUG1("Loaded %u words\n", count);
      } else {
	clOptions->filter.loadFromFile(clOptions->filterFilename, false);
      }
    } else {
      DEBUG1("Bad word filter specified without -filterChat or -filterCallsigns\n");
    }
  }

#ifdef HAVE_ADNS_H
  /* start up our resolver if we have ADNS */
  if (adns_init(&adnsState, adns_if_nosigpipe, 0) < 0) {
    perror("ADNS init failed");
    exit(1);
  }
#endif

  /* initialize the poll arbiter for voting if necessary */
  if (clOptions->voteTime > 0) {
    votingarbiter = new VotingArbiter(clOptions->voteTime, clOptions->vetoTime, clOptions->votesRequired, clOptions->votePercentage, clOptions->voteRepeatTime);
    DEBUG1("There is a voting arbiter with the following settings:\n");
    DEBUG1("\tvote time is %d seconds\n", clOptions->voteTime);
    DEBUG1("\tveto time is %d seconds\n", clOptions->vetoTime);
    DEBUG1("\tvotes required are %d\n", clOptions->votesRequired);
    DEBUG1("\tvote percentage necessary is %f\n", clOptions->votePercentage);
    DEBUG1("\tvote repeat time is %d seconds\n", clOptions->voteRepeatTime);
    DEBUG1("\tavailable voters is initially set to %d\n", maxPlayers);

    // override the default voter count to the max number of players possible
    votingarbiter->setAvailableVoters(maxPlayers);
    BZDB.setPointer("poll", (void *)votingarbiter);
    BZDB.setPermission("poll", StateDatabase::ReadOnly);
  }

  if (clOptions->pingInterface) {
    serverAddress = Address::getHostAddress(clOptions->pingInterface);
  }

// TimR use 0.0.0.0 by default, multicast will need to have a -i specified for now.
//  if (!pingInterface)
//    pingInterface = serverAddress.getHostName();

  // my address to publish.  allow arguments to override (useful for
  // firewalls).  use my official hostname if it appears to be
  // canonicalized, otherwise use my IP in dot notation.
  // set publicized address if not set by arguments
  if (clOptions->publicizedAddress.length() == 0) {
    clOptions->publicizedAddress = Address::getHostName();
    if (clOptions->publicizedAddress.find('.') == std::string::npos)
      clOptions->publicizedAddress = serverAddress.getDotNotation();
    if (clOptions->wksPort != ServerPort) {
      clOptions->publicizedAddress += string_util::format(":%d", clOptions->wksPort);
    }
  }

  /* print debug information about how the server is running */
  if (clOptions->publicizeServer) {
    DEBUG1("Running a public server with the following settings:\n");
    DEBUG1("\tpublic address is %s\n", clOptions->publicizedAddress.c_str());
  } else {
    DEBUG1("Running a private server with the following settings:\n");
  }
  // print networking info
  DEBUG1("\tlistening on %s:%i\n",
      serverAddress.getDotNotation().c_str(), clOptions->wksPort);
  DEBUG1("\twith title of \"%s\"\n", clOptions->publicizedTitle.c_str());

  // prep ping reply
  pingReply.serverId.serverHost = serverAddress;
  pingReply.serverId.port = htons(clOptions->wksPort);
  pingReply.serverId.number = 0;
  pingReply.gameStyle = clOptions->gameStyle;
  pingReply.maxPlayers = (uint8_t)maxRealPlayers;
  pingReply.maxShots = clOptions->maxShots;
  pingReply.rogueMax = (uint8_t)clOptions->maxTeam[0];
  pingReply.redMax = (uint8_t)clOptions->maxTeam[1];
  pingReply.greenMax = (uint8_t)clOptions->maxTeam[2];
  pingReply.blueMax = (uint8_t)clOptions->maxTeam[3];
  pingReply.purpleMax = (uint8_t)clOptions->maxTeam[4];
  pingReply.observerMax = (uint8_t)clOptions->maxTeam[5];
  pingReply.shakeWins = clOptions->shakeWins;
  pingReply.shakeTimeout = clOptions->shakeTimeout;
#ifdef TIMELIMIT
  pingReply.maxTime = (uint16_t)clOptions->timeLimit;
#else
  pingReply.maxTime = 0;
#endif
  pingReply.maxPlayerScore = clOptions->maxPlayerScore;
  pingReply.maxTeamScore = clOptions->maxTeamScore;

  // start listening and prepare world database
  if (!defineWorld()) {
#if defined(_WIN32)
    WSACleanup();
#endif /* defined(_WIN32) */
    std::cerr << "ERROR: A world was not specified" << std::endl;
    return 1;
  }

  if (!serverStart()) {
#if defined(_WIN32)
    WSACleanup();
#endif /* defined(_WIN32) */
    std::cerr << "ERROR: Unable to start the server" << std::endl;
    return 2;
  }

  TimeKeeper lastSuperFlagInsertion = TimeKeeper::getCurrent();
  const float flagExp = -logf(0.5f) / FlagHalfLife;

  // load up the access permissions & stuff
  if(groupsFile.size())
    readGroupsFile(groupsFile);
  // make sure that the 'admin' & 'default' groups exist
  PlayerAccessMap::iterator itr = groupAccess.find("DEFAULT");
  if (itr == groupAccess.end()) {
    PlayerAccessInfo info;
    info.explicitAllows[PlayerAccessInfo::idleStats] = true;
    info.explicitAllows[PlayerAccessInfo::lagStats] = true;
    info.explicitAllows[PlayerAccessInfo::flagHistory] = true;
    groupAccess["DEFAULT"] = info;
  }
  itr = groupAccess.find("REGISTERED");
  if (itr == groupAccess.end()) {
    PlayerAccessInfo info;
    info.explicitAllows[PlayerAccessInfo::vote] = true;
    info.explicitAllows[PlayerAccessInfo::poll] = true;
    groupAccess["REGISTERED"] = info;
  }
  itr = groupAccess.find("ADMIN");
  if (itr == groupAccess.end()) {
    PlayerAccessInfo info;
    for (int i = 0; i < PlayerAccessInfo::lastPerm; i++)
      info.explicitAllows[i] = true;
    groupAccess["ADMIN"] = info;
  }
  if (passFile.size())
    readPassFile(passFile);
  if (userDatabaseFile.size())
    readPermsFile(userDatabaseFile);


  /* MAIN SERVER RUN LOOP
   *
   * the main loop runs at approximately 2 iterations per 5 seconds
   * when there are no players on the field.  this can increase to
   * about 100 iterations per 5 seconds with a single player, though
   * average is about 20-40 iterations per five seconds.  Adding
   * world weapons will increase the number of iterations
   * substantially (about x10)
   **/
  int i;
  while (!done) {
    maxFileDescriptor = 0;
    // prepare select set
    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    for (i = 0; i < curMaxPlayers; i++) {
      if (player[i].fd != NotConnected) {
	//DEBUG1("fdset fd,read %i %lx\n",player[i].fd,read_set);
	FD_SET(player[i].fd, &read_set);

	if (player[i].outmsgSize > 0)
	  FD_SET(player[i].fd, &write_set);
	if (player[i].fd > maxFileDescriptor)
	  maxFileDescriptor = player[i].fd;
      }
    }
    // always listen for connections
    FD_SET(wksSocket, &read_set);
    if (wksSocket > maxFileDescriptor)
      maxFileDescriptor = wksSocket;
    FD_SET(udpSocket, &read_set);
    if (udpSocket > maxFileDescriptor)
      maxFileDescriptor = udpSocket;

    // check for list server socket connected
    if (listServerLinksCount)
      if (listServerLink.socket != NotConnected) {
	if (listServerLink.phase == ListServerLink::CONNECTING)
	  FD_SET(listServerLink.socket, &write_set);
	else
	  FD_SET(listServerLink.socket, &read_set);
	if (listServerLink.socket > maxFileDescriptor)
	  maxFileDescriptor = listServerLink.socket;
      }

    // find timeout when next flag would hit ground
    TimeKeeper tm = TimeKeeper::getCurrent();
    // lets start by waiting 3 sec
    float waitTime = 3.0f;
#ifdef TIMELIMIT
    if (countdownActive && clOptions->timeLimit > 0.0f)
	waitTime = 1.0f;
#endif
    if (numFlagsInAir > 0) {
      for (i = 0; i < numFlags; i++)
	if (flag[i].flag.status != FlagNoExist &&
	    flag[i].flag.status != FlagOnTank &&
	    flag[i].flag.status != FlagOnGround &&
	    flag[i].dropDone - tm < waitTime)
	  waitTime = flag[i].dropDone - tm;
    }

    // get time for next lagping
    bool someoneIsConnected = false;
    for (int p=0;p<curMaxPlayers;p++)
    {
      if (player[p].state >= PlayerDead &&
	  player[p].type == TankPlayer &&
	  player[p].nextping - tm < waitTime) {
	waitTime = player[p].nextping - tm;
	someoneIsConnected = true;
      }
    }

    // if there are world weapons, update much more frequently
    if (someoneIsConnected && wWeapons.count() > 0) {
      waitTime *= 0.1f;  // a tenth of what we would have waited
    }

    // minmal waitTime
    if (waitTime < 0.0f)
      waitTime = 0.0f;

    // we have no pending packets
    nfound = 0;

    // wait for communication or for a flag to hit the ground
    struct timeval timeout;
    timeout.tv_sec = long(floorf(waitTime));
    timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));
    nfound = select(maxFileDescriptor+1, (fd_set*)&read_set, (fd_set*)&write_set, 0, &timeout);
    //if (nfound)
    //	DEBUG1("nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);

#ifdef TIMELIMIT
    // see if game time ran out
    if (!gameOver && countdownActive && clOptions->timeLimit > 0.0f) {
      float newTimeElapsed = tm - gameStartTime;
      float timeLeft = clOptions->timeLimit - newTimeElapsed;
      if (timeLeft <= 0.0f) {
	timeLeft = 0.0f;
	gameOver = true;
	countdownActive = false;
      }
      if (timeLeft == 0.0f || newTimeElapsed - clOptions->timeElapsed >= 30.0f) {
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUShort(bufStart, (uint16_t)(int)timeLeft);
	broadcastMessage(MsgTimeUpdate, (char*)buf-(char*)bufStart, bufStart);
	clOptions->timeElapsed = newTimeElapsed;
	if (clOptions->oneGameOnly && timeLeft == 0.0f) {
	  done = true;
	  exitCode = 0;
	}
      }
    }
#endif

    // kick idle players
    if (clOptions->idlekickthresh > 0) {
      for (int i=0;i<curMaxPlayers;i++) {
        if (player[i].state > PlayerInLimbo && player[i].team != ObserverTeam) {
          int idletime = (int)(tm - player[i].lastupdate);
	  int pausetime = 0;
          if (player[i].paused && tm - player[i].pausedSince > idletime)
            pausetime = (int)(tm - player[i].pausedSince);
	  idletime = idletime > pausetime ? idletime : pausetime;
          if (idletime >
	      (tm - player[i].lastmsg < clOptions->idlekickthresh ?
	       3 * clOptions->idlekickthresh : clOptions->idlekickthresh)) {
            DEBUG1("kicking Player %s [%d] idle %d\n", player[i].callSign, i, idletime);
            char message[MessageLen] = "You were kicked because of idling too long";
            sendMessage(ServerPlayer, i,  message, true);
            removePlayer(i, "idling");
          }
	}
      }
    }

    // update notResponding
    float notRespondingTime = BZDB.eval(StateDatabase::BZDB_NOTRESPONDINGTIME);
    for (int h = 0; h < curMaxPlayers; h++) {
      if (player[h].state > PlayerInLimbo) {
	bool oldnr = player[h].notResponding;
	player[h].notResponding = ((TimeKeeper::getCurrent() - player[h].lastupdate) > notRespondingTime);
	// if player is the rabbit, anoint a new one
	if (!oldnr && player[h].notResponding && h == rabbitIndex)
	  anointNewRabbit();
	// if player is holding a flag, drop it
	if (!oldnr && player[h].notResponding) {
	  for (int j = 0; j < numFlags; j++) {
	    if (flag[j].player == h) {
	      dropFlag(h, player[h].lastState.pos);
	    }
	  }
	}
      }
    }

#ifdef HAVE_ADNS_H
    for (int h = 0; h < curMaxPlayers; h++) {
      if (player[h].adnsQuery) {
	// check to see if query has completed
	adns_answer *answer;
	if (adns_check(adnsState, &player[h].adnsQuery, &answer, 0) != 0) {
	  if (getErrno() != EAGAIN) {
	    DEBUG1("Player [%d] failed to resolve: errno %d\n", h, getErrno());
	    player[h].adnsQuery = NULL;
	  }
	} else {
	  // we got our reply.
	  if (answer->status == adns_s_ok) {
	    if (player[h].hostname)
	      free(player[h].hostname); // shouldn't happen, but just in case
	    player[h].hostname = strdup(*answer->rrs.str);
	    DEBUG1("Player [%d] resolved to hostname: %s\n", h, player[h].hostname);
	    free(answer);
	    player[h].adnsQuery = NULL;
	    // check against ban lists
	    if (!clOptions->acl.hostValidate(player[h].hostname)) {
	      removePlayer(h, "bannedhost");
	    }
	  } else {
	    DEBUG1("Player [%d] got bad status from resolver: %s\n", h, adns_strerror(answer->status));
	    free(answer);
	    player[h].adnsQuery = NULL;
	  }
	}
      }
    }
#endif

    // manage voting poll for collective kicks/bans
    if ((clOptions->voteTime > 0) && (votingarbiter != NULL)) {
      if (votingarbiter->knowsPoll()) {
	char message[MessageLen];

	std::string person = votingarbiter->getPollPlayer();
	std::string action = votingarbiter->getPollAction();

	static unsigned short int voteTime = 0;

	/* flags to only blather once */
	static bool announcedOpening = false;
	static bool announcedClosure = false;
	static bool announcedResults = false;

	/* once a poll begins, announce its commencement */
	if (!announcedOpening) {
	  voteTime = votingarbiter->getVoteTime();

	  sprintf(message, "A poll to %s %s has begun.  Players have up to %d seconds to vote.", action.c_str(), person.c_str(), voteTime);
	  sendMessage(ServerPlayer, AllPlayers, message, true);
	  announcedOpening = true;
	}

	static TimeKeeper lastAnnounce = TimeKeeper::getNullTime();

	/* make a heartbeat announcement every 15 seconds */
	if (((voteTime - (int)(TimeKeeper::getCurrent() - votingarbiter->getStartTime()) - 1) % 15 == 0) &&
	    ((int)(TimeKeeper::getCurrent() - lastAnnounce) != 0) &&
	    (votingarbiter->timeRemaining() > 0)) {
	  sprintf(message, "%d seconds remain in the poll to %s %s.", votingarbiter->timeRemaining(), action.c_str(), person.c_str());
	  sendMessage(ServerPlayer, AllPlayers, message, true);
	  lastAnnounce = TimeKeeper::getCurrent();
	}

	if (votingarbiter->isPollClosed()) {

	  if (!announcedResults) {
	    sprintf(message, "Poll Results: %ld in favor, %ld oppose, %ld abstain", votingarbiter->getYesCount(), votingarbiter->getNoCount(), votingarbiter->getAbstentionCount());
	    sendMessage(ServerPlayer, AllPlayers, message, true);
	    announcedResults = true;
	  }

	  if (votingarbiter->isPollSuccessful()) {
	    if (!announcedClosure) {
	      // a poll that exists and is closed has ended successfully
	      sprintf(message, "The poll is now closed and was successful.  %s is scheduled to be %s.", person.c_str(), action == "ban" ? "temporarily banned" : "kicked");
	      sendMessage(ServerPlayer, AllPlayers, message, true);
	      announcedClosure = true;
	    }
	  } else {
	    if (!announcedClosure) {
	      sprintf(message, "The poll to %s %s was not successful", action.c_str(), person.c_str());
	      sendMessage(ServerPlayer, AllPlayers, message, true);
	      announcedClosure = true;

	      // go ahead and reset the poll (don't bother waiting for veto timeout)
	      votingarbiter->forgetPoll();
	      announcedClosure = false;
	    }
	  }

	  /* the poll either terminates by itself or via a veto command */
	  if (votingarbiter->isPollExpired()) {

	    /* maybe successful, maybe not */
	    if (votingarbiter->isPollSuccessful()) {
	      // perform the action of the poll, if any
	      sprintf(message, "%s has been %s", person.c_str(), action == "ban" ? "banned for 10 minutes." : "kicked.");
	      sendMessage(ServerPlayer, AllPlayers, message, true);

	      /* regardless of whether or not the player was found, if the poll
	       * is a ban poll, ban the weenie
	       */
	      if (action == "ban") {
		clOptions->acl.ban(votingarbiter->getPollPlayerIP().c_str(), person.c_str(), 10);
	      }

	      // lookup the player id
	      bool foundPlayer=false;
	      int v;
	      for (v = 0; v < curMaxPlayers; v++) {
		if (strncmp(player[v].callSign, person.c_str(), 256)==0) {
		  foundPlayer=true;
		  break;
		}
	      }
	      if (foundPlayer) {
		// notify the player
		sprintf(message, "You have been %s due to sufficient votes to have you removed", action == "ban" ? "temporarily banned" : "kicked");
		sendMessage(ServerPlayer, v, message, true);
		sprintf(message, "/poll %s", action.c_str());
		removePlayer(v, message);
	      }
	    } /* end if poll is successful */

	    // get ready for the next poll
	    votingarbiter->forgetPoll();

	    announcedClosure = false;
	    announcedOpening = false;
	    announcedResults = false;
	  }

	} else {
	  // the poll may get enough votes early
	  if (votingarbiter->isPollSuccessful()) {
	    sprintf(message, "Enough votes were collected to %s %s early.", action.c_str(), person.c_str());
	    sendMessage(ServerPlayer, AllPlayers, message, true);

	    // close the poll since we have enough votes (next loop will kick off notification)
	    votingarbiter->closePoll();

	  } // the poll is over
	} // is the poll closed
      } // knows of a poll
    } // voting is allowed and an arbiter exists


    // periodic advertising broadcast
    static const std::vector<std::string>* adLines = clOptions->textChunker.getTextChunk("admsg");
    if (clOptions->advertisemsg || adLines != NULL) {
      static TimeKeeper lastbroadcast = TimeKeeper::getCurrent();
      if (TimeKeeper::getCurrent() - lastbroadcast > 900) {
	// every 15 minutes
	char message[MessageLen];
	if (clOptions->advertisemsg != NULL) {
	  // split the admsg into several lines if it contains '\n'
	  const char* c = clOptions->advertisemsg;
	  const char* j;
	  while ((j = strstr(c, "\\n")) != NULL) {
	    int l = j - c < MessageLen - 1 ? j - c : MessageLen - 1;
	    strncpy(message, c, l);
	    message[l] = '\0';
	    sendMessage(ServerPlayer, AllPlayers, message, true);
	    c = j + 2;
	  }
	  strncpy(message, c, MessageLen - 1);
	  message[strlen(c) < MessageLen - 1 ? strlen(c) : MessageLen -1] = '\0';
	  sendMessage(ServerPlayer, AllPlayers, message, true);
	}
	// multi line from file advert
	if (adLines != NULL){
	  for (int j = 0; j < (int)adLines->size(); j ++) {
	    sendMessage(ServerPlayer, AllPlayers, (*adLines)[j].c_str());
	  }
	}
	lastbroadcast = TimeKeeper::getCurrent();
      }
    }

    // if any flags were in the air, see if they've landed
    if (numFlagsInAir > 0) {
      for (i = 0; i < numFlags; i++) {
	if (flag[i].flag.status == FlagInAir ||
	    flag[i].flag.status == FlagComing) {
	  if (flag[i].dropDone - tm <= 0.0f) {
	    flag[i].flag.status = FlagOnGround;
	    numFlagsInAir--;
	    sendFlagUpdate(i);
	  }
	}
	else if (flag[i].flag.status == FlagGoing) {
	  if (flag[i].dropDone - tm <= 0.0f) {
	    flag[i].flag.status = FlagNoExist;
	    numFlagsInAir--;
	    resetFlag(i);
	  }
	}
      }
    }

    // check team flag timeouts
    if (clOptions->gameStyle & TeamFlagGameStyle) {
      for (i = RedTeam; i < CtfTeams; ++i) {
        if (team[i].flagTimeout - tm < 0 && team[i].team.size == 0) {
          int flagid = lookupFirstTeamFlag(i);
	  if (flagid >= 0) {
	    for (int n = 0; n < clOptions->numTeamFlags[i]; n++) {
              if (flag[flagid+n].flag.status != FlagNoExist &&
	          flag[flagid+n].player == -1) {
	        DEBUG1("Flag timeout for team %d\n", i);
                zapFlag(flagid+n);
	      }
	    }
	  }
	}
      }
    }

    // maybe add a super flag (only if game isn't over)
    if (!gameOver && clOptions->numExtraFlags > 0) {
      float t = expf(-flagExp * (tm - lastSuperFlagInsertion));
      if ((float)bzfrand() > t) {
	// find an empty slot for an extra flag
	for (i = numFlags - clOptions->numExtraFlags; i < numFlags; i++)
	  if (flag[i].flag.type == Flags::Null)
	    break;
	if (i != numFlags)
	  randomFlag(i);
	lastSuperFlagInsertion = tm;
      }
    }

    // send lag pings
    for (int j=0;j<curMaxPlayers;j++)
    {
      if (player[j].state >= PlayerDead && player[j].type == TankPlayer
	  && player[j].nextping-tm < 0)
      {
	player[j].pingseqno = (player[j].pingseqno + 1) % 10000;
	if (player[j].pingpending) // ping lost
          updateLagLost(j);

	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUShort(bufStart, player[j].pingseqno);
	directMessage(j, MsgLagPing, (char*)buf - (char*)bufStart, bufStart);
	player[j].pingpending = true;
	player[j].lastping = tm;
	player[j].nextping = tm;
	player[j].nextping += 10.0f;
	player[j].pingssent++;
      }
    }

    // occasionally add ourselves to the list again (in case we were
    // dropped for some reason).
    if (clOptions->publicizeServer)
      if (tm - listServerLastAddTime > ListServerReAddTime) {
	// if there are no list servers and nobody is playing then
	// try publicizing again because we probably failed to get
	// the list last time we published, and if we don't do it
	// here then unless somebody stumbles onto this server then
	// quits we'll never try publicizing ourself again.
	if (listServerLinksCount == 0) {
	  // count the number of players
	  int i;
	  for (i = 0; i < curMaxPlayers; i++)
	    if (player[i].state > PlayerInLimbo)
	      break;

	  // if nobody playing then publicize
	  if (i == curMaxPlayers)
	    publicize();
	}

	// send add request
        sendMessageToListServer(ListServerLink::ADD);
	listServerLastAddTime = tm;
      }

    for (i = 0; i < curMaxPlayers; i++) {
      // kick any clients that need to be
      if (player[i].toBeKicked) {
	removePlayer(i, player[i].toBeKickedReason.c_str(), false);
	player[i].toBeKicked = false;
      }
    }
    // check messages
    if (nfound > 0) {
      //DEBUG1("chkmsg nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);
      // first check initial contacts
      if (FD_ISSET(wksSocket, &read_set))
	acceptClient();

      // check for connection to list server
      if (listServerLinksCount)
	if (listServerLink.socket != NotConnected)
	  if (FD_ISSET(listServerLink.socket, &write_set))
	    sendMessageToListServerForReal();
	  else if (FD_ISSET(listServerLink.socket, &read_set)) 
	    readListServer();

      // check if we have any UDP packets pending
      if (FD_ISSET(udpSocket, &read_set)) {
	TimeKeeper receiveTime = TimeKeeper::getCurrent();
	while (true) {
	  struct sockaddr_in uaddr;
	  unsigned char ubuf[MaxPacketLen];
	  AddrLen recvlen = sizeof(uaddr);
	  int n = recvfrom(udpSocket, (char *) ubuf, MaxPacketLen, MSG_PEEK,
	      (struct sockaddr*)&uaddr, &recvlen);
	  if (n < 0)
	    break;
	  if (n < 4) {
	    // flush malformed packet
            recvfrom(udpSocket, (char *) ubuf, MaxPacketLen, 0, (struct sockaddr *)&uaddr, &recvlen);
            continue;
	  }

	  // read head
	  uint16_t len, code;
	  void *buf = ubuf;
	  buf = nboUnpackUShort(buf, len);
	  buf = nboUnpackUShort(buf, code);
	  if (n == 6 && len == 2 && code == MsgPingCodeRequest) {
	    respondToPing();
            // flush PingCodeRequest packet (since we don't do a uread in this case)
            recvfrom(udpSocket, (char *) ubuf, MaxPacketLen, 0, (struct sockaddr *)&uaddr, &recvlen);
            continue;
 	  }

	  int numpackets;
	  int result = uread(&i, &numpackets, n, ubuf, uaddr);
	  if (result <= 0)
	    break;

	  // clear out message
	  player[i].udplen = 0;

	  // handle the command for UDP
	  handleCommand(i, code, len, player[i].udpmsg);

	  // don't spend more than 250ms receiving udp
	  if (TimeKeeper::getCurrent() - receiveTime > 0.25f) {
	    DEBUG2("Too much UDP traffic, will hope to catch up later\n");
	    break;
	  }
	}
      }

      // now check messages from connected players and send queued messages
      for (i = 0; i < curMaxPlayers; i++) {
	if (player[i].fd != NotConnected && FD_ISSET(player[i].fd, &write_set)) {
	  pflush(i);
	}

	if (player[i].state >= PlayerInLimbo && FD_ISSET(player[i].fd, &read_set)) {
	  // read header if we don't have it yet
	  if (player[i].tcplen < 4) {
	    pread(i, 4 - player[i].tcplen);

	    // if header not ready yet then skip the read of the body
	    if (player[i].tcplen < 4)
	      continue;
	  }

	  // read body if we don't have it yet
	  uint16_t len, code;
	  void *buf = player[i].tcpmsg;
	  buf = nboUnpackUShort(buf, len);
	  buf = nboUnpackUShort(buf, code);
	  if (len>MaxPacketLen) {
	    DEBUG1("Player [%d] sent huge packet length (len=%d), possible attack from %s\n",
		   i,len,player[i].peer.getDotNotation().c_str());
	    removePlayer(i, "large packet recvd", false);
	    continue;
	  }
	  if (player[i].tcplen < 4 + (int)len) {
	    pread(i, 4 + (int)len - player[i].tcplen);

	    // if body not ready yet then skip the command handling
	    if (player[i].tcplen < 4 + (int)len)
	      continue;
	  }

	  // clear out message
	  player[i].tcplen = 0;

	  // simple ruleset, if player sends a MsgShotBegin over TCP
	  // he/she must not be using the UDP link
	  if (clOptions->requireUDP && (player[i].type != ComputerPlayer)) {
	    if (code == MsgShotBegin) {
	      char message[MessageLen];
	      sprintf(message,"Your end is not using UDP, turn on udp");
	      sendMessage(ServerPlayer, i, message, true);

	      sprintf(message,"upgrade your client http://BZFlag.org/ or");
	      sendMessage(ServerPlayer, i, message, true);

	      sprintf(message,"Try another server, Bye!");
	      sendMessage(ServerPlayer, i, message, true);
	      removePlayer(i, "no UDP");
	      continue;
	    }
	  }

	  // handle the command
	  handleCommand(i, code, len, player[i].tcpmsg);
	}
      }
    }
    else if (nfound < 0) {
      if (getErrno() != EINTR) {
	// test code - do not uncomment, will cause big stuttering
	// sleep(1);
      }
    }

    //Fire world weapons
    wWeapons.fire();

  }

  serverStop();

  // free misc stuff
  delete clOptions; clOptions = NULL;
  delete[] flag;  flag = NULL;
  delete world; world = NULL;
  delete[] worldDatabase; worldDatabase = NULL;
  delete votingarbiter; votingarbiter = NULL;
#if defined(_WIN32)
  WSACleanup();
#endif /* defined(_WIN32) */

  // done
  return exitCode;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
