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

// interface header
#include "bzfs.h"

// implementation-specific system headers
#include <algorithm>
#include <assert.h>
#include <errno.h>
#include <vector>
#include <string>
#include <time.h>

// implementation-specific bzflag headers
#include "NetHandler.h"
#include "VotingArbiter.h"
#include "version.h"
#include "md5.h"
#include "BZDBCache.h"
#include "ShotUpdate.h"
#include "CommandManager.h"
#include "TimeBomb.h"
#include "ConfigFileManager.h"
#include "bzsignal.h"

// implementation-specific bzfs-specific headers
#include "RejoinList.h"
#include "GameKeeper.h"
#include "ListServerConnection.h"
#include "WorldInfo.h"
#include "TeamBases.h"
#include "WorldWeapons.h"
#include "BZWReader.h"
#include "PackVars.h"
#include "SpawnPosition.h"
#include "commands.h"
#include "FlagInfo.h"
#include "MasterBanList.h"

const int udpBufSize = 128000;

// (2000m / 2^16) = 3.05 cm resolution
const float smallWorldSize = 2000.0f;

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
bool handlePings = true;
static PingPacket pingReply;
// highest fd used
static int maxFileDescriptor;
// Last known position, vel, etc
PlayerState lastState[MaxPlayers  + ReplayObservers];
// team info
TeamInfo team[NumTeams];
// num flags in flag list
int numFlags;
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

char hexDigest[50];

TimeKeeper gameStartTime;
bool countdownActive = false;

static ListServerLink *listServerLink = NULL;
static int listServerLinksCount = 0;

// FIXME: should be static, but needed by SpawnPosition
WorldInfo *world = NULL;
// FIXME: should be static, but needed by RecordReplay
char *worldDatabase = NULL;
uint32_t worldDatabaseSize = 0;


BasesList bases;

// FIXME - define a well-known constant for a null playerid in address.h?
// might be handy in other players, too.
// Client does not check for rabbit to be 255, but it still works
// because 255 should be > curMaxPlayers and thus no matchign player will
// be found.
// FIXME: should be static, but needed by RecordReplay
uint8_t rabbitIndex = NoPlayer;

static RejoinList rejoinList;

static TimeKeeper lastWorldParmChange;

void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message);

void removePlayer(int playerIndex, const char *reason, bool notify=true);
void resetFlag(FlagInfo &flag);
static void dropFlag(GameKeeper::Player &playerData, float pos[3]);
static void dropAssignedFlag(int playerIndex);

int getCurMaxPlayers()
{
  return curMaxPlayers;
}

static bool realPlayer(const PlayerId& id)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(id);
  return playerData && playerData->player.isPlaying();
}

static int pwrite(GameKeeper::Player &playerData, const void *b, int l)
{
  int result = playerData.netHandler->pwrite(b, l);
  if (result == -1)
    removePlayer(playerData.getIndex(), "ECONNRESET/EPIPE", false);
  return result;
}

static char sMsgBuf[MaxPacketLen];
char *getDirectMessageBuffer()
{
  return &sMsgBuf[2*sizeof(short)];
}


// FIXME? 4 bytes before msg must be valid memory, will get filled in with len+code
// usually, the caller gets a buffer via getDirectMessageBuffer(), but for example
// for MsgShotBegin the receiving buffer gets used directly
static int directMessage(GameKeeper::Player &playerData,
			 uint16_t code, int len, const void *msg)
{
  // send message to one player
  void *bufStart = (char *)msg - 2*sizeof(short);

  void *buf = bufStart;
  buf = nboPackUShort(buf, uint16_t(len));
  buf = nboPackUShort(buf, code);
  return pwrite(playerData, bufStart, len + 4);
}

void directMessage(int playerIndex, uint16_t code, int len, const void *msg)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  directMessage(*playerData, code, len, msg);
}


void broadcastMessage(uint16_t code, int len, const void *msg)
{
  // send message to everyone
  for (int i = 0; i < curMaxPlayers; i++) {
    if (realPlayer(i)) {
      directMessage(i, code, len, msg);
    }
  }

  // record the packet
  if (Record::enabled()) {
    Record::addPacket (code, len, msg);
  }

  return;
}


//
// global variable callback
//
static void onGlobalChanged(const std::string& msg, void*)
{
  // This Callback is removed in replay mode. As
  // well, the /set and /reset commands are blocked.

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

static int lookupPlayer(const PlayerId& id)
{
  if (id == ServerPlayer)
    return id;

  if (!realPlayer(id))
    return InvalidPlayer;

  return id;
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

void sendFlagUpdate(FlagInfo &flag)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart,1);
  buf = flag.pack(buf);
  broadcastMessage(MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart);
}

