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

// every ListServerReAddTime server add ourself to the list
// server again.  this is in case the list server has reset
// or dropped us for some reason.
static const float ListServerReAddTime = 30.0f * 60.0f;

static const float FlagHalfLife = 45.0f;
// do NOT change
int NotConnected = -1;
int InvalidPlayer = -1;

float speedTolerance = 1.125f;

// Command Line Options
CmdLineOptions *clOptions;

// server address to listen on
Address serverAddress;
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
// Last known position, vel, etc
PlayerState lastState[MaxPlayers];
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

static float maxWorldHeight = 0.0f;

static char hexDigest[50];

#ifdef TIMELIMIT
TimeKeeper gameStartTime;
bool countdownActive = false;
#endif
static ListServerLink *listServerLink = NULL;
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

void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message, bool fullBuffer=false);


static void getSpawnLocation( int playerId, float* pos, float *azimuth);

void removePlayer(int playerIndex, const char *reason, bool notify=true);
void resetFlag(int flagIndex);
static void dropFlag(int playerIndex, float pos[3]);
static void dropAssignedFlag(int playerIndex);


// util functions
int getPlayerIDByRegName(const std::string &regName)
{
  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].getName() == regName)
      return i;
  }
  return -1;
}


bool hasPerm(int playerIndex, PlayerAccessInfo::AccessPerm right)
{
  return player[playerIndex].hasPermission(right);
}

static void pwrite(int playerIndex, const void *b, int l)
{
  int result = player[playerIndex].pwrite(b, l, udpSocket);
  if (result == -1)
    removePlayer(playerIndex, "ECONNRESET/EPIPE", false);
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
  if (!player[playerIndex].isConnected())
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
    if (player[i].isPlaying())
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

static bool realPlayer(const PlayerId& id)
{
  return id<=curMaxPlayers && player[id].isPlaying();
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
    if (pPlayerInfo->isMyUdpAddrPort(uaddr)) {
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
      if ((pi <= curMaxPlayers) && (player[pi].setUdpIn(uaddr))) {
	if (uaddr.sin_port)
	  // send client the message that we are ready for him
	  sendUDPupdate(pi);
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
      pPlayerInfo->debugUdpInfo();
    }
    DEBUG2("\n");
    *playerIndex = 0;
    return 0;
  }

  *playerIndex = pi;
  pPlayerInfo = &player[pi];

  pPlayerInfo->debugUdpRead(n, uaddr, udpSocket);

  if (n > 0) {
    *nopackets = 1;
    pPlayerInfo->udpFillRead(ubuf, n);
    return n;
  }
  return 0;
}


static bool pread(int playerIndex, int l)
{
  PlayerInfo& p = player[playerIndex];

  // read more data into player's message buffer
  const RxStatus e = p.receive(l);

  if (e == ReadAll) {
    return true;
  } else {
    if (e == ReadReset) {
      removePlayer(playerIndex, "ECONNRESET/EPIPE", false);
    } else if (e == ReadError) {
      // dump other errors and remove the player
      nerror("error on read");
      removePlayer(playerIndex, "Read error", false);
    } else if (e == ReadDiscon) {
      // disconnected
      removePlayer(playerIndex, "Disconnected", false);
    }
    return false;
  }
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
  buf = pPlayer->packUpdate(buf);
  if (playerIndex == index) {
    // send all players info about player[playerIndex]
    for (int i = 0; i < curMaxPlayers; i++)
      if (player[i].isPlaying())
	directMessage(i, MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
  } else
    directMessage(index, MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
}

void sendIPUpdate(int targetPlayer = -1, int playerIndex = -1) {
  // targetPlayer = -1: send to all players with the PLAYERLIST permission
  // playerIndex = -1: send info about all players

  // send to who?
  std::vector<int> receivers;
  if (targetPlayer != -1 && 
      hasPerm(targetPlayer, PlayerAccessInfo::playerList)) {
    receivers.push_back(targetPlayer);
  }
  else {
    for (int i = 0; i < curMaxPlayers; ++i) {
      if (player[i].isPlaying() && 
	  hasPerm(i, PlayerAccessInfo::playerList))
	receivers.push_back(i);
    }
  }

  // pack and send the message(s)
  void *buf, *bufStart = getDirectMessageBuffer();
  if (playerIndex != -1) {
    buf = nboPackUByte(bufStart, 1);
    buf = player[playerIndex].packAdminInfo(buf);
    for (unsigned int i = 0; i < receivers.size(); ++i) {
      directMessage(receivers[i], MsgAdminInfo,
		    (char*)buf - (char*)bufStart, bufStart);
    }
  }
  else {
    int i, numPlayers = 0;
    for (i = 0; i <= int(ObserverTeam); i++)
      numPlayers += team[i].team.size;
    int ipsPerPackage = (MaxPacketLen - 3) / (PlayerIdPLen + 7);
    int c = 0;
    buf = nboPackUByte(bufStart, 0); // will be overwritten later
    for (i = 0; i < curMaxPlayers; ++i) {
      if (player[i].isPlaying()) {
	buf = player[i].packAdminInfo(buf);
	++c;
      }
      if (c == ipsPerPackage || i + 1 == curMaxPlayers) {
	int size = (char*)buf - (char*)bufStart;
	buf = nboPackUByte(bufStart, c);
	c = 0;
	for (unsigned int j = 0; j < receivers.size(); ++j)
	  directMessage(receivers[j], MsgAdminInfo, size, bufStart);
      }
    }
  }
}

PingPacket getTeamCounts()
{
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
  return pingReply;
}

static void publicize()
{
  /* // hangup any previous list server sockets
  if (listServerLinksCount)
    listServerLink.closeLink(); */

  listServerLinksCount = 0;

  if (clOptions->publicizeServer) {
    // list server initialization
    listServerLink = new ListServerLink(clOptions->listServerURL, clOptions->publicizedAddress, clOptions->publicizedTitle);
    listServerLinksCount++;
  } else {
    // don't use a list server; we need a ListServerLink object anyway
    // pass no arguments to the constructor, so the object will exist but do nothing if called
    listServerLink = new ListServerLink();
    listServerLinksCount = 0;
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
    player[i].resetComm();
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
  for (i = 0; i < MaxPlayers; i++) {
    player[i].closeComm();
  }

  // remove from list server and disconnect
  // this destructor must be explicitly called
  listServerLink->~ListServerLink();
}


static void relayPlayerPacket(int index, uint16_t len, const void *rawbuf, uint16_t code)
{
  // relay packet to all players except origin
  for (int i = 0; i < curMaxPlayers; i++) {
    PlayerInfo& pi = player[i];
    if (i != index && pi.isPlaying()) {
      if ((code == MsgPlayerUpdate) && pi.haveFlag() && 
          (flag[pi.getFlag()].flag.type == Flags::Lag)) {
        // delay sending to this player
        pi.delayQueueAddPacket(len+4, rawbuf,
			       BZDB.eval(StateDatabase::BZDB_FAKELAG));
      } 
      else {
        // send immediately
        pwrite(i, rawbuf, len + 4);
      }
    }
  }
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
    const float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);
    // make pyramids
    if (haveRed) {
      // around red base
      const float *pos = bases[RedTeam].getBasePosition(0);
      world->addPyramid(
	  pos[0] + 0.5f * baseSize - pyrBase,
	  pos[1] - 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * baseSize + pyrBase,
	  pos[1] - 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * baseSize + pyrBase,
	  pos[1] + 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * baseSize - pyrBase,
	  pos[1] + 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
    }

    if (haveGreen) {
      // around green base
      const float *pos = bases[GreenTeam].getBasePosition(0);
      world->addPyramid(
	  pos[0] - 0.5f * baseSize + pyrBase,
	  pos[1] - 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * baseSize - pyrBase,
	  pos[1] - 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * baseSize - pyrBase,
	  pos[1] + 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * baseSize + pyrBase,
	  pos[1] + 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
    }

    if (haveBlue) {
      // around blue base
      const float *pos = bases[BlueTeam].getBasePosition(0);
      world->addPyramid(
	  pos[0] - 0.5f * baseSize - pyrBase,
	  pos[1] + 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * baseSize + pyrBase,
	  pos[1] + 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * baseSize - pyrBase,
	  pos[1] + 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * baseSize + pyrBase,
	  pos[1] + 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
    }

    if (havePurple) {
      // around purple base
      const float *pos = bases[PurpleTeam].getBasePosition(0);
      world->addPyramid(
	  pos[0] - 0.5f * baseSize - pyrBase,
	  pos[1] - 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] - 0.5f * baseSize + pyrBase,
	  pos[1] - 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * baseSize - pyrBase,
	  pos[1] - 0.5f * baseSize - pyrBase, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  pos[0] + 0.5f * baseSize + pyrBase,
	  pos[1] - 0.5f * baseSize + pyrBase, 0.0f, 0.0f,
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

      int numBoxes = int((0.5 + 0.4 * bzfrand()) * actCitySize * actCitySize);
      float boxHeight = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
      float boxBase = BZDB.eval(StateDatabase::BZDB_BOXBASE);

      for (i = 0; i < numBoxes;) {
	if (clOptions->randomHeights)
	  h = boxHeight * (2.0f * (float)bzfrand() + 0.5f);
	float x = worldSize * ((float)bzfrand() - 0.5f);
	float y = worldSize * ((float)bzfrand() - 0.5f);
	// don't place near center and bases
	if ((redGreen &&
	     (hypotf(fabs(x - redPosition[0]), fabs(y - redPosition[1])) <=
	      boxBase * 4 ||
	      hypotf(fabs(-x - redPosition[0]),fabs(-y - redPosition[1])) <=
	      boxBase * 4)) ||
	    (bluePurple &&
	     (hypotf(fabs(y - bluePosition[0]), fabs(-x - bluePosition[1])) <=
	      boxBase * 4 ||
	      hypotf(fabs(-y - bluePosition[0]), fabs(x - bluePosition[1])) <=
	      boxBase * 4)) ||
	    (redGreen && bluePurple &&
	     (hypotf(fabs(x - bluePosition[0]), fabs(y - bluePosition[1])) <=
	      boxBase * 4 ||
	      hypotf(fabs(-x - bluePosition[0]), fabs(-y - bluePosition[1])) <=
	      boxBase * 4 ||
	      hypotf(fabs(y - redPosition[0]), fabs(-x - redPosition[1])) <=
	      boxBase * 4 ||
	      hypotf(fabs(-y - redPosition[0]), fabs(x - redPosition[1])) <=
	      boxBase * 4)) ||
	    (hypotf(fabs(x), fabs(y)) <= worldSize / 12))
	  continue;

	float angle = 2.0f * M_PI * (float)bzfrand();
	if (redGreen) {
	  world->addBox(x, y, 0.0f, angle, boxBase, boxBase, h);
	  world->addBox(-x, -y, 0.0f, angle, boxBase, boxBase, h);
	  i += 2;
	}
	if (bluePurple) {
	  world->addBox(y, -x, 0.0f, angle, boxBase, boxBase, h);
	  world->addBox(-y, x, 0.0f, angle, boxBase, boxBase, h);
	  i += 2;
	}
      }

      // make pyramids
      h = BZDB.eval(StateDatabase::BZDB_PYRHEIGHT);
      const int numPyrs = int((0.5 + 0.4 * bzfrand()) * actCitySize * actCitySize * 2);
      for (i = 0; i < numPyrs; i++) {
	if (clOptions->randomHeights)
	  h = pyrHeight * (2.0f * (float)bzfrand() + 0.5f);
	float x = worldSize * ((float)bzfrand() - 0.5f);
	float y = worldSize * ((float)bzfrand() - 0.5f);
	// don't place near center or bases
	if ((redGreen &&
	     (hypotf(fabs(x - redPosition[0]), fabs(y - redPosition[1])) <=
	      pyrBase * 6 ||
	      hypotf(fabs(-x - redPosition[0]), fabs(-y - redPosition[1])) <=
	      pyrBase * 6)) ||
	    (bluePurple &&
	     (hypotf(fabs(y - bluePosition[0]), fabs(-x - bluePosition[1])) <=
	      pyrBase * 6 ||
	      hypotf(fabs(-y - bluePosition[0]), fabs(x - bluePosition[1])) <=
	      pyrBase * 6)) ||
	    (redGreen && bluePurple &&
	     (hypotf(fabs(x - bluePosition[0]), fabs(y - bluePosition[1])) <=
	      pyrBase * 6 ||
	      hypotf(fabs(-x - bluePosition[0]),fabs(-y - bluePosition[1])) <=
	      pyrBase * 6 ||
	      hypotf(fabs(y - redPosition[0]), fabs(-x - redPosition[1])) <=
	      pyrBase * 6 ||
	      hypotf(fabs(-y - redPosition[0]), fabs(x - redPosition[1])) <=
	      pyrBase * 6)) ||
	    (hypotf(fabs(x), fabs(y)) <= worldSize/12))
	  continue;

	float angle = 2.0f * M_PI * (float)bzfrand();
	if (redGreen) {
	  world->addPyramid(x, y, 0.0f, angle,pyrBase, pyrBase, h);
	  world->addPyramid(-x, -y, 0.0f, angle,pyrBase, pyrBase, h);
	  i += 2;
	}
	if (bluePurple) {
	  world->addPyramid(y, -x,0.0f, angle, pyrBase, pyrBase, h);
	  world->addPyramid(-y, x,0.0f, angle, pyrBase, pyrBase, h);
	  i += 2;
	}
      }

      // make teleporters
      if (clOptions->useTeleporters) {
	const int teamFactor = redGreen && bluePurple ? 4 : 2;
	const int numTeleporters = (8 + int(8 * (float)bzfrand())) / teamFactor * teamFactor;
	const int numLinks = 2 * numTeleporters / teamFactor;
	float teleBreadth = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
	float teleWidth = BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
	float teleHeight = BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
	int (*linked)[2] = new int[numLinks][2];
	for (i = 0; i < numTeleporters;) {
	  const float x = (worldSize - 4.0f * teleBreadth) * ((float)bzfrand() - 0.5f);
	  const float y = (worldSize - 4.0f * teleBreadth) * ((float)bzfrand() - 0.5f);
	  const float rotation = 2.0f * M_PI * (float)bzfrand();

	  // if too close to building then try again
	  if (NOT_IN_BUILDING != world->inBuilding(NULL, x, y, 0,
						   1.75f * teleBreadth,
						   1.0f))
	    continue;
	  // if to close to a base then try again
	  if ((redGreen &&
	       (hypotf(fabs(x - redPosition[0]), fabs(y - redPosition[1])) <=
		baseSize * 4 ||
		hypotf(fabs(x - greenPosition[0]), fabs(y - greenPosition[1])) <=
		baseSize * 4)) ||
	      (bluePurple &&
	       (hypotf(fabs(x - bluePosition[0]), fabs(y - bluePosition[1])) <=
		baseSize * 4 ||
		hypotf(fabs(x - purplePosition[0]), fabs(y - purplePosition[1])) <=
		baseSize * 4)))
	    continue;

	  linked[i / teamFactor][0] = linked[i / teamFactor][1] = 0;
	  if (redGreen) {
	    world->addTeleporter(x, y, 0.0f, rotation, 0.5f * teleWidth,
		teleBreadth, 2.0f * teleHeight, teleWidth);
	    world->addTeleporter(-x, -y, 0.0f, rotation + M_PI, 0.5f * teleWidth,
		teleBreadth, 2.0f * teleHeight, teleWidth);
	    i += 2;
	  }
	  if (bluePurple) {
	    world->addTeleporter(y, -x, 0.0f, rotation + M_PI / 2,
				 0.5f * teleWidth, teleBreadth, 2.0f * teleWidth,
				 teleWidth);
	    world->addTeleporter(-y, x, 0.0f, rotation + M_PI * 3 / 2,
				 0.5f * teleWidth, teleBreadth, 2.0f * teleWidth,
				 teleWidth);
	    i += 2;
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

    } else {

      float boxBase = BZDB.eval(StateDatabase::BZDB_BOXBASE);
      float avenueSize = BZDB.eval(StateDatabase::BZDB_AVENUESIZE);
      // pyramids in center
      world->addPyramid(
	  -(boxBase + 0.25f * avenueSize),
	  -(boxBase + 0.25f * avenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  (boxBase + 0.25f * avenueSize),
	  -(boxBase + 0.25f * avenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  -(boxBase + 0.25f * avenueSize),
	  (boxBase + 0.25f * avenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(
	  (boxBase + 0.25f * avenueSize),
	  (boxBase + 0.25f * avenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(0.0f, -(boxBase + 0.5f * avenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(0.0f,  (boxBase + 0.5f * avenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(-(boxBase + 0.5f * avenueSize), 0.0f, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid( (boxBase + 0.5f * avenueSize), 0.0f, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);

      // halfway out from city center
      world->addPyramid(0.0f, -(3.0f * boxBase + 1.5f * avenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(0.0f,  (3.0f * boxBase + 1.5f * avenueSize), 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid(-(3.0f * boxBase + 1.5f * avenueSize), 0.0f, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      world->addPyramid( (3.0f * boxBase + 1.5f * avenueSize), 0.0f, 0.0f, 0.0f,
	  pyrBase, pyrBase, pyrHeight);
      // add boxes, four at once with same height so no team has an advantage
      const float xmin = -0.5f * ((2.0f * boxBase + avenueSize) * (actCitySize - 1));
      const float ymin = -0.5f * ((2.0f * boxBase + avenueSize) * (actCitySize - 1));
      const float boxHeight = BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
      for (int j = 0; j <= actCitySize / 2; j++)
	for (int i = 0; i < actCitySize / 2; i++)
      if (i != actCitySize / 2 || j != actCitySize / 2) {
	float h = boxHeight;
	if (clOptions->randomHeights)
	  h *= 2.0f * (float)bzfrand() + 0.5f;
	world->addBox(
	    xmin + float(i) * (2.0f * boxBase + avenueSize),
	    ymin + float(j) * (2.0f * boxBase + avenueSize), 0.0f,
	    clOptions->randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    boxBase, boxBase, h);
	world->addBox(
	    -1.0f * (xmin + float(i) * (2.0f * boxBase + avenueSize)),
	    -1.0f * (ymin + float(j) * (2.0f * boxBase + avenueSize)), 0.0f,
	    clOptions->randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    boxBase, boxBase, h);
	world->addBox(
	    -1.0f * (ymin + float(j) * (2.0f * boxBase + avenueSize)),
	    xmin + float(i) * (2.0f * boxBase + avenueSize), 0.0f,
	    clOptions->randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    boxBase, boxBase, h);
	world->addBox(
	    ymin + float(j) * (2.0f * boxBase + avenueSize),
	    -1.0f * (xmin + float(i) * (2.0f * boxBase + avenueSize)), 0.0f,
	    clOptions->randomBoxes ? (0.5f * M_PI * ((float)bzfrand() - 0.5f)) : 0.0f,
	    boxBase, boxBase, h);
      }
      // add teleporters
      if (clOptions->useTeleporters) {
	float teleWidth = BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
	float teleBreadth = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
	float teleHeight = BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
	const float xoff = boxBase + 0.5f * avenueSize;
	const float yoff = boxBase + 0.5f * avenueSize;
	world->addTeleporter( xmin - xoff,  ymin - yoff, 0.0f, 1.25f * M_PI,
			     0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth);
	world->addTeleporter( xmin - xoff, -ymin + yoff, 0.0f, 0.75f * M_PI,
			     0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth);
	world->addTeleporter(-xmin + xoff,  ymin - yoff, 0.0f, 1.75f * M_PI,
			     0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth);
	world->addTeleporter(-xmin + xoff, -ymin + yoff, 0.0f, 0.25f * M_PI,
			     0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth);
	world->addTeleporter(-3.5f * teleBreadth, -3.5f * teleBreadth, 0.0f, 1.25f * M_PI,
			     0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth);
	world->addTeleporter(-3.5f * teleBreadth,  3.5f * teleBreadth, 0.0f, 0.75f * M_PI,
			     0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth);
	world->addTeleporter( 3.5f * teleBreadth, -3.5f * teleBreadth, 0.0f, 1.75f * M_PI,
			     0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth);
	world->addTeleporter( 3.5f * teleBreadth,  3.5f * teleBreadth, 0.0f, 0.25f * M_PI,
			     0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth);

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

  float worldfactor = worldSize / (float)DEFAULT_WORLD;
  int actCitySize = int(clOptions->citySize * worldfactor + 0.5f);
  int numTeleporters = 8 + int(8 * (float)bzfrand() * worldfactor);
  float boxBase = BZDB.eval(StateDatabase::BZDB_BOXBASE);
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
	  boxBase, boxBase, h);
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
    float teleBreadth = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
    float teleWidth = BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
    float teleHeight = BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
    int (*linked)[2] = new int[numTeleporters][2];
    for (i = 0; i < numTeleporters;) {
      const float x = (worldSize - 4.0f * teleBreadth) * ((float)bzfrand() - 0.5f);
      const float y = (worldSize - 4.0f * teleBreadth) * ((float)bzfrand() - 0.5f);
      const float rotation = 2.0f * M_PI * (float)bzfrand();

      // if too close to building then try again
      if (NOT_IN_BUILDING != world->inBuilding(NULL, x, y, 0,
					       1.75f * teleBreadth, 1.0f))
	continue;

      world->addTeleporter(x, y, 0.0f, rotation,
	  0.5f*teleWidth, teleBreadth, 2.0f*teleHeight, teleWidth);
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

  if (worldDatabase)
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
  if (!worldDatabase)
    return false;
  memset(worldDatabase, 0, worldDatabaseSize);

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
  md5.update((unsigned char *)worldDatabase, worldDatabaseSize);
  md5.finalize();
  if (clOptions->worldFile == NULL)
    strcpy(hexDigest, "t");
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
    player[i].dumpScore();
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
    if (!player[playerIndex].exist())
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

  buffer[8] = (uint8_t)playerIndex;
  send(fd, (const char*)buffer, sizeof(buffer), 0);

  // FIXME add new client server welcome packet here when client code is ready
  player[playerIndex].initPlayer(clientAddr, fd, playerIndex);
  lastState[playerIndex].order = 0;

#ifdef HAVE_ADNS_H
  // launch the asynchronous query to look up this hostname
  if (adns_submit_reverse
      (PlayerInfo::adnsState, (struct sockaddr *)&clientAddr,
       adns_r_ptr,
       (adns_queryflags)(adns_qf_quoteok_cname|adns_qf_cname_loose),
       0, &player[playerIndex].adnsQuery) != 0) {
    DEBUG1("Player [%d] failed to submit reverse resolve query: errno %d\n", playerIndex, getErrno());
    player[playerIndex].adnsQuery = NULL;
  } else {
    DEBUG2("Player [%d] submitted reverse resolve query\n", playerIndex);
  }
#endif


  player[playerIndex].initStatistics();

  // if game was over and this is the first player then game is on
  if (gameOver) {
    int count = 0;
    for (int i = 0; i < curMaxPlayers; i++)
      if (player[i].isPlaying())
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
      if (player[i].isPlaying() && player[i].isTeam(team))
        directMessage(i, MsgMessage, len, bufStart);
  } else if (targetPlayer == AdminPlayers){
		// admin messages
		for (int i = 0; i < curMaxPlayers; i++){
			if (player[i].isPlaying() && 
					hasPerm(i, PlayerAccessInfo::adminMessages)){
					directMessage(i, MsgMessage, len, bufStart);
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
    player[playerIndex].dropUnconnected();
  int teamNum;
  for (teamNum = RogueTeam; teamNum < RabbitTeam; teamNum++)
    team[teamNum].team.size = 0;
  for (playerIndex = 0; playerIndex < maxPlayers; playerIndex++)
    if (player[playerIndex].isPlaying()) {
      teamNum = player[playerIndex].getTeam();
      if (teamNum == RabbitTeam)
	teamNum = RogueTeam;
      team[teamNum].team.size++;
    }
}

static void addPlayer(int playerIndex)
{
  // don't allow empty callsign
  if (player[playerIndex].getCallSign()[0] == '\0')
    rejectPlayer(playerIndex, RejectBadCallsign);

  // look if there is as name clash, we won't allow this
  int i;
  for (i = 0; i < curMaxPlayers; i++) {
    if (i == playerIndex || !player[i].isPlaying())
      continue;
    if (strcasecmp(player[i].getCallSign(),
		   player[playerIndex].getCallSign()) == 0) {
      rejectPlayer(playerIndex, RejectRepeatCallsign);
      return;
    }
  }

  // make sure the callsign is not obscene/filtered
  if (clOptions->filterCallsigns) {
    DEBUG2("checking callsign: %s\n",player[playerIndex].getCallSign());
    bool filtered = false;
    char cs[CallSignLen];
    memcpy(cs, player[playerIndex].getCallSign(), sizeof(char) * CallSignLen);
    filtered = clOptions->filter.filter(cs, clOptions->filterSimple);
    if (filtered) {
      rejectPlayer(playerIndex, RejectBadCallsign);
      return ;
    }
  }

  if (!player[playerIndex].isCallSignReadable()) {
    DEBUG2("rejecting unreadable callsign: %s\n",
	   player[playerIndex].getCallSign());
    rejectPlayer(playerIndex, RejectBadCallsign);
  }

  // make sure the email is not obscene/filtered
  if (clOptions->filterCallsigns) {
    DEBUG2("checking email: %s\n",player[playerIndex].getEMail());
    char em[EmailLen];
    memcpy(em, player[playerIndex].getEMail(), sizeof(char) * EmailLen);
    bool filtered = clOptions->filter.filter(em, clOptions->filterSimple);
    if (filtered) {
      rejectPlayer(playerIndex, RejectBadEmail);
      return ;
    }
  }

  if (!player[playerIndex].isEMailReadable()) {
    DEBUG2("rejecting unreadable player email: %s (%s)\n", 
	   player[playerIndex].getCallSign(),
	   player[playerIndex].getEMail());
    rejectPlayer(playerIndex, RejectBadEmail);
  }


  TeamColor t = player[playerIndex].getTeam();

  // count current number of players and players+observers
  int numplayers = 0;
  for (i = 0; i < int(ObserverTeam); i++)
    numplayers += team[i].team.size;
  const int numplayersobs = numplayers + team[ObserverTeam].team.size;

  // no player slots open -> try observer
  if (numplayers == maxRealPlayers) {
    t = ObserverTeam;
    player[playerIndex].setTeam(ObserverTeam);
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
	t = ObserverTeam;
	player[playerIndex].setTeam(ObserverTeam);
      } else if (minIndex.size() == 1) {
        // only one team has a slot open anyways
        t = minIndex[0];
	player[playerIndex].setTeam(minIndex[0]);
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
          t = minIndex[rand() % minIndex.size()];
          player[playerIndex].setTeam(t);
        }
      }
    }
  }

  // reject player if asks for bogus team or rogue and rogues aren't allowed
  // or if the team is full or if the server is full
  if (!player[playerIndex].isHuman() && !player[playerIndex].isBot()) {
    rejectPlayer(playerIndex, RejectBadType);
        return;
  } else if (t == NoTeam) {
    rejectPlayer(playerIndex, RejectBadTeam);
        return;
  } else if (t == ObserverTeam && player[playerIndex].isBot()) {
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
  player[playerIndex].resetPlayer
    ((clOptions->gameStyle & TeamFlagGameStyle) != 0);

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
  if (!player[playerIndex].isConnected())
    return;

  // player is signing on (has already connected via addClient).
  player[playerIndex].signingOn();
  // update team state and if first player on team,
  // add team's flag and reset it's score
  bool resetTeamFlag = false;
  int teamIndex = int(player[playerIndex].getTeam());
  team[teamIndex].team.size++;
  if (team[teamIndex].team.size == 1
      && Team::isColorTeam(player[playerIndex].getTeam())) {
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
  if (!player[playerIndex].isBot()) {
    int i;
    if (player[playerIndex].isConnected()) {
      sendTeamUpdate(playerIndex);
      sendFlagUpdate(-1, playerIndex);
    }
    for (i = 0; i < curMaxPlayers && player[playerIndex].isConnected(); i++)
      if (player[i].isPlaying() && i != playerIndex)
	sendPlayerUpdate(i, playerIndex);
  }

  // if new player connection was closed (because of an error) then stop here
  if (!player[playerIndex].isConnected())
    return;

  // send MsgAddPlayer to everybody -- this concludes MsgEnter response
  // to joining player
  sendPlayerUpdate(playerIndex, playerIndex);

  // send update of info for team just joined
  sendTeamUpdate(-1, teamIndex);
  
  // send IP update to everyone with PLAYERLIST permission
  sendIPUpdate(-1, playerIndex);
  
  // send rabbit information
  if (clOptions->gameStyle & int(RabbitChaseGameStyle)) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, rabbitIndex);
    directMessage(playerIndex, MsgNewRabbit, (char*)buf-(char*)bufStart, bufStart);
  }

#ifdef TIMELIMIT
  // send time update to new player if we're counting down
  if (countdownActive && clOptions->timeLimit > 0.0f
      && !player[playerIndex].isBot()) {
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
  if (!player[playerIndex].isConnected())
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
  listServerLink->queueMessage(ListServerLink::ADD);

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

  if (player[playerIndex].isObserver())
    sendMessage(ServerPlayer, playerIndex, "You are in observer mode.");
#endif

  if (player[playerIndex].isRegistered()) {
    // nick is in the DB send him a message to identify.
    if (player[playerIndex].isIdentifyRequired())
      sendMessage(ServerPlayer, playerIndex, "This callsign is registered.  You must identify yourself before playing.");
    else
      sendMessage(ServerPlayer, playerIndex, "This callsign is registered.");
    sendMessage(ServerPlayer, playerIndex, "Identify with /identify <your password>");
  }

  dropAssignedFlag(playerIndex);
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
	
	// decide how sticky the flag will be
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
  float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);
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
    ObstacleLocation *obj;
    float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    if (!world->getZonePoint( std::string(pFlagInfo->flag.type->flagAbbv), pFlagInfo->flag.position)) {
      pFlagInfo->flag.position[0] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
      pFlagInfo->flag.position[1] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
      pFlagInfo->flag.position[2] = 0.0f;
    }
    int topmosttype = world->inBuilding(&obj,
					pFlagInfo->flag.position[0],
					pFlagInfo->flag.position[1],
					pFlagInfo->flag.position[2],
					r,
					flagHeight);
    while (topmosttype != NOT_IN_BUILDING) {
      if ((clOptions->flagsOnBuildings
	   && ((topmosttype == IN_BOX_NOTDRIVETHROUGH) || (topmosttype == IN_BASE)))
	  && (obj->pos[2] < (pFlagInfo->flag.position[2] + flagHeight - Epsilon))
	  && ((obj->pos[2] + obj->size[2] - Epsilon) > pFlagInfo->flag.position[2])
	  && (world->inRect(obj->pos, obj->rotation, obj->size, pFlagInfo->flag.position[0], pFlagInfo->flag.position[1], 0.0f)))
      {
	pFlagInfo->flag.position[2] = obj->pos[2] + obj->size[2];
      }
      else
      {
	pFlagInfo->flag.position[0] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	pFlagInfo->flag.position[1] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
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
    player[playerIndex].resetFlag();

    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, playerIndex);
    buf = nboPackUShort(buf, uint16_t(flagIndex));
    buf = flag[flagIndex].flag.pack(buf);
    broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
  }

  // if flag was flying then it flies no more
  if (flag[flagIndex].flag.status == FlagInAir ||
      flag[flagIndex].flag.status == FlagComing ||
      flag[flagIndex].flag.status == FlagGoing)
    numFlagsInAir--;

  // reset flag status
  resetFlag(flagIndex);
}

// try to get over a bug where extraneous flag are attached to a tank
// not really found why, but this should fix
// Should be called when we sure that tank does not hold any
static void dropAssignedFlag(int playerIndex) {
  for (int flagIndex  = 0; flagIndex < numFlags; flagIndex++)
    if (flag[flagIndex].flag.status == FlagOnTank
	&& flag[flagIndex].flag.owner == playerIndex)
      resetFlag(flagIndex);
} // dropAssignedFlag

// Take into account the quality of player wins/(wins+loss)
// Try to penalize winning casuality 
static float rabbitRank (PlayerInfo& player) {
  if (clOptions->rabbitSelection == RandomRabbitSelection)
    return (float)bzfrand();
  
  // otherwise do score-based ranking
  return player.scoreRanking();
}

static void anointNewRabbit(int killerId = NoPlayer)
{
  float topRatio = -100000.0f;
  int i;
  int oldRabbit = rabbitIndex;
  rabbitIndex = NoPlayer;

  if (clOptions->rabbitSelection == KillerRabbitSelection)
    // check to see if the rabbit was just killed by someone; if so, make them the rabbit if they're still around.
    if (killerId != oldRabbit && realPlayer(killerId)
	&& player[killerId].canBeRabbit())
      rabbitIndex = killerId;
  
  if (rabbitIndex == NoPlayer) {
    for (i = 0; i < curMaxPlayers; i++) {
      if (i != oldRabbit && player[i].canBeRabbit()) {
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
      if (player[i].canBeRabbit(true)) {
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
      player[oldRabbit].wasARabbit();
    }
    if (rabbitIndex != NoPlayer) {
      player[rabbitIndex].setTeam(RabbitTeam);
      void *buf, *bufStart = getDirectMessageBuffer();
      buf = nboPackUByte(bufStart, rabbitIndex);
      broadcastMessage(MsgNewRabbit, (char*)buf-(char*)bufStart, bufStart);
    }
  }
}


static void pausePlayer(int playerIndex, bool paused)
{
  player[playerIndex].setPaused(paused);
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

  // send a super kill to be polite
  if (notify)
    directMessage(playerIndex, MsgSuperKill, 0, getDirectMessageBuffer());

  // if there is an active poll, cancel any vote this player may have made
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");
  if ((arbiter != NULL) && (arbiter->knowsPoll())) {
    arbiter->retractVote(std::string(player[playerIndex].getCallSign()));
  }

  bool wasPlaying = player[playerIndex].removePlayer(reason);

  // player is outta here.  if player never joined a team then
  // don't count as a player.

  if (wasPlaying) {

    if (clOptions->gameStyle & int(RabbitChaseGameStyle))
      if (playerIndex == rabbitIndex)
	anointNewRabbit();

    if (!player[playerIndex].isTeam(NoTeam)) {
      int flagid = player[playerIndex].getFlag();
      if (flagid >= 0) {
	// do not simply zap team flag
	Flag &carriedflag = flag[flagid].flag;
	if (carriedflag.type->flagTeam != ::NoTeam) {
	  dropFlag(playerIndex, lastState[playerIndex].pos);
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
      int teamNum = int(player[playerIndex].getTeam());
      --team[teamNum].team.size;

      // if last active player on team then remove team's flag if no one
      // is carrying it
      if (Team::isColorTeam(player[playerIndex].getTeam())
	  && team[teamNum].team.size == 0 &&
	  (clOptions->gameStyle & int(TeamFlagGameStyle))) {
	int flagid = lookupFirstTeamFlag(teamNum);
	if (flagid >= 0) {
	  for (int n = 0; n < clOptions->numTeamFlags[teamNum]; n++) {
	    if (flag[flagid+n].player == -1
		|| player[flag[flagid+n].player].isTeam((TeamColor)teamNum))
	      zapFlag(flagid+n);
	  }
	}
      }

      // send team update
      sendTeamUpdate(-1, teamNum);
    }

    fixTeamCount();

    // tell the list server the new number of players
    listServerLink->queueMessage(ListServerLink::ADD);
  }

  while ((playerIndex >= 0)
	 && (playerIndex+1 == curMaxPlayers)
	 && !player[playerIndex].exist()
	 && !player[playerIndex].isConnected())
    {
      playerIndex--;
      curMaxPlayers--;
    }

  if (wasPlaying) {
    // anybody left?
    int i;
    for (i = 0; i < curMaxPlayers; i++)
      if (player[i].isPlaying())
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
}

// are the two teams foes withthe current game style?
static bool areFoes(TeamColor team1, TeamColor team2)
{
  return team1!=team2 ||
         (team1==RogueTeam && !(clOptions->gameStyle & int(RabbitChaseGameStyle)));
}

static float enemyProximityCheck(TeamColor team, float *pos, float &enemyAngle)
{
  float worstDist = 1e12f; // huge number

  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].isAlive() && areFoes(player[i].getTeam(), team)) {
      float *enemyPos = lastState[i].pos;
      if (fabs(enemyPos[2] - pos[2]) < 1.0f) {
        float x = enemyPos[0] - pos[0];
        float y = enemyPos[1] - pos[1];
        float distSq = x * x + y * y;
        if (distSq < worstDist) {
          worstDist = distSq;
	  enemyAngle = lastState[i].azimuth;
	}
      }
    }
  }

  return sqrtf(worstDist);
}

static void getSpawnLocation(int playerId, float* spawnpos, float *azimuth)
{
  const float tankRadius = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
  const TeamColor team = player[playerId].getTeam();
  *azimuth = (float)bzfrand() * 2.0f * M_PI;

  if (player[playerId].shouldRestartAtBase() &&
      (team >= RedTeam) && (team <= PurpleTeam) && 
      (bases.find(team) != bases.end())) {
    TeamBases &teamBases = bases[team];
    const TeamBase &base = teamBases.getRandomBase( (int) (bzfrand() * 100) );
    base.getRandomPosition( spawnpos[0], spawnpos[1], spawnpos[2] );
    player[playerId].setRestartOnBase(false);
  }
  else {
    bool onGroundOnly = !clOptions->respawnOnBuildings
      || player[playerId].isBot();
    const float size = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    ObstacleLocation *building;

    // keep track of how much time we spend searching for a location
    TimeKeeper start = TimeKeeper::getCurrent();

    int inAirAttempts = 50;
    int tries = 0;
    float minProximity = size / 3.0f;
    float bestDist = -1.0f;
    float pos[3];
    bool foundspot = false;
    while (!foundspot) {
      if (!world->getZonePoint(std::string(Team::getName(team)), pos)) {
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
      }
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
	float enemyAngle;
        float dist = enemyProximityCheck(team, pos, enemyAngle);
        if (dist > bestDist) { // best so far
          bestDist = dist;
          spawnpos[0] = pos[0];
          spawnpos[1] = pos[1];
          spawnpos[2] = pos[2];
	  *azimuth = fmod((enemyAngle + M_PI), 2.0f * M_PI);
        }
        if (bestDist < minProximity) { // not good enough, keep looking
          foundspot = false;
          minProximity *= 0.99f; // relax requirements a little
        }
      }
    }
  }
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
    if (player[i].isPlaying())
      numPlayers++;

  // first send number of teams and players being sent
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, NumTeams);
  buf = nboPackUShort(buf, numPlayers);
  directMessage(playerIndex, MsgQueryPlayers, (char*)buf-(char*)bufStart, bufStart);

  // now send the teams and players
  if (player[playerIndex].isConnected())
    sendTeamUpdate(playerIndex);
  for (i = 0; i < curMaxPlayers && player[playerIndex].isConnected(); i++)
    if (player[i].isPlaying())
      sendPlayerUpdate(i, playerIndex);
}

static void playerAlive(int playerIndex)
{
  // ignore multiple MsgAlive; also observer should not send MsgAlive; diagnostic?
  if (!player[playerIndex].isDead() ||
      player[playerIndex].isObserver())
    return;

  // make sure the user identifies themselves if required.
  if (!player[playerIndex].isAllowedToEnter()) {
    sendMessage(ServerPlayer, playerIndex, "This callsign is registered.  You must identify yourself before playing or use a different callsign.");
    removePlayer(playerIndex, "unidentified");
    return;
  }
  
  if (player[playerIndex].isBot()
      && BZDB.isTrue(StateDatabase::BZDB_DISABLEBOTS)) {
    sendMessage(ServerPlayer, playerIndex, "I'm sorry, we do not allow bots on this server.");
    removePlayer(playerIndex, "ComputerPlayer");
    return;
  }

  // player is coming alive.
  player[playerIndex].setAlive();
  dropAssignedFlag(playerIndex);

  // send MsgAlive
  float pos[3], fwd;
  getSpawnLocation(playerIndex, pos, &fwd);
  // update last position immediately
  lastState[playerIndex].pos[0] = pos[0];
  lastState[playerIndex].pos[1] = pos[1];
  lastState[playerIndex].pos[2] = pos[2];
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackVector(buf,pos);
  buf = nboPackFloat(buf,fwd);
  broadcastMessage(MsgAlive, (char*)buf-(char*)bufStart, bufStart);

  if (clOptions->gameStyle & int(RabbitChaseGameStyle)) {
    player[playerIndex].wasNotARabbit();
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
  if (!victim->isAlive()) return;

  victim->setDead();

  // killing rabbit or killing anything when I am a dead ex-rabbit is allowed
  bool teamkill = false;
  if (killer) {
    const bool rabbitinvolved = killer->isARabbitKill(*victim);
    const bool foe = areFoes(victim->getTeam(), killer->getTeam());
    teamkill = !foe && !rabbitinvolved;
  }

  //update tk-score
  if ((victimIndex != killerIndex) && teamkill) {
    bool isTk = killer->setAndTestTK((float)clOptions->teamKillerKickRatio);
    if (isTk) {
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
  int flagid = victim->getFlag();
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
  victim->setOneMoreLoss();
  if (killer) {
    if (victimIndex != killerIndex) {
      if (teamkill) {
        if (clOptions->teamKillerDies)
          playerKilled(killerIndex, killerIndex, reason, -1);
        else
          killer->setOneMoreLoss();
      } else
        killer->setOneMoreWin();
    }

    buf = nboPackUByte(bufStart, 2);
    buf = killer->packScore(buf);
  }
  else {
    buf = nboPackUByte(bufStart, 1);
  }

  buf = victim->packScore(buf);
  broadcastMessage(MsgScore, (char*)buf-(char*)bufStart, bufStart);

  // see if the player reached the score limit
  if (clOptions->maxPlayerScore != 0
      && killerIndex != InvalidPlayer
      && killerIndex != ServerPlayer
      && killer->scoreReached(clOptions->maxPlayerScore)) {
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
      if (killer && victim->getTeam() == killer->getTeam()) {
	if (!killer->isTeam(RogueTeam))
	  if (killerIndex == victimIndex)
	    team[int(victim->getTeam())].team.lost += 1;
	  else
	    team[int(victim->getTeam())].team.lost += 2;
      } else {
	if (killer && !killer->isTeam(RogueTeam)) {
	  winningTeam = int(killer->getTeam());
	  team[winningTeam].team.won++;
	}
	if (!victim->isTeam(RogueTeam))
	  team[int(victim->getTeam())].team.lost++;
	if (killer)
	  killerTeam = killer->getTeam();
      }
      sendTeamUpdate(-1,int(victim->getTeam()), killerTeam);
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
  if (player[playerIndex].isObserver() ||
      !player[playerIndex].isAlive() ||
      player[playerIndex].haveFlag() ||
      flag[flagIndex].flag.status != FlagOnGround)
    return;

  //last Pos might be lagged by TankSpeed so include in calculation
  const float tankRadius = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
  const float tankSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);
  const float radius2 = (tankSpeed + tankRadius + BZDBCache::flagRadius) * (tankSpeed + tankRadius + BZDBCache::flagRadius);
  const float* tpos = lastState[playerIndex].pos;
  const float* fpos = flag[flagIndex].flag.position;
  const float delta = (tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
		      (tpos[1] - fpos[1]) * (tpos[1] - fpos[1]);

  if ((fabs(tpos[2] - fpos[2]) < 0.1f) && (delta > radius2)) {
       DEBUG2("Player %s [%d] %f %f %f tried to grab distant flag %f %f %f: distance=%f\n",
    player[playerIndex].getCallSign(), playerIndex,
    tpos[0], tpos[1], tpos[2], fpos[0], fpos[1], fpos[2], sqrt(delta));
    return;
  }

  // okay, player can have it
  flag[flagIndex].flag.status = FlagOnTank;
  flag[flagIndex].flag.owner = playerIndex;
  flag[flagIndex].player = playerIndex;
  flag[flagIndex].numShots = 0;
  player[playerIndex].setFlag(flagIndex);

  // send MsgGrabFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = flag[flagIndex].flag.pack(buf);
  broadcastMessage(MsgGrabFlag, (char*)buf-(char*)bufStart, bufStart);

  player[playerIndex].addFlagToHistory(flag[flagIndex].flag.type);
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
  ObstacleLocation* container;
  int topmosttype = NOT_IN_BUILDING;
  ObstacleLocation* topmost = 0;
  // player wants to drop flag.  we trust that the client won't tell
  // us to drop a sticky flag until the requirements are satisfied.
  const int flagIndex = player[playerIndex].getFlag();
  if (flagIndex < 0)
    return;
  FlagInfo &drpFlag = flag[flagIndex];
  if (drpFlag.flag.status != FlagOnTank)
    return;
  int flagTeam = drpFlag.flag.type->flagTeam;
  bool isTeamFlag = (flagTeam != ::NoTeam);

  // okay, go ahead and drop it
  drpFlag.player = -1;
  numFlagsInAir++;

  // limited flags that have been fired should be disposed of 
  bool limited = clOptions->flagLimit[drpFlag.flag.type] != -1;
  if (limited && drpFlag.numShots > 0) drpFlag.grabs = 0;

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
  else if (clOptions->flagsOnBuildings && (topmosttype == IN_BOX_NOTDRIVETHROUGH || topmosttype == IN_BASE)) {
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
  
  // removed any delayed packets (in case it was a "Lag Flag")
  player[playerIndex].delayQueueDequeuePackets();

  // player no longer has flag -- send MsgDropFlag
  player[playerIndex].resetFlag();

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = drpFlag.flag.pack(buf);
  broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);

  // notify of new flag state
  sendFlagUpdate(flagIndex);

}

static void captureFlag(int playerIndex, TeamColor teamCaptured)
{
  // Sanity check
  if (teamCaptured < RedTeam || teamCaptured > PurpleTeam)
    return;

  // player captured a flag.  can either be enemy flag in player's own
  // team base, or player's own flag in enemy base.
  int flagIndex = player[playerIndex].getFlag();
  if (flagIndex < 0 || (flag[flagIndex].flag.type->flagTeam == ::NoTeam))
    return;

  // player no longer has flag and put flag back at it's base
  player[playerIndex].resetFlag();
  resetFlag(flagIndex);

  // send MsgCaptureFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = nboPackUShort(buf, uint16_t(teamCaptured));
  broadcastMessage(MsgCaptureFlag, (char*)buf-(char*)bufStart, bufStart);

  // everyone on losing team is dead
  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i].isConnected() &&
	flag[flagIndex].flag.type->flagTeam == int(player[i].getTeam()) &&
	player[i].isAlive()) {
      player[i].setDead();
      player[i].setRestartOnBase(true);
    }

  // update score (rogues can't capture flags)
  int winningTeam = (int)NoTeam;
  if (int(flag[flagIndex].flag.type->flagTeam)
      != int(player[playerIndex].getTeam())) {
    // player captured enemy flag
    winningTeam = int(player[playerIndex].getTeam());
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
  if (shooter.isObserver())
    return;
  FiringInfo firingInfo;
  firingInfo.unpack(buf);
  const ShotUpdate &shot = firingInfo.shot;

  // verify playerId
  if (shot.player != playerIndex) {
    DEBUG2("Player %s [%d] shot playerid mismatch\n", shooter.getCallSign(),
	   playerIndex);
    return;
  }

  // make sure the shooter flag is a valid index to prevent segfaulting later
  if (!shooter.haveFlag()) {
    firingInfo.flagType = Flags::Null;
    repack = true;
  }

  // verify player flag
  if ((firingInfo.flagType != Flags::Null)
      && (firingInfo.flagType != flag[shooter.getFlag()].flag.type)) {
    DEBUG2("Player %s [%d] shot flag mismatch %s %s\n", shooter.getCallSign(),
	   playerIndex, firingInfo.flagType->flagAbbv,
	   flag[shooter.getFlag()].flag.type->flagAbbv);
    firingInfo.flagType = Flags::Null;
    firingInfo.shot.vel[0] = BZDB.eval(StateDatabase::BZDB_SHOTSPEED)
      * cos(lastState[playerIndex].azimuth);
    firingInfo.shot.vel[1] = BZDB.eval(StateDatabase::BZDB_SHOTSPEED)
      * sin(lastState[playerIndex].azimuth);
    firingInfo.shot.vel[2] = 0.0f;
  }

  // verify shot number
  if ((shot.id & 0xff) > clOptions->maxShots - 1) {
    DEBUG2("Player %s [%d] shot id out of range %d %d\n",
	   shooter.getCallSign(),
	   playerIndex,	shot.id & 0xff, clOptions->maxShots);
    return;
  }

  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  float tankSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);
  float lifetime = BZDB.eval(StateDatabase::BZDB_RELOADTIME);
  if (firingInfo.flagType == Flags::ShockWave) {
    shotSpeed = 0.0f;
    tankSpeed = 0.0f;
  } else if (firingInfo.flagType == Flags::Velocity) {
    tankSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
  } else if (firingInfo.flagType == Flags::Thief) {
    tankSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
  } else if ((firingInfo.flagType == Flags::Burrow) && (firingInfo.shot.pos[2] < BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT))) {
    tankSpeed *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
  } else if (firingInfo.flagType == Flags::Agility) {
    tankSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
  } else {
    //If shot is different height than player, can't be sure they didn't drop V in air
    if (lastState[playerIndex].pos[2] != (shot.pos[2]-BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT))) {
      tankSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
    }
  }

  // FIXME, we should look at the actual TankSpeed ;-)
  shotSpeed += tankSpeed;

  // verify lifetime
  if (fabs(firingInfo.lifetime - lifetime) > Epsilon) {
    DEBUG2("Player %s [%d] shot lifetime mismatch %f %f\n",
	   shooter.getCallSign(),
	   playerIndex, firingInfo.lifetime, lifetime);
    return;
  }

  // verify velocity
  if (hypotf(shot.vel[0], hypotf(shot.vel[1], shot.vel[2])) > shotSpeed * 1.01f) {
    DEBUG2("Player %s [%d] shot over speed %f %f\n", shooter.getCallSign(),
	   playerIndex, hypotf(shot.vel[0], hypotf(shot.vel[1], shot.vel[2])),
	   shotSpeed);
    return;
  }

  // verify position
  float dx = lastState[playerIndex].pos[0] - shot.pos[0];
  float dy = lastState[playerIndex].pos[1] - shot.pos[1];
  float dz = lastState[playerIndex].pos[2] - shot.pos[2];

  float front = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT);
  if (firingInfo.flagType == Flags::Obesity)
    front *= BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);

  float delta = dx*dx + dy*dy + dz*dz;
  if (delta > (BZDB.eval(StateDatabase::BZDB_TANKSPEED) * BZDB.eval(StateDatabase::BZDB_VELOCITYAD) + front) *
	      (BZDB.eval(StateDatabase::BZDB_TANKSPEED) * BZDB.eval(StateDatabase::BZDB_VELOCITYAD) + front)) {
    DEBUG2("Player %s [%d] shot origination %f %f %f too far from tank %f %f %f: distance=%f\n",
	    shooter.getCallSign(), playerIndex,
	    shot.pos[0], shot.pos[1], shot.pos[2],
	    lastState[playerIndex].pos[0], lastState[playerIndex].pos[1],
	    lastState[playerIndex].pos[2], sqrt(delta));
    return;
  }

  // repack if changed
  if (repack)
    firingInfo.pack(buf);


  // if shooter has a flag

  char message[MessageLen];
  if (shooter.haveFlag()){

    FlagInfo & fInfo = flag[shooter.getFlag()];
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
	    lastPos[i] = lastState[playerIndex].pos[i];
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

  } else if (strncmp(message + 1, "poll", 4) == 0) {
    handlePollCmd(t, message);

  } else if (strncmp(message + 1, "vote", 4) == 0) {
    handleVoteCmd(t, message);

  } else if (strncmp(message + 1, "veto", 4) == 0) {
    handleVetoCmd(t, message);

  } else if (strncmp(message + 1, "viewreports", 11) == 0) {
    handleViewReportsCmd(t, message);

  } else if (strncmp(message + 1, "clientquery", 11) == 0) {
    handleClientqueryCmd(t, message);

  } else {
    char reply[MessageLen];
    snprintf(reply, MessageLen, "Unknown command [%s]", message + 1);
    sendMessage(ServerPlayer, t, reply, true);
  }
}

static void handleCommand(int t, uint16_t code, uint16_t len,
			  const void *rawbuf)
{
  void *buf = (void*)((char*)rawbuf + 4);
#ifdef NETWORK_STATS
  player[t].countMessage(code, len, 0);
#endif
  switch (code) {
    // player joining
    case MsgEnter: {
      player[t].unpackEnter(buf);
      addPlayer(t);
      player[t].debugAdd();
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
	player[t].setPlayedEarly();
#endif
      playerAlive(t);
      break;
    }

    // player sent version string
    case MsgVersion: {
      buf = player[t].setClientVersion(len, buf);
      break;
    }

    // player declaring self destroyed
    case MsgKilled: {
      if (player[t].isObserver())
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
      if (player[t].isObserver())
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
      if (player[t].isObserver())
	break;
      uint16_t from, to;
      buf = nboUnpackUShort(buf, from);
      buf = nboUnpackUShort(buf, to);
      sendTeleport(t, from, to);
      break;
    }

    // player sending a message
    case MsgMessage: {
      // data: target player/team, message string
      PlayerId targetPlayer;
      char message[MessageLen];
      buf = nboUnpackUByte(buf, targetPlayer);
      buf = nboUnpackString(buf, message, sizeof(message));
      message[MessageLen - 1] = '\0';
      player[t].hasSent(message);
      // check for command
      if (message[0] == '/') {
				/* make commands case insensitive for user-friendlyness */
				unsigned int pos=1;
				while ((pos < strlen(message)) && (isAlphanumeric(message[pos]))) {
					message[pos] = tolower((int)message[pos]);
					pos++;
				}
				parseCommand(message, t);
      }
			else if (targetPlayer == AdminPlayers && hasPerm(t, PlayerAccessInfo::adminMessages)) {
				sendMessage (t, AdminPlayers, message, true);			
			}
      // check if the target player is invalid
      else if (targetPlayer < LastRealPlayer && 
               !player[targetPlayer].isPlaying()) {
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
      }
      break;
    }

    // player has transferred flag to another tank
    case MsgTransferFlag: {
	PlayerId from, to;

	buf = nboUnpackUByte(buf, from);
	buf = nboUnpackUByte(buf, to);

	if (to == ServerPlayer) {
	  zapFlag (player[from].getFlag());
	  return;
	}

	// Sanity check
	if (from >= curMaxPlayers)
	  return;
	if (to >= curMaxPlayers)
	  return;

	int flagIndex = player[from].getFlag();
	if (flagIndex == -1)
	  return;

	zapFlag(player[to].getFlag());

	void *bufStart = getDirectMessageBuffer();
	void *buf = nboPackUByte(bufStart, from);
	buf = nboPackUByte(buf, to);
	buf = nboPackUShort(buf, uint16_t(flagIndex));
	flag[flagIndex].flag.owner = to;
	flag[flagIndex].player = to;
	player[to].resetFlag();
	player[to].setFlag(flagIndex);
	player[from].resetFlag();
	buf = flag[flagIndex].flag.pack(buf);
	broadcastMessage(MsgTransferFlag, (char*)buf - (char*)bufStart, bufStart);
	break;
    }

    case MsgUDPLinkEstablished:
      player[t].setUdpOut();
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
      bool warn;
      bool kick;
      int lag = player[t].updatePingLag(buf, clOptions->lagwarnthresh,
					(float)clOptions->maxlagwarn, warn, kick);
      if (warn) {
	char message[MessageLen];
	sprintf(message,"*** Server Warning: your lag is too high (%d ms) ***",
		lag);
	sendMessage(ServerPlayer, t, message, true);
	if (kick) {
	  // drop the player
	  sprintf
	    (message,
	     "You have been kicked due to excessive lag (you have been warned %d times).",
	     clOptions->maxlagwarn);
	  sendMessage(ServerPlayer, t, message, true);
	  removePlayer(t, "lag");
	}
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
	if ((id >= curMaxPlayers) || !player[id].isBot()) {
	  // FIXME - Commented out autokick occasionally being kicked
	  // out with Robot
	  // Should check why!
// 	  char message[MessageLen];
	  DEBUG1("kicking Player %s [%d] Invalid Id %s [%d]\n",
		 player[t].getCallSign(), t, player[id].getCallSign(), id);
// 	  strcpy(message, "Autokick: Using invalid PlayerId, don't cheat.");
// 	  sendMessage(ServerPlayer, t, message, true);
// 	  removePlayer(t, "Using invalid PlayerId");
// 	  break;
	} else
	  t = id;
      }

      // silently drop old packet
      if (state.order <= lastState[t].order)
	break;

      player[t].updateLagPlayerUpdate(timestamp,
				      state.order - lastState[t].order > 1);

      TimeKeeper now = TimeKeeper::getCurrent();
      //Don't kick players up to 10 seconds after a world parm has changed,
      static const float heightFudge = 1.1f; /* 10% */
      if (now - lastWorldParmChange > 10.0f) {
	float gravity;
	
	if (flag[player[t].getFlag()].flag.type == Flags::Wings)
          gravity = BZDB.eval(StateDatabase::BZDB_WINGSGRAVITY);
	else
	  gravity = BZDB.eval(StateDatabase::BZDB_GRAVITY);

	if (gravity < 0.0f) {
	  float maxTankHeight;
	  
	  if (flag[player[t].getFlag()].flag.type == Flags::Wings)
	    maxTankHeight = maxWorldHeight + heightFudge * ((BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY)*BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY)*(1+BZDB.eval(StateDatabase::BZDB_WINGSJUMPCOUNT))) / (2.0f * -gravity));
          else
	    maxTankHeight = maxWorldHeight + heightFudge * ((BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY)*BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY)) / (2.0f * -gravity));

	  if (state.pos[2] > maxTankHeight) {
	    DEBUG1("Kicking Player %s [%d] jumped too high [max: %f height: %f]\n", player[t].getCallSign(), t, maxTankHeight, state.pos[2]);
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
	  DEBUG1("Kicking Player %s [%d] Out of map bounds at position (%.2f,%.2f,%.2f)\n", player[t].getCallSign(), t, state.pos[0], state.pos[1], state.pos[2]);
	  sendMessage(ServerPlayer, t, "Autokick: Player location was outside the playing area.", true);
	  removePlayer(t, "Out of map bounds");
	}

	// Speed problems occur around flag drops, so don't check for
	// a short period of time after player drops a flag. Currently
	// 2 second, adjust as needed.
	if (player[t].isFlagTransitSafe()) {
	  // check for highspeed cheat; if inertia is enabled, skip test for now
	  if (clOptions->linearAcceleration == 0.0f) {
	    // Doesn't account for going fast backwards, or jumping/falling
	    float curPlanarSpeedSqr = state.velocity[0]*state.velocity[0] +
				      state.velocity[1]*state.velocity[1];

	    float maxPlanarSpeedSqr = BZDB.eval(StateDatabase::BZDB_TANKSPEED)*BZDB.eval(StateDatabase::BZDB_TANKSPEED);

	    bool logOnly = false;

	    // if tank is not driving cannot be sure it didn't toss (V) in flight
	    // if tank is not alive cannot be sure it didn't just toss (V)
  	    if (flag[player[t].getFlag()].flag.type == Flags::Velocity)
	      maxPlanarSpeedSqr *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD) * BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
	    else if (flag[player[t].getFlag()].flag.type == Flags::Thief)
	      maxPlanarSpeedSqr *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD) * BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
	    else if (flag[player[t].getFlag()].flag.type == Flags::Agility)
	      maxPlanarSpeedSqr *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL) * BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
 	    else if ((flag[player[t].getFlag()].flag.type == Flags::Burrow) &&
	      (lastState[t].pos[2] == state.pos[2]) && 
	      (lastState[t].velocity[2] == state.velocity[2]) &&
	      (state.pos[2] <= BZDB.eval(StateDatabase::BZDB_BURROWDEPTH)))
	      // if we have burrow and are not actively burrowing
	      // You may have burrow and still be above ground. Must check z in ground!!
 	      maxPlanarSpeedSqr *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD) * BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
	    else {
	      // If player is moving vertically, or not alive the speed checks
	      // seem to be problematic. If this happens, just log it for now,
	      // but don't actually kick
	      if ((lastState[t].pos[2] != state.pos[2])
	      ||  (lastState[t].velocity[2] != state.velocity[2])
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
		player[t].getCallSign(), t,
		sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
	      } else {
		DEBUG1("Kicking Player %s [%d] tank too fast (tank: %f, allowed: %f)\n",
		       player[t].getCallSign(), t,
		       sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
		sendMessage(ServerPlayer, t, "Autokick: Player tank is moving too fast.", true);
		removePlayer(t, "too fast");
	      }
	      break;
	    }
	  }
	}
      }

      lastState[t] = state;

      // Player might already be dead and did not know it yet (e.g. teamkill)
      // do not propogate
      if (!player[t].isAlive() && (state.status & short(PlayerState::Alive)))
        break;
    }

    //Fall thru
    case MsgGMUpdate:
    case MsgAudio:
    case MsgVideo:
      // observer shouldn't send bulk messages anymore, they used to when it was
      // a server-only hack; but the check does not hurt, either
      if (player[t].isObserver())
	break;
      relayPlayerPacket(t, len, rawbuf, code);
      break;

    // FIXME handled inside uread, but not discarded
    case MsgUDPLinkRequest:
      break;

    // unknown msg type
    default:
      player[t].debugUnknownPacket(code);
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
	StateDatabase::Permission permission = BZDB.getPermission(args[0]);
	if ((permission == StateDatabase::ReadWrite) || (permission == StateDatabase::Locked)) {
	  BZDB.set(args[0], args[1], StateDatabase::Server);
	  lastWorldParmChange = TimeKeeper::getCurrent();
	  return args[0] + " set";
	}
	return "variable " + args[0] + " is not writeable";
      } else {
	return "variable " + args[0] + " does not exist";
      }
    case 1:
      if (BZDB.isSet(args[0])) {
	return args[0] + " is " + BZDB.get(args[0]);
      } else {
	return "variable " + args[0] + " does not exist";
      }
    default:
      return "usage: set <name> [<value>]";
  }
}

static void resetAllCallback(const std::string &name, void*)
{
  StateDatabase::Permission permission = BZDB.getPermission(name);
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
    } else if (BZDB.isSet(args[0])) {
      StateDatabase::Permission permission = BZDB.getPermission(args[0]);
      if ((permission == StateDatabase::ReadWrite) || (permission == StateDatabase::Locked)) {
	BZDB.set(args[0], BZDB.getDefault(args[0]), StateDatabase::Server);
	lastWorldParmChange = TimeKeeper::getCurrent();
	return args[0] + " reset";
      }
      return "variable " + args[0] + " is not writeable";
    } else {
      return "variable " + args[0] + " does not exist";
    }
  } else {
    return "usage: reset <name>";
  }
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
  // don't die on broken pipe
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
  if (adns_init(&PlayerInfo::adnsState, adns_if_nosigpipe, 0) < 0) {
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
    clOptions->publicizedAddress += string_util::format(":%d", clOptions->wksPort);
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
    std::cerr << "ERROR: Unable to start the server, perhaps one is already running?" << std::endl;
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
      player[i].fdSet(&read_set, &write_set, maxFileDescriptor);
    }
    // always listen for connections
    _FD_SET(wksSocket, &read_set);
    if (wksSocket > maxFileDescriptor)
      maxFileDescriptor = wksSocket;
    _FD_SET(udpSocket, &read_set);
    if (udpSocket > maxFileDescriptor)
      maxFileDescriptor = udpSocket;

    // check for list server socket connected
    if (listServerLinksCount)
      if (listServerLink->isConnected()) {
	if (listServerLink->phase == ListServerLink::CONNECTING)
	  _FD_SET(listServerLink->linkSocket, &write_set);
	else
	  _FD_SET(listServerLink->linkSocket, &read_set);
	if (listServerLink->linkSocket > maxFileDescriptor)
	  maxFileDescriptor = listServerLink->linkSocket;
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

    int p;
    // get time for next lagping
    bool someoneIsConnected = false;
    for (p = 0; p < curMaxPlayers; p++) {
      if (player[p].nextPing(waitTime)) {
	someoneIsConnected = true;
      }
    }

    // get time for next delayed packet (Lag Flag)
    for (p = 0; p < curMaxPlayers; p++) {
      if (player[p].isPlaying()) {
        float nextTime = player[p].delayQueueNextPacketTime();
        if (nextTime < waitTime) {
          waitTime = nextTime;
        }
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
	broadcastMessage(MsgTimeUpdate, (char*)buf - (char*)bufStart, bufStart);
	clOptions->timeElapsed = newTimeElapsed;
	if (clOptions->oneGameOnly && timeLeft == 0.0f) {
	  done = true;
	  exitCode = 0;
	}
      }
    }
#endif

    // send delayed packets
    for (p = 0; p < curMaxPlayers; p++) {
      void *data;
      int length;
      if (player[p].delayQueueGetPacket(&length, &data)) {
        pwrite (p, data, length);
        free (data);
      }
    }

    // kick idle players
    if (clOptions->idlekickthresh > 0) {
      for (int i = 0; i < curMaxPlayers; i++) {
	if (player[i].isTooMuchIdling(tm, clOptions->idlekickthresh)) {
	  char message[MessageLen]
	    = "You were kicked because you were idle too long";
	  sendMessage(ServerPlayer, i,  message, true);
	  removePlayer(i, "idling");
	}
      }
    }

    // update notResponding
    for (int h = 0; h < curMaxPlayers; h++) {
      if (player[h].hasStartedToNotRespond()) {
	// if player is the rabbit, anoint a new one
	if (h == rabbitIndex)
	  anointNewRabbit();
	// if player is holding a flag, drop it
	for (int j = 0; j < numFlags; j++) {
	  if (flag[j].player == h) {
	    dropFlag(h, lastState[h].pos);
	  }
	}
      }
    }

#ifdef HAVE_ADNS_H
    for (int h = 0; h < curMaxPlayers; h++) {
      if (player[h].checkDNSResolution()) {
	// check against ban lists
	if (!clOptions->acl.hostValidate(player[h].hostname)) {
	  removePlayer(h, "bannedhost");
	}
      }
    }
#endif

    // manage voting poll for collective kicks/bans/sets
    if ((clOptions->voteTime > 0) && (votingarbiter != NULL)) {
      if (votingarbiter->knowsPoll()) {
	char message[MessageLen];

	std::string target = votingarbiter->getPollTarget();
	std::string action = votingarbiter->getPollAction();
	std::string realIP = votingarbiter->getPollTargetIP();

	static unsigned short int voteTime = 0;

	/* flags to only blather once */
	static bool announcedOpening = false;
	static bool announcedClosure = false;
	static bool announcedResults = false;

	/* once a poll begins, announce its commencement */
	if (!announcedOpening) {
	  voteTime = votingarbiter->getVoteTime();
	  sprintf(message, "A poll to %s %s has begun.  Players have up to %d seconds to vote.", action.c_str(), target.c_str(), voteTime);
	  sendMessage(ServerPlayer, AllPlayers, message, true);
	  announcedOpening = true;
	}

	static TimeKeeper lastAnnounce = TimeKeeper::getNullTime();

	/* make a heartbeat announcement every 15 seconds */
	if (((voteTime - (int)(TimeKeeper::getCurrent() - votingarbiter->getStartTime()) - 1) % 15 == 0) &&
	    ((int)(TimeKeeper::getCurrent() - lastAnnounce) != 0) &&
	    (votingarbiter->timeRemaining() > 0)) {
	  sprintf(message, "%d seconds remain in the poll to %s %s.", votingarbiter->timeRemaining(), action.c_str(), target.c_str());
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
	      std::string pollAction;
	      if (action == "ban")
		pollAction = "temporarily banned";
	      else if (action == "kick")
		pollAction = "kicked";
	      else
		pollAction = action;
	      // a poll that exists and is closed has ended successfully
	      if(action != "flagreset")
	      	sprintf(message, "The poll is now closed and was successful.  %s is scheduled to be %s.", target.c_str(), pollAction.c_str());
	      else
	      	sprintf(message, "The poll is now closed and was successful.  Currently unused flags are scheduled to be reset.");
   	      sendMessage(ServerPlayer, AllPlayers, message, true);
	      announcedClosure = true;
	    }
	  } else {
	    if (!announcedClosure) {
	      sprintf(message, "The poll to %s %s was not successful", action.c_str(), target.c_str());
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
	      std::string pollAction;
	      if (action == "ban")
		pollAction = "banned for 10 minutes.";
	      else if (action == "kick")
		pollAction = "kicked.";
	      else
		pollAction = action;
	      if (action != "flagreset")
	      	sprintf(message, "%s has been %s", target.c_str(), pollAction.c_str());
	      else
	      	sprintf(message, "All unused flags have now been reset.");
	      sendMessage(ServerPlayer, AllPlayers, message, true);

	      /* regardless of whether or not the player was found, if the poll
	       * is a ban poll, ban the weenie
	       */
	      if (action == "ban") {
		clOptions->acl.ban(realIP.c_str(), target.c_str(), 10);
	      }
	      
	      if ((action == "ban") || (action == "kick")) {
		// lookup the player id
		bool foundPlayer = false;
		int v;
		for (v = 0; v < curMaxPlayers; v++) {
		  if (strncmp(player[v].getCallSign(),
			      target.c_str(), 256) == 0) {
		    foundPlayer = true;
		    break;
		  }
		}
		// show the delinquent no mercy; make sure he is kicked even if he changed
		// his callsign by finding a corresponding IP and matching it to the saved one
		if (!foundPlayer) {
		  for (v = 0; v < curMaxPlayers; v++) {
		    if (player[v].isAtIP(realIP)) {
		      foundPlayer = true;
		      break;
		    }
		  }
		}
		if (foundPlayer) {
		  // notify the player
		  sprintf(message, "You have been %s due to sufficient votes to have you removed", action == "ban" ? "temporarily banned" : "kicked");
		  sendMessage(ServerPlayer, v, message, true);
		  sprintf(message, "/poll %s", action.c_str());
		  removePlayer(v, message);
		}
     	      } else if (action == "set") {
		parseCommand(string_util::format("set %s", target.c_str()).c_str(), ServerPlayer);
	      } else if (action == "flagreset") {
		parseCommand(string_util::format("flag reset unused").c_str(), ServerPlayer);
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
	    if (action != "flagreset")
	      sprintf(message, "Enough votes were collected to %s %s early.", action.c_str(), target.c_str());
 	    else
 	      sprintf(message, "Enough votes were collected to reset all unused flags early.");

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
	if (adLines != NULL) {
	  for (int j = 0; j < (int)adLines->size(); j++) {
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
	} else if (flag[i].flag.status == FlagGoing) {
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
              if (flag[flagid + n].flag.status != FlagNoExist &&
	          flag[flagid + n].player == -1) {
	        DEBUG1("Flag timeout for team %d\n", i);
                zapFlag(flagid + n);
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
    for (int j=0;j<curMaxPlayers;j++) {
      int nextPingSeqno = player[j].getNextPingSeqno();
      if (nextPingSeqno > 0) {
	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUShort(bufStart, nextPingSeqno);
	directMessage(j, MsgLagPing, (char*)buf - (char*)bufStart, bufStart);
      }
    }

    // occasionally add ourselves to the list again (in case we were
    // dropped for some reason).
    if (clOptions->publicizeServer)
      if (tm - listServerLink->lastAddTime > ListServerReAddTime) {
	// if there are no list servers and nobody is playing then
	// try publicizing again because we probably failed to get
	// the list last time we published, and if we don't do it
	// here then unless somebody stumbles onto this server then
	// quits we'll never try publicizing ourself again.
	if (listServerLinksCount == 0) {
	  // count the number of players
	  int i;
	  for (i = 0; i < curMaxPlayers; i++)
	    if (player[i].isPlaying())
	      break;

	  // if nobody playing then publicize
	  if (i == curMaxPlayers)
	    publicize();
	}

	// send add request
        listServerLink->queueMessage(ListServerLink::ADD);
      }

    for (i = 0; i < curMaxPlayers; i++) {
      // kick any clients that need to be
      std::string reasonToKick = player[i].reasonToKick();
      if (reasonToKick != "") {
	removePlayer(i, reasonToKick.c_str(), false);
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
	if (listServerLink->isConnected())
	  if (FD_ISSET(listServerLink->linkSocket, &write_set))
	    listServerLink->sendQueuedMessages();
	  else if (FD_ISSET(listServerLink->linkSocket, &read_set)) 
	    listServerLink->read();

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

	  // handle the command for UDP
	  handleCommand(i, code, len, player[i].getUdpBuffer());

	  // don't spend more than 250ms receiving udp
	  if (TimeKeeper::getCurrent() - receiveTime > 0.25f) {
	    DEBUG2("Too much UDP traffic, will hope to catch up later\n");
	    break;
	  }
	}
      }

      // now check messages from connected players and send queued messages
      for (i = 0; i < curMaxPlayers; i++) {
	// send whatever we have ... if any
	if (player[i].pflush(&write_set) == -1) {
	  removePlayer(i, "ECONNRESET/EPIPE", false);
	}

	if (player[i].exist() && player[i].fdIsSet(&read_set)) {
	  // read header if we don't have it yet
	  if (!pread(i, 4))
	    // if header not ready yet then skip the read of the body
	    continue;

	  // read body if we don't have it yet
	  uint16_t len, code;
	  void *buf = player[i].getTcpBuffer();
	  buf = nboUnpackUShort(buf, len);
	  buf = nboUnpackUShort(buf, code);
	  if (len>MaxPacketLen) {
	    player[i].debugHugePacket(len);
	    removePlayer(i, "large packet recvd", false);
	    continue;
	  }
	  // if body not ready yet then skip the command handling
	  if (!pread(i, 4 + (int)len))
	    continue;

	  // clear out message
	  player[i].cleanTcp();

	  // simple ruleset, if player sends a MsgShotBegin over TCP
	  // he/she must not be using the UDP link
	  if (clOptions->requireUDP && !player[i].isBot()) {
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
	  handleCommand(i, code, len, player[i].getTcpBuffer());
	}
      }
    } else if (nfound < 0) {
      if (getErrno() != EINTR) {
	// test code - do not uncomment, will cause big stuttering
	// sleep(1);
      }
    }

    // Fire world weapons
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