// Update the player "playerIndex" with all the flags status
static void sendFlagUpdate(int playerIndex)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;
  int result;

  void *buf, *bufStart = getDirectMessageBuffer();

  buf = nboPackUShort(bufStart,0); //placeholder
  int cnt = 0;
  int length = sizeof(uint16_t);
  for (int flagIndex = 0; flagIndex < numFlags; flagIndex++) {
    FlagInfo &flag = *FlagInfo::get(flagIndex);
    if (flag.exist()) {
      if ((length + sizeof(uint16_t) + FlagPLen)
	  > MaxPacketLen - 2*sizeof(uint16_t)) {
	nboPackUShort(bufStart, cnt);
	result = directMessage(*playerData, MsgFlagUpdate,
			       (char*)buf - (char*)bufStart, bufStart);
	if (result == -1)
	  return;
	cnt    = 0;
	length = sizeof(uint16_t);
	buf    = nboPackUShort(bufStart,0); //placeholder
      }

      buf     = flag.pack(buf);
      length += sizeof(uint16_t)+FlagPLen;
      cnt++;
    }
  }

  if (cnt > 0) {
    nboPackUShort(bufStart, cnt);
    result = directMessage(*playerData, MsgFlagUpdate,
			   (char*)buf - (char*)bufStart, bufStart);
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


static void sendPlayerUpdate(GameKeeper::Player *playerData, int index)
{
  if (!playerData->player.isPlaying())
    return;

  void *bufStart = getDirectMessageBuffer();
  void *buf      = playerData->packPlayerUpdate(bufStart);

  if (playerData->getIndex() == index) {
    // send all players info about player[playerIndex]
    broadcastMessage(MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
  } else {
    directMessage(index, MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
  }
}

void sendIPUpdate(int targetPlayer = -1, int playerIndex = -1) {
  // targetPlayer = -1: send to all players with the PLAYERLIST permission
  // playerIndex = -1: send info about all players

  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (playerIndex >= 0) {
    if (!playerData || !playerData->player.isPlaying())
      return;
  }

  // send to who?
  std::vector<int> receivers
    = GameKeeper::Player::allowed(PlayerAccessInfo::playerList, targetPlayer);

  // pack and send the message(s)
  void *buf, *bufStart = getDirectMessageBuffer();
  if (playerIndex >= 0) {
    buf = nboPackUByte(bufStart, 1);
    buf = playerData->packAdminInfo(buf);
    for (unsigned int i = 0; i < receivers.size(); ++i) {
      directMessage(receivers[i], MsgAdminInfo,
		    (char*)buf - (char*)bufStart, bufStart);
    }
    if (Record::enabled()) {
      Record::addPacket (MsgAdminInfo,
                         (char*)buf - (char*)bufStart, bufStart, HiddenPacket);
    }
  } else {
    int i, numPlayers = 0;
    for (i = 0; i <= int(ObserverTeam); i++)
      numPlayers += team[i].team.size;
    int ipsPerPackage = (MaxPacketLen - 3) / (PlayerIdPLen + 7);
    int c = 0;
    buf = nboPackUByte(bufStart, 0); // will be overwritten later
    for (i = 0; i < curMaxPlayers; ++i) {
      playerData = GameKeeper::Player::getPlayerByIndex(i);
      if (!playerData)
	continue;
      if (playerData->player.isPlaying()) {
	buf = playerData->packAdminInfo(buf);
	++c;
      }
      if (c == ipsPerPackage || ((i + 1 == curMaxPlayers) && c)) {
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

  if (listServerLink)
    delete listServerLink;

  if (clOptions->publicizeServer) {
    // list server initialization
    for (std::vector<std::string>::const_iterator i = clOptions->listServerURL.begin(); i < clOptions->listServerURL.end(); i++) {
      listServerLink = new ListServerLink(i->c_str(), clOptions->publicizedAddress, clOptions->publicizedTitle);
      listServerLinksCount++;
    }
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

  addr.sin_port = htons(clOptions->wksPort);
  if (!NetHandler::initHandlers(addr)) {
    close(wksSocket);
    return false;
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
  for (int i = 0; i < curMaxPlayers; i++)
    directMessage(i, MsgSuperKill, 0, getDirectMessageBuffer());

  // close connections
  NetHandler::destroyHandlers();

  // remove from list server and disconnect
  // this destructor must be explicitly called
  listServerLink->~ListServerLink();

  // clean up Kerberos
  Authentication::cleanUp();
}


static void relayPlayerPacket(int index, uint16_t len, const void *rawbuf, uint16_t code)
{
  if (Record::enabled()) {
    Record::addPacket (code, len, (char*)rawbuf + 4);
  }

  // relay packet to all players except origin
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData)
      continue;
    PlayerInfo& pi = playerData->player;

    if (i != index && pi.isPlaying()) {
      if (((code == MsgPlayerUpdate) ||(code == MsgPlayerUpdateSmall))
          && pi.haveFlag()
	  && (FlagInfo::get(pi.getFlag())->flag.type == Flags::Lag)) {
        // delay sending to this player
	playerData->delayq.addPacket(len+4, rawbuf,
				     BZDB.eval(StateDatabase::BZDB_FAKELAG));
      } else {
        // send immediately
        pwrite(*playerData, rawbuf, len + 4);
      }
    }
  }
}

static WorldInfo *defineTeamWorld()
{
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
	Obstacle* obs;
	if (NOT_IN_BUILDING != world->inCylinderNoOctree(&obs, x, y, 0,
						 1.75f * teleBreadth, 1.0f))
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
	      teleBreadth, 2.0f * teleHeight, teleWidth, false);
	  world->addTeleporter(-x, -y, 0.0f, rotation + M_PI, 0.5f * teleWidth,
	      teleBreadth, 2.0f * teleHeight, teleWidth, false);
	  i += 2;
	}
	if (bluePurple) {
	  world->addTeleporter(y, -x, 0.0f, rotation + M_PI / 2,
			       0.5f * teleWidth, teleBreadth, 2.0f * teleWidth,
			       teleWidth, false);
	  world->addTeleporter(-y, x, 0.0f, rotation + M_PI * 3 / 2,
			       0.5f * teleWidth, teleBreadth, 2.0f * teleWidth,
			       teleWidth, false);
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
    for (int j = 0; j <= actCitySize / 2; j++) {
      for (int i = 0; i < actCitySize / 2; i++) {
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
      }
    }
    // add teleporters
    if (clOptions->useTeleporters) {
      float teleWidth = BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
      float teleBreadth = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
      float teleHeight = BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
      const float xoff = boxBase + 0.5f * avenueSize;
      const float yoff = boxBase + 0.5f * avenueSize;
      world->addTeleporter( xmin - xoff,  ymin - yoff, 0.0f, 1.25f * M_PI,
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter( xmin - xoff, -ymin + yoff, 0.0f, 0.75f * M_PI,
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter(-xmin + xoff,  ymin - yoff, 0.0f, 1.75f * M_PI,
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter(-xmin + xoff, -ymin + yoff, 0.0f, 0.25f * M_PI,
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter(-3.5f * teleBreadth, -3.5f * teleBreadth, 0.0f, 1.25f * M_PI,
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter(-3.5f * teleBreadth,  3.5f * teleBreadth, 0.0f, 0.75f * M_PI,
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter( 3.5f * teleBreadth, -3.5f * teleBreadth, 0.0f, 1.75f * M_PI,
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
      world->addTeleporter( 3.5f * teleBreadth,  3.5f * teleBreadth, 0.0f, 0.25f * M_PI,
			   0.5f * teleWidth, teleBreadth, 2.0f * teleHeight, teleWidth, false);
//
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
    } else {
      const float *pos  = bases[t].getBasePosition(0);
      const float  size = BZDB.eval(StateDatabase::BZDB_BASESIZE);
      world->addBase(pos[0], pos[1], pos[2], 0.0f,
		     size, size, 0.0f, t, false, false);
    }
  }

  world->finishWorld();

  return world;
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
	  Obstacle* obs;
      if (NOT_IN_BUILDING != world->inCylinderNoOctree(&obs, x, y, 0,
					       1.75f * teleBreadth, 1.0f))
	continue;

      world->addTeleporter(x, y, 0.0f, rotation,
	  0.5f*teleWidth, teleBreadth, 2.0f*teleHeight, teleWidth, false);
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

  world->finishWorld();

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
  if (clOptions->worldFile != "") {
    BZWReader* reader = new BZWReader(clOptions->worldFile);
    world = reader->defineWorldFromFile();
    delete reader;

    if (clOptions->gameStyle & TeamFlagGameStyle) {
      for (int i = RedTeam; i <= PurpleTeam; i++) {
	if ((clOptions->maxTeam[i] > 0) && bases.find(i) == bases.end()) {
	  std::cerr << "base was not defined for "
		    << Team::getName((TeamColor)i)
		    << ", capture the flag game style removed" << std::endl;
	  clOptions->gameStyle &= (~TeamFlagGameStyle);
	  break;
	}
      }
    }

  } else if (clOptions->gameStyle & TeamFlagGameStyle) {
    world = defineTeamWorld();
  } else {
    world = defineRandomWorld();
  }

  if (world == NULL)
    return false;

  maxWorldHeight = world->getMaxWorldHeight();

  // package up world
  if (clOptions->gameStyle & TeamFlagGameStyle) {
    world->packDatabase(&bases);
  } else {
    world->packDatabase(NULL);
  }

  // now get world packaged for network transmission
  worldDatabaseSize = 4 + WorldCodeHeaderSize +
      world->getDatabaseSize() + 4 + WorldCodeEndSize;

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
  buf = nboPackUInt(buf, 0); // time-of-day will go here
  buf = nboPackUInt(buf, world->getUncompressedSize());
  buf = nboPackUInt(buf, world->getDatabaseSize());
  buf = nboPackString(buf, world->getDatabase(), world->getDatabaseSize());
  buf = nboPackUShort(buf, WorldCodeEndSize);
  buf = nboPackUShort(buf, WorldCodeEnd);

  MD5 md5;
  md5.update((unsigned char *)worldDatabase, worldDatabaseSize);
  md5.finalize();
  if (clOptions->worldFile == "")
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
  FlagInfo::setNoFlagInAir();
  for (i = 0; i < numFlags; i++)
    resetFlag(*FlagInfo::get(i));

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
  if (!clOptions->printScore) {
    return;
  }
  if (clOptions->timeLimit > 0.0f) {
    std::cout << "#time " << clOptions->timeLimit - clOptions->timeElapsed << std::endl;
  }
  std::cout << "#teams";
  for (int i = int(RedTeam); i < NumTeams; i++) {
    std::cout << ' ' << team[i].team.won << '-' << team[i].team.lost << ' ' << Team::getName(TeamColor(i));
  }
  GameKeeper::Player::dumpScore();
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

   // send server version and playerid
  char buffer[9];
  memcpy(buffer, getServerVersion(), 8);
  // send 0xff if list is full
  buffer[8] = (char)0xff;

  BanInfo info(clientAddr.sin_addr);
  if (!clOptions->acl.validate(clientAddr.sin_addr,&info)) {

    std::string rejectionMessage;

    rejectionMessage = "REFUSED: ";
    if (info.reason.size())
      rejectionMessage += info.reason;
    else
      rejectionMessage += "General Ban";

    if (info.bannedBy.size()) {
      rejectionMessage += " by ";
      rejectionMessage += info.bannedBy;
    }

    if (info.fromMaster)
      rejectionMessage += " [you are on the master ban list]";

    // send back 0xff before closing
    rejectionMessage += (char)0xff;
    send(fd, rejectionMessage.c_str(), rejectionMessage.size(), 0);

    close(fd);
    return;
  }

  int keepalive = 1, n;
  n = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
		 (SSOType)&keepalive, sizeof(int));
  if (n < 0) {
    nerror("couldn't set keepalive");
  }

  PlayerId playerIndex;

  // find open slot in players list
  PlayerId minPlayerId = 0, maxPlayerId = (PlayerId)maxPlayers;
  if (Replay::enabled()) {
     minPlayerId = MaxPlayers;
     maxPlayerId = MaxPlayers + ReplayObservers;
  }
  for (playerIndex = minPlayerId; playerIndex < maxPlayerId; playerIndex++) {
    if (!GameKeeper::Player::getPlayerByIndex(playerIndex)) {
      break;
    }
  }

  if (playerIndex < maxPlayerId) {
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
  new GameKeeper::Player(playerIndex, clientAddr, fd);

  // if game was over and this is the first player then game is on
  if (gameOver) {
    int count = GameKeeper::Player::count();
    if (count == 1) {
      gameOver = false;
      gameStartTime = TimeKeeper::getCurrent();
      if (clOptions->timeLimit > 0.0f && !clOptions->timeManualStart) {
	clOptions->timeElapsed = 0.0f;
	countdownActive = true;
      }
    }
  }
}


static void respondToPing(Address addr)
{
  // reply with current game info
  pingReply.sourceAddr = addr;
  pingReply.rogueCount = (uint8_t)team[0].team.size;
  pingReply.redCount = (uint8_t)team[1].team.size;
  pingReply.greenCount = (uint8_t)team[2].team.size;
  pingReply.blueCount = (uint8_t)team[3].team.size;
  pingReply.purpleCount = (uint8_t)team[4].team.size;
  pingReply.observerCount = (uint8_t)team[5].team.size;
}


void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message)
{
  long int msglen = strlen(message) + 1; // include null terminator
  const char *msg = message;

  if (message[0] == '/' && message[1] == '/')
    msg = &message[1];
  /* filter all outbound messages */
  if (clOptions->filterChat) {
    char message2[MessageLen] = {0};
    strncpy(message2, message, MessageLen);
    if (clOptions->filterSimple) {
      clOptions->filter.filter(message2, true);
    } else {
      clOptions->filter.filter(message2, false);
    }
    msg = message2;
  }

  if (msglen > MessageLen) {
    msglen = MessageLen;
    DEBUG1("WARNING: Network message being sent is too long! "
           "(message is %d, cutoff at %d)\n", msglen, MessageLen);
  }

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUByte(buf, targetPlayer);
  buf = nboPackString(buf, msg, msglen);

  ((char*)bufStart)[MessageLen - 1] = '\0'; // always terminate

  int len = 2 + msglen;
  bool broadcast = false;

  if (targetPlayer <= LastRealPlayer) {
    directMessage(targetPlayer, MsgMessage, len, bufStart);
    if (playerIndex <= LastRealPlayer && targetPlayer != playerIndex)
      directMessage(playerIndex, MsgMessage, len, bufStart);
  }
  // FIXME this teamcolor <-> player id conversion is in several files now
  else if (targetPlayer >= 244 && targetPlayer <= 250) {
    TeamColor team = TeamColor(250 - targetPlayer);
    // send message to all team members only
    GameKeeper::Player *playerData;
    for (int i = 0; i < curMaxPlayers; i++)
      if ((playerData = GameKeeper::Player::getPlayerByIndex(i))
	  && playerData->player.isPlaying()
	  && playerData->player.isTeam(team))
        directMessage(i, MsgMessage, len, bufStart);
  } else if (targetPlayer == AdminPlayers){
    // admin messages
    std::vector<int> admins
      = GameKeeper::Player::allowed(PlayerAccessInfo::adminMessages);
    for (unsigned int i = 0; i < admins.size(); ++i)
      directMessage(admins[i], MsgMessage, len, bufStart);
  } else {
    broadcastMessage(MsgMessage, len, bufStart);
    broadcast = true;
  }

  if (Record::enabled() && !broadcast) { // don't record twice
    Record::addPacket (MsgMessage, len, bufStart, HiddenPacket);
  }
}


static void rejectPlayer(int playerIndex, uint16_t code, const char *reason)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, code);
  buf = nboPackString(buf, reason, strlen(reason) + 1);
  directMessage(playerIndex, MsgReject, sizeof (uint16_t) + MessageLen, bufStart);
  return;
}

// Team Size is wrong at some time
// as we are fixing, look also at unconnected player slot to rub it
static void fixTeamCount() {
  int playerIndex, teamNum;
  for (teamNum = RogueTeam; teamNum < RabbitTeam; teamNum++)
    team[teamNum].team.size = 0;
  for (playerIndex = 0; playerIndex < maxPlayers; playerIndex++) {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(playerIndex);
    if (p && p->player.isPlaying()) {
      teamNum = p->player.getTeam();
      if (teamNum == RabbitTeam)
	teamNum = RogueTeam;
      team[teamNum].team.size++;
    }
  }
}

struct TeamSize {
  TeamColor color;
  int       current;
  int       max;
  const bool operator<(const TeamSize x) const { return x.current < current; }
};

static TeamColor teamSelect(TeamColor t, std::vector<TeamSize> teams)
{
  if (teams.size() == 0)
    return RogueTeam;

  // see if the player's choice was a weak team
  for (int i = 0; i < (int) teams.size(); i++)
    if (teams[i].color == t)
      return t;

  return teams[rand() % teams.size()].color;
}

static TeamColor autoTeamSelect(TeamColor t)
{
  // Asking for Observer gives observer
  if (t == ObserverTeam)
    return ObserverTeam;
  // When replaying, joining tank can only be observer
  if (Replay::enabled())
    return ObserverTeam;

  // count current number of players
  int numplayers = 0, i = 0;
  for (i = 0; i < int(ObserverTeam); i++)
    numplayers += team[i].team.size;

  // if no player are available, join as Observer
  if (numplayers == maxRealPlayers)
    return ObserverTeam;

  // If tank ask for rogues, and rogues are allowed, give it
  if ((t == RogueTeam)
      && team[RogueTeam].team.size < clOptions->maxTeam[RogueTeam])
    return RogueTeam;

  // If no auto-team, server or client, go back with client choise
  if (!clOptions->autoTeam && t != AutomaticTeam)
    return t;

  // Fill a vector with teams status, not putting in not enabled teams
  std::vector<TeamSize> teams;

  for (i = (int)RedTeam; i < (int)ObserverTeam; i++) {
    TeamSize currTeam = {(TeamColor)i,
			 team[i].team.size,
			 clOptions->maxTeam[i]};
    if (currTeam.max > 0)
      teams.push_back(currTeam);
  }

  // Give rogue if that is the only team
  if (teams.size() == 0)
    return RogueTeam;

  // Sort it by current team number
  std::sort(teams.begin(), teams.end());

  // all teams are empty, select just one of them
  if (teams[0].current == 0) {
    return teamSelect(t, teams);
  }

  std::vector<TeamSize>::iterator it;

  // if you come with a 1-1-x-x try to add a player to these 1 to have team
  if ((teams[0].current == 1) && (teams[1].current == 1)) {
    // remove teams full
    it = teams.begin();
    while (it < teams.end())
      if (it->current == it->max) {
	teams.erase(it);
	it = teams.begin();
      } else {
	it++;
      }
    // if there is still a team with 1 player select all 1 tank team
    // to choose from
    if (teams.size() > 0)
      if (teams[0].current == 1) {
	it = teams.begin();
	while (it < teams.end())
	  if (it->current == 0) {
	    teams.erase(it);
	    it = teams.begin();
	  } else {
	    it++;
	  }
      }
    return teamSelect(t, teams);
  }

  int maxTeamSize = teams[0].current;

  // remove teams full
  it = teams.begin();
  while (it < teams.end())
    if (it->current == it->max) {
      teams.erase(it);
      it = teams.begin();
    } else {
      it++;
    }

  // Looking for unbalanced team
  bool unBalanced = false;
  for (it = teams.begin(); it < teams.end(); it++)
    if (it->current < maxTeamSize) {
      unBalanced = true;
      break;
    }

  if (unBalanced) {
    // remove bigger teams
    it = teams.begin();
    while (it < teams.end())
      if (it->current == maxTeamSize) {
	teams.erase(it);
	it = teams.begin();
      } else {
	it++;
      }
    // Search for the second biggest team to this and remove the lowers
    int secondTeamSize = teams[0].current;
    it = teams.begin();
    while (it < teams.end())
      if (it->current < secondTeamSize) {
	teams.erase(it);
	it = teams.begin();
      } else {
	it++;
      }
  }
  return teamSelect(t, teams);
}

static void addPlayer(int playerIndex)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  // make sure the callsign is not obscene/filtered
  if (clOptions->filterCallsigns) {
    DEBUG2("checking callsign: %s\n",playerData->player.getCallSign());
    bool filtered = false;
    char cs[CallSignLen];
    memcpy(cs, playerData->player.getCallSign(), sizeof(char) * CallSignLen);
    filtered = clOptions->filter.filter(cs, clOptions->filterSimple);
    if (filtered) {
      rejectPlayer(playerIndex, RejectBadCallsign,
                   "The callsign was rejected.  Try a different callsign.");
      return ;
    }
  }

  // make sure the email is not obscene/filtered
  if (clOptions->filterCallsigns) {
    DEBUG2("checking email: %s\n",playerData->player.getEMail());
    char em[EmailLen];
    memcpy(em, playerData->player.getEMail(), sizeof(char) * EmailLen);
    bool filtered = clOptions->filter.filter(em, clOptions->filterSimple);
    if (filtered) {
      rejectPlayer(playerIndex, RejectBadEmail,
                   "The e-mail was rejected.  Try a different e-mail.");
      return ;
    }
  }

  TeamColor t = autoTeamSelect(playerData->player.getTeam());
  playerData->player.setTeam(t);

  // count current number of players and players+observers
  int numplayers = 0;
  for (int i = 0; i < int(ObserverTeam); i++)
    numplayers += team[i].team.size;
  const int numplayersobs = numplayers + team[ObserverTeam].team.size;

  // no quick rejoining, make 'em wait
  // you can switch to observer immediately, or switch from observer
  // to regular player immediately, but only if last time time you
  // were a regular player isn't in the rejoin list
  if (playerData->player.getTeam() != ObserverTeam) {
    float waitTime = rejoinList.waitTime (playerIndex);
    if (waitTime > 0.0f) {
      char buffer[MessageLen];
      DEBUG2 ("Player %s [%d] rejoin wait of %.1f seconds\n",
              playerData->player.getCallSign(), playerIndex, waitTime);
      sprintf (buffer, "Can't rejoin for %.1f seconds.", waitTime);
      rejectPlayer(playerIndex, RejectRejoinWaitTime, buffer);
      return ;
    }
  }

  // reject player if asks for bogus team or rogue and rogues aren't allowed
  // or if the team is full or if the server is full
  if (!playerData->player.isHuman() && !playerData->player.isBot()) {
    rejectPlayer(playerIndex, RejectBadType,
                 "Communication error joining game [Rejected].");
    return;
  } else if (t == NoTeam) {
    rejectPlayer(playerIndex, RejectBadTeam,
                 "Communication error joining game [Rejected].");
    return;
  } else if (t == ObserverTeam && playerData->player.isBot()) {
    rejectPlayer(playerIndex, RejectServerFull,
                 "This game is full.  Try again later.");
    return;
  } else if (numplayersobs == maxPlayers) {
    // server is full
    rejectPlayer(playerIndex, RejectServerFull,
                 "This game is full.  Try again later.");
    return;
  } else if (team[int(t)].team.size >= clOptions->maxTeam[int(t)]) {
    rejectPlayer(playerIndex, RejectTeamFull,
                 "This team is full.  Try another team.");
    return ;
  }
  // accept player
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  int result = directMessage(*playerData, MsgAccept,
			     (char*)buf-(char*)bufStart, bufStart);
  if (result == -1)
    return;

  //send SetVars
  { // scoping is mandatory
     PackVars pv(bufStart, playerIndex);
     BZDB.iterate(PackVars::packIt, &pv);
  }

  // abort if we hung up on the client
  if (!GameKeeper::Player::getPlayerByIndex(playerIndex))
    return;

  // player is signing on (has already connected via addClient).
  playerData->signingOn((clOptions->gameStyle & TeamFlagGameStyle) != 0);

  // update team state and if first player on team, reset it's score
  int teamIndex = int(playerData->player.getTeam());
  team[teamIndex].team.size++;
  if (team[teamIndex].team.size == 1
      && Team::isColorTeam((TeamColor)teamIndex)) {
    team[teamIndex].team.won = 0;
    team[teamIndex].team.lost = 0;
  }

  // send new player updates on each player, all existing flags, and all teams.
  // don't send robots any game info.  watch out for connection being closed
  // because of an error.
  if (!playerData->player.isBot()) {
    sendTeamUpdate(playerIndex);
    sendFlagUpdate(playerIndex);
    GameKeeper::Player *otherData;
    for (int i = 0; i < curMaxPlayers && NetHandler::exists(playerIndex); i++)
      if (i != playerIndex) {
	otherData = GameKeeper::Player::getPlayerByIndex(i);
	if (otherData)
	  sendPlayerUpdate(otherData, playerIndex);
      }
  }

  // if new player connection was closed (because of an error) then stop here
  if (!NetHandler::exists(playerIndex))
    return;

  // send MsgAddPlayer to everybody -- this concludes MsgEnter response
  // to joining player
  sendPlayerUpdate(playerData, playerIndex);

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

  // again check if player was disconnected
  if (!NetHandler::exists(playerIndex))
    return;

  // send time update to new player if we're counting down
  if (countdownActive && clOptions->timeLimit > 0.0f
      && !playerData->player.isBot()) {
    float timeLeft = clOptions->timeLimit - (TimeKeeper::getCurrent() - gameStartTime);
    if (timeLeft < 0.0f) {
      // oops
      timeLeft = 0.0f;
    }

    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUShort(bufStart, (uint16_t)(int)timeLeft);
    result = directMessage(*playerData, MsgTimeUpdate,
			   (char*)buf-(char*)bufStart, bufStart);
    if (result == -1)
      return;
  }

  // if first player on team add team's flag
  if (team[teamIndex].team.size == 1
      && Team::isColorTeam((TeamColor)teamIndex)) {
    if (clOptions->gameStyle & int(TeamFlagGameStyle)) {
      int flagid = FlagInfo::lookupFirstTeamFlag(teamIndex);
      if (flagid >= 0 && !FlagInfo::get(flagid)->exist()) {
	// reset those flags
	for (int n = 0; n < clOptions->numTeamFlags[teamIndex]; n++)
	  resetFlag(*FlagInfo::get(n + flagid));
      }
    }
  }

  fixTeamCount();

  // tell the list server the new number of players
  listServerLink->queueMessage(ListServerLink::ADD);

#ifdef PRINTSCORE
  dumpScore();
#endif
  char message[MessageLen] = {0};

#ifdef SERVERLOGINMSG
  sprintf(message,"BZFlag server %s, http://BZFlag.org/", getAppVersion());
  sendMessage(ServerPlayer, playerIndex, message);

  if (clOptions->servermsg != "") {

    // split the servermsg into several lines if it contains '\n'
    const char* i = clOptions->servermsg.c_str();
    const char* j;
    while ((j = strstr(i, "\\n")) != NULL) {
      unsigned int l = j - i < MessageLen - 1 ? j - i : MessageLen - 1;
      strncpy(message, i, l);
      message[l] = '\0';
      sendMessage(ServerPlayer, playerIndex, message);
      i = j + 2;
    }
    strncpy(message, i, MessageLen - 1);
    message[strlen(i) < MessageLen - 1 ? strlen(i) : MessageLen - 1] = '\0';
    sendMessage(ServerPlayer, playerIndex, message);
  }

  // look for a startup message -- from a file
  static const std::vector<std::string>* lines = clOptions->textChunker.getTextChunk("srvmsg");
  if (lines != NULL){
    for (int i = 0; i < (int)lines->size(); i ++){
      sendMessage(ServerPlayer, playerIndex, (*lines)[i].c_str());
    }
  }

  if (playerData->player.isObserver())
    sendMessage(ServerPlayer, playerIndex, "You are in observer mode.");
#endif


  if (NetHandler::exists(playerIndex)
      && playerData->accessInfo.isRegistered()) {
    // nick is in the DB send him a message to identify.
    if (playerData->accessInfo.isIdentifyRequired())
      sendMessage(ServerPlayer, playerIndex, "This callsign is registered.  You must identify yourself before playing.");
    else
      sendMessage(ServerPlayer, playerIndex, "This callsign is registered.");
    sendMessage(ServerPlayer, playerIndex, "Identify with /identify <your password>");
  }

  dropAssignedFlag(playerIndex);
}


void resetFlag(FlagInfo &flag)
{
  // NOTE -- must not be called until world is defined
  assert(world != NULL);

  float flagHeight = BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT);
  float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);

  // reposition flag
  float flagPos[3] = {0.0f, 0.0f, 0.0f};

  int teamIndex = flag.teamIndex();
  if ((teamIndex >= ::RedTeam) &&  (teamIndex <= ::PurpleTeam)
      && (bases.find(teamIndex) != bases.end())) {
    TeamBases &teamBases = bases[teamIndex];
    const TeamBase &base = teamBases.getRandomBase(flag.getIndex());
    flagPos[0] = base.position[0];
    flagPos[1] = base.position[1];
    flagPos[2] = base.position[2] + base.size[2];
  } else {
    // random position (not in a building)
    float r = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
    if (flag.flag.type == Flags::Obesity)
      r *= 2.0f * BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    const Obstacle *obj;
    float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    if (!world->getZonePoint(std::string(flag.flag.type->flagAbbv),
			     flagPos)) {
      flagPos[0] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
      flagPos[1] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
      flagPos[2] = 0.0f;
    }

    const float waterLevel = world->getWaterLevel();
    if (waterLevel >= 0.0f) {
      // precautionary measure
      clOptions->flagsOnBuildings = true;
    }

    int topmosttype = world->cylinderInBuilding(&obj, flagPos, r, flagHeight);

    while ((topmosttype != NOT_IN_BUILDING) || (flagPos[2] <= waterLevel)) {
      if (world->getZonePoint(std::string(flag.flag.type->flagAbbv),
			      flagPos)
	  && (flagPos[2] > waterLevel)) {
        // if you got a related flag zone specified, always use
        // it. there may be obstacles in the zone that cause the
        // NOT_IN_BUILDING test to fail a couple of times, but
        // hopefully the map maker is using zones wisely.
      } else if ((clOptions->flagsOnBuildings
		  && ((topmosttype == IN_BOX_NOTDRIVETHROUGH)
		      || (topmosttype == IN_BASE)))
		 && (obj->getPosition()[2]
		     < (flagPos[2] + flagHeight - Epsilon))
		 && ((obj->getPosition()[2] + obj->getSize()[2] - Epsilon)
		     > flagPos[2])
		 && (world->inRect(obj->getPosition(), obj->getRotation(),
				   obj->getSize(),
				   flagPos[0], flagPos[1], 0.0f))) {
	flagPos[2] = obj->getPosition()[2] + obj->getSize()[2];
      } else {
	flagPos[0] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	flagPos[1] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	flagPos[2] = 0.0f;
      }
      topmosttype = world->cylinderInBuilding(&obj, flagPos, r, flagHeight);
    }
  }

  bool teamIsEmpty = true;
  if (teamIndex != ::NoTeam)
    teamIsEmpty = (team[teamIndex].team.size == 0);

  // reset a flag's info
  flag.resetFlag(flagPos, teamIsEmpty);

  sendFlagUpdate(flag);
}


void sendDrop(FlagInfo &flag)
{
  // see if someone had grabbed flag.  tell 'em to drop it.
  const int playerIndex = flag.player;

  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  flag.player      = -1;
  playerData->player.resetFlag();

  void *bufStart = getDirectMessageBuffer();
  void *buf      = nboPackUByte(bufStart, playerIndex);
  buf            = flag.pack(buf);
  broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
}

void zapFlag(FlagInfo &flag)
{
  // called when a flag must just disappear -- doesn't fly
  // into air, just *poof* vanishes.

  sendDrop(flag);

  // if flag was flying then it flies no more
  flag.landing(TimeKeeper::getSunExplodeTime());

  flag.flag.status = FlagNoExist;

  // reset flag status
  resetFlag(flag);
}

// try to get over a bug where extraneous flag are attached to a tank
// not really found why, but this should fix
// Should be called when we sure that tank does not hold any
static void dropAssignedFlag(int playerIndex) {
  for (int flagIndex = 0; flagIndex < numFlags; flagIndex++) {
    FlagInfo &flag = *FlagInfo::get(flagIndex);
    if (flag.flag.status == FlagOnTank && flag.flag.owner == playerIndex)
      resetFlag(flag);
  }
} // dropAssignedFlag

static void anointNewRabbit(int killerId = NoPlayer)
{
  GameKeeper::Player *killerData
    = GameKeeper::Player::getPlayerByIndex(killerId);
  GameKeeper::Player *oldRabbitData
    = GameKeeper::Player::getPlayerByIndex(rabbitIndex);
  int oldRabbit = rabbitIndex;
  rabbitIndex = NoPlayer;

  if (clOptions->rabbitSelection == KillerRabbitSelection)
    // check to see if the rabbit was just killed by someone; if so, make them the rabbit if they're still around.
    if (killerId != oldRabbit && killerData && killerData->player.isPlaying()
	&& killerData->player.canBeRabbit())
      rabbitIndex = killerId;

  if (rabbitIndex == NoPlayer)
    rabbitIndex = GameKeeper::Player::anointRabbit(oldRabbit);

  if (rabbitIndex != oldRabbit) {
    DEBUG3("rabbitIndex is set to %d\n", rabbitIndex);
    if (oldRabbitData) {
      oldRabbitData->player.wasARabbit();
    }
    if (rabbitIndex != NoPlayer) {
      GameKeeper::Player *rabbitData
	= GameKeeper::Player::getPlayerByIndex(rabbitIndex);
      rabbitData->player.setTeam(RabbitTeam);
      void *buf, *bufStart = getDirectMessageBuffer();
      buf = nboPackUByte(bufStart, rabbitIndex);
      broadcastMessage(MsgNewRabbit, (char*)buf-(char*)bufStart, bufStart);
    }
  } else {
    DEBUG3("no other than old rabbit to choose from, rabbitIndex is %d\n",
	   rabbitIndex);
  }
}


static void pausePlayer(int playerIndex, bool paused)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  playerData->player.setPaused(paused);
  if (clOptions->gameStyle & int(RabbitChaseGameStyle)) {
    if (paused && (rabbitIndex == playerIndex)) {
      anointNewRabbit();
    } else if (!paused && (rabbitIndex == NoPlayer)) {
      anointNewRabbit();
    }
  }

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUByte(buf, paused);
  broadcastMessage(MsgPause, (char*)buf-(char*)bufStart, bufStart);
}

static void zapFlagByPlayer(int playerIndex)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  int flagid = playerData->player.getFlag();
  if (flagid < 0)
    return;

  FlagInfo &flag = *FlagInfo::get(flagid);
  // do not simply zap team flag
  Flag &carriedflag = flag.flag;
  if (carriedflag.type->flagTeam != ::NoTeam) {
    dropFlag(*playerData, playerData->lastState->pos);
  } else {
    zapFlag(flag);
  }
}

void removePlayer(int playerIndex, const char *reason, bool notify)
{
  // player is signing off or sent a bad packet.  since the
  // bad packet can come before MsgEnter, we must be careful
  // not to undo operations that haven't been done.
  // first shutdown connection

  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  // send a super kill to be polite
  if (notify) {
    // send message to one player
    // do not use directMessage as he can remove player
    void *buf  = sMsgBuf;
    buf        = nboPackUShort(buf, 0);
    buf        = nboPackUShort(buf, MsgSuperKill);
    playerData->netHandler->pwrite(sMsgBuf, 4);
  }

  // make them wait from the time they left, but only if they are
  // not already waiting, and they are not currently an observer.
  if ((playerData->player.getTeam() != ObserverTeam) &&
      (rejoinList.waitTime (playerIndex) <= 0.0f)) {
    rejoinList.add (playerIndex);
  }

  // if there is an active poll, cancel any vote this player may have made
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");
  if ((arbiter != NULL) && (arbiter->knowsPoll())) {
    arbiter->retractVote(std::string(playerData->player.getCallSign()));
  }

  // status message
  DEBUG1("Player %s [%d] removed at %s: %s\n",
	 playerData->player.getCallSign(), 
	 playerIndex, 
	 TimeKeeper::timestamp(), 
	 reason);
  bool wasPlaying = playerData->player.isPlaying();
  playerData->netHandler->closing();

  zapFlagByPlayer(playerIndex);

  // player is outta here.  if player never joined a team then
  // don't count as a player.

  if (wasPlaying) {
    // tell everyone player has left
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, playerIndex);
    broadcastMessage(MsgRemovePlayer, (char*)buf-(char*)bufStart, bufStart);

    // decrease team size
    int teamNum = int(playerData->player.getTeam());
    --team[teamNum].team.size;

    // if last active player on team then remove team's flag if no one
    // is carrying it
    if (Team::isColorTeam((TeamColor)teamNum)
	&& team[teamNum].team.size == 0 &&
	(clOptions->gameStyle & int(TeamFlagGameStyle))) {
      int flagid = FlagInfo::lookupFirstTeamFlag(teamNum);
      if (flagid >= 0) {
	GameKeeper::Player *otherData;
	for (int n = 0; n < clOptions->numTeamFlags[teamNum]; n++) {
	  FlagInfo &flag = *FlagInfo::get(flagid+n);
	  otherData
	    = GameKeeper::Player::getPlayerByIndex(flag.player);
	  if (!otherData || otherData->player.isTeam((TeamColor)teamNum))
	    zapFlag(flag);
	}
      }
    }

    // send team update
    sendTeamUpdate(-1, teamNum);

    // delete this player before 'fixing' the count
    delete playerData;
    playerData = NULL;

    fixTeamCount();

    // tell the list server the new number of players
    listServerLink->queueMessage(ListServerLink::ADD);
  }

  delete playerData;

  if (clOptions->gameStyle & int(RabbitChaseGameStyle))
    if (playerIndex == rabbitIndex)
      anointNewRabbit();

  // recompute curMaxPlayers
  if (playerIndex + 1 == curMaxPlayers)
    while (true) {
      curMaxPlayers--;
      if (curMaxPlayers <= 0
	  || GameKeeper::Player::getPlayerByIndex(curMaxPlayers - 1))
	break;
    }

  if (wasPlaying) {
    // if everybody left then reset world
    if (GameKeeper::Player::count() == 0) {
      if (clOptions->worldFile == "") {
	bases.clear();
      }

      if (clOptions->oneGameOnly) {
	done = true;
	exitCode = 0;
      }
      else if ((clOptions->worldFile == "") &&
               (!Replay::enabled()) && (!defineWorld())) {
	done = true;
	exitCode = 1;
      } else {
	// republicize ourself.  this dereferences the URL chain
	// again so we'll notice any pointer change when any game
	// is over (i.e. all players have quit).
	publicize();
      }
    }
  }
}

// are the two teams foes with the current game style?
bool areFoes(TeamColor team1, TeamColor team2)
{
  return team1!=team2 ||
         (team1==RogueTeam && !(clOptions->gameStyle & int(RabbitChaseGameStyle)));
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
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  // count the number of active players
  int numPlayers = GameKeeper::Player::count();

  // first send number of teams and players being sent
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, NumTeams);
  buf = nboPackUShort(buf, numPlayers);
  int result = directMessage(*playerData, MsgQueryPlayers,
			     (char*)buf-(char*)bufStart, bufStart);
  if (result == -1)
    return;

  // now send the teams and players
  sendTeamUpdate(playerIndex);
  GameKeeper::Player *otherData;
  for (int i = 0; i < curMaxPlayers && NetHandler::exists(playerIndex); i++) {
    otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData)
      sendPlayerUpdate(otherData, playerIndex);
  }
}

static void playerAlive(int playerIndex)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  // ignore multiple MsgAlive; also observer should not send MsgAlive;
  // diagnostic?
  if (!playerData->player.isDead() || playerData->player.isObserver())
    return;

  // make sure the user identifies themselves if required.
  if (!playerData->accessInfo.isAllowedToEnter()) {
    sendMessage(ServerPlayer, playerIndex, "This callsign is registered.  You must identify yourself before playing or use a different callsign.");
    removePlayer(playerIndex, "unidentified");
    return;
  }

  if (playerData->player.isBot()
      && BZDB.isTrue(StateDatabase::BZDB_DISABLEBOTS)) {
    sendMessage(ServerPlayer, playerIndex, "I'm sorry, we do not allow bots on this server.");
    removePlayer(playerIndex, "ComputerPlayer");
    return;
  }

  // player is coming alive.
  playerData->player.setAlive();
  dropAssignedFlag(playerIndex);

  // send MsgAlive
  SpawnPosition* spawnPosition = new SpawnPosition(playerIndex,
      (!clOptions->respawnOnBuildings) || (playerData->player.isBot()),
       clOptions->gameStyle & TeamFlagGameStyle);
  // update last position immediately
  lastState[playerIndex].pos[0] = spawnPosition->getX();
  lastState[playerIndex].pos[1] = spawnPosition->getY();
  lastState[playerIndex].pos[2] = spawnPosition->getZ();
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackVector(buf, lastState[playerIndex].pos);
  buf = nboPackFloat(buf, spawnPosition->getAzimuth());
  broadcastMessage(MsgAlive, (char*)buf - (char*)bufStart, bufStart);
  delete spawnPosition;

  if (clOptions->gameStyle & int(RabbitChaseGameStyle)) {
    playerData->player.wasNotARabbit();
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
  GameKeeper::Player *killerData = NULL;
  GameKeeper::Player *victimData
    = GameKeeper::Player::getPlayerByIndex(victimIndex);

  if (!victimData || !victimData->player.isPlaying())
    return;

  if (killerIndex != InvalidPlayer && killerIndex != ServerPlayer)
    killerData = GameKeeper::Player::getPlayerByIndex(killerIndex);

  // aliases for convenience
  // Warning: killer should not be used when killerIndex == InvalidPlayer or ServerPlayer
  PlayerInfo *killer = realPlayer(killerIndex) ? &killerData->player : 0,
             *victim = &victimData->player;

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

  // update tk-score
  if ((victimIndex != killerIndex) && teamkill) {
    killerData->score.tK();
    if (killerData->score.isTK()) {
      char message[MessageLen];
      strcpy(message, "You have been automatically kicked for team killing" );
      sendMessage(ServerPlayer, killerIndex, message);
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
  zapFlagByPlayer(victimIndex);

  victimData = GameKeeper::Player::getPlayerByIndex(victimIndex);
  // change the player score
  bufStart = getDirectMessageBuffer();
  victimData->score.killedBy();
  if (killer) {
    if (victimIndex != killerIndex) {
      if (teamkill) {
        if (clOptions->teamKillerDies)
          playerKilled(killerIndex, killerIndex, reason, -1);
        else
          killerData->score.killedBy();
      } else {
        killerData->score.kill();
      }
    }

    buf = nboPackUByte(bufStart, 2);
    buf = nboPackUByte(buf, killerIndex);
    buf = killerData->score.pack(buf);
  }
  else {
    buf = nboPackUByte(bufStart, 1);
  }

  buf = nboPackUByte(buf, victimIndex);
  buf = victimData->score.pack(buf);
  broadcastMessage(MsgScore, (char*)buf-(char*)bufStart, bufStart);

  // see if the player reached the score limit
  if (clOptions->maxPlayerScore != 0
      && killerIndex != InvalidPlayer
      && killerIndex != ServerPlayer
      && killerData->score.reached()) {
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

static void grabFlag(int playerIndex, FlagInfo &flag)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);

  // player wants to take possession of flag
  if (playerData->player.isObserver() ||
      !playerData->player.isAlive() ||
      playerData->player.haveFlag() ||
      flag.flag.status != FlagOnGround)
    return;

  //last Pos might be lagged by TankSpeed so include in calculation
  const float tankRadius = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
  const float tankSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);
  const float radius2 = (tankSpeed + tankRadius + BZDBCache::flagRadius) * (tankSpeed + tankRadius + BZDBCache::flagRadius);
  const float* tpos = lastState[playerIndex].pos;
  const float* fpos = flag.flag.position;
  const float delta = (tpos[0] - fpos[0]) * (tpos[0] - fpos[0]) +
		      (tpos[1] - fpos[1]) * (tpos[1] - fpos[1]);

  if ((fabs(tpos[2] - fpos[2]) < 0.1f) && (delta > radius2)) {
       DEBUG2("Player %s [%d] %f %f %f tried to grab distant flag %f %f %f: distance=%f\n",
    playerData->player.getCallSign(), playerIndex,
    tpos[0], tpos[1], tpos[2], fpos[0], fpos[1], fpos[2], sqrt(delta));
    return;
  }

  // okay, player can have it
  flag.grab(playerIndex);
  playerData->player.setFlag(flag.getIndex());

  // send MsgGrabFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = flag.pack(buf);
  broadcastMessage(MsgGrabFlag, (char*)buf-(char*)bufStart, bufStart);

  playerData->flagHistory.add(flag.flag.type);
}

static void dropFlag(GameKeeper::Player &playerData, float pos[3])
{
  const float flagHeight = BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT);
  const float size = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  if (pos[0] < -size || pos[0] > size)
    pos[0] = 0.0;
  if (pos[1] < -size || pos[1] > size)
    pos[1] = 0.0;
  if (pos[2] > maxWorldHeight)
    pos[2] = maxWorldHeight;

  assert(world != NULL);
  const Obstacle* container;
  int topmosttype = NOT_IN_BUILDING;
  const Obstacle* topmost = 0;
  // player wants to drop flag.  we trust that the client won't tell
  // us to drop a sticky flag until the requirements are satisfied.
  const int flagIndex = playerData.player.getFlag();
  if (flagIndex < 0)
    return;
  FlagInfo &drpFlag = *FlagInfo::get(flagIndex);
  if (drpFlag.flag.status != FlagOnTank)
    return;
  int flagTeam = drpFlag.flag.type->flagTeam;
  bool isTeamFlag = (flagTeam != ::NoTeam);

  // limited flags that have been fired should be disposed of
  bool limited = clOptions->flagLimit[drpFlag.flag.type] != -1;
  if (limited && drpFlag.numShots > 0) drpFlag.grabs = 0;

  topmosttype = world->cylinderInBuilding(&container, pos, 0, flagHeight);

  // the tank is inside a building - find the roof
  if (topmosttype != NOT_IN_BUILDING) {
    topmost = container;
    int tmp;
    for (float i = container->getPosition()[2] + container->getSize()[2];
	 (tmp = world->cylinderInBuilding(&container,
					  pos[0], pos[1], i, 0, flagHeight)) !=
	   NOT_IN_BUILDING; i += 0.1f) {
      topmosttype = tmp;
      topmost = container;
    }
  } else {
    // the tank is _not_ inside a building - find the floor
    for (float i = pos[2]; ; i -= 0.1f) {
      topmosttype = world->cylinderInBuilding
	(&topmost, pos[0], pos[1], i, 0, flagHeight);
      if (topmosttype != NOT_IN_BUILDING)
	break;
      if (i < 0)
	break;
    }
  }
  // if topmosttype is NOT_IN_BUILDING position has reached ground

  const float waterLevel = world->getWaterLevel();
  float obstacleTop = 0.0f;
  if (topmosttype != NOT_IN_BUILDING) {
    obstacleTop = topmost->getPosition()[2] + topmost->getSize()[2];
  }

  // note: sticky/bad flags should always have grabs=1

  bool vanish;
  if (isTeamFlag)
    vanish = false;
  else if (--drpFlag.grabs == 0)
    vanish = true;
  else if ((topmosttype == NOT_IN_BUILDING) && (waterLevel <= 0.0f))
    vanish = false;
  else if (clOptions->flagsOnBuildings && (obstacleTop > waterLevel)
	   && (topmosttype == IN_BOX_NOTDRIVETHROUGH
	       || topmosttype == IN_BASE))
    vanish = false;
  else
    vanish = true;

  float landingPos[3] = {pos[0], pos[1], obstacleTop};

  // With Team Flag, we should absolutely go for finding a landing
  // position, while, for other flags, we could stay with default, or
  // just let them vanish
  if (isTeamFlag) {
    if (topmosttype == IN_BASE) {
      // figure out landing spot -- if flag in a Bad Place
      // when dropped, move to safety position or make it going
      BaseBuilding *landingBase = (BaseBuilding *)topmost;
      TeamColor     teamBase    = (TeamColor)landingBase->getTeam();
      if (teamBase != flagTeam) {
	std::string teamName = Team::getName((TeamColor) flagTeam);
	if (!world->getSafetyPoint(teamName, pos, landingPos)) {
	  bases[teamBase].getSafetyZone(landingPos[0],
					landingPos[1],
					landingPos[2]);
	  topmosttype = world->cylinderInBuilding
	    (&container, landingPos, BZDB.eval(StateDatabase::BZDB_TANKRADIUS),
	     flagHeight);
	  if ((topmosttype != NOT_IN_BUILDING) || (landingPos[2] <= waterLevel)) {
	    TeamBases &teamBases = bases[flagTeam];
	    const TeamBase &base = teamBases.getRandomBase(flagIndex);
	    landingPos[0] = base.position[0];
	    landingPos[1] = base.position[1];
	    landingPos[2] = base.position[2] + base.size[2];
	  }
	}
      }
    } else if ((topmosttype == NOT_IN_BUILDING) && (waterLevel <= 0.0f)) {
      // use default landing position
    } else if (clOptions->flagsOnBuildings && (obstacleTop > waterLevel)
	       && (topmosttype == IN_BOX_NOTDRIVETHROUGH)) {
      // use default landing position
    } else {
      // people were cheating by dropping their flag above the nearest
      // convenient building which makes it fly all the way back to
      // your own base.  make it fly to the center of the board.
      std::string teamName = Team::getName ((TeamColor) flagTeam);
      if (!world->getSafetyPoint(teamName, pos, landingPos)) {
	const float pos[3] = {0.0f, 0.0f, 0.0f};
	topmosttype = world->cylinderInBuilding
	  (&container, pos, BZDB.eval(StateDatabase::BZDB_TANKRADIUS),
	   flagHeight);
	if ((topmosttype == NOT_IN_BUILDING) && (waterLevel <= 0.0f)) {
	  landingPos[0] = 0.0f;
	  landingPos[1] = 0.0f;
	  landingPos[2] = 0.0f;
	} else {// oh well, whatcha gonna do?
	  TeamBases &teamBases = bases[flagTeam];
	  const TeamBase &base = teamBases.getRandomBase(flagIndex);
	  landingPos[0] = base.position[0];
	  landingPos[1] = base.position[1];
	  landingPos[2] = base.position[2] + base.size[2];
	}
      }
    }

    // if it is a team flag, check if there are any players left in
    // that team - if not, start the flag timeout
    if (team[drpFlag.flag.type->flagTeam].team.size == 0) {
      team[flagIndex + 1].flagTimeout = TimeKeeper::getCurrent();
      team[flagIndex + 1].flagTimeout += (float)clOptions->teamFlagTimeout;
    }
  }

  drpFlag.dropFlag(pos, landingPos, vanish);

  // removed any delayed packets (in case it was a "Lag Flag")
  playerData.delayq.dequeuePackets();

  // player no longer has flag -- send MsgDropFlag
  sendDrop(drpFlag);

  // notify of new flag state
  sendFlagUpdate(drpFlag);

}

static void captureFlag(int playerIndex, TeamColor teamCaptured)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  // Sanity check
  if (teamCaptured < RedTeam || teamCaptured > PurpleTeam)
    return;

  // player captured a flag.  can either be enemy flag in player's own
  // team base, or player's own flag in enemy base.
  int flagIndex = playerData->player.getFlag();
  if (flagIndex < 0)
    return;
  FlagInfo &flag = *FlagInfo::get(flagIndex);

  int teamIndex = flag.teamIndex();
  if (teamIndex == ::NoTeam)
    return;

  { //cheat checking
    TeamColor base = whoseBase(lastState[playerIndex].pos[0],
			       lastState[playerIndex].pos[1],
			       lastState[playerIndex].pos[2]);
    if ((teamIndex == playerData->player.getTeam() &&
	 base == playerData->player.getTeam()))	{
      DEBUG1("\"%s\" (%d) sent MsgCaptureFlag for taking its own flag onto its own base",
	     playerData->player.getCallSign(), playerIndex);
      return; //sanity check
    }
    if ((teamIndex != playerData->player.getTeam() &&
	 base != playerData->player.getTeam())) {
      DEBUG1("\"%s\" (%d) tried to capture %s flag without reaching its own base",
	     playerData->player.getCallSign(), playerIndex,
	     Team::getName((TeamColor)teamIndex));
      //char message[MessageLen];
      //strcpy(message, "Autokick: Tried to capture opponent flag without landing on your base");
      //sendMessage(ServerPlayer, playerIndex, message);
      //removePlayer(playerIndex, "capturecheat"); //FIXME: kicks honest players at times
      return;
    }
  }

  // player no longer has flag and put flag back at it's base
  playerData->player.resetFlag();
  resetFlag(flag);

  // send MsgCaptureFlag
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, uint16_t(flagIndex));
  buf = nboPackUShort(buf, uint16_t(teamCaptured));
  broadcastMessage(MsgCaptureFlag, (char*)buf-(char*)bufStart, bufStart);

  GameKeeper::Player *otherData;
  // everyone on losing team is dead
  for (int i = 0; i < curMaxPlayers; i++)
    if ((otherData = GameKeeper::Player::getPlayerByIndex(i))
	&& teamIndex == int(otherData->player.getTeam()) &&
	otherData->player.isAlive()) {
      otherData->player.setDead();
      otherData->player.setRestartOnBase(true);
    }

  // update score (rogues can't capture flags)
  int winningTeam = (int)NoTeam;
  if (teamIndex != int(playerData->player.getTeam())) {
    // player captured enemy flag
    winningTeam = int(playerData->player.getTeam());
    team[winningTeam].team.won++;
  }
  team[teamIndex].team.lost++;
  sendTeamUpdate(-1, winningTeam, teamIndex);
#ifdef PRINTSCORE
  dumpScore();
#endif
  if (winningTeam != (int)NoTeam)
    checkTeamScore(playerIndex, winningTeam);
}

static void shotFired(int playerIndex, void *buf, int len)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  bool repack = false;
  const PlayerInfo &shooter = playerData->player;
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

  float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  FlagInfo &fInfo = *FlagInfo::get(shooter.getFlag());
  // verify player flag
  if ((firingInfo.flagType != Flags::Null)
      && (firingInfo.flagType != fInfo.flag.type)) {
    DEBUG2("Player %s [%d] shot flag mismatch %s %s\n", shooter.getCallSign(),
	   playerIndex, firingInfo.flagType->flagAbbv,
	   fInfo.flag.type->flagAbbv);
    firingInfo.flagType = Flags::Null;
    firingInfo.shot.vel[0] = shotSpeed * cos(lastState[playerIndex].azimuth);
    firingInfo.shot.vel[1] = shotSpeed * sin(lastState[playerIndex].azimuth);
    firingInfo.shot.vel[2] = 0.0f;
    repack = true;
  }

  // verify shot number
  if ((shot.id & 0xff) > clOptions->maxShots - 1) {
    DEBUG2("Player %s [%d] shot id out of range %d %d\n",
	   shooter.getCallSign(),
	   playerIndex,	shot.id & 0xff, clOptions->maxShots);
    return;
  }

  const float maxTankSpeed  = BZDB.eval(StateDatabase::BZDB_TANKSPEED);
  const float tankSpeedMult = BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
  float tankSpeed           = maxTankSpeed;
  float lifetime = BZDB.eval(StateDatabase::BZDB_RELOADTIME);
  if (firingInfo.flagType == Flags::ShockWave) {
    shotSpeed = 0.0f;
    tankSpeed = 0.0f;
  } else if (firingInfo.flagType == Flags::Velocity) {
    tankSpeed *= tankSpeedMult;
  } else if (firingInfo.flagType == Flags::Thief) {
    tankSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
  } else if ((firingInfo.flagType == Flags::Burrow) && (firingInfo.shot.pos[2] < BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT))) {
    tankSpeed *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
  } else if (firingInfo.flagType == Flags::Agility) {
    tankSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
  } else {
    //If shot is different height than player, can't be sure they didn't drop V in air
    if (lastState[playerIndex].pos[2] != (shot.pos[2]-BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT))) {
      tankSpeed *= tankSpeedMult;
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
  if (delta > (maxTankSpeed * tankSpeedMult + front)
      * (maxTankSpeed * tankSpeedMult + front)) {
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

    fInfo.numShots++; // increase the # shots fired

    int limit = clOptions->flagLimit[fInfo.flag.type];
    if (limit != -1){ // if there is a limit for players flag
      int shotsLeft = limit -  fInfo.numShots;
      if (shotsLeft > 0) { //still have some shots left
	// give message each shot below 5, each 5th shot & at start
	if (shotsLeft % 5 == 0 || shotsLeft <= 3 || shotsLeft == limit-1){
	  sprintf(message,"%d shots left",shotsLeft);
	  sendMessage(ServerPlayer, playerIndex, message);
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
	  dropFlag(*playerData, lastPos);
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
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(t);
  if (!playerData)
    return;

  if (strncmp(message + 1, "me ", 3) == 0) {
    handleMeCmd(playerData, message);

  } else if (strncmp(message + 1, "msg", 3) == 0) {
    handleMsgCmd(playerData, message);

  } else if (strncmp(message + 1, "part ", 5) == 0) {
    handlePartCmd(playerData, message);

  } else if (strncmp(message + 1, "password", 8) == 0) {
    handlePasswordCmd(playerData, message);

  } else if (strncmp(message + 1, "set ", 4) == 0) {
    handleSetCmd(playerData, message);

  } else if (strncmp(message + 1, "reset", 5) == 0) {
    handleResetCmd(playerData, message);

  } else if (strncmp(message + 1, "shutdownserver", 8) == 0) {
    handleShutdownserverCmd(playerData, message);

  } else if (strncmp(message + 1, "superkill", 8) == 0) {
    handleSuperkillCmd(playerData, message);

  } else if (strncmp(message + 1, "gameover", 8) == 0) {
    handleGameoverCmd(playerData, message);

  } else if (strncmp(message + 1, "countdown", 9) == 0) {
    handleCountdownCmd(playerData, message);

  } else if (strncmp(message + 1, "flag ", 5) == 0) {
    handleFlagCmd(playerData,message);

  } else if (strncmp(message + 1, "kick", 4) == 0) {
    handleKickCmd(playerData, message);

  } else if (strncmp(message+1, "banlist", 7) == 0) {
    handleBanlistCmd(playerData, message);

  } else if (strncmp(message+1, "hostbanlist", 11) == 0) {
    handleHostBanlistCmd(playerData, message);

  } else if (strncmp(message+1, "ban", 3) == 0) {
    handleBanCmd(playerData, message);

  } else if (strncmp(message+1, "hostban", 7) == 0) {
    handleHostBanCmd(playerData, message);

  } else if (strncmp(message+1, "unban", 5) == 0) {
    handleUnbanCmd(playerData, message);

  } else if (strncmp(message+1, "hostunban", 9) == 0) {
    handleHostUnbanCmd(playerData, message);

  } else if (strncmp(message+1, "lagwarn",7) == 0) {
    handleLagwarnCmd(playerData, message);

  } else if (strncmp(message+1, "lagstats",8) == 0) {
    handleLagstatsCmd(playerData, message);

  } else if (strncmp(message+1, "idlestats",9) == 0) {
    handleIdlestatsCmd(playerData, message);

  } else if (strncmp(message+1, "flaghistory", 11 ) == 0) {
    handleFlaghistoryCmd(playerData, message);

  } else if (strncmp(message+1, "playerlist", 10) == 0) {
    handlePlayerlistCmd(playerData, message);

  } else if (strncmp(message+1, "report", 6) == 0) {
    handleReportCmd(playerData, message);

  } else if (strncmp(message+1, "help", 4) == 0) {
    handleHelpCmd(playerData, message);

  } else if (strncmp(message + 1, "identify", 8) == 0) {
    handleIdentifyCmd(playerData, message);

  } else if (strncmp(message + 1, "register", 8) == 0) {
    handleRegisterCmd(playerData, message);

  } else if (strncmp(message + 1, "ghost", 5) == 0) {
    handleGhostCmd(playerData, message);

  } else if (strncmp(message + 1, "deregister", 10) == 0) {
    handleDeregisterCmd(playerData, message);

  } else if (strncmp(message + 1, "setpass", 7) == 0) {
    handleSetpassCmd(playerData, message);

  } else if (strncmp(message + 1, "grouplist", 9) == 0) {
    handleGrouplistCmd(playerData, message);

  } else if (strncmp(message + 1, "showgroup", 9) == 0) {
    handleShowgroupCmd(playerData, message);

  } else if (strncmp(message + 1, "groupperms", 10) == 0) {
    handleGrouppermsCmd(playerData, message);

  } else if (strncmp(message + 1, "setgroup", 8) == 0) {
    handleSetgroupCmd(playerData, message);

  } else if (strncmp(message + 1, "removegroup", 11) == 0) {
    handleRemovegroupCmd(playerData, message);

  } else if (strncmp(message + 1, "reload", 6) == 0) {
    handleReloadCmd(playerData, message);

  } else if (strncmp(message + 1, "poll", 4) == 0) {
    handlePollCmd(playerData, message);

  } else if (strncmp(message + 1, "vote", 4) == 0) {
    handleVoteCmd(playerData, message);

  } else if (strncmp(message + 1, "veto", 4) == 0) {
    handleVetoCmd(playerData, message);

  } else if (strncmp(message + 1, "viewreports", 11) == 0) {
    handleViewReportsCmd(playerData, message);

  } else if (strncmp(message + 1, "clientquery", 11) == 0) {
    handleClientqueryCmd(playerData, message);

  } else if (strncmp(message + 1, "date", 4) == 0 || strncmp(message + 1, "time", 4) == 0) {
    handleDateCmd(playerData, message);

  } else if (strncmp(message + 1, "record", 6) == 0) {
    handleRecordCmd(playerData, message);

  } else if (strncmp(message + 1, "replay", 6) == 0) {
    handleReplayCmd(playerData, message);

  } else if (strncmp(message + 1, "masterban", 9) == 0) {
    handleMasterBanCmd(playerData, message);

  } else {
    char reply[MessageLen];
    snprintf(reply, MessageLen, "Unknown command [%s]", message + 1);
    sendMessage(ServerPlayer, t, reply);
  }
}


/** observers and paused players should not be sending updates.. punish the
 * ones that are paused since they are probably cheating.
 */
static bool invalidPlayerAction(PlayerInfo &p, int t, const char *action) {
  if (p.isObserver() || p.isPaused()) {
    if (p.isPaused()) {
      char buffer[MessageLen];
      DEBUG1("Player \"%s\" tried to %s while paused\n", p.getCallSign(), action);
      snprintf(buffer, MessageLen, "Autokick: Looks like you tried to %s while paused.", action);
      sendMessage(ServerPlayer, t, buffer);
      snprintf(buffer, MessageLen, "Invalid attempt to %s while paused", action);
      removePlayer(t, buffer);
    } else {
      DEBUG1("Player %s tried to %s as an observer\n", p.getCallSign(), action);
    }
    return true;
  }
  return false;
}

bool checkSpam(char* message, GameKeeper::Player* playerData, int t)
{
  std::string tempmsg = message, lmsg = playerData->player.getLastMsg();
  const std::string::iterator sizer = tempmsg.end();
  for (std::string::iterator c = tempmsg.begin(); c <= sizer; c++)
    if (isspace(*c))
      tempmsg.erase(c);
  if (playerData->player.getLastMsg().size() > 0 &&
      strncasecmp(tempmsg.c_str(), lmsg.c_str(),
      lmsg.size() > tempmsg.size() ? tempmsg.size() : lmsg.size()) == 0 &&
      TimeKeeper::getCurrent() - playerData->player.getLastMsgTime()
      <= clOptions->msgTimer) {
    playerData->player.incSpamWarns();
    sendMessage(ServerPlayer, t, "***Server Warning: Please do not spam.");
    if (playerData->player.getSpamWarns() > clOptions->spamWarnMax
	|| clOptions->spamWarnMax == 0) {
      sendMessage(ServerPlayer, t, "You were kicked because of spamming.");
      DEBUG2("Kicking player %s [%d] for spamming too much [2 messages sent with "
	     "less than %d second(s) in between; player was warned %d times]",
	     playerData->player.getCallSign(), t,
	     TimeKeeper::getCurrent()
	     - playerData->player.getLastMsgTime(),
	     playerData->player.getSpamWarns());
      removePlayer(t, "spam");
      return true;
    }
  }
  playerData->player.setLastMsg(tempmsg);
  return false;
}

static void handleCommand(int t, const void *rawbuf)
{
  NetHandler *handler = NetHandler::getHandler(t);
  if (!handler)
    return;
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(t);
  if (!playerData)
    return;

  uint16_t len, code;
  void *buf = (char *)rawbuf;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);

  switch (code) {
    // player joining
    case MsgEnter: {
      uint16_t rejectCode;
      char     rejectMsg[128];
      bool result = playerData->loadEnterData(buf, rejectCode, rejectMsg);
      std::string timeStamp = TimeKeeper::timestamp();
      DEBUG1("Player %s [%d] has joined from %s at %s\n",
	     playerData->player.getCallSign(), 
	     t, handler->getTargetIP(), timeStamp.c_str());
      if (result) {
	addPlayer(t);
      } else {
	rejectPlayer(t, rejectCode, rejectMsg);
      }
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
      unsigned short numClientFlags = len/2;

      /* Unpack incoming message containing the list of flags our client supports */
      for (int i = 0; i < numClientFlags; i++) {
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
      // player moved before countdown started
      if (clOptions->timeLimit>0.0f && !countdownActive) {
	playerData->player.setPlayedEarly();
      }
      playerAlive(t);
      break;
    }

    // player sent version string
    case MsgVersion: {
      buf = playerData->player.setClientVersion(len, buf);
      break;
    }

    // player declaring self destroyed
    case MsgKilled: {
      if (playerData->player.isObserver())
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

      if (invalidPlayerAction(playerData->player, t, "grab a flag"))
	break;

      buf = nboUnpackUShort(buf, flag);
      // Sanity check
      if (flag < numFlags)
	grabFlag(t, *FlagInfo::get(flag));
      break;
    }

    // player requesting to drop flag
    case MsgDropFlag: {
      // data: position of drop
      float pos[3];
      buf = nboUnpackVector(buf, pos);
      dropFlag(*playerData, pos);
      break;
    }

    // player has captured a flag
    case MsgCaptureFlag: {
      // data: team whose territory flag was brought to
      uint16_t team;

      if (invalidPlayerAction(playerData->player, t, "capture a flag"))
	break;

      buf = nboUnpackUShort(buf, team);
      captureFlag(t, TeamColor(team));
      break;
    }

    // shot fired
    case MsgShotBegin:
      if (invalidPlayerAction(playerData->player, t, "shoot"))
	break;

      // Sanity check
      if (len == FiringInfoPLen)
	shotFired(t, buf, int(len));
      break;

    // shot ended prematurely
    case MsgShotEnd: {
      if (playerData->player.isObserver())
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
      uint16_t from, to;

      if (invalidPlayerAction(playerData->player, t, "teleport"))
	break;

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
      playerData->player.hasSent(message);
      // check for spamming
      if (checkSpam(message, playerData, t))
        break;
      // check for command
      if (message[0] == '/' && message[1] != '/') {
	/* make commands case insensitive for user-friendlyness */
	unsigned int pos = 1;
	while ((pos < strlen(message)) && (isAlphanumeric(message[pos]))) {
	  message[pos] = tolower((int)message[pos]);
	  pos++;
	}
	if (Record::enabled()) {
	  void *buf, *bufStart = getDirectMessageBuffer();
	  buf = nboPackUByte (bufStart, t);       // the src player
	  buf = nboPackUByte (buf, targetPlayer); // the dst player
	  buf = nboPackString (buf, message, strlen(message) + 1);
	  Record::addPacket (MsgMessage, (char*)buf - (char*)bufStart, bufStart,
	                     HiddenPacket);
	}
	parseCommand(message, t);
      }
      else if (targetPlayer == AdminPlayers) {
        if (playerData->accessInfo.hasPerm(PlayerAccessInfo::adminMessages)) {
	  sendMessage (t, AdminPlayers, message);
        }
        else {
	  sendMessage(ServerPlayer, t,
	              "You do not have permission to speak on the admin channel.");
        }
      }
      else if ((targetPlayer < LastRealPlayer) && !realPlayer(targetPlayer)) {
        // check for bogus targets
	sendMessage(ServerPlayer, t, "The player you tried to talk to does not exist!");
      }
      else {
        // most messages should come here
	sendMessage(t, targetPlayer, message);
      }
      break;
    }

    // player has transferred flag to another tank
    case MsgTransferFlag: {
	PlayerId from, to;

	buf = nboUnpackUByte(buf, from);
	buf = nboUnpackUByte(buf, to);

	GameKeeper::Player *fromData
	  = GameKeeper::Player::getPlayerByIndex(from);
	if (!fromData)
	  return;

	int flagIndex = fromData->player.getFlag();
	if (to == ServerPlayer) {
	  if (flagIndex >= 0)
	    zapFlag (*FlagInfo::get(flagIndex));
	  return;
	}

	// Sanity check
	if (to >= curMaxPlayers)
	  return;

	if (flagIndex == -1)
	  return;

	GameKeeper::Player *toData
	  = GameKeeper::Player::getPlayerByIndex(to);
	if (!toData)
	  return;

	int oFlagIndex = toData->player.getFlag();
	if (oFlagIndex >= 0)
	  zapFlag (*FlagInfo::get(oFlagIndex));

	void *bufStart = getDirectMessageBuffer();
	void *buf = nboPackUByte(bufStart, from);
	buf = nboPackUByte(buf, to);
	FlagInfo &flag = *FlagInfo::get(flagIndex);
	flag.flag.owner = to;
	flag.player = to;
	toData->player.resetFlag();
	toData->player.setFlag(flagIndex);
	fromData->player.resetFlag();
	buf = flag.pack(buf);
	broadcastMessage(MsgTransferFlag, (char*)buf - (char*)bufStart, bufStart);
	break;
    }

    case MsgUDPLinkEstablished:
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
      bool warn, kick;
      int lag = playerData->lagInfo.updatePingLag(buf, warn, kick);
      if (warn) {
	char message[MessageLen];
	sprintf(message,"*** Server Warning: your lag is too high (%d ms) ***",
		lag);
	sendMessage(ServerPlayer, t, message);
	if (kick) {
	  // drop the player
	  sprintf
	    (message,
	     "You have been kicked due to excessive lag (you have been warned %d times).",
	     clOptions->maxlagwarn);
	  sendMessage(ServerPlayer, t, message);
	  removePlayer(t, "lag");
	}
      }
      break;
    }

    // player is sending his position/speed (bulk data)
    case MsgPlayerUpdate:
    case MsgPlayerUpdateSmall: {
      float timestamp;
      PlayerId id;
      PlayerState state;

      buf = nboUnpackFloat(buf, timestamp);
      buf = nboUnpackUByte(buf, id);
      buf = state.unpack(buf, code);

      // silently drop old packet
      if (state.order <= lastState[t].order)
	break;

      playerData->lagInfo.updateLag(timestamp,
                                    state.order - lastState[t].order > 1);
      playerData->player.updateIdleTime();

      //Don't kick players up to 10 seconds after a world parm has changed,
      TimeKeeper now = TimeKeeper::getCurrent();

      if (now - lastWorldParmChange > 10.0f) {

        // see if the player is too high
        static const float heightFudge = 1.10f; /* 10% */

	float wingsGravity = BZDB.eval(StateDatabase::BZDB_WINGSGRAVITY);
	float normalGravity = BZDB.eval(StateDatabase::BZDB_GRAVITY);
	if ((wingsGravity < 0.0f) && (normalGravity < 0.0f)) {

	  float wingsMaxHeight = BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY);
	  wingsMaxHeight *= wingsMaxHeight;
	  wingsMaxHeight *= (1 + BZDB.eval(StateDatabase::BZDB_WINGSJUMPCOUNT));
	  wingsMaxHeight /= (-wingsGravity * 0.5f);

	  float normalMaxHeight = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
	  normalMaxHeight *= normalMaxHeight;
	  normalMaxHeight /= (-normalGravity * 0.5f);

	  float maxHeight;
	  if (wingsMaxHeight > normalMaxHeight) {
	    maxHeight = wingsMaxHeight;
          } else {
            maxHeight = normalMaxHeight;
          }

          // final adjustments
          maxHeight *= heightFudge;
          maxHeight += maxWorldHeight;

	  if (state.pos[2] > maxHeight) {
	    DEBUG1("Kicking Player %s [%d] jumped too high [max: %f height: %f]\n",
		   playerData->player.getCallSign(), t, maxHeight, state.pos[2]);
	    sendMessage(ServerPlayer, t, "Autokick: Player location was too high.");
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
	  DEBUG1("Kicking Player %s [%d] Out of map bounds at position (%.2f,%.2f,%.2f)\n",
		 playerData->player.getCallSign(), t,
		 state.pos[0], state.pos[1], state.pos[2]);
	  sendMessage(ServerPlayer, t, "Autokick: Player location was outside the playing area.");
	  removePlayer(t, "Out of map bounds");
	}

	// Speed problems occur around flag drops, so don't check for
	// a short period of time after player drops a flag. Currently
	// 2 second, adjust as needed.
	if (playerData->player.isFlagTransitSafe()) {

          // we'll be checking against the player's flag type
          int pFlag = playerData->player.getFlag();

	  // check for highspeed cheat; if inertia is enabled, skip test for now
	  if (clOptions->linearAcceleration == 0.0f) {
	    // Doesn't account for going fast backwards, or jumping/falling
	    float curPlanarSpeedSqr = state.velocity[0]*state.velocity[0] +
				      state.velocity[1]*state.velocity[1];

	    float maxPlanarSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);

	    bool logOnly = false;

	    // if tank is not driving cannot be sure it didn't toss
	    // (V) in flight

	    // if tank is not alive cannot be sure it didn't just toss
	    // (V)
	    if (pFlag >= 0) {
	      FlagInfo &flag = *FlagInfo::get(pFlag);
	      if (flag.flag.type == Flags::Velocity)
	        maxPlanarSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
	      else if (flag.flag.type == Flags::Thief)
	        maxPlanarSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
	      else if (flag.flag.type == Flags::Agility)
	        maxPlanarSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
	      else if ((flag.flag.type == Flags::Burrow) &&
	        (lastState[t].pos[2] == state.pos[2]) &&
	        (lastState[t].velocity[2] == state.velocity[2]) &&
	        (state.pos[2] <= BZDB.eval(StateDatabase::BZDB_BURROWDEPTH)))
	        // if we have burrow and are not actively burrowing
	        // You may have burrow and still be above ground. Must
	        // check z in ground!!
	        maxPlanarSpeed *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
	    }
	    float maxPlanarSpeedSqr = maxPlanarSpeed * maxPlanarSpeed;

	    // If player is moving vertically, or not alive the speed checks
	    // seem to be problematic. If this happens, just log it for now,
	    // but don't actually kick
	    if ((lastState[t].pos[2] != state.pos[2])
	    ||  (lastState[t].velocity[2] != state.velocity[2])
	    ||  ((state.status & PlayerState::Alive) == 0)) {
	      logOnly = true;
	    }

	    // allow a 10% tolerance level for speed if -speedtol is not sane
	    float realtol = 1.1f;
	    if (speedTolerance > 1.0f)
	      realtol = speedTolerance;
	    maxPlanarSpeedSqr *= realtol;
	    if (curPlanarSpeedSqr > maxPlanarSpeedSqr) {
	      if (logOnly) {
		DEBUG1("Logging Player %s [%d] tank too fast (tank: %f, allowed: %f){Dead or v[z] != 0}\n",
		playerData->player.getCallSign(), t,
		sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
	      } else {
		DEBUG1("Kicking Player %s [%d] tank too fast (tank: %f, allowed: %f)\n",
		       playerData->player.getCallSign(), t,
		       sqrt(curPlanarSpeedSqr), sqrt(maxPlanarSpeedSqr));
		sendMessage(ServerPlayer, t, "Autokick: Player tank is moving too fast.");
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
      if (!playerData->player.isAlive()
	  && (state.status & short(PlayerState::Alive)))
        break;
    }

    //Fall thru
    case MsgGMUpdate:
    case MsgAudio:
    case MsgVideo:
      // observer shouldn't send bulk messages anymore, they used to when it was
      // a server-only hack; but the check does not hurt, either
      if (playerData->player.isObserver())
	break;
      relayPlayerPacket(t, len, rawbuf, code);
      break;

    // FIXME handled inside uread, but not discarded
    case MsgUDPLinkRequest:
      break;

    case MsgKrbPrincipal:
      playerData->authentication.setPrincipalName((char *)buf, len);
      break;

    case MsgKrbTicket:
      playerData->authentication.verifyCredential((char *)buf, len);
      // Not really the place here, but for initial testing we need something
      if (playerData->authentication.isTrusted())
	sendMessage(ServerPlayer, t, "Welcome, we trust you");
      break;

    // unknown msg type
    default:
      DEBUG1("Player [%d] sent unknown packet type (%x), \
possible attack from %s\n",
	     t, code, handler->getTargetIP());
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


static void doStuffOnPlayer(GameKeeper::Player &playerData)
{
  int p = playerData.getIndex();

  // send delayed packets
  void *data;
  int length;
  if (playerData.delayq.getPacket(&length, &data)) {
    int result = pwrite(playerData, data, length);
    free(data);
    if (result == -1)
      return;
  }

  // kick idle players
  if (clOptions->idlekickthresh > 0) {
    if (playerData.player.isTooMuchIdling(clOptions->idlekickthresh)) {
      char message[MessageLen]
	= "You were kicked because you were idle too long";
      sendMessage(ServerPlayer, p,  message);
      removePlayer(p, "idling");
      return;
    }
  }

  // Checking hostname resolution and ban player if has to
  const char *hostname = playerData.netHandler->getHostname();
  // check against ban lists
	HostBanInfo info("*");
  if (hostname && !clOptions->acl.hostValidate(hostname,&info)) {
		std::string reason = "bannedhost for: ";

		if (info.reason.size())
			reason += info.reason;
		else
			reason += "General Ban";

		if (info.bannedBy.size())
		{
			reason += " by ";
			reason += info.bannedBy;
		}

		if (info.fromMaster)
			reason += " from the master server";
    removePlayer(p, reason.c_str());
    return;
  }

  // update notResponding
  if (playerData.player.hasStartedToNotRespond()) {
    // if player is the rabbit, anoint a new one
    if (p == rabbitIndex) {
      anointNewRabbit();
      // Should recheck if player is still available
      if (!GameKeeper::Player::getPlayerByIndex(p))
	return;
    }
    // if player is holding a flag, drop it
    for (int j = 0; j < numFlags; j++)
      if (FlagInfo::get(j)->player == p) {
	dropFlag(playerData, lastState[p].pos);
	// Should recheck if player is still available
	if (!GameKeeper::Player::getPlayerByIndex(p))
	  return;
      }
  }

  // send lag pings
  bool warn;
  bool kick;
  int nextPingSeqno = playerData.lagInfo.getNextPingSeqno(warn, kick);
  if (nextPingSeqno > 0) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUShort(bufStart, nextPingSeqno);
    int result = directMessage(playerData, MsgLagPing,
			       (char*)buf - (char*)bufStart, bufStart);
    if (result == -1)
      return;
    if (warn) {
      char message[MessageLen];
      sprintf(message, "*** Server Warning: your lag is too high ***");
      sendMessage(ServerPlayer, p, message);
      // Should recheck if player is still available
      if (!GameKeeper::Player::getPlayerByIndex(p))
	return;
      if (kick) {
	// drop the player
	sprintf(message,
		"You have been kicked due to excessive lag\
 (you have been warned %d times).",
		clOptions->maxlagwarn);
	sendMessage(ServerPlayer, p, message);
	removePlayer(p, "lag");
	return;
      }
    }
  }

  // kick any clients that need to be
  std::string reasonToKick = playerData.netHandler->reasonToKick();
  if (reasonToKick != "") {
    removePlayer(p, reasonToKick.c_str(), false);
    return;
  }

}

/** main parses command line options and then enters an event and activity
 * dependant main loop.  once inside the main loop, the server is up and
 * running and should be ready to process connections and activity.
 */
int main(int argc, char **argv)
{
  int nfound;
  VotingArbiter *votingarbiter = (VotingArbiter *)NULL;

  setvbuf(stdout, (char *)NULL, _IOLBF, 0);
  setvbuf(stderr, (char *)NULL, _IOLBF, 0);

  Record::init();


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
      DEBUG2("Failed to initialize Winsock.  Terminating.\n");
      return 1;
    }
    if (LOBYTE(wsaData.wVersion) != major ||
	HIBYTE(wsaData.wVersion) != minor) {
      DEBUG2("Version mismatch in Winsock;"
	  "  got %d.%d, expected %d.%d.  Terminating.\n",
	  (int)LOBYTE(wsaData.wVersion),
	  (int)HIBYTE(wsaData.wVersion),
				 major,
				 minor);
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

  // Loading lag thresholds
  LagInfo::setThreshold(clOptions->lagwarnthresh,(float)clOptions->maxlagwarn);
  // Loading extra flag number
  FlagInfo::setExtra(clOptions->numExtraFlags);

  // enable replay server mode
  if (clOptions->replayServer) {

    Replay::init();

    // we don't send flags to a client that isn't expecting them
    numFlags = 0;

    // disable the BZDB callbacks
    for (unsigned int gi = 0; gi < numGlobalDBItems; ++gi) {
      assert(globalDBItems[gi].name != NULL);
      BZDB.removeCallback(std::string(globalDBItems[gi].name),
                          onGlobalChanged, (void*) NULL);
    }

    // maxPlayers is sent in the world data to the client.
    // the client then uses this to setup it's players
    // data structure, so we need to send it the largest
    // PlayerId it might see.
    maxPlayers = MaxPlayers + ReplayObservers;

    if (clOptions->maxTeam[ObserverTeam] == 0) {
      std::cerr << "replay needs at least 1 observer, set to 1" << std::endl;
      clOptions->maxTeam[ObserverTeam] = 1;
    }
    else if (clOptions->maxTeam[ObserverTeam] > ReplayObservers) {
      std::cerr << "observer count limited to " << ReplayObservers <<
                   " for replay" << std::endl;
      clOptions->maxTeam[ObserverTeam] = ReplayObservers;
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

  Authentication::init(clOptions->publicizedAddress.c_str(),
		       clOptions->wksPort,
		       clOptions->password.c_str());

  /* initialize the poll arbiter for voting if necessary */
  if (clOptions->voteTime > 0) {
    votingarbiter = new VotingArbiter(clOptions->voteTime, clOptions->vetoTime, clOptions->votesRequired, clOptions->votePercentage, clOptions->voteRepeatTime);
    DEBUG1("There is a voting arbiter with the following settings:\n" \
           "\tvote time is %d seconds\n" \
           "\tveto time is %d seconds\n" \
           "\tvotes required are %d\n" \
           "\tvote percentage necessary is %f\n" \
           "\tvote repeat time is %d seconds\n" \
           "\tavailable voters is initially set to %d\n",
           clOptions->voteTime, clOptions->vetoTime, clOptions->votesRequired, clOptions->votePercentage, clOptions->voteRepeatTime,
           maxPlayers);

    // override the default voter count to the max number of players possible
    votingarbiter->setAvailableVoters(maxPlayers);
    BZDB.setPointer("poll", (void *)votingarbiter);
    BZDB.setPermission("poll", StateDatabase::ReadOnly);
  }

  if (clOptions->pingInterface != "") {
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

  // get the master ban list
  if (clOptions->publicizeServer && !clOptions->suppressMasterBanList){
    MasterBanList	banList;
    for (std::vector<std::string>::const_iterator i = clOptions->masterBanListURL.begin(); i != clOptions->masterBanListURL.end(); i++) {
      clOptions->acl.merge(banList.get(i->c_str()));
      DEBUG1("Loaded master ban list from %s\n", i->c_str());
    }
  }

  Score::setTeamKillRatio(clOptions->teamKillerKickRatio);
  Score::setWinLimit(clOptions->maxPlayerScore);
  if (clOptions->rabbitSelection == RandomRabbitSelection)
    Score::setRandomRanking();
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
  pingReply.maxTime = (uint16_t)clOptions->timeLimit;
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

  // no original world weapons in replay mode
  if (Replay::enabled()) {
    world->getWorldWeapons().clear();
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
    PlayerAccessInfo::readGroupsFile(groupsFile);
  // make sure that the 'admin' & 'default' groups exist
  PlayerAccessMap::iterator itr = groupAccess.find("DEFAULT");
  if (itr == groupAccess.end()) {
    PlayerAccessInfo info;
    info.explicitAllows[PlayerAccessInfo::idleStats] = true;
    info.explicitAllows[PlayerAccessInfo::lagStats] = true;
    info.explicitAllows[PlayerAccessInfo::date] = true;
    info.explicitAllows[PlayerAccessInfo::flagHistory] = true;
    info.explicitAllows[PlayerAccessInfo::actionMessage] = true;
    info.explicitAllows[PlayerAccessInfo::privateMessage] = true;
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
    PlayerAccessInfo::readPermsFile(userDatabaseFile);

  if (clOptions->startRecording) {
    Record::start (ServerPlayer);
  }


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

    // see if the octree needs to be reloaded
    world->checkCollisionManager();

    maxFileDescriptor = 0;
    // prepare select set
    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    NetHandler::setFd(&read_set, &write_set, maxFileDescriptor);
    // always listen for connections
    _FD_SET(wksSocket, &read_set);
    if (wksSocket > maxFileDescriptor) {
      maxFileDescriptor = wksSocket;
    }

    // check for list server socket connected
    if (listServerLinksCount) {
      if (listServerLink->isConnected()) {
	if (listServerLink->phase == ListServerLink::CONNECTING)
	  _FD_SET(listServerLink->linkSocket, &write_set);
	else
	  _FD_SET(listServerLink->linkSocket, &read_set);
	if (listServerLink->linkSocket > maxFileDescriptor)
	  maxFileDescriptor = listServerLink->linkSocket;
      }
    }

    // find timeout when next flag would hit ground
    TimeKeeper tm = TimeKeeper::getCurrent();
    // lets start by waiting 3 sec
    float waitTime = 3.0f;
    if (countdownActive && clOptions->timeLimit > 0.0f) {
	waitTime = 1.0f;
    }

    // get time for next flag drop
    float dropTime;
    while ((dropTime = FlagInfo::getNextDrop(tm)) <= 0.0f) {
      // if any flags were in the air, see if they've landed
      for (i = 0; i < numFlags; i++) {
	FlagInfo &flag = *FlagInfo::get(i);
	if (flag.landing(tm)) {
	  if (flag.flag.status == FlagOnGround) {
	    sendFlagUpdate(flag);
          } else {
	    resetFlag(flag);
          }
        }
      }
    }
    if (dropTime < waitTime) {
      waitTime = dropTime;
    }

    // get time for next Player internal action
    GameKeeper::Player::updateLatency(waitTime);

    // get time for the next world weapons shot
    if (world->getWorldWeapons().count() > 0) {
      float nextTime = world->getWorldWeapons().nextTime ();
      if (nextTime < waitTime) {
        waitTime = nextTime;
      }
    }

    // get time for the next replay packet (if active)
    if (Replay::enabled()) {
      float nextTime = Replay::nextTime ();
      if (nextTime < waitTime) {
        waitTime = nextTime;
      }
    }

    // minmal waitTime
    if (waitTime < 0.0f) {
      waitTime = 0.0f;
    }

    // if there are buffered UDP, no wait at all
    if (NetHandler::anyUDPPending()) {
      waitTime = 0.0f;
    }


    /**************
     *  SELECT()  *
     **************/

    // wait for an incoming communication, a flag to hit the ground,
    // a game countdown to end, a world weapon needed to be fired, 
    // or a replay packet waiting to be sent.
    struct timeval timeout;
    timeout.tv_sec = long(floorf(waitTime));
    timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));
    nfound = select(maxFileDescriptor+1, (fd_set*)&read_set, (fd_set*)&write_set, 0, &timeout);
    //if (nfound)
    //	DEBUG1("nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);

    // send replay packets 
    // (this check and response should follow immediately after the select() call)
    if (Replay::playing()) {
      Replay::sendPackets ();
    }
    
    // Synchronize PlayerInfo
    tm = TimeKeeper::getCurrent();
    PlayerInfo::setCurrentTime(tm);

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

    for (int p = 0; p < curMaxPlayers; p++) {
      GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(p);
      if (!playerData)
	continue;
      doStuffOnPlayer(*playerData);
    }

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
	  sendMessage(ServerPlayer, AllPlayers, message);
	  announcedOpening = true;
	}

	static TimeKeeper lastAnnounce = TimeKeeper::getNullTime();

	/* make a heartbeat announcement every 15 seconds */
	if (((voteTime - (int)(TimeKeeper::getCurrent() - votingarbiter->getStartTime()) - 1) % 15 == 0) &&
	    ((int)(TimeKeeper::getCurrent() - lastAnnounce) != 0) &&
	    (votingarbiter->timeRemaining() > 0)) {
	  sprintf(message, "%d seconds remain in the poll to %s %s.", votingarbiter->timeRemaining(), action.c_str(), target.c_str());
	  sendMessage(ServerPlayer, AllPlayers, message);
	  lastAnnounce = TimeKeeper::getCurrent();
	}

	if (votingarbiter->isPollClosed()) {

	  if (!announcedResults) {
	    sprintf(message, "Poll Results: %ld in favor, %ld oppose, %ld abstain", votingarbiter->getYesCount(), votingarbiter->getNoCount(), votingarbiter->getAbstentionCount());
	    sendMessage(ServerPlayer, AllPlayers, message);
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
	      sendMessage(ServerPlayer, AllPlayers, message);
	      announcedClosure = true;
	    }
	  } else {
	    if (!announcedClosure) {
	      sprintf(message, "The poll to %s %s was not successful", action.c_str(), target.c_str());
	      sendMessage(ServerPlayer, AllPlayers, message);
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
	      if (action == "ban") {
		int hours = 0;
		int minutes = clOptions->banTime % 60;
		if (clOptions->banTime > 60) {
		  hours = clOptions->banTime / 60;
		}
		pollAction = std::string("banned for ");
		if (hours > 0) {
		  pollAction += string_util::format("%d hour%s%s",
						    hours,
						    hours == 1 ? "." : "s",
						    minutes > 0 ? " and " : "");
		}
		if (minutes > 0) {
		  pollAction += string_util::format("%d minute%s",
						    minutes,
						    minutes > 1 ? "s" : "");
		}
		pollAction += ".";
	      } else if (action == "kick") {
		pollAction = std::string("kicked.");
	      } else {
		pollAction = action;
	      }
	      if (action != "flagreset")
		sprintf(message, "%s has been %s", target.c_str(), pollAction.c_str());
	      else
		sprintf(message, "All unused flags have now been reset.");
	      sendMessage(ServerPlayer, AllPlayers, message);

	      /* regardless of whether or not the player was found, if the poll
	       * is a ban poll, ban the weenie
	       */
	      if (action == "ban") {
		clOptions->acl.ban(realIP.c_str(), target.c_str(), clOptions->banTime);
	      }

	      if ((action == "ban") || (action == "kick")) {
		// lookup the player id
		bool foundPlayer = false;
		int v;
		for (v = 0; v < curMaxPlayers; v++) {
		  GameKeeper::Player *otherData
		    = GameKeeper::Player::getPlayerByIndex(v);
		  if (strncmp(otherData->player.getCallSign(),
			      target.c_str(), 256) == 0) {
		    foundPlayer = true;
		    break;
		  }
		}
		// show the delinquent no mercy; make sure he is kicked even if he changed
		// his callsign by finding a corresponding IP and matching it to the saved one
		if (!foundPlayer) {
		  v = NetHandler::whoIsAtIP(realIP);
		  foundPlayer = (v >= 0);
		}
		if (foundPlayer) {
		  // notify the player
		  sprintf(message, "You have been %s due to sufficient votes to have you removed", action == "ban" ? "temporarily banned" : "kicked");
		  sendMessage(ServerPlayer, v, message);
		  sprintf(message, "/poll %s", action.c_str());
		  removePlayer(v, message);
		}
	      } else if (action == "set") {
		CMDMGR.run(string_util::format("/set %s", target.c_str()));
	      } else if (action == "flagreset") {
		CMDMGR.run("flag reset unused");
	      }
	    } /* end if poll is successful */

	    // get ready for the next poll
	    votingarbiter->forgetPoll();

	    announcedClosure = false;
	    announcedOpening = false;
	    announcedResults = false;

	  } // the poll expired

	} else {
	  // the poll may get enough votes early
	  if (votingarbiter->isPollSuccessful()) {
	    if (action != "flagreset")
	      sprintf(message, "Enough votes were collected to %s %s early.", action.c_str(), target.c_str());
	    else
	      sprintf(message, "Enough votes were collected to reset all unused flags early.");

	    sendMessage(ServerPlayer, AllPlayers, message);

	    // close the poll since we have enough votes (next loop will kick off notification)
	    votingarbiter->closePoll();

	  } // the poll is over
	} // is the poll closed
      } // knows of a poll
    } // voting is allowed and an arbiter exists


    // periodic advertising broadcast
    static const std::vector<std::string>* adLines = clOptions->textChunker.getTextChunk("admsg");
    if ((clOptions->advertisemsg != "") || adLines != NULL) {
      static TimeKeeper lastbroadcast = TimeKeeper::getCurrent();
      if (TimeKeeper::getCurrent() - lastbroadcast > 900) {
	// every 15 minutes
	char message[MessageLen];
	if (clOptions->advertisemsg != "") {
	  // split the admsg into several lines if it contains '\n'
	  const char* c = clOptions->advertisemsg.c_str();
	  const char* j;
	  while ((j = strstr(c, "\\n")) != NULL) {
	    int l = j - c < MessageLen - 1 ? j - c : MessageLen - 1;
	    strncpy(message, c, l);
	    message[l] = '\0';
	    sendMessage(ServerPlayer, AllPlayers, message);
	    c = j + 2;
	  }
	  strncpy(message, c, MessageLen - 1);
	  message[strlen(c) < MessageLen - 1 ? strlen(c) : MessageLen -1] = '\0';
	  sendMessage(ServerPlayer, AllPlayers, message);
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

    // check team flag timeouts
    if (clOptions->gameStyle & TeamFlagGameStyle) {
      for (i = RedTeam; i < CtfTeams; ++i) {
        if (team[i].flagTimeout - tm < 0 && team[i].team.size == 0) {
          int flagid = FlagInfo::lookupFirstTeamFlag(i);
	  if (flagid >= 0) {
	    for (int n = 0; n < clOptions->numTeamFlags[i]; n++) {
	      FlagInfo &flag = *FlagInfo::get(flagid + n);
              if (flag.exist() && flag.player == -1) {
	        DEBUG1("Flag timeout for team %d\n", i);
                zapFlag(flag);
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
	for (i = numFlags - clOptions->numExtraFlags; i < numFlags; i++) {
	  FlagInfo &flag = *FlagInfo::get(i);
	  if (flag.flag.type == Flags::Null) {
	    // flag in now entering game
	    flag.addFlag();
	    sendFlagUpdate(flag);
	    break;
	  }
	}
	lastSuperFlagInsertion = tm;
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
	  // if nobody playing then publicize
	  if (GameKeeper::Player::count() == 0)
	    publicize();
	}

	// send add request
        listServerLink->queueMessage(ListServerLink::ADD);
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
      if (NetHandler::isUdpFdSet(&read_set)) {
	TimeKeeper receiveTime = TimeKeeper::getCurrent();
	while (true) {
	  struct sockaddr_in uaddr;
	  unsigned char ubuf[MaxPacketLen];
	  bool     udpLinkRequest;
	  // interface to the UDP Receive routines
	  int id = NetHandler::udpReceive((char *) ubuf, &uaddr,
					  udpLinkRequest);
	  if (id == -1) {
	    break;
	  } else if (id == -2) {
	    // if I'm ignoring pings
	    // then ignore the ping.
	    if (handlePings) {
	      respondToPing(Address(uaddr));
	      pingReply.write(NetHandler::getUdpSocket(), &uaddr);
	    }
	    continue;
	  } else {
	    if (udpLinkRequest)
	      // send client the message that we are ready for him
	      sendUDPupdate(id);

	    // handle the command for UDP
	    handleCommand(id, ubuf);

	    // don't spend more than 250ms receiving udp
	    if (TimeKeeper::getCurrent() - receiveTime > 0.25f) {
	      DEBUG2("Too much UDP traffic, will hope to catch up later\n");
	      break;
	    }
	  }
	}
      }

      // now check messages from connected players and send queued messages
      for (i = 0; i < curMaxPlayers; i++) {
	// send whatever we have ... if any
	NetHandler *netPlayer = NetHandler::getHandler(i);
	if (netPlayer && netPlayer->pflush(&write_set) == -1) {
	  removePlayer(i, "ECONNRESET/EPIPE", false);
	}

	netPlayer = NetHandler::getHandler(i);
	if (netPlayer && netPlayer->isFdSet(&read_set)) {

	  const RxStatus e = netPlayer->tcpReceive();
	  if (e != ReadAll) {
	    if (e == ReadReset) {
	      removePlayer(i, "ECONNRESET/EPIPE", false);
	    } else if (e == ReadError) {
	      // dump other errors and remove the player
	      nerror("error on read");
	      removePlayer(i, "Read error", false);
	    } else if (e == ReadDiscon) {
	      // disconnected
	      removePlayer(i, "Disconnected", false);
	    } else if (e == ReadHuge) {
	      removePlayer(i, "large packet recvd", false);
	    }
	    continue;
	  }

	  uint16_t len, code;
	  void *buf = netPlayer->getTcpBuffer();
	  buf = nboUnpackUShort(buf, len);
	  buf = nboUnpackUShort(buf, code);

	  // trying to get the real player from the message cause it
	  // happens bots share tcp connection with the player
	  PlayerId t = i;
	  switch (code) {
	  case MsgShotBegin: {
	    nboUnpackUByte(buf, t);
	    break;
	  }
	  case MsgPlayerUpdate:
	  case MsgPlayerUpdateSmall: {
	    float timestamp;
	    buf = nboUnpackFloat(buf, timestamp);
	    buf = nboUnpackUByte(buf, t);
	    break;
	  }
	  default:
	    break;
	  }
	  // Make sure is a bot
	  GameKeeper::Player *playerData;
	  if (t != i) {
	    playerData = GameKeeper::Player::getPlayerByIndex(t);
	    if (!playerData || !playerData->player.isBot()) {
	      t = i;
	      playerData = GameKeeper::Player::getPlayerByIndex(t);
	    }
	    // Should check also if bot and player are related
	  } else {
	    playerData = GameKeeper::Player::getPlayerByIndex(t);
	  }

	  // simple ruleset, if player sends a MsgShotBegin over TCP
	  // he/she must not be using the UDP link
	  if (clOptions->requireUDP && !playerData->player.isBot()) {
	    if (code == MsgShotBegin) {
	      char message[MessageLen];
	      sprintf(message,"Your end is not using UDP, turn on udp");
	      sendMessage(ServerPlayer, i, message);

	      sprintf(message,"upgrade your client http://BZFlag.org/ or");
	      sendMessage(ServerPlayer, i, message);

	      sprintf(message,"Try another server, Bye!");
	      sendMessage(ServerPlayer, i, message);
	      removePlayer(i, "no UDP");
	      continue;
	    }
	  }

	  // handle the command
	  handleCommand(t, netPlayer->getTcpBuffer());
	}
      }
    } else if (nfound < 0) {
      if (getErrno() != EINTR) {
	// test code - do not uncomment, will cause big stuttering
	// sleep(1);
      }
    } else {
      if (NetHandler::anyUDPPending())
	NetHandler::flushAllUDP();
    }

    // Fire world weapons
    world->getWorldWeapons().fire();
  }

  serverStop();

  // free misc stuff
  delete clOptions; clOptions = NULL;
  FlagInfo::setSize(0);
  delete world; world = NULL;
  delete[] worldDatabase; worldDatabase = NULL;
  delete votingarbiter; votingarbiter = NULL;

  Record::kill();
  Replay::kill();

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
