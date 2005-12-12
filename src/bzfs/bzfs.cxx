/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
#include "PhysicsDriver.h"
#include "CommandManager.h"
#include "TimeBomb.h"
#include "ConfigFileManager.h"
#include "bzsignal.h"

// implementation-specific bzfs-specific headers
#include "RejoinList.h"
#include "ListServerConnection.h"
#include "WorldInfo.h"
#include "WorldWeapons.h"
#include "BZWReader.h"
#include "PackVars.h"
#include "SpawnPosition.h"
#include "DropGeometry.h"
#include "commands.h"
#include "MasterBanList.h"
#include "Filter.h"
#include "WorldEventManager.h"
#include "WorldGenerators.h"

// common implementation headers
#include "Obstacle.h"
#include "ObstacleMgr.h"
#include "BaseBuilding.h"
#include "AnsiCodes.h"
#include "GameTime.h"
#include "bzfsAPI.h"

// only include this if we are going to use plugins and export the API
#ifdef _USE_BZ_API
#include "bzfsPlugins.h"
#endif

// pass through the SELECT loop
static bool dontWait = true;

// every ListServerReAddTime server add ourself to the list
// server again.  this is in case the list server has reset
// or dropped us for some reason.
static const float ListServerReAddTime = 30.0f * 60.0f;

static const float FlagHalfLife = 10.0f;

// do NOT change
static const int InvalidPlayer = -1;

float speedTolerance = 1.125f;
static bool doSpeedChecks = true;

// Command Line Options
CmdLineOptions *clOptions;

// server address to listen on
static Address serverAddress;
// well known service socket
static int wksSocket;
bool handlePings = true;
static PingPacket pingReply;
// highest fd used
static int maxFileDescriptor;
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
static bool disableHeightChecks = false;

char hexDigest[50];

TimeKeeper gameStartTime;
TimeKeeper countdownPauseStart = TimeKeeper::getNullTime();
bool countdownActive = false;
int countdownDelay = -1;

static ListServerLink *listServerLink = NULL;
static int listServerLinksCount = 0;

// FIXME: should be static, but needed by SpawnPosition
WorldInfo *world = NULL;
// FIXME: should be static, but needed by RecordReplay
char *worldDatabase = NULL;
uint32_t worldDatabaseSize = 0;
char worldSettings[4 + WorldSettingsSize];
float pluginWorldSize = -1;
float pluginWorldHeight = -1;
float	pluginMaxWait = 1000.0;
Filter   filter;

BasesList bases;

// global keeper of world Events
WorldEventManager	worldEventManager;

// FIXME - define a well-known constant for a null playerid in address.h?
// might be handy in other players, too.
// Client does not check for rabbit to be 255, but it still works
// because 255 should be > curMaxPlayers and thus no matching player will
// be found.
// FIXME: should be static, but needed by RecordReplay
uint8_t rabbitIndex = NoPlayer;

static RejoinList rejoinList;

static TimeKeeper lastWorldParmChange;
static bool       isIdentifyFlagIn = false;
static bool       playerHadWorld   = false;

void sendFilteredMessage(int playerIndex, PlayerId dstPlayer, const char *message);
static void dropPlayerFlag(GameKeeper::Player &playerData, const float dropPos[3]);
static void dropAssignedFlag(int playerIndex);
static std::string evaluateString(const std::string&);

int getCurMaxPlayers()
{
  return curMaxPlayers;
}

static bool realPlayer(const PlayerId& id)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(id);
  return playerData && playerData->player.isPlaying();
}

static void dropHandler(NetHandler *handler, const char *reason)
{
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData)
      continue;
    if (playerData->netHandler != handler)
      continue;
    removePlayer(i, reason, false);
  }
  delete handler;
}

static int pwrite(NetHandler *handler, const void *b, int l)
{
  int result = handler->pwrite(b, l);
  if (result == -1) {
    dropHandler(handler, "ECONNRESET/EPIPE");
  } else if (result == -2) {
    dropHandler(handler, "send queue too big");
  }
  return result;
}

static void pwriteBroadcast(const void *b, int l, int mask)
{
  int result;
  std::list<NetHandler*>::const_iterator it;
  // send message to everyone
  for (it = NetHandler::netConnections.begin();
       it != NetHandler::netConnections.end();
       it++) {
    NetHandler *handler = *it;
    if (handler->getClientKind() & mask) {
      result = handler->pwrite(b, l);
      if (result == -1) {
	dropHandler(handler, "ECONNRESET/EPIPE");
      } else if (result == -2) {
	dropHandler(handler, "send queue too big");
      }
    }
  }
}

static char sMsgBuf[MaxPacketLen];
char *getDirectMessageBuffer()
{
  return &sMsgBuf[2*sizeof(uint16_t)];
}

// FIXME? 4 bytes before msg must be valid memory, will get filled in with len+code
// usually, the caller gets a buffer via getDirectMessageBuffer(), but for example
// for MsgShotBegin the receiving buffer gets used directly
static int directMessage(NetHandler *handler,
			 uint16_t code, int len, const void *msg)
{
  // send message to one player
  void *bufStart = (char *)msg - 2*sizeof(uint16_t);

  void *buf = bufStart;
  buf = nboPackUShort(buf, uint16_t(len));
  buf = nboPackUShort(buf, code);
  return pwrite(handler, bufStart, len + 4);
}

void directMessage(int playerIndex, uint16_t code, int len, const void *msg)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;
  if (!playerData->netHandler)
    return;

  directMessage(playerData->netHandler, code, len, msg);
}


void broadcastMessage(uint16_t code, int len, const void *msg, bool alsoTty)
{
  void *bufStart = (char *)msg - 2*sizeof(uint16_t);
  void *buf = nboPackUShort(bufStart, uint16_t(len));
  nboPackUShort(buf, code);

  // send message to everyone
  int mask = NetHandler::clientBZFlag | NetHandler::clientBZBot;
  if (alsoTty)
    mask |= NetHandler::clientBZAdmin;
  pwriteBroadcast(bufStart, len + 4, mask);

  // record the packet
  if (Record::enabled()) {
    Record::addPacket(code, len, msg);
  }
}

// relay message only for human. Bots will get message locally.
static void relayMessage(uint16_t code, int len, const void *msg)
{
  void *bufStart = (char *)msg - 2*sizeof(uint16_t);
  void *buf = nboPackUShort(bufStart, uint16_t(len));
  nboPackUShort(buf, code);

  // send message to human kind
  pwriteBroadcast(bufStart, len + 4, NetHandler::clientBZFlag);

  // record the packet
  if (Record::enabled()) {
    Record::addPacket(code, len, msg);
  }
}


//
// global variable callback
//
static void onGlobalChanged(const std::string& name, void*)
{
  // This Callback is removed in replay mode. As
  // well, the /set and /reset commands are blocked.

  std::string value = BZDB.get(name);
  void *bufStart = getDirectMessageBuffer();
  void *buf = nboPackUShort(bufStart, 1);
  buf = nboPackUByte(buf, name.length());
  buf = nboPackString(buf, name.c_str(), name.length());
  buf = nboPackUByte(buf, value.length());
  buf = nboPackString(buf, value.c_str(), value.length());
  broadcastMessage(MsgSetVar, (char*)buf - (char*)bufStart, bufStart);
}


static void sendUDPupdate(NetHandler *handler)
{
  // confirm inbound UDP with a TCP message
  directMessage(handler, MsgUDPLinkEstablished, 0, getDirectMessageBuffer());
  // request/test outbound UDP with a UDP back to where we got client's packet
  directMessage(handler, MsgUDPLinkRequest, 0, getDirectMessageBuffer());
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
  bool hide
    = (flag.flag.type->flagTeam == ::NoTeam)
    && !isIdentifyFlagIn
    && (flag.player == -1);
  buf = flag.pack(buf, hide);
  broadcastMessage(MsgFlagUpdate, (char*)buf - (char*)bufStart, bufStart,
		   false);
}


static float nextGameTime()
{
  float nextTime = +MAXFLOAT;
  const TimeKeeper nowTime = TimeKeeper::getCurrent();
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *gkPlayer = GameKeeper::Player::getPlayerByIndex(i);
    if ((gkPlayer != NULL) && gkPlayer->player.isHuman()) {
      const TimeKeeper& pTime = gkPlayer->getNextGameTime();
      const float pNextTime = (float)(pTime - nowTime);
      if (pNextTime < nextTime) {
        nextTime = pNextTime;
      }
    }
  }
  return nextTime;
}

static int makeGameTime(void* bufStart, float lag)
{
  void *buf = bufStart;
  buf = GameTime::pack(buf, lag);
  return ((char*)buf - (char*)bufStart);
}

static void sendGameTime(GameKeeper::Player* gkPlayer)
{
  if (Replay::enabled()) {
    return;
  }
  if (gkPlayer != NULL) {
    void* buf = getDirectMessageBuffer();
    const float lag = gkPlayer->lagInfo.getLagAvg();
    const int length = makeGameTime(buf, lag);
    directMessage(gkPlayer->netHandler, MsgGameTime, length, buf);
    gkPlayer->updateNextGameTime();
  }
  return;
}

static void sendPendingGameTime()
{
  const TimeKeeper nowTime = TimeKeeper::getCurrent();
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *gkPlayer = GameKeeper::Player::getPlayerByIndex(i);
    if ((gkPlayer != NULL)
	&& gkPlayer->player.isHuman()
	&& (gkPlayer->getNextGameTime() - nowTime) < 0.0f) {
      sendGameTime(gkPlayer);
    }
  }
  return;
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
	result = directMessage(playerData->netHandler, MsgFlagUpdate,
			       (char*)buf - (char*)bufStart, bufStart);
	if (result == -1)
	  return;
	cnt    = 0;
	length = sizeof(uint16_t);
	buf    = nboPackUShort(bufStart,0); //placeholder
      }

      bool hide
	= (flag.flag.type->flagTeam == ::NoTeam)
	&& !isIdentifyFlagIn
	&& (flag.player == -1);
      buf = flag.pack(buf, hide);
      length += sizeof(uint16_t)+FlagPLen;
      cnt++;
    }
  }

  if (cnt > 0) {
    nboPackUShort(bufStart, cnt);
    result = directMessage(playerData->netHandler, MsgFlagUpdate,
			   (char*)buf - (char*)bufStart, bufStart);
  }
}

void sendTeamUpdate(int teamIndex1, int teamIndex2)
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

  broadcastMessage(MsgTeamUpdate, (char*)buf - (char*)bufStart, bufStart,
		   false);
}

static int sendTeamUpdateD(NetHandler *handler)
{
  // send all teams
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, CtfTeams);
  for (int t = 0; t < CtfTeams; t++) {
    buf = nboPackUShort(buf, t);
    buf = team[t].team.pack(buf);
  }
  return directMessage(handler, MsgTeamUpdate,
		       (char*)buf - (char*)bufStart, bufStart);
}

static void sendPlayerUpdateB(GameKeeper::Player *playerData)
{
  if (!playerData->player.isPlaying())
    return;

  void *bufStart = getDirectMessageBuffer();
  void *buf      = playerData->packPlayerUpdate(bufStart);

  // send all players info about player[playerIndex]
  relayMessage(MsgAddPlayer, (char*)buf - (char*)bufStart, bufStart);
}

static int sendPlayerUpdateD(NetHandler *handler,
			     GameKeeper::Player *otherData)
{
  if (!otherData->player.isPlaying())
    return 0;

  void *bufStart = getDirectMessageBuffer();
  void *buf      = otherData->packPlayerUpdate(bufStart);

  return directMessage(handler, MsgAddPlayer,
		       (char*)buf - (char*)bufStart, bufStart);
}

void sendPlayerInfo() {
  void *buf, *bufStart = getDirectMessageBuffer();
  int i, numPlayers = 0;
  for (i = 0; i < int(NumTeams); i++)
    numPlayers += team[i].team.size;
  buf = nboPackUByte(bufStart, numPlayers);
  for (i = 0; i < curMaxPlayers; ++i) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData)
      continue;

    if (playerData->player.isPlaying()) {
      // see if any events want to update the playerInfo before it is sent out
      bz_GetPlayerInfoEventData playerInfoData;
      playerInfoData.playerID = i;
      playerInfoData.callsign = playerData->player.getCallSign();
      playerInfoData.team = convertTeam(playerData->player.getTeam());
      playerInfoData.verified = playerData->accessInfo.isVerified();
      playerInfoData.registered = playerData->accessInfo.isRegistered();
      playerInfoData.admin = playerData->accessInfo.showAsAdmin();

      worldEventManager.callEvents(bz_eGetPlayerInfoEvent,&playerInfoData);

      buf = PackPlayerInfo(buf,i,GetPlayerProperties(playerInfoData.registered,playerInfoData.verified,playerInfoData.admin));
    }
  }
  broadcastMessage(MsgPlayerInfo, (char*)buf - (char*)bufStart, bufStart);
}

void sendIPUpdate(int targetPlayer, int playerIndex) {
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
      Record::addPacket(MsgAdminInfo,
			(char*)buf - (char*)bufStart, bufStart, HiddenPacket);
    }
  } else {
    int ipsPerPackage = (MaxPacketLen - 3) / (PlayerIdPLen + 7);
    int i, c = 0;
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

void publicize()
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
      listServerLink = new ListServerLink(i->c_str(),
	    clOptions->publicizedAddress, clOptions->publicizedTitle, clOptions->advertiseGroups);
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

  // clean up Kerberos
  Authentication::cleanUp();
}


static void relayPlayerPacket(int index, uint16_t len, const void *rawbuf, uint16_t code)
{
  if (Record::enabled()) {
    Record::addPacket(code, len, (char*)rawbuf + 4);
  }

  // relay packet to all players except origin
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData)
      continue;
    PlayerInfo& pi = playerData->player;

    if (i != index && pi.isPlaying()) {
      pwrite(playerData->netHandler, rawbuf, len + 4);
    }
  }
}


static bool defineWorld()
{
  // clean up old database
  if (world) {
    delete world;
  }
  if (worldDatabase) {
    delete[] worldDatabase;
  }

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
		    << std::endl;
	  return false;
	}
      }
    }
  } else {
    // check and see if anyone wants to define the world from an event
    bz_GenerateWorldEventData	worldData;
    worldData.ctf  = clOptions->gameStyle & TeamFlagGameStyle;
    worldData.time = TimeKeeper::getCurrent().getSeconds();

    world = new WorldInfo;
    worldEventManager.callEvents(bz_eGenerateWorldEvent, &worldData);
    if (!worldData.handled) {
      delete world;
      if (clOptions->gameStyle & TeamFlagGameStyle)
	world = defineTeamWorld();
      else
	world = defineRandomWorld();
    } else {
      float worldSize = BZDBCache::worldSize;
      if (pluginWorldSize > 0)
	worldSize = pluginWorldSize;

      float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
      if (pluginWorldHeight > 0)
	wallHeight = pluginWorldHeight;

      world->addWall(0.0f, 0.5f * worldSize, 0.0f, (float)(1.5 * M_PI), 0.5f * worldSize, wallHeight);
      world->addWall(0.5f * worldSize, 0.0f, 0.0f, (float)M_PI, 0.5f * worldSize, wallHeight);
      world->addWall(0.0f, -0.5f * worldSize, 0.0f, (float)(0.5 * M_PI), 0.5f * worldSize, wallHeight);
      world->addWall(-0.5f * worldSize, 0.0f, 0.0f, 0.0f, 0.5f * worldSize, wallHeight);

      OBSTACLEMGR.makeWorld();
      world->finishWorld();
    }
  }

  if (world == NULL) {
    return false;
  }

  maxWorldHeight = world->getMaxWorldHeight();

  // package up world
  world->packDatabase();

  // now get world packaged for network transmission
  worldDatabaseSize = 4 + WorldCodeHeaderSize +
      world->getDatabaseSize() + 4 + WorldCodeEndSize;

  worldDatabase = new char[worldDatabaseSize];
  // this should NOT happen but it does sometimes
  if (!worldDatabase) {
    return false;
  }
  memset(worldDatabase, 0, worldDatabaseSize);

  void *buf = worldDatabase;
  buf = nboPackUShort(buf, WorldCodeHeaderSize);
  buf = nboPackUShort(buf, WorldCodeHeader);
  buf = nboPackUShort(buf, mapVersion);
  buf = nboPackUInt(buf, world->getUncompressedSize());
  buf = nboPackUInt(buf, world->getDatabaseSize());
  buf = nboPackString(buf, world->getDatabase(), world->getDatabaseSize());
  buf = nboPackUShort(buf, WorldCodeEndSize);
  buf = nboPackUShort(buf, WorldCodeEnd);

  TimeKeeper startTime = TimeKeeper::getCurrent();
  MD5 md5;
  md5.update((unsigned char *)worldDatabase, worldDatabaseSize);
  md5.finalize();
  if (clOptions->worldFile == "") {
    strcpy(hexDigest, "t");
  } else {
    strcpy(hexDigest, "p");
  }
  std::string digest = md5.hexdigest();
  strcat(hexDigest, digest.c_str());
  TimeKeeper endTime = TimeKeeper::getCurrent();
  DEBUG3("MD5 generation: %.3f seconds\n", endTime - startTime);

  // water levels probably require flags on buildings
  const float waterLevel = world->getWaterLevel();
  if (!clOptions->flagsOnBuildings && (waterLevel > 0.0f)) {
    clOptions->flagsOnBuildings = true;
    clOptions->respawnOnBuildings = true;
    DEBUG1("WARNING: enabling flag and tank spawns on buildings due to waterLevel\n");
  }

  // reset other stuff
  int i;
  for (i = 0; i < NumTeams; i++) {
    team[i].team.size = 0;
    team[i].team.won = 0;
    team[i].team.lost = 0;
  }
  FlagInfo::setNoFlagInAir();
  for (i = 0; i < numFlags; i++) {
    resetFlag(*FlagInfo::get(i));
  }

  return true;
}

static bool saveWorldCache()
{
  FILE* file;
  file = fopen (clOptions->cacheOut.c_str(), "wb");
  if (file == NULL) {
    return false;
  }
  size_t written =
    fwrite (worldDatabase, sizeof(char), worldDatabaseSize, file);
  fclose (file);
  if (written != worldDatabaseSize) {
    return false;
  }
  return true;
}

TeamColor whoseBase(float x, float y, float z)
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

static void handleTcp(NetHandler &netPlayer, int i, const RxStatus e);

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

  int keepalive = 1, n;
  n = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
		 (SSOType)&keepalive, sizeof(int));
  if (n < 0) {
    nerror("couldn't set keepalive");
  }

  PlayerId playerIndex;

  // find open slot in players list
  PlayerId minPlayerId = 0, maxPlayerId = (PlayerId)MaxPlayers;
  if (Replay::enabled()) {
     minPlayerId = MaxPlayers;
     maxPlayerId = MaxPlayers + ReplayObservers;
  }
  playerIndex = GameKeeper::Player::getFreeIndex(minPlayerId, maxPlayerId);

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
  new GameKeeper::Player(playerIndex, clientAddr, fd, handleTcp);

  // send the GameTime
  GameKeeper::Player* gkPlayer =
    GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (gkPlayer != NULL
      && gkPlayer->player.isHuman()) {
    sendGameTime(gkPlayer);
  }

  // if game was over and this is the first player then game is on
  if (gameOver) {
    int count = GameKeeper::Player::count();
    if (count == 0) {
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


void sendPlayerMessage(GameKeeper::Player *playerData, PlayerId dstPlayer,
		       const char *message)
{
  const PlayerId srcPlayer = playerData->getIndex();

  // reformat any '/me' action messages
  // this is here instead of in commands.cxx to allow player-player/player-channel targetted messages
  if (strncasecmp(message, "/me", 3) == 0) {

    // don't bother with empty messages
    if (message[3] == '\0' || (message[3] == ' ' && message[4] == '\0')) {
      char reply[MessageLen] = {0};
      sprintf(reply, "%s, the /me command requires an argument", playerData->player.getCallSign());
      sendMessage(ServerPlayer, srcPlayer, reply);
      return;
    }

    // don't intercept other messages beginning with /me...
    if (message[3] != ' ') {
      parseServerCommand(message, srcPlayer);
      return;
    }

    // check for permissions
    if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::actionMessage)) {
      char reply[MessageLen] = {0};
      sprintf(reply, "%s, you are not presently authorized to perform /me actions", playerData->player.getCallSign());
      sendMessage(ServerPlayer, srcPlayer, reply);
      return;
    }

    // format and send it
    std::string actionMsg = TextUtils::format("* %s %s\t*",
				playerData->player.getCallSign(), message + 4);
    message = actionMsg.c_str();
  }

  // check for a server command
  else if ((message[0] == '/') && (message[1] != '/')) {
    // record server commands
    if (Record::enabled()) {
      void *buf, *bufStart = getDirectMessageBuffer();
      buf = nboPackUByte(bufStart, srcPlayer);
      buf = nboPackUByte(buf, dstPlayer);
      buf = nboPackString(buf, message, strlen(message) + 1);
      Record::addPacket(MsgMessage, (char*)buf - (char*)bufStart, bufStart,
			 HiddenPacket);
    }
    parseServerCommand(message, srcPlayer);
    return; // bail out
  }

  // check if the player has permission to use the admin channel
  if ((dstPlayer == AdminPlayers) &&
      !playerData->accessInfo.hasPerm(PlayerAccessInfo::adminMessageSend)) {
    sendMessage(ServerPlayer, srcPlayer,
		"You do not have permission to speak on the admin channel.");
    return; // bail out
  }

  // check for bogus targets
  if ((dstPlayer < LastRealPlayer) && !realPlayer(dstPlayer)) {
    sendMessage(ServerPlayer, srcPlayer,
		"The player you tried to talk to does not exist!");
    return; // bail out
  }

  // filter the message, and send it
  sendFilteredMessage(srcPlayer, dstPlayer, message);

  return;
}

void sendFilteredMessage(int sendingPlayer, PlayerId recipientPlayer, const char *message)
{
  const char* msg = message;

  if (clOptions->filterChat) {
    char filtered[MessageLen] = {0};
    strncpy(filtered, message, MessageLen);
    if (clOptions->filterSimple) {
      clOptions->filter.filter(filtered, true);
    } else {
      clOptions->filter.filter(filtered, false);
    }
    msg = filtered;

	if (strcmp(message,filtered) != 0)	// the filter did do something so barf a message
	{
		bz_MessageFilteredEventData	eventData;

		eventData.player = sendingPlayer;
		eventData.time = TimeKeeper::getCurrent().getSeconds();
		eventData.rawMessage = message;
		eventData.filteredMessage = filtered;

		worldEventManager.callEvents(bz_eMessagFilteredEvent,&eventData);
	}
  }

  // check that the player has the talk permission
  GameKeeper::Player *senderData = GameKeeper::Player::getPlayerByIndex(sendingPlayer);

  if (!senderData) {
    return;
  }
  if (!senderData->accessInfo.hasPerm(PlayerAccessInfo::talk)) {

    // if the player were sending to is an admin
    GameKeeper::Player *recipientData = GameKeeper::Player::getPlayerByIndex(recipientPlayer);

    // don't care if they're real, just care if they're an admin
    if (recipientData && recipientData->accessInfo.isOperator()) {
      sendMessage(sendingPlayer, recipientPlayer, msg);
      return;
    } else if (recipientPlayer == AdminPlayers) {
      sendMessage(sendingPlayer, recipientPlayer, msg);
      return;
    }

    sendMessage(ServerPlayer, sendingPlayer, "We're sorry, you are not allowed to talk!");
    return; //bail out
  }

  sendMessage(sendingPlayer, recipientPlayer, msg);
}

void sendMessage(int playerIndex, PlayerId dstPlayer, const char *message)
{
  long int msglen = strlen(message) + 1; // include null terminator
  const char *msg = message;

  if (message[0] == '/' && message[1] == '/')
    msg = &message[1];

  if (msglen > MessageLen) {
    DEBUG1("WARNING: Network message being sent is too long! "
	   "(message is %d, cutoff at %d)\n", msglen, MessageLen);
    msglen = MessageLen;
  }

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUByte(buf, dstPlayer);
  buf = nboPackString(buf, msg, msglen);

  ((char*)bufStart)[MessageLen - 1] = '\0'; // always terminate

  int len = 2 + msglen;
  bool broadcast = false;

  if (dstPlayer <= LastRealPlayer) {
    GameKeeper::Player *playerData
      = GameKeeper::Player::getPlayerByIndex(dstPlayer);
    if (playerData && !playerData->player.isBot()) {
      directMessage(dstPlayer, MsgMessage, len, bufStart);
      if (playerIndex <= LastRealPlayer && dstPlayer != playerIndex)
	directMessage(playerIndex, MsgMessage, len, bufStart);
    }
  }
  // FIXME this teamcolor <-> player id conversion is in several files now
  else if (dstPlayer >= 244 && dstPlayer <= 250) {
    TeamColor _team = TeamColor(250 - dstPlayer);
    // send message to all team members only
    GameKeeper::Player *playerData;
    for (int i = 0; i < curMaxPlayers; i++)
      if ((playerData = GameKeeper::Player::getPlayerByIndex(i))
	  && playerData->player.isPlaying()
	  && playerData->player.isTeam(_team))
	directMessage(i, MsgMessage, len, bufStart);
  } else if (dstPlayer == AdminPlayers){
    // admin messages

    // Notify any plugins
    if (playerIndex == ServerPlayer) {
      bz_ServerMsgEventData serverMsgData;
      serverMsgData.to = BZ_NULLUSER;
      serverMsgData.team = eAdministrators;
      serverMsgData.message = message;
      serverMsgData.time = TimeKeeper::getCurrent().getSeconds();
      worldEventManager.callEvents(bz_eServerMsgEvent, &serverMsgData);
    }

    std::vector<int> admins
      = GameKeeper::Player::allowed(PlayerAccessInfo::adminMessageReceive);
    for (unsigned int i = 0; i < admins.size(); ++i)
      directMessage(admins[i], MsgMessage, len, bufStart);

  } else {
    // message to all players

    // Notify any plugins
    if (playerIndex == ServerPlayer) {
      bz_ServerMsgEventData serverMsgData;
      serverMsgData.to = BZ_ALLUSERS;
      serverMsgData.message = message;
      serverMsgData.time = TimeKeeper::getCurrent().getSeconds();
      worldEventManager.callEvents(bz_eServerMsgEvent, &serverMsgData);
    }

    broadcastMessage(MsgMessage, len, bufStart);
    broadcast = true;
  }

  if (Record::enabled() && !broadcast) { // don't record twice
    Record::addPacket(MsgMessage, len, bufStart, HiddenPacket);
  }
}

static void rejectPlayer(int playerIndex, uint16_t code, const char *reason)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, code);
  buf = nboPackString(buf, reason, strlen(reason) + 1);
  directMessage(playerIndex, MsgReject, sizeof (uint16_t) + MessageLen, bufStart);
  // Fixing security hole, because a client can ignore the reject message
  // then he can avoid a ban, hostban...
  removePlayer(playerIndex, "/rejected", true);
  return;
}

// FIXME this is a workaround for a bug, still needed?
// Team Size is wrong at some time
static void fixTeamCount() {
  int playerIndex, teamNum;
  for (teamNum = RogueTeam; teamNum < NumTeams; teamNum++)
    team[teamNum].team.size = 0;
  for (playerIndex = 0; playerIndex < curMaxPlayers; playerIndex++) {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(playerIndex);
    if (p && p->player.isPlaying()) {
      teamNum = p->player.getTeam();
      if (teamNum == RabbitTeam)
	teamNum = HunterTeam;
      team[teamNum].team.size++;
    }
  }
}


// helper struct and predicates used in autoTeamSelect()
struct TeamSize {
  TeamColor color;
  int       current;
  int       max;
  bool operator < (const TeamSize x) const { return x.current < current; }
};

bool teamFull(const TeamSize &x)  { return x.current == x.max; }
bool teamEmpty(const TeamSize &x) { return x.current == 0; }

struct teamHasSize
{
  int sz;
  teamHasSize(int sz_) : sz(sz_) {}
  bool operator()(const TeamSize &x) const { return x.current == sz; }
};

struct teamHasntSize
{
  int sz;
  teamHasntSize(int sz_) : sz(sz_) {}
  bool operator()(const TeamSize &x) const { return x.current != sz; }
};


static TeamColor teamSelect(TeamColor t, const std::vector<TeamSize> &teams)
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
  for (i = 0; i < int(NumTeams); i++)
    if (i != int(ObserverTeam))
      numplayers += team[i].team.size;

  // if no player are available, join as Observer
  if (numplayers == maxRealPlayers)
    return ObserverTeam;

  // if we're running rabbit chase, all non-observers start as hunters
  if (clOptions->gameStyle & int(RabbitChaseGameStyle))
    return HunterTeam;

  // If tank ask for rogues, and rogues are allowed, give it
  if ((t == RogueTeam)
      && team[RogueTeam].team.size < clOptions->maxTeam[RogueTeam])
    return RogueTeam;

  // If no auto-team, server or client, go back with client choice
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
  if (teams.empty())
    return RogueTeam;

  // Sort it by current team number
  std::sort(teams.begin(), teams.end());

  // all teams are empty, select just one of them
  if (teams[0].current == 0)
    return teamSelect(t, teams);

  int maxTeamSize = teams[0].current;

  teams.erase(std::remove_if(teams.begin(), teams.end(), teamFull), teams.end());
  // no non-full teams? then there must be a free rogue spot
  if (teams.empty())
    return RogueTeam;

  bool unBalanced = (teams.back().current < maxTeamSize);

  if (unBalanced) {
    // if you come with a 1-1-x-x try to add a player to these 1 to have team
    if ((maxTeamSize == 1) && (teams.size() >= 2) && (teams[1].current == 1)) {
      // remove empty teams
      teams.erase(std::remove_if(teams.begin(), teams.end(), teamEmpty), teams.end());
    } else {
      // remove biggest teams
      teams.erase(std::remove_if(teams.begin(), teams.end(), teamHasSize(maxTeamSize)), teams.end());
      // Search for the lowest existing team and remove uppers. If there
      // are non empty teams don't start a new team but try to balance the lower
      if (teams[0].current > 0) {
	teams.erase(std::remove_if(teams.begin(), teams.end(), teamEmpty), teams.end());
	const int lowerTeam = teams.back().current;
	teams.erase(std::remove_if(teams.begin(), teams.end(), teamHasntSize(lowerTeam)), teams.end());
      }
    }
  }
  return teamSelect(t, teams);
}

static std::string evaluateString(const std::string &raw)
{
  std::string eval;
  const int rawLen = (int)raw.size();
  for (int i = 0; i < rawLen; i++) {
    char current = raw[i];
    if (current != '\\') {
      eval += current;
    } else {
      char next = raw[i+1];
      switch (next) {
	case '\\' : {
	  eval += '\\';
	  i++;
	  break;
	}
	case 'n' : {
	  eval += "\\n";
	  i++;
	  break;
	}
	case '{' : {
	  unsigned int start = (i + 2);
	  std::string::size_type end = raw.find_first_of('}', start);
	  if (end == std::string::npos) {
	    i = rawLen; // unterminated, ignore the rest of the string
	  } else {
	    const std::string var = raw.substr(start, end - start);
	    i += (end - start) + 2;
	    if (BZDB.isSet(var)) {
	      eval += BZDB.get(var);
	    } else {
	      eval += "*BADBZDB*";
	    }
	  }
	  break;
	}
	case '(' : {
	  unsigned int start = (i + 2);
	  std::string::size_type end = raw.find_first_of(')', start);
	  if (end == std::string::npos) {
	    i = rawLen; // unterminated, ignore the rest of the string
	  } else {
	    const std::string var = raw.substr(start, end - start);
	    i += (end - start) + 2;
	    if (var == "uptime") {
	      char buffer[16];
	      const float uptime = (float)(TimeKeeper::getCurrent() - TimeKeeper::getStartTime());
	      snprintf(buffer, 16, "%i", (int)uptime);
	      eval += buffer;
	    }
	    else {
	      eval += "*BADVAR*";
	    }
	  }
	  break;
	}
	default: {
	  break;
	}
      }
    }
  }
  return eval;
}

static bool spawnSoon = false;

static void playerAlive(int playerIndex);

static void addPlayer(int playerIndex, GameKeeper::Player *playerData)
{
  uint16_t rejectCode;
  char rejectMsg[MessageLen];
  // check for a name clash
  bool resultEnter = playerData->loadEnterData(rejectCode, rejectMsg);

  // Name clash ... if the new player is not verified, reject it
  // We cannot have 2 players with same nick
  if (!resultEnter && playerData->_LSAState != GameKeeper::Player::verified) {
    rejectPlayer(playerIndex, rejectCode, rejectMsg);
    return;
  }

  if (!resultEnter) {
    // Find the user already logged on and kick it. The new player
    // has been globally authenticated.
    for (int i = 0; i < curMaxPlayers; i++) {
      // don't kick _us_, kick the other guy
      if (playerIndex == i)
	continue;
      GameKeeper::Player *otherPlayer
	= GameKeeper::Player::getPlayerByIndex(i);
      if (!otherPlayer)
	continue;
      if (strcasecmp(otherPlayer->player.getCallSign(),
		     playerData->player.getCallSign()) == 0) {
	sendMessage(ServerPlayer, i ,
		    "Another client has demonstrated ownership of your "
		    "callsign with the correct password.  You have been "
		    "ghosted.");
	removePlayer(i, "Ghost", true);
	break;
      }
    }
  }

  if (clOptions->filterCallsigns) {
    int filterIndex = 0;
    Filter::Action filterAction = filter.check(*playerData, filterIndex);
    if (filterAction == Filter::DROP) {
      rejectPlayer(playerIndex, RejectBadCallsign, "Player has been banned");
      return;
    }
  }

  const bool playerIsAntiBanned =
    playerData->accessInfo.hasPerm(PlayerAccessInfo::antiban);

  // check against the ip ban list
  in_addr playerIP = playerData->netHandler->getIPAddress();
  BanInfo info(playerIP);
  if (!playerIsAntiBanned && !clOptions->acl.validate(playerIP,&info)) {
    std::string rejectionMessage;

    rejectionMessage = BanRefusalString;
    if (info.reason.size ())
      rejectionMessage += info.reason;
    else
      rejectionMessage += "General Ban";

    rejectionMessage += ColorStrings[WhiteColor];
    if (info.bannedBy.size ()) {
      rejectionMessage += " by ";
      rejectionMessage += ColorStrings[BlueColor];
      rejectionMessage += info.bannedBy;
    }

    rejectionMessage += ColorStrings[GreenColor];
    if (info.fromMaster)
      rejectionMessage += " [you are on the master ban list]";

    rejectPlayer (playerIndex, RejectIPBanned, rejectionMessage.c_str ());
    return;
  }

  // check against the id ban list
  const std::string& bzid = playerData->getBzIdentifier();
  IdBanInfo idInfo("");
  if (!playerIsAntiBanned && !clOptions->acl.idValidate(bzid.c_str(), &idInfo)) {
    std::string rejectionMessage;
    
    rejectionMessage = BanRefusalString;
    if (idInfo.reason.size()) {
      rejectionMessage += idInfo.reason;
    } else {
      rejectionMessage += "General Ban";
    }

    rejectionMessage += ColorStrings[WhiteColor];
    if (idInfo.bannedBy.size()) {
      rejectionMessage += " by ";
      rejectionMessage += ColorStrings[BlueColor];
      rejectionMessage += idInfo.bannedBy;
    }

    rejectionMessage += ColorStrings[GreenColor];
    if (idInfo.fromMaster) {
      rejectionMessage += " [from the master server]";
    }
    rejectPlayer(playerIndex, RejectIDBanned, rejectionMessage.c_str());
    return;
  }
  
  // check against id and hostname ban lists (on the next cycle)
  playerData->setNeedThisHostbanChecked(true);

  // see if any watchers don't want this guy
  bz_AllowPlayerEventData allowData;
  allowData.callsign = playerData->player.getCallSign();
  allowData.ipAddress = playerData->netHandler->getTargetIP();
  allowData.playerID = playerIndex;
  allowData.time = TimeKeeper::getCurrent().getSeconds();

  worldEventManager.callEvents(bz_eAllowPlayer,&allowData);
  if (!allowData.allow) {
    rejectPlayer(playerIndex, RejectBadRequest, allowData.reason.c_str());
    return;
  }

  // pick a team
  TeamColor t = autoTeamSelect(playerData->player.getTeam());

  bz_GetAutoTeamEventData autoTeamData;
  autoTeamData.playeID = playerIndex;
  autoTeamData.team = convertTeam(t);
  autoTeamData.callsign = playerData->player.getCallSign();

  worldEventManager.callEvents(bz_eGetAutoTeamEvent,&autoTeamData);

  playerData->player.setTeam(convertTeam(autoTeamData.team));
  playerData->player.endShotCredit = 0;	// reset shotEndCredit

  // count current number of players and players+observers
  int numplayers = 0;
  for (int i = 0; i < int(ObserverTeam); i++)
    numplayers += team[i].team.size;
  const int numplayersobs = numplayers + team[ObserverTeam].team.size;

  // no quick rejoining, make 'em wait
  // you can switch to observer immediately, or switch from observer
  // to regular player immediately, but only if last time time you
  // were a regular player isn't in the rejoin list. As well, this all
  // only applies if the game isn't currently empty.
  if ((playerData->player.getTeam() != ObserverTeam) &&
      (GameKeeper::Player::count() >= 0)) {
    float waitTime = rejoinList.waitTime (playerIndex);
    if (waitTime > 0.0f) {
      char buffer[MessageLen];
      DEBUG2 ("Player %s [%d] rejoin wait of %.1f seconds\n",
	      playerData->player.getCallSign(), playerIndex, waitTime);
      snprintf (buffer, MessageLen, "You are unable to begin playing for %.1f seconds.", waitTime);
      sendMessage(ServerPlayer, playerIndex, buffer);
    }
  }

  // reject player if asks for bogus team or rogue and rogues aren't allowed
  // or if the team is full or if the server is full
  if (!playerData->player.isHuman()
      && !playerData->player.isBot()
      && !playerData->player.isChat()) {
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
  NetHandler *netHandler = playerData->netHandler;

  // accept player
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  int result = directMessage(netHandler, MsgAccept,
			     (char*)buf-(char*)bufStart, bufStart);
  if (result < 0)
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
  if (playerData->player.isHuman()) {
    result = sendTeamUpdateD(netHandler);
    if (result < 0)
      return;
    sendFlagUpdate(playerIndex);
  }
  if (!playerData->player.isBot()) {
    GameKeeper::Player *otherData;
    for (int i = 0; i < curMaxPlayers; i++) {
      if (i == playerIndex)
	continue;
      otherData = GameKeeper::Player::getPlayerByIndex(i);
      if (!otherData)
	continue;
      result = sendPlayerUpdateD(playerData->netHandler, otherData);
      if (result < 0)
	break;
    }
    
    if (clOptions->gameStyle & HandicapGameStyle) {
      int numHandicaps = 0;

      // Send handicap for all players
      bufStart = getDirectMessageBuffer();
      buf = nboPackUByte(bufStart, numHandicaps);
      for (int i = 0; i < curMaxPlayers
	     && GameKeeper::Player::getPlayerByIndex(playerIndex); i++)
	if (i != playerIndex) {
	  otherData = GameKeeper::Player::getPlayerByIndex(i);
	  if (otherData) {
	    numHandicaps++;
	    buf = nboPackUByte(buf, i);
	    buf = nboPackShort(buf, otherData->score.getHandicap());
	  }
	}
      nboPackUByte(bufStart, numHandicaps);
      broadcastMessage(MsgHandicap, (char*)buf-(char*)bufStart, bufStart);
    }
  }

  // if new player connection was closed (because of an error) then stop here
  if (!GameKeeper::Player::getPlayerByIndex(playerIndex))
    return;

  // send MsgAddPlayer to everybody -- this concludes MsgEnter response
  // to joining player
  sendPlayerUpdateB(playerData);

  // send update of info for team just joined
  sendTeamUpdate(teamIndex);

  // send IP update to everyone with PLAYERLIST permission
  sendIPUpdate(-1, playerIndex);
  sendIPUpdate(playerIndex, -1);

  // send rabbit information
  if (clOptions->gameStyle & int(RabbitChaseGameStyle)) {
    bufStart = getDirectMessageBuffer();
    buf = nboPackUByte(bufStart, rabbitIndex);
    directMessage(playerIndex, MsgNewRabbit, (char*)buf-(char*)bufStart, bufStart);
  }

  // again check if player was disconnected
  if (!GameKeeper::Player::getPlayerByIndex(playerIndex))
    return;

  // send time update to new player if we're counting down
  if (countdownActive && clOptions->timeLimit > 0.0f
      && !playerData->player.isBot()) {
    float timeLeft = clOptions->timeLimit - (float)(TimeKeeper::getCurrent() - gameStartTime);
    if (timeLeft < 0.0f) {
      // oops
      timeLeft = 0.0f;
    }

    bufStart = getDirectMessageBuffer();
    buf = nboPackInt(bufStart, (int32_t)timeLeft);
    result = directMessage(playerData->netHandler, MsgTimeUpdate,
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
    const std::string srvmsg = evaluateString(clOptions->servermsg);

    // split the servermsg into several lines if it contains '\n'
    const char* i = srvmsg.c_str();
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
      const std::string srvmsg = evaluateString((*lines)[i]);
      sendMessage(ServerPlayer, playerIndex, srvmsg.c_str());
    }
  }

  if (playerData->player.isObserver())
    sendMessage(ServerPlayer, playerIndex, "You are in observer mode.");
#endif


  if (GameKeeper::Player::getPlayerByIndex(playerIndex)
      && playerData->accessInfo.isRegistered()
      && playerData->_LSAState != GameKeeper::Player::verified) {
    // nick is in the DB send him a message to identify.
    if (playerData->accessInfo.isIdentifyRequired()) {
      sendMessage(ServerPlayer, playerIndex,
		  "This callsign is registered.  "
		  "You must identify yourself before playing.");
    } else {
      sendMessage(ServerPlayer, playerIndex, "This callsign is registered.");
    }
    if (playerData->accessInfo.hasRealPassword())
      sendMessage(ServerPlayer, playerIndex, "Identify with /identify <your password>");
  }

  dropAssignedFlag(playerIndex);

  sendPlayerInfo();

  // call any on join events
  bz_PlayerJoinPartEventData	joinEventData;
  joinEventData.eventType = bz_ePlayerJoinEvent;
  joinEventData.playerID = playerIndex;
  joinEventData.team = convertTeam(playerData->player.getTeam());
  joinEventData.callsign = playerData->player.getCallSign();
  joinEventData.time = TimeKeeper::getCurrent().getSeconds();

  if ((joinEventData.team != eNoTeam) && (joinEventData.callsign.size() != 0))	// don't give events if we don't have a real player slot
    worldEventManager.callEvents(bz_ePlayerJoinEvent,&joinEventData);
  if (spawnSoon)
    playerAlive(playerIndex);
}


void resetFlag(FlagInfo &flag)
{
  // NOTE -- must not be called until world is defined
  assert(world != NULL);

  float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);

  // reposition flag (middle of the map might be a bad idea)
  float flagPos[3] = {0.0f, 0.0f, 0.0f};

  int teamIndex = flag.teamIndex();
  if ((teamIndex >= ::RedTeam) &&  (teamIndex <= ::PurpleTeam)
      && (bases.find(teamIndex) != bases.end())) {
    if (!world->getFlagSpawnPoint(&flag, flagPos)) {
      // return the flag to the center of the top of one of the team
      // bases.. we assume it'll fit.
      TeamBases &teamBases = bases[teamIndex];
      const TeamBase &base = teamBases.getRandomBase(flag.getIndex());
      flagPos[0] = base.position[0];
      flagPos[1] = base.position[1];
      flagPos[2] = base.position[2] + base.size[2];
    }
  } else {
    // random position (not in a building)
    const float waterLevel = world->getWaterLevel();
    float minZ = 0.0f;
    if (waterLevel > minZ) {
      minZ = waterLevel;
    }
    float maxZ = MAXFLOAT;
    if (!clOptions->flagsOnBuildings) {
      maxZ = 0.0f;
    }
    float worldSize = BZDBCache::worldSize;
    int i;
    for (i = 0; i < 10000; i++) {
      if (!world->getFlagSpawnPoint(&flag, flagPos)) {
	flagPos[0] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	flagPos[1] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	flagPos[2] = world->getMaxWorldHeight() * (float)bzfrand();
      }
      if (DropGeometry::dropFlag(flagPos, minZ, maxZ)) {
	break;
      }
    }
    if (i == 10000) {
      std::cerr << "Unable to position flags on this world.\n";
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
  buf	    = flag.pack(buf);
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


static void pausePlayer(int playerIndex, bool paused = true)
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

  bz_PlayerPausedEventData	pauseEventData;
  pauseEventData.player = playerIndex;
  pauseEventData.time = TimeKeeper::getCurrent().getSeconds();

  worldEventManager.callEvents(bz_ePlayerPausedEvent,&pauseEventData);
}

static void autopilotPlayer(int playerIndex, bool autopilot)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  playerData->player.setAutoPilot(autopilot);

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUByte(buf, autopilot);
  broadcastMessage(MsgAutoPilot, (char*)buf-(char*)bufStart, bufStart);
}

void zapFlagByPlayer(int playerIndex)
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
    dropPlayerFlag(*playerData, playerData->lastState.pos);
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

  // call any on part events
  bz_PlayerJoinPartEventData partEventData;
  partEventData.eventType = bz_ePlayerPartEvent;
  partEventData.playerID = playerIndex;
  partEventData.team = convertTeam(playerData->player.getTeam());
  partEventData.callsign = playerData->player.getCallSign();
  partEventData.time = TimeKeeper::getCurrent().getSeconds();
  if (reason)
    partEventData.reason = reason;

  if ((partEventData.team != eNoTeam) && (partEventData.callsign.size() != 0))	// don't give events if we don't have a real player slot
    worldEventManager.callEvents(bz_ePlayerPartEvent,&partEventData);

  if (notify) {
    // send a super kill to be polite
    // send message to one player
    // do not use directMessage as he can remove player
    void *buf  = sMsgBuf;
    buf	= nboPackUShort(buf, 0);
    buf	= nboPackUShort(buf, MsgSuperKill);
    playerData->netHandler->pwrite(sMsgBuf, 4);
  }


  // if there is an active poll, cancel any vote this player may have made
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");
  if ((arbiter != NULL) && (arbiter->knowsPoll())) {
    arbiter->retractVote(std::string(playerData->player.getCallSign()));
  }

  // status message
  std::string timeStamp = TimeKeeper::timestamp();
  DEBUG1("Player %s [%d] removed at %s: %s\n",
	 playerData->player.getCallSign(),
	 playerIndex, timeStamp.c_str(), reason);
  bool wasPlaying = playerData->player.isPlaying();
  playerData->netHandler->closing();

  zapFlagByPlayer(playerIndex);

  // player is outta here.  if player never joined a team then
  // don't count as a player.

  if (wasPlaying) {
    // make them wait from the time they left, but only if they are
    // not already waiting, and they are not currently an observer.
    if ((playerData->player.getTeam() != ObserverTeam) &&
	(rejoinList.waitTime (playerIndex) <= 0.0f) &&
	!playerData->accessInfo.hasPerm(PlayerAccessInfo::rejoin)) {
      rejoinList.add (playerIndex);
    }

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
    sendTeamUpdate(teamNum);
  }

  playerData->close();

  if (wasPlaying) {
    // 'fixing' the count after deleting player
    fixTeamCount();

    // tell the list server the new number of players
    listServerLink->queueMessage(ListServerLink::ADD);
  }

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
  return team1!=team2 || (team1==RogueTeam);
}


static void sendWorld(NetHandler *handler, uint32_t ptr)
{
  playerHadWorld = true;
  // send another small chunk of the world database
  assert((world != NULL) && (worldDatabase != NULL));
  void *buf, *bufStart = getDirectMessageBuffer();
  uint32_t size = MaxPacketLen - 2*sizeof(uint16_t) - sizeof(uint32_t);
  uint32_t left = worldDatabaseSize - ptr;
  if (ptr >= worldDatabaseSize) {
    size = 0;
    left = 0;
  } else if (ptr + size >= worldDatabaseSize) {
    size = worldDatabaseSize - ptr;
    left = 0;
  }
  buf = nboPackUInt(bufStart, uint32_t(left));
  buf = nboPackString(buf, (char*)worldDatabase + ptr, size);
  directMessage(handler, MsgGetWorld, (char*)buf - (char*)bufStart, bufStart);
}


static void makeGameSettings()
{
  void* buf = worldSettings;

  // the header
  buf = nboPackUShort (buf, WorldSettingsSize); // length
  buf = nboPackUShort (buf, MsgGameSettings);   // code

  // the settings
  buf = nboPackFloat  (buf, BZDBCache::worldSize);
  buf = nboPackUShort (buf, clOptions->gameStyle);
  // An hack to fix a bug on the client
  buf = nboPackUShort (buf, PlayerSlot);
  buf = nboPackUShort (buf, clOptions->maxShots);
  buf = nboPackUShort (buf, numFlags);
  buf = nboPackUShort (buf, clOptions->shakeTimeout);
  buf = nboPackUShort (buf, clOptions->shakeWins);
  buf = nboPackUInt   (buf, 0); // FIXME - used to be sync time

  return;
}


static void sendQueryGame(NetHandler *handler)
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
  buf = nboPackUShort(buf, team[5].team.size);
  buf = nboPackUShort(buf, pingReply.rogueMax);
  buf = nboPackUShort(buf, pingReply.redMax);
  buf = nboPackUShort(buf, pingReply.greenMax);
  buf = nboPackUShort(buf, pingReply.blueMax);
  buf = nboPackUShort(buf, pingReply.purpleMax);
  buf = nboPackUShort(buf, pingReply.observerMax);
  buf = nboPackUShort(buf, pingReply.shakeWins);
  // 1/10ths of second
  buf = nboPackUShort(buf, pingReply.shakeTimeout);
  buf = nboPackUShort(buf, pingReply.maxPlayerScore);
  buf = nboPackUShort(buf, pingReply.maxTeamScore);
  buf = nboPackUShort(buf, pingReply.maxTime);
  buf = nboPackUShort(buf, (uint16_t)clOptions->timeElapsed);

  // send it
  directMessage(handler, MsgQueryGame, (char*)buf-(char*)bufStart, bufStart);
}

static void sendQueryPlayers(NetHandler *handler)
{
  int result;
  // count the number of active players
  int numPlayers = GameKeeper::Player::count();

  // first send number of teams and players being sent
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort(bufStart, NumTeams);
  buf = nboPackUShort(buf, numPlayers);
  result = directMessage(handler, MsgQueryPlayers,
			 (char*)buf-(char*)bufStart, bufStart);
  if (result < 0)
    return;

  // now send the teams and players
  result = sendTeamUpdateD(handler);
  if (result < 0)
    return;
  GameKeeper::Player *otherData;
  for (int i = 0; i < curMaxPlayers; i++) {
    otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (!otherData)
      continue;
    result = sendPlayerUpdateD(handler, otherData);
    if (result < 0)
      return;
  }
}

static void playerAlive(int playerIndex)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  if (!playerData->player.isPlaying()) {
    spawnSoon = true;
    return;
  }
  spawnSoon = false;
  // ignore multiple MsgAlive; also observer should not send MsgAlive;
  // diagnostic?
  if (playerData->player.isAlive() || playerData->player.isObserver())
    return;

  // make sure the user identifies themselves if required.
  if (!playerData->accessInfo.isAllowedToEnter()) {
    sendMessage(ServerPlayer, playerIndex, "This callsign is registered.  You must identify yourself");
    sendMessage(ServerPlayer, playerIndex, "before playing or use a different callsign.");
    removePlayer(playerIndex, "unidentified", true);
    return;
  }

  bz_AllowSpawnData	spawnAllowData;
  spawnAllowData.playerID = playerIndex;
  spawnAllowData.team = convertTeam(playerData->player.getTeam());

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::spawn)) {
    sendMessage(ServerPlayer, playerIndex, "You do not have permission to spawn on this server.");
    sendMessage(ServerPlayer, playerIndex, "This server may require identification before you can join.");
    sendMessage(ServerPlayer, playerIndex, "Please use /identify, or /register if you have not registerd your callsign or");
    sendMessage(ServerPlayer, playerIndex, "register on http://my.BZFlag.org/bb/ and use that callsign/password.");
	spawnAllowData.allow = false;
	}

  if (playerData->player.isBot()
      && BZDB.isTrue(StateDatabase::BZDB_DISABLEBOTS)) {
    sendMessage(ServerPlayer, playerIndex, "I'm sorry, we do not allow bots on this server.");
    removePlayer(playerIndex, "ComputerPlayer", true);
    return;
  }

  // check for any spawn allow events
  worldEventManager.callEvents(bz_eAllowSpawn,&spawnAllowData);

  if(!spawnAllowData.allow)
  {
    // client won't send another enter so kick em =(
    removePlayer(playerIndex, "Not allowed to spawn", true);
    return;
  }

  // player is coming alive.
  dropAssignedFlag(playerIndex);

  // get the spawn position
  SpawnPosition spawnPosition
    (playerIndex,
     (!clOptions->respawnOnBuildings) || (playerData->player.isBot()),
     clOptions->gameStyle & TeamFlagGameStyle);

  // see if there is anyone to handle the spawn event, and if they want to change it.
  bz_GetPlayerSpawnPosEventData	spawnData;
  spawnData.playerID = playerIndex;
  spawnData.team   = convertTeam(playerData->player.getTeam());
  spawnData.pos[0]   = spawnPosition.getX();
  spawnData.pos[1]   = spawnPosition.getY();
  spawnData.pos[2]   = spawnPosition.getZ();
  spawnData.rot      = spawnPosition.getAzimuth();

  worldEventManager.callEvents(bz_eGetPlayerSpawnPosEvent,&spawnData);

  // update last position immediately
  playerData->setPlayerState(spawnData.pos, spawnData.rot);

  // send MsgAlive
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackVector(buf, playerData->lastState.pos);
  buf = nboPackFloat(buf, playerData->lastState.azimuth);
  broadcastMessage(MsgAlive, (char*)buf - (char*)bufStart, bufStart);

  // call any events for a playerspawn
  bz_PlayerSpawnEventData	spawnEvent;
  spawnEvent.playerID = playerIndex;
  spawnEvent.team = convertTeam(playerData->player.getTeam());

  playerData->getPlayerState(spawnEvent.pos, spawnEvent.rot);

  worldEventManager.callEvents(bz_ePlayerSpawnEvent,&spawnEvent);

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
	if (clOptions->oneGameOnly) {
	  done = true;
	  exitCode = 0;
	}
  }
}

// FIXME - needs extra checks for killerIndex=ServerPlayer (world weapons)
// (was broken before); it turns out that killerIndex=-1 for world weapon?
// No need to check on victimIndex.
//   It is taken as the index of the udp table when called by incoming message
//   It is taken by killerIndex when autocalled, but only if != -1
// killer could be InvalidPlayer or a number within [0 curMaxPlayer)
void playerKilled(int victimIndex, int killerIndex, int reason,
			int16_t shotIndex, const FlagType* flagType, int phydrv, bool respawnOnBase )
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

  victim->setRestartOnBase(respawnOnBase);
  victim->setDead();

  // call any events for a playerdeath
  bz_PlayerDieEventData	dieEvent;
  dieEvent.playerID = victimIndex;
  dieEvent.team = convertTeam(victim->getTeam());
  dieEvent.killerID = killerIndex;
  if (killer)
    dieEvent.killerTeam = convertTeam(killer->getTeam());
  dieEvent.flagKilledWith = flagType->flagAbbv;
  victimData->getPlayerState(dieEvent.pos, dieEvent.rot);

  worldEventManager.callEvents(bz_ePlayerDieEvent,&dieEvent);

  // killing rabbit or killing anything when I am a dead ex-rabbit is allowed
  bool teamkill = false;
  if (killer) {
    const bool rabbitinvolved = killer->isARabbitKill(*victim);
    const bool foe = areFoes(victim->getTeam(), killer->getTeam());
    teamkill = !foe && !rabbitinvolved;
  }

  // send MsgKilled
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, victimIndex);
  buf = nboPackUByte(buf, killerIndex);
  buf = nboPackShort(buf, reason);
  buf = nboPackShort(buf, shotIndex);
  buf = flagType->pack(buf);
  if (reason == PhysicsDriverDeath) {
    buf = nboPackInt(buf, phydrv);
  }
  broadcastMessage(MsgKilled, (char*)buf-(char*)bufStart, bufStart);

  // update tk-score
  if ((victimIndex != killerIndex) && teamkill) {
    killerData->score.tK();
    if (killerData->score.isTK()) {
      char message[MessageLen];
      strcpy(message, "You have been automatically kicked for team killing" );
      sendMessage(ServerPlayer, killerIndex, message);
      snprintf(message, MessageLen, "Player %s removed: team killing", killerData->player.getCallSign());
      sendMessage(ServerPlayer, AdminPlayers, message);
      removePlayer(killerIndex, "teamkilling", true);
    }
  }

  // zap flag player was carrying.  clients should send a drop flag
  // message before sending a killed message, so this shouldn't happen.
  zapFlagByPlayer(victimIndex);

  victimData = GameKeeper::Player::getPlayerByIndex(victimIndex);
  // victimData will be NULL if the player has been kicked for TK'ing
  // so don't bother doing any score stuff for him
  if (victimData != NULL) {
    // change the player score
    bufStart = getDirectMessageBuffer();
    victimData->score.killedBy();
    if (killer) {
      if (victimIndex != killerIndex) {
	if (teamkill) {
	  if (clOptions->teamKillerDies)
	    playerKilled(killerIndex, killerIndex, reason, -1, Flags::Null, -1);
	  else
	    killerData->score.killedBy();
	} else {
	  killerData->score.kill();
	}
      }
      buf = nboPackUByte(bufStart, 2);
      buf = nboPackUByte(buf, killerIndex);
      buf = killerData->score.pack(buf);
    } else {
      buf = nboPackUByte(bufStart, 1);
    }

    buf = nboPackUByte(buf, victimIndex);
    buf = victimData->score.pack(buf);
    broadcastMessage(MsgScore, (char*)buf-(char*)bufStart, bufStart);

    if (clOptions->gameStyle & HandicapGameStyle) {
      bufStart = getDirectMessageBuffer();
      if (killer) {
	buf = nboPackUByte(bufStart, 2);
	buf = nboPackUByte(buf, killerIndex);
	buf = nboPackShort(buf, killerData->score.getHandicap());
      } else {
	buf = nboPackUByte(bufStart, 1);
      }
      buf = nboPackUByte(buf, victimIndex);
      buf = nboPackShort(buf, victimData->score.getHandicap());
      broadcastMessage(MsgHandicap, (char*)buf-(char*)bufStart, bufStart);
    }

    // see if the player reached the score limit
    if (clOptions->maxPlayerScore != 0
	&& killerIndex != InvalidPlayer
	&& killerIndex != ServerPlayer
	&& killerData->score.reached()) {
      bufStart = getDirectMessageBuffer();
      buf = nboPackUByte(bufStart, killerIndex);
      buf = nboPackUShort(buf, uint16_t(NoTeam));
      broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
      gameOver = true;
      if (clOptions->oneGameOnly) {
        done = true;
        exitCode = 0;
      }
    }
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
      sendTeamUpdate(int(victim->getTeam()), killerTeam);
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
  if (!playerData ||
      playerData->player.isObserver() ||
      !playerData->player.isAlive() ||
      playerData->player.haveFlag() ||
      flag.flag.status != FlagOnGround)
    return;

  //last Pos might be lagged by TankSpeed so include in calculation
  const float tankRadius = BZDBCache::tankRadius;
  const float tankSpeed = BZDBCache::tankSpeed;
  const float radius2 = (tankSpeed + tankRadius + BZDBCache::flagRadius) * (tankSpeed + tankRadius + BZDBCache::flagRadius);
  const float* tpos = playerData->lastState.pos;
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


void dropFlag(FlagInfo& drpFlag, const float dropPos[3])
{
  assert(world != NULL);

  const float size = BZDBCache::worldSize;
  float pos[3];
  pos[0] = ((dropPos[0] < -size) || (dropPos[0] > size)) ? 0.0f : dropPos[0];
  pos[1] = ((dropPos[1] < -size) || (dropPos[1] > size)) ? 0.0f : dropPos[1];
  pos[2] = (dropPos[2] > maxWorldHeight) ? maxWorldHeight : dropPos[2];

  // player wants to drop flag.  we trust that the client won't tell
  // us to drop a sticky flag until the requirements are satisfied.
  const int flagIndex = drpFlag.getIndex();
  if (drpFlag.flag.status != FlagOnTank) {
    return;
  }
  int flagTeam = drpFlag.flag.type->flagTeam;
  bool isTeamFlag = (flagTeam != ::NoTeam);

  // limited flags that have been fired should be disposed of
  bool limited = clOptions->flagLimit[drpFlag.flag.type] != -1;
  if (limited && drpFlag.numShots > 0) drpFlag.grabs = 0;


  const float waterLevel = world->getWaterLevel();
  float minZ = 0.0f;
  if (waterLevel > minZ) {
    minZ = waterLevel;
  }
  const float maxZ = MAXFLOAT;

  float landing[3] = {pos[0], pos[1], pos[2]};
  bool safelyDropped =
	DropGeometry::dropTeamFlag(landing, minZ, maxZ, flagTeam);

  bool vanish;

  if (isTeamFlag) {
    vanish = false;
  } else if (--drpFlag.grabs <= 0) {
    vanish = true;
    drpFlag.grabs = 0;
  } else if (!clOptions->flagsOnBuildings && (landing[2] > 0.0f)) {
    vanish = true;
  } else {
    vanish = !safelyDropped;
  }

  // With Team Flag, we should absolutely go for finding a landing
  // position, while, for other flags, we could stay with default, or
  // just let them vanish
  if (isTeamFlag && !safelyDropped) {
    // figure out landing spot -- if flag in a Bad Place
    // when dropped, move to safety position or make it going
    std::string teamName = Team::getName((TeamColor) flagTeam);
    if (!world->getFlagDropPoint(&drpFlag, pos, landing)) {
      // try the center
      landing[0] = landing[1] = landing[2] = 0.0f;
      safelyDropped =
	DropGeometry::dropTeamFlag(landing, minZ, maxZ, flagTeam);
      if (!safelyDropped) {
	// ok, we give up, send it home
	TeamBases &teamBases = bases[flagTeam];
	const TeamBase &base = teamBases.getRandomBase(flagIndex);
	landing[0] = base.position[0];
	landing[1] = base.position[1];
	landing[2] = base.position[2] + base.size[2];
      }
    }
  }

  if (isTeamFlag) {
    // if it is a team flag, check if there are any players left in
    // that team - if not, start the flag timeout
    if (team[drpFlag.flag.type->flagTeam].team.size == 0) {
      team[flagIndex + 1].flagTimeout = TimeKeeper::getCurrent();
      team[flagIndex + 1].flagTimeout += (float)clOptions->teamFlagTimeout;
    }
  }

  drpFlag.dropFlag(pos, landing, vanish);

  // player no longer has flag -- send MsgDropFlag
  sendDrop(drpFlag);

  // notify of new flag state
  sendFlagUpdate(drpFlag);

}


static void dropPlayerFlag(GameKeeper::Player &playerData, const float dropPos[3])
{
  const int flagIndex = playerData.player.getFlag();
  if (flagIndex < 0) {
    return;
  }
  dropFlag(*FlagInfo::get(flagIndex), dropPos);
  return;
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
    TeamColor base = whoseBase(playerData->lastState.pos[0],
			       playerData->lastState.pos[1],
			       playerData->lastState.pos[2]);
    if ((teamIndex == playerData->player.getTeam() &&
	 base == playerData->player.getTeam()))	{
      DEBUG1("Player %s [%d] might have sent MsgCaptureFlag for taking their own "
	     "flag onto their own base\n",
	     playerData->player.getCallSign(), playerIndex);
      //return; //sanity check
    }
    if ((teamIndex != playerData->player.getTeam() &&
	 base != playerData->player.getTeam())) {
      DEBUG1("Player %s [%d] (%s) might have tried to capture %s flag without "
	     "reaching their own base. (Player position: %f %f %f)\n",
	     playerData->player.getCallSign(), playerIndex,
	     Team::getName(playerData->player.getTeam()),
	     Team::getName((TeamColor)teamIndex),
	     playerData->lastState.pos[0], playerData->lastState.pos[1],
	     playerData->lastState.pos[2]);
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
  broadcastMessage(MsgCaptureFlag, (char*)buf-(char*)bufStart, bufStart,
		   false);

  // find any events for capturing the flags on the capped team or events for ANY team
  bz_CTFCaptureEventData	eventData;
  eventData.teamCapped = convertTeam((TeamColor)teamIndex);
  eventData.teamCapping = convertTeam(teamCaptured);
  eventData.playerCapping = playerIndex;
  playerData->getPlayerState(eventData.pos, eventData.rot);
  eventData.time = TimeKeeper::getCurrent().getSeconds();

  worldEventManager.callEvents(bz_eCaptureEvent,&eventData);

  // everyone on losing team is dead
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
    if ((p == NULL) || (teamIndex != (int)p->player.getTeam())) {
      continue;
    }
    p->player.setDead();
    p->player.setRestartOnBase(true);
    zapFlagByPlayer(i);
  }

  // update score (rogues can't capture flags)
  int winningTeam = (int)NoTeam;
  if (teamIndex != int(playerData->player.getTeam())) {
    // player captured enemy flag
    winningTeam = int(playerData->player.getTeam());
    team[winningTeam].team.won++;
  }
  team[teamIndex].team.lost++;
  sendTeamUpdate(winningTeam, teamIndex);
#ifdef PRINTSCORE
  dumpScore();
#endif
  if (winningTeam != (int)NoTeam)
    checkTeamScore(playerIndex, winningTeam);
}

static void shotUpdate(void *buf, int len, NetHandler *handler)
{
  ShotUpdate shot;
  shot.unpack(buf);

  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(shot.player);
  if (!playerData)
    return;
  if (playerData->netHandler != handler)
    return;

  const PlayerInfo &shooter = playerData->player;
  if (!shooter.isAlive() || shooter.isObserver())
    return;

  if (!playerData->updateShot(shot.id & 0xff, shot.id >> 8))
    return;

  relayMessage(MsgGMUpdate, len, buf);

}

static void shotFired(void *buf, int len, NetHandler *handler)
{
  FiringInfo firingInfo;
  firingInfo.unpack(buf);

  const ShotUpdate &shot = firingInfo.shot;

  int playerIndex = shot.player;

  // verify playerId
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  if (playerData->netHandler != handler)
    return;

  const PlayerInfo &shooter = playerData->player;
  if (!shooter.isAlive() || shooter.isObserver())
    return;

  FlagInfo &fInfo = *FlagInfo::get(shooter.getFlag());

  if (shooter.haveFlag())
    firingInfo.flagType = fInfo.flag.type;
  else
    firingInfo.flagType = Flags::Null;

  if (!playerData->addShot(shot.id & 0xff, shot.id >> 8, firingInfo))
    return;

  char message[MessageLen];
  if (shooter.haveFlag()) {
    fInfo.numShots++; // increase the # shots fired
    int limit = clOptions->flagLimit[fInfo.flag.type];
    if (limit != -1){ // if there is a limit for players flag
      int shotsLeft = limit -  fInfo.numShots;
      if (shotsLeft > 0) { //still have some shots left
	// give message each shot below 5, each 5th shot & at start
	if (shotsLeft % 5 == 0 || shotsLeft <= 3 || shotsLeft == limit-1){
	  if (shotsLeft > 1)
	    sprintf(message,"%d shots left",shotsLeft);
	  else
	    strcpy(message,"1 shot left");
	  sendMessage(ServerPlayer, playerIndex, message);
	}
      } else { // no shots left
	if (shotsLeft == 0 || (limit == 0 && shotsLeft < 0)){
	  // drop flag at last known position of player
	  // also handle case where limit was set to 0
	  float lastPos [3];
	  for (int i = 0; i < 3; i ++){
	    lastPos[i] = playerData->lastState.pos[i];
	  }
	  fInfo.grabs = 0; // recycle this flag now
	  dropPlayerFlag(*playerData, lastPos);
	} else { // more shots fired than allowed
	  // do nothing for now -- could return and not allow shot
	}
      } // end no shots left
    } // end is limit
  } // end of player has flag

  if (firingInfo.flagType == Flags::GuidedMissile)
    playerData->player.endShotCredit--;

  relayMessage(MsgShotBegin, len, buf);

}

static void sendShotEnd(const PlayerId& id, int16_t shotIndex, uint16_t reason)
{
  // shot has ended prematurely -- send MsgShotEnd
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, id);
  buf = nboPackShort(buf,      shotIndex);
  buf = nboPackUShort(buf,     reason);
  relayMessage(MsgShotEnd, (char*)buf-(char*)bufStart, bufStart);
}

static void shotEnded(const PlayerId& id, int16_t shotIndex, uint16_t reason)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(id);
  if (!playerData)
    return;

  FiringInfo firingInfo;
  playerData->removeShot(shotIndex & 0xff, shotIndex >> 8, firingInfo);

  // shot has ended prematurely -- send MsgShotEnd
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, id);
  buf = nboPackShort(buf, shotIndex);
  buf = nboPackUShort(buf, reason);
  relayMessage(MsgShotEnd, (char*)buf-(char*)bufStart, bufStart);
}

static void sendTeleport(int playerIndex, uint16_t from, uint16_t to)
{
  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, playerIndex);
  buf = nboPackUShort(buf, from);
  buf = nboPackUShort(buf, to);
  broadcastMessage(MsgTeleport, (char*)buf-(char*)bufStart, bufStart, false);
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
      removePlayer(t, buffer, true);
    } else {
      DEBUG1("Player %s tried to %s as an observer\n", p.getCallSign(), action);
    }
    return true;
  }
  return false;
}


static void lagKick(int playerIndex)
{
  char message[MessageLen];
  sprintf(message,
	  "You have been kicked due to excessive lag (you have been warned %d times).",
	  clOptions->maxlagwarn);
  sendMessage(ServerPlayer, playerIndex, message);
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  snprintf(message, MessageLen,"Lagkick: %s", playerData->player.getCallSign());
  sendMessage(ServerPlayer, AdminPlayers, message);
  removePlayer(playerIndex, "lag", true);
}


static void adjustTolerances()
{
  // check for handicap adjustment
  if ((clOptions->gameStyle & HandicapGameStyle) != 0) {
    const float speedAdj = BZDB.eval(StateDatabase::BZDB_HANDICAPVELAD);
    speedTolerance *= speedAdj * speedAdj;
  }

  // check for physics driver disabling
  disableHeightChecks = false;
  bool disableSpeedChecks = false;
  int i = 0;
  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(i);
  while (phydrv) {
    const float* v = phydrv->getLinearVel();
    const float av = phydrv->getAngularVel();
    if (!phydrv->getIsDeath()) {
      if (!phydrv->getIsSlide() &&
	  ((v[0] != 0.0f) || (v[1] != 0.0f) || (av != 0.0f))) {
	disableSpeedChecks = true;
      }
      if (v[2] > 0.0f) {
	disableHeightChecks = true;
      }
    }
    i++;
    phydrv = PHYDRVMGR.getDriver(i);
  }

  if (disableSpeedChecks) {
    doSpeedChecks = false;
    speedTolerance = MAXFLOAT;
    DEBUG1("Warning: disabling speed checking due to physics drivers\n");
  }
  if (disableHeightChecks) {
    DEBUG1("Warning: disabling height checking due to physics drivers\n");
  }

  return;
}


bool checkSpam(char* message, GameKeeper::Player* playerData, int t)
{
  PlayerInfo &player = playerData->player;
  const std::string &oldMsg = player.getLastMsg();
  float dt = (float)(TimeKeeper::getCurrent() - player.getLastMsgTime());

  // don't consider whitespace
  std::string newMsg = TextUtils::no_whitespace(message);

  // if it's first message, or enough time since last message - can't
  // be spam yet
  if (oldMsg.length() > 0 && dt < clOptions->msgTimer) {
    // might be spam, start doing comparisons
    // does it match the last message? (disregarding whitespace and case)
    if (TextUtils::compare_nocase(newMsg, oldMsg) == 0) {
      player.incSpamWarns();
      sendMessage(ServerPlayer, t, "***Server Warning: Please do not spam.");

      // has this player already had his share of warnings?
      if (player.getSpamWarns() > clOptions->spamWarnMax
	  || clOptions->spamWarnMax == 0) {
	sendMessage(ServerPlayer, t, "You were kicked because of spamming.");
	DEBUG2("Kicking player %s [%d] for spamming too much: "
	       "2 messages sent within %fs after %d warnings",
	       player.getCallSign(), t, dt, player.getSpamWarns());
	removePlayer(t, "spam", true);
	return true;
      }
    }
  }

  // record this message for next time
  player.setLastMsg(newMsg);
  return false;
}


static void handleCommand(const void *rawbuf, bool udp, NetHandler *handler)
{
  if (!rawbuf) {
    std::cerr << "WARNING: handleCommand got a null rawbuf?!" << std::endl;
    return;
  }

  uint16_t len, code;
  void *buf = (char *)rawbuf;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
  char buffer[MessageLen];

  if (udp) {
    switch (code) {
    case MsgShotBegin:
    case MsgShotEnd:
    case MsgPlayerUpdate:
    case MsgPlayerUpdateSmall:
    case MsgGMUpdate:
    case MsgUDPLinkRequest:
    case MsgUDPLinkEstablished:
      break;
    default:
      DEBUG1("Received packet type (%x) via udp, possible attack from %s\n",
	     code, handler->getTargetIP());
      return;
    }
  }

  GameKeeper::Player *playerData = NULL;

  int t = 0;
  switch (code) {
  case MsgEnter:
  case MsgExit:
  case MsgAlive:
  case MsgKilled:
  case MsgGrabFlag:
  case MsgDropFlag:
  case MsgCaptureFlag:
  case MsgShotEnd:
  case MsgHit:
  case MsgTeleport:
  case MsgMessage:
  case MsgTransferFlag:
  case MsgPause:
  case MsgAutoPilot:
  case MsgLagPing:
  case MsgKrbPrincipal:
  case MsgKrbTicket:
  case MsgNewRabbit:
    uint8_t playerId;
    buf        = nboUnpackUByte(buf, playerId);
    playerData = GameKeeper::Player::getPlayerByIndex(playerId);
    if (!playerData)
      return;
    if (playerData->netHandler != handler)
      return;
    t = playerId;
    break;
  default:
    break;
  }

  switch (code) {
    // player joining
    case MsgEnter: {
      uint16_t rejectCode;
      char     rejectMsg[MessageLen];
      if (!playerData->player.unpackEnter(buf, rejectCode, rejectMsg)) {
	rejectPlayer(t, rejectCode, rejectMsg);
	break;
      }
      playerData->accessInfo.setName(playerData->player.getCallSign());
      std::string timeStamp = TimeKeeper::timestamp();
      DEBUG1("Player %s [%d] has joined from %s at %s with token \"%s\"\n",
	     playerData->player.getCallSign(),
	     t, handler->getTargetIP(), timeStamp.c_str(),
	     playerData->player.getToken());

      if (!clOptions->publicizeServer) {
	playerData->_LSAState = GameKeeper::Player::notRequired;
      } else if (strlen(playerData->player.getCallSign())) {
	playerData->_LSAState = GameKeeper::Player::required;
      }
      dontWait = true;
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
      directMessage(handler, MsgNegotiateFlags, (char*)buf-(char*)bufStart,
		    bufStart);
      break;
    }



    // player wants more of world database
    case MsgGetWorld: {
      // data: count (bytes read so far)
      uint32_t ptr;
      buf = nboUnpackUInt(buf, ptr);
      sendWorld(handler, ptr);
      break;
    }

    case MsgWantSettings: {
      pwrite(handler, worldSettings, 4 + WorldSettingsSize);
      break;
    }

    case MsgWantWHash: {
      void *obuf, *obufStart = getDirectMessageBuffer();
      if (clOptions->cacheURL.size() > 0) {
	obuf = nboPackString(obufStart, clOptions->cacheURL.c_str(),
			    clOptions->cacheURL.size() + 1);
	directMessage(handler, MsgCacheURL, (char*)obuf-(char*)obufStart,
		      obufStart);
      }
      obuf = nboPackString(obufStart, hexDigest, strlen(hexDigest)+1);
      directMessage(handler, MsgWantWHash, (char*)obuf-(char*)obufStart,
		    obufStart);
      break;
    }

    case MsgQueryGame:
      sendQueryGame(handler);
      break;

    case MsgQueryPlayers:
      sendQueryPlayers(handler);
      break;

    // player is coming alive
    case MsgAlive: {
      // player is on the waiting list
      float waitTime = rejoinList.waitTime(t);
      if (waitTime > 0.0f) {
	snprintf (buffer, MessageLen, "You are unable to begin playing for %.1f seconds.", waitTime);
	sendMessage(ServerPlayer, t, buffer);

	// Make them pay dearly for trying to rejoin quickly
	playerAlive(t);
	playerKilled(t, t, 0, -1, Flags::Null, -1);

	break;
      }

      // player moved before countdown started
      if (clOptions->timeLimit>0.0f && !countdownActive) {
	playerData->player.setPlayedEarly();
      }
      playerAlive(t);
      break;
    }

    // player declaring self destroyed
    case MsgKilled: {
      if (playerData->player.isObserver())
	break;
      // data: id of killer, shot id of killer
      PlayerId killer;
      FlagType* flagType;
      int16_t shot, reason;
      int phydrv = -1;
      buf = nboUnpackUByte(buf, killer);
      buf = nboUnpackShort(buf, reason);
      buf = nboUnpackShort(buf, shot);
      buf = FlagType::unpack(buf, flagType);
      if (reason == PhysicsDriverDeath) {
	int32_t inPhyDrv;
	buf = nboUnpackInt(buf, inPhyDrv);
	phydrv = int(inPhyDrv);
      }

      // Sanity check on shot: Here we have the killer
      if (killer != ServerPlayer) {
	int si = (shot == -1 ? -1 : shot & 0x00FF);
	if ((si < -1) || (si >= clOptions->maxShots))
	  break;
      }
      playerData->player.endShotCredit--;
      playerKilled(t, lookupPlayer(killer), reason, shot, flagType, phydrv);

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
      dropPlayerFlag(*playerData, pos);
      break;
    }

    // player has captured a flag
    case MsgCaptureFlag: {
      // data: team whose territory flag was brought to
      uint16_t _team;

      if (invalidPlayerAction(playerData->player, t, "capture a flag"))
	break;

      buf = nboUnpackUShort(buf, _team);
      captureFlag(t, TeamColor(_team));
      break;
    }

    // shot fired
    case MsgShotBegin:
      // Sanity check
      if (len == FiringInfoPLen)
	shotFired(buf, int(len), handler);
      break;

    // shot ended prematurely
    case MsgShotEnd: {
      if (playerData->player.isObserver())
	break;

      // data: shooter id, shot number, reason
      PlayerId sourcePlayer = t;
      int16_t shot;
      uint16_t reason;
      buf = nboUnpackShort(buf, shot);
      buf = nboUnpackUShort(buf, reason);
      shotEnded(sourcePlayer, shot, reason);
      break;
    }

    // tank is being hit
    case MsgHit: {
      if (playerData->player.isObserver())
	break;
      if (!playerData->player.isAlive())
	break;

      PlayerId hitPlayer = t;
      PlayerId shooterPlayer;
      FiringInfo firingInfo;
      int16_t shot;
      buf = nboUnpackUByte(buf, shooterPlayer);
      buf = nboUnpackShort(buf, shot);
      GameKeeper::Player *shooterData
	= GameKeeper::Player::getPlayerByIndex(shooterPlayer);
      if (!shooterData)
	break;
      if (shooterData->removeShot(shot & 0xff, shot >> 8, firingInfo)) {
	sendShotEnd(shooterPlayer, shot, 1);
	const int flagIndex = playerData->player.getFlag();
	FlagInfo *flagInfo  = NULL;
	if (flagIndex >= 0) {
	  flagInfo = FlagInfo::get(flagIndex);
	  sendDrop(*flagInfo);
	}

	if (!flagInfo || flagInfo->flag.type != Flags::Shield)
	  playerKilled(hitPlayer, shooterPlayer, 1, shot,
		       firingInfo.flagType, false, false);
      }
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
      PlayerId dstPlayer;
      char message[MessageLen];
      buf = nboUnpackUByte(buf, dstPlayer);
      buf = nboUnpackString(buf, message, sizeof(message));
      message[MessageLen - 1] = '\0';
      playerData->player.hasSent();
      if (dstPlayer == AllPlayers) {
	DEBUG1("Player %s [%d] -> All: %s\n", playerData->player.getCallSign(),
	       t, message);
      } else if (dstPlayer == AdminPlayers) {
	DEBUG1("Player %s [%d] -> Admin: %s\n",
	       playerData->player.getCallSign(), t, message);
      } else if (dstPlayer > LastRealPlayer) {
	DEBUG1("Player %s [%d] -> Team: %s\n",
	       playerData->player.getCallSign(), t, message);
      } else {
	GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(dstPlayer);
	if (p != NULL) {
	  DEBUG1("Player %s [%d] -> Player %s [%d]: %s\n",
	       playerData->player.getCallSign(), t, p->player.getCallSign(), dstPlayer, message);
	} else {
	      DEBUG1("Player %s [%d] -> Player Unknown [%d]: %s\n",
	       playerData->player.getCallSign(), t, dstPlayer, message);
	}
      }
      // check for spamming
      if (checkSpam(message, playerData, t))
	break;

      GameKeeper::Player *toData = GameKeeper::Player::getPlayerByIndex(dstPlayer);
      int toTeam = -1;
      if (toData)
		toTeam = toData->player.getTeam();

      bz_ChatEventData chatData;
      chatData.from = t;
      chatData.to = BZ_NULLUSER;

	  if (dstPlayer == AllPlayers)
		chatData.to = BZ_ALLUSERS;
	  else if ( dstPlayer == AdminPlayers )
		chatData.team = eAdministrators;
	  else if ( dstPlayer > LastRealPlayer )
		chatData.team =convertTeam((TeamColor)(250-dstPlayer));
	  else
		chatData.to = dstPlayer;


      chatData.message = message;
      chatData.time = TimeKeeper::getCurrent().getSeconds();

      // send any events that want to watch the chat
      // everyone
      if ((strlen(message) < 2) || !((message[0] == '/') && (message[1] != '/')))
	worldEventManager.callEvents(bz_eChatMessageEvent,&chatData);

      // send the actual Message after all the callbacks have done there magic to it.
      if (chatData.message.size())
	sendPlayerMessage (playerData, dstPlayer, chatData.message.c_str());
      break;
    }

    // player has transferred flag to another tank
    case MsgTransferFlag: {
      PlayerId from, to;

      from = t;

      buf = nboUnpackUByte(buf, to);

      GameKeeper::Player *fromData = playerData;

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

      void *obufStart = getDirectMessageBuffer();
      void *obuf = nboPackUByte(obufStart, from);
      obuf = nboPackUByte(obuf, to);
      FlagInfo &flag = *FlagInfo::get(flagIndex);
      flag.flag.owner = to;
      flag.player = to;
      toData->player.resetFlag();
      toData->player.setFlag(flagIndex);
      fromData->player.resetFlag();
      obuf = flag.pack(obuf);
      broadcastMessage(MsgTransferFlag, (char*)obuf - (char*)obufStart,
		       obufStart);
      break;
    }

    case MsgUDPLinkEstablished:
      DEBUG2("Connection at %s outbound UDP up\n", handler->getTargetIP());
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

    case MsgAutoPilot: {
      uint8_t autopilot;
      nboUnpackUByte(buf, autopilot);
      autopilotPlayer(t, autopilot != 0);
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
	if (kick)
	  lagKick(t);
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

      playerData = GameKeeper::Player::getPlayerByIndex(id);
      if (!playerData)
	return;
      if (playerData->netHandler != handler)
	return;
      t = id;

      buf = state.unpack(buf, code);

      // observer updates are not relayed 
      if (playerData->player.isObserver()) {
        // skip all of the checks
        playerData->setPlayerState(state, timestamp);
	break;
      }
      
      // silently drop old packet
      if (state.order <= playerData->lastState.order) {
	break;
      }

      // Don't kick players up to 10 seconds after a world parm has changed,
      TimeKeeper now = TimeKeeper::getCurrent();

      if (now - lastWorldParmChange > 10.0f) {

	// see if the player is too high
	if (!disableHeightChecks) {

	  static const float heightFudge = 1.10f; /* 10% */

	  float wingsGravity = BZDB.eval(StateDatabase::BZDB_WINGSGRAVITY);
	  float normalGravity = BZDBCache::gravity;
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
	      removePlayer(t, "too high", true);
	      break;
	    }
	  }
	}

	// make sure the player is still in the map
	// test all the map bounds + some fudge factor, just in case
	static const float positionFudge = 10.0f; /* linear distance */
	bool InBounds = true;
	float worldSize = BZDBCache::worldSize;
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
	  removePlayer(t, "Out of map bounds", true);
	}

	// Speed problems occur around flag drops, so don't check for
	// a short period of time after player drops a flag. Currently
	// 2 second, adjust as needed.
	if (playerData->player.isFlagTransitSafe()) {

	  // we'll be checking against the player's flag type
	  int pFlag = playerData->player.getFlag();

	  // check for highspeed cheat; if inertia is enabled, skip test for now
	  if (BZDB.eval(StateDatabase::BZDB_INERTIALINEAR) == 0.0f) {
	    // Doesn't account for going fast backwards, or jumping/falling
	    float curPlanarSpeedSqr = state.velocity[0]*state.velocity[0] +
				      state.velocity[1]*state.velocity[1];

	    float maxPlanarSpeed = BZDBCache::tankSpeed;

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
		(playerData->lastState.pos[2] == state.pos[2]) &&
		(playerData->lastState.velocity[2] == state.velocity[2]) &&
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
	    if ((playerData->lastState.pos[2] != state.pos[2])
	    ||  (playerData->lastState.velocity[2] != state.velocity[2])
	    ||  ((state.status & PlayerState::Alive) == 0)) {
	      logOnly = true;
	    }

	    // allow a 10% tolerance level for speed if -speedtol is not sane
	    if (doSpeedChecks) {
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
                  removePlayer(t, "too fast", true);
                }
                break;
              }
            }
	  }
	}
      }

      playerData->setPlayerState(state, timestamp);

      // Player might already be dead and did not know it yet (e.g. teamkill)
      // do not propogate
      if (!playerData->player.isAlive() &&
	  (state.status & short(PlayerState::Alive)))
	break;

      // observer shouldn't send bulk messages anymore, they used to
      // when it was a server-only hack; but the check does not hurt,
      // either
      if (playerData->player.isObserver())
 	break;

      relayPlayerPacket(t, len, rawbuf, code);
      break;
    }

    case MsgGMUpdate:
      shotUpdate(buf, int(len), handler);
      break;

    // FIXME handled inside uread, but not discarded
    case MsgUDPLinkRequest:
      break;

    case MsgKrbPrincipal:
      playerData->authentication.setPrincipalName((char *)buf, len);
      break;

    case MsgKrbTicket:
      playerData->freeTCPMutex();
      playerData->authentication.verifyCredential((char *)buf, len);
      playerData->passTCPMutex();
      // Not really the place here, but for initial testing we need something
      if (playerData->authentication.isTrusted())
	sendMessage(ServerPlayer, t, "Welcome, we trust you");
      break;

    // unknown msg type
    default:
      DEBUG1("Received an unknown packet type (%x), possible attack from %s\n",
	     code, handler->getTargetIP());
  }
}

static void handleTcp(NetHandler &netPlayer, int i, const RxStatus e)
{
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
      DEBUG1("Player [%d] sent huge packet length, possible attack\n", i);
      removePlayer(i, "large packet recvd", false);
    }
    return;
  }

  uint16_t len, code;
  void *buf = netPlayer.getTcpBuffer();
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);

  // trying to get the real player from the message: bots share tcp
  // connection with the player
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

  // simple ruleset, if player sends a MsgShotBegin over TCP he/she
  // must not be using the UDP link
  if (clOptions->requireUDP && playerData != NULL && !playerData->player.isBot()) {
    if (code == MsgShotBegin) {
      char message[MessageLen];
      sprintf(message,"Your end is not using UDP, turn on udp");
      sendMessage(ServerPlayer, i, message);

      sprintf(message,"upgrade your client http://BZFlag.org/ or");
      sendMessage(ServerPlayer, i, message);

      sprintf(message,"Try another server, Bye!");
      sendMessage(ServerPlayer, i, message);
      removePlayer(i, "no UDP", true);
      return;
    }
  }

  // handle the command
  handleCommand(netPlayer.getTcpBuffer(), false, &netPlayer);
}

static void terminateServer(int /*sig*/)
{
  bzSignal(SIGINT, SIG_PF(terminateServer));
  bzSignal(SIGTERM, SIG_PF(terminateServer));
  exitCode = 0;
  done = true;
}


static std::string cmdSet(const std::string&, const CommandManager::ArgList& args, bool *worked)
{
  if(worked)*worked = true;
  switch (args.size()) {
    case 2:
      if (BZDB.isSet(args[0])) {
	StateDatabase::Permission permission = BZDB.getPermission(args[0]);
	if ((permission == StateDatabase::ReadWrite) || (permission == StateDatabase::Locked)) {
	  BZDB.set(args[0], args[1], StateDatabase::Server);
	  lastWorldParmChange = TimeKeeper::getCurrent();
	  return args[0] + " set";
	}
	if(worked)*worked = false;
	return "variable " + args[0] + " is not writeable";
      } else {
	if(worked)*worked = false;
	return "variable " + args[0] + " does not exist";
      }
    case 1:
      if (BZDB.isSet(args[0])) {
	return args[0] + " is " + BZDB.get(args[0]);
      } else {
	if(worked)*worked = false;
	return "variable " + args[0] + " does not exist";
      }
    default:
	  if(worked)*worked = false;
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

static std::string cmdReset(const std::string&, const CommandManager::ArgList& args, bool*)
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

static bool requestAuthentication;

static void doStuffOnPlayer(GameKeeper::Player &playerData)
{
  int p = playerData.getIndex();

  // kick idle players
  if (clOptions->idlekickthresh > 0) {
    if ((playerData.player.isTooMuchIdling(clOptions->idlekickthresh)) &&
       (!playerData.accessInfo.hasPerm(PlayerAccessInfo::antikick))) {
      char message[MessageLen]
	= "You were kicked because you were idle too long";
      sendMessage(ServerPlayer, p,  message);
      removePlayer(p, "idling", true);
      return;
    }
  }

  // Check authorization
  if (playerData._LSAState == GameKeeper::Player::required) {
     requestAuthentication = true;
     playerData._LSAState = GameKeeper::Player::requesting;
  } else if (playerData.netHandler->reverseDNSDone()) {
    if ((playerData._LSAState == GameKeeper::Player::verified)	||
        (playerData._LSAState == GameKeeper::Player::timedOut)	||
        (playerData._LSAState == GameKeeper::Player::failed)	||
        (playerData._LSAState == GameKeeper::Player::notRequired)) {
      addPlayer(p, &playerData);
      playerData._LSAState = GameKeeper::Player::done;
    }
  }

  // Check host bans
  const char *hostname = playerData.netHandler->getHostname();
  if (hostname && playerData.needsHostbanChecked()) {
    if (!playerData.accessInfo.hasPerm(PlayerAccessInfo::antiban)) {
      HostBanInfo hostInfo("*");
      if (!clOptions->acl.hostValidate(hostname, &hostInfo)) {
	std::string reason = "banned host for: ";
	if (hostInfo.reason.size())
	  reason += hostInfo.reason;
	else
	  reason += "General Ban";

	if (hostInfo.bannedBy.size()) {
	  reason += " by ";
	  reason += hostInfo.bannedBy;
	}

	if (hostInfo.fromMaster)
	  reason += " from the master server";

	rejectPlayer(p, RejectHostBanned, reason.c_str());
	return;
      }
    }
    playerData.setNeedThisHostbanChecked(false);
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
    for (int j = 0; j < numFlags; j++) {
      if (FlagInfo::get(j)->player == p) {
	dropPlayerFlag(playerData, playerData.lastState.pos);
	// Should recheck if player is still available
	if (!GameKeeper::Player::getPlayerByIndex(p))
	  return;
      }
    }
  }

  // send lag pings
  bool warn;
  bool kick;
  int nextPingSeqno = playerData.lagInfo.getNextPingSeqno(warn, kick);
  if (nextPingSeqno > 0) {
    void *buf, *bufStart = getDirectMessageBuffer();
    buf = nboPackUShort(bufStart, nextPingSeqno);
    int result = directMessage(playerData.netHandler, MsgLagPing,
			       (char*)buf - (char*)bufStart, bufStart);
    if (result == -1)
      return;
    if (warn) {
      char message[MessageLen];
      sprintf(message, "*** Server Warning: your lag is too high (failed to return ping) ***");
      sendMessage(ServerPlayer, p, message);
      // Should recheck if player is still available
      if (!GameKeeper::Player::getPlayerByIndex(p))
	return;
      if (kick) {
	lagKick(p);
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


void initGroups()
{
  // make sure that the 'admin' & 'default' groups exist

  PlayerAccessInfo info;
  info.explicitAllows[PlayerAccessInfo::actionMessage] = true;
  info.explicitAllows[PlayerAccessInfo::adminMessageSend] = true;
  info.explicitAllows[PlayerAccessInfo::date] = true;
  info.explicitAllows[PlayerAccessInfo::flagHistory] = true;
  info.explicitAllows[PlayerAccessInfo::idleStats] = true;
  info.explicitAllows[PlayerAccessInfo::lagStats] = true;
  info.explicitAllows[PlayerAccessInfo::privateMessage] = true;
  info.explicitAllows[PlayerAccessInfo::spawn] = true;
  info.explicitAllows[PlayerAccessInfo::talk] = true;
  info.groupState[PlayerAccessInfo::isGroup] = true;
  info.groupState[PlayerAccessInfo::isDefault] = true;
  groupAccess["EVERYONE"] = info;

  // VERIFIED
  info.explicitAllows.reset();
  info.groupState.reset();
  info.explicitAllows[PlayerAccessInfo::poll] = true;
  info.explicitAllows[PlayerAccessInfo::vote] = true;
  info.explicitAllows[PlayerAccessInfo::pollBan] = true;
  info.explicitAllows[PlayerAccessInfo::pollKick] = true;
  info.explicitAllows[PlayerAccessInfo::pollSet] = true;
  info.explicitAllows[PlayerAccessInfo::pollFlagReset] = true;
  info.groupState[PlayerAccessInfo::isGroup] = true;
  info.groupState[PlayerAccessInfo::isDefault] = true;
  groupAccess["VERIFIED"] = info;

  //  LOCAL.ADMIN
  info.explicitAllows.reset();
  info.groupState.reset();
  for (int i = 0; i < PlayerAccessInfo::lastPerm; i++)
    info.explicitAllows[i] = true;
  info.groupState[PlayerAccessInfo::isGroup] = true;
  info.groupState[PlayerAccessInfo::isDefault] = true;
  info.explicitAllows[PlayerAccessInfo::hideAdmin] = false;
  groupAccess["LOCAL.ADMIN"] = info;

  // load databases
  if (groupsFile.size())
    PlayerAccessInfo::readGroupsFile(groupsFile);
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

  // parse arguments  (finalized later)
  parse(argc, argv, *clOptions);
  setDebugTimestamp (clOptions->timestampLog, clOptions->timestampMicros);

  if (clOptions->bzdbVars.length() > 0) {
    DEBUG1("Loading variables from %s\n", clOptions->bzdbVars.c_str());
    bool success = CFGMGR.read(clOptions->bzdbVars);
    if (success) {
      DEBUG1("Successfully loaded variable(s)\n");
    } else {
      DEBUG1("WARNING: unable to load the variable file\n");
    }
  }

#ifdef _USE_BZ_API
  // see if we are going to load any plugins
  initPlugins();
  // check for python by default
  //	loadPlugin(std::string("python"),std::string(""));
  for (unsigned int plugin = 0; plugin < clOptions->pluginList.size(); plugin++)
  {
    if (!loadPlugin(clOptions->pluginList[plugin].plugin,
		    clOptions->pluginList[plugin].command)) {
      std::string text = "WARNING: unable to load the plugin; ";
      text += clOptions->pluginList[plugin].plugin + "\n";
      DEBUG0(text.c_str());
    }
  }
#endif

  // start listening and prepare world database
  if (!defineWorld()) {
#if defined(_WIN32)
    WSACleanup();
#endif /* defined(_WIN32) */
    std::cerr << "ERROR: A world was not specified" << std::endl;
    return 1;
  } else if (clOptions->cacheOut != "") {
    if (!saveWorldCache()) {
      std::cerr << "ERROR: could not save world cache file: "
		<< clOptions->cacheOut << std::endl;
    }
    done = true;
  }

  // make flags, check sanity, etc...
  // (do this after the world has been loaded)
  finalizeParsing(argc, argv, *clOptions, world->getEntryZones());
  {
    FlagInfo::setNoFlagInAir();
    for (int i = 0; i < numFlags; i++) {
      resetFlag(*FlagInfo::get(i));
    }
  }

  // loading extra flag number
  FlagInfo::setExtra(clOptions->numExtraFlags);

  // loading lag thresholds
  LagInfo::setThreshold(clOptions->lagwarnthresh,(float)clOptions->maxlagwarn);

  // loading player callsign/email filters
  PlayerInfo::setFilterParameters(clOptions->filterCallsigns,
				  clOptions->filter,
				  clOptions->filterSimple);

  GameKeeper::Player::setMaxShots(clOptions->maxShots);

  // enable replay server mode
  if (clOptions->replayServer) {

    Replay::init();

    // we don't send flags to a client that isn't expecting them
    numFlags = 0;
    clOptions->numExtraFlags = 0;

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
    } else if (clOptions->maxTeam[ObserverTeam] > ReplayObservers) {
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

  // loading authentication parameters
  Authentication::init(clOptions->publicizedAddress.c_str(),
		       clOptions->wksPort,
		       clOptions->password.c_str());

  /* initialize the poll arbiter for voting if necessary */
  if (clOptions->voteTime > 0) {
    votingarbiter =
      new VotingArbiter(clOptions->voteTime, clOptions->vetoTime,
                        clOptions->votesRequired, clOptions->votePercentage,
                        clOptions->voteRepeatTime);
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

  if (clOptions->pingInterface != "") {
    serverAddress = Address::getHostAddress(clOptions->pingInterface);
  }

  // my address to publish.  allow arguments to override (useful for
  // firewalls).  use my official hostname if it appears to be
  // canonicalized, otherwise use my IP in dot notation.
  // set publicized address if not set by arguments
  if (clOptions->publicizedAddress.length() == 0) {
    clOptions->publicizedAddress = Address::getHostName();
    if (clOptions->publicizedAddress.find('.') == std::string::npos)
      clOptions->publicizedAddress = serverAddress.getDotNotation();
    clOptions->publicizedAddress += TextUtils::format(":%d", clOptions->wksPort);
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
    MasterBanList banList;
    std::vector<std::string>::const_iterator it;
    for (it = clOptions->masterBanListURL.begin();
         it != clOptions->masterBanListURL.end(); it++) {
      clOptions->acl.merge(banList.get(it->c_str()));
      DEBUG1("Loaded master ban list from %s\n", it->c_str());
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

  // adjust speed and height checking as required
  adjustTolerances();

  // setup the game settings
  makeGameSettings();

  // no original world weapons in replay mode
  if (Replay::enabled()) {
    world->getWorldWeapons().clear();
  }

  TimeKeeper nextSuperFlagInsertion = TimeKeeper::getCurrent();
  const float flagExp = -logf(0.5f) / FlagHalfLife;

  // load up the access permissions & stuff
  initGroups();
  if (passFile.size())
    readPassFile(passFile);
  if (userDatabaseFile.size())
    PlayerAccessInfo::readPermsFile(userDatabaseFile);

  // See if an ID flag is in the game.
  // If not, we could hide type info for all flags
  if (clOptions->flagCount[Flags::Identify] > 0) {
    isIdentifyFlagIn = true;
  }
  if ((clOptions->numExtraFlags > 0)
      && !clOptions->flagDisallowed[Flags::Identify]) {
    isIdentifyFlagIn = true;
  }

  if (clOptions->startRecording) {
    Record::start(ServerPlayer);
  }

  // trap some signals
  if (bzSignal(SIGINT, SIG_IGN) != SIG_IGN) {
    bzSignal(SIGINT, SIG_PF(terminateServer));
  }
  bzSignal(SIGTERM, SIG_PF(terminateServer));

  // start the server
  if (!serverStart()) {
#if defined(_WIN32)
    WSACleanup();
#endif /* defined(_WIN32) */
    std::cerr << "ERROR: Unable to start the server, perhaps one is already running?" << std::endl;
    return 2;
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

  GameKeeper::Player::passTCPMutex();
  int i;
  int readySetGo = -1; // match countdown timer
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
    FD_SET((unsigned int)wksSocket, &read_set);
    if (wksSocket > maxFileDescriptor) {
      maxFileDescriptor = wksSocket;
    }

    // Check for cURL needed activity
    int cURLmaxFile = cURLManager::fdset(read_set, write_set);
    if (cURLmaxFile > maxFileDescriptor)
      maxFileDescriptor = cURLmaxFile;

    // find timeout when next flag would hit ground
    TimeKeeper tm = TimeKeeper::getCurrent();
    // lets start by waiting 3 sec
    float waitTime = 3.0f;

    if (countdownDelay >= 0) {
      // 3 seconds too slow for match countdowns
      waitTime = 0.5f;
    } else if (countdownActive && clOptions->timeLimit > 0.0f) {
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
    } else {
      // game time updates
      const float nextGT = nextGameTime();
      if (nextGT < waitTime) {
        waitTime = nextGT;
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

    // see if we are within the plug requested max wait time
    if (waitTime > pluginMaxWait)
      waitTime = pluginMaxWait;

    // don't wait (used by CURL and MsgEnter)
    if (dontWait) {
      waitTime = 0.0f;
      dontWait = false;
    }

    /**************
     *  SELECT()  *
     **************/

    // wait for an incoming communication, a flag to hit the ground,
    // a game countdown to end, a world weapon needed to be fired,
    // or a replay packet waiting to be sent.
    GameKeeper::Player::freeTCPMutex();
    struct timeval timeout;
    timeout.tv_sec = long(floorf(waitTime));
    timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));
    nfound = select(maxFileDescriptor+1, (fd_set*)&read_set, (fd_set*)&write_set, 0, &timeout);
    //if (nfound)
    //	DEBUG1("nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);

    // send replay packets
    // (this check and response should follow immediately after the select() call)
    GameKeeper::Player::passTCPMutex();
    if (Replay::playing()) {
      Replay::sendPackets ();
    }

    // game time updates
    if (!Replay::enabled()) {
      sendPendingGameTime();
    }
    
    // synchronize PlayerInfo
    tm = TimeKeeper::getCurrent();
    PlayerInfo::setCurrentTime(tm);
    NetHandler::setCurrentTime(tm);

    // players see a countdown
    if (countdownDelay >= 0) {
      static TimeKeeper timePrevious = tm;
      if (readySetGo == -1)
	readySetGo = countdownDelay;

      if (tm - timePrevious > 1.0f) {
	timePrevious = tm;
	if (readySetGo == 0) {
	  sendMessage(ServerPlayer, AllPlayers, "The match has started!...Good Luck Teams!");
	  countdownDelay = -1; // reset back to "unset"
	  readySetGo = -1; // reset back to "unset"
	  countdownActive = true;
	  gameOver = false;

	  // start server's clock
	  gameStartTime = tm;
	  clOptions->timeElapsed = 0.0f;

	  // start client's clock
	  void *msg = getDirectMessageBuffer();
	  nboPackInt(msg, (int32_t)(int)clOptions->timeLimit);
	  broadcastMessage(MsgTimeUpdate, sizeof(int32_t), msg);

	  // kill any players that are playing already
	  GameKeeper::Player *player;
	  if (clOptions->gameStyle & int(TeamFlagGameStyle)) {
	    for (int j = 0; j < curMaxPlayers; j++) {
	      void *buf, *bufStart = getDirectMessageBuffer();
	      player = GameKeeper::Player::getPlayerByIndex(j);
	      if (!player || player->player.isObserver() || !player->player.isPlaying())
					continue;

	      // the server gets to capture the flag -- send some
	      // bogus player id

	      // curMaxPlayers should never exceed 255, so this should
	      // be a safe cast
	      TeamColor vteam = player->player.getTeam();

	      buf = nboPackUByte(bufStart, (uint8_t)curMaxPlayers);
	      buf = nboPackUShort
		(buf, uint16_t(FlagInfo::lookupFirstTeamFlag(vteam)));
	      buf = nboPackUShort(buf, uint16_t(1 + (int(vteam) % 4)));
	      directMessage(j, MsgCaptureFlag, (char*)buf - (char*)bufStart,
			    bufStart);

	      // kick 'em while they're down
	      playerKilled(j, curMaxPlayers, 0, -1, Flags::Null, -1);

	      // be sure to reset the player!
	      player->player.setDead();
	      zapFlagByPlayer(j);
	      player->player.setPlayedEarly(false);
	    }
	  }

	  // reset all flags
	  for (int j = 0; j < numFlags; j++) {
	    zapFlag(*FlagInfo::get(j));
	  }

          // fire off a game start event
          bz_GameStartEndEventData	gameData;
          gameData.eventType = bz_eGameStartEvent;
          gameData.time = TimeKeeper::getCurrent().getSeconds();
          gameData.duration = clOptions->timeLimit;
          worldEventManager.callEvents(bz_eGameStartEvent,&gameData);

	} else {
	  if ((readySetGo == countdownDelay) && (countdownDelay > 0))
	    sendMessage(ServerPlayer, AllPlayers, "Start your engines!......");

	  sendMessage(ServerPlayer, AllPlayers, TextUtils::format("%i...", readySetGo).c_str());
	  --readySetGo;
	}
      } // end check if second has elapsed
    } // end check if countdown delay is active

    // see if game time ran out or if we are paused
    if (!gameOver && countdownActive && clOptions->timeLimit > 0.0f) {
      float newTimeElapsed = (float)(tm - gameStartTime);
      float timeLeft = clOptions->timeLimit - newTimeElapsed;
      if (timeLeft <= 0.0f && !countdownPauseStart) {
	timeLeft = 0.0f;
	gameOver = true;
	countdownActive = false;
	countdownPauseStart = TimeKeeper::getNullTime ();
	clOptions->countdownPaused = false;
       
        // fire off a game end event
        bz_GameStartEndEventData	gameData;
        gameData.eventType = bz_eGameEndEvent;
        gameData.time = TimeKeeper::getCurrent().getSeconds();
        gameData.duration = clOptions->timeLimit;
        worldEventManager.callEvents(bz_eGameEndEvent,&gameData);
      }

      if (countdownActive && clOptions->countdownPaused && !countdownPauseStart) {
	// we have a new pause
	countdownPauseStart = tm;
	void *buf, *bufStart = getDirectMessageBuffer ();
	buf = nboPackInt (bufStart, -1);
	broadcastMessage (MsgTimeUpdate, (char *) buf - (char *) bufStart, bufStart);
      }

      if (countdownActive && !clOptions->countdownPaused && countdownPauseStart) {
	// resumed
	gameStartTime += (tm - countdownPauseStart);
	countdownPauseStart = TimeKeeper::getNullTime ();
	newTimeElapsed = (float)(tm - gameStartTime);
	timeLeft = clOptions->timeLimit - newTimeElapsed;
	void *buf, *bufStart = getDirectMessageBuffer ();
	buf = nboPackInt (bufStart, (int32_t) timeLeft);
	broadcastMessage (MsgTimeUpdate, (char *) buf - (char *) bufStart, bufStart);
      }

      if ((timeLeft == 0.0f || newTimeElapsed - clOptions->timeElapsed >= 30.0f)
	  && !clOptions->countdownPaused) {
	// send update every 30 seconds
	void *buf, *bufStart = getDirectMessageBuffer ();
	buf = nboPackInt (bufStart, (int32_t) timeLeft);
	broadcastMessage (MsgTimeUpdate, (char *) buf - (char *) bufStart, bufStart);
	clOptions->timeElapsed = newTimeElapsed;
	if (clOptions->oneGameOnly && timeLeft == 0.0f) {
	  done = true;
	  exitCode = 0;
	}
      }
    }

    requestAuthentication = false;
    for (int p = 0; p < curMaxPlayers; p++) {
      GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(p);
      if (!playerData)
	continue;
      doStuffOnPlayer(*playerData);
    }
    if (requestAuthentication) {
       // Request the listserver authentication
       listServerLink->queueMessage(ListServerLink::ADD);
    }
    GameKeeper::Player::setAllNeedHostbanChecked(false);

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
	  snprintf(message, MessageLen, "A poll to %s %s has begun.  Players have up to %d seconds to vote.", action.c_str(), target.c_str(), voteTime);
	  sendMessage(ServerPlayer, AllPlayers, message);
	  announcedOpening = true;
	}

	static TimeKeeper lastAnnounce = TimeKeeper::getNullTime();

	/* make a heartbeat announcement every 15 seconds */
	if (((voteTime - (int)(tm - votingarbiter->getStartTime()) - 1) % 15 == 0) &&
	    ((int)(tm - lastAnnounce) != 0) &&
	    (votingarbiter->timeRemaining() > 0)) {
	  snprintf(message, MessageLen, "%d seconds remain in the poll to %s %s.", votingarbiter->timeRemaining(), action.c_str(), target.c_str());
	  sendMessage(ServerPlayer, AllPlayers, message);
	  lastAnnounce = tm;
	}

	if (votingarbiter->isPollClosed()) {

	  if (!announcedResults) {
	    snprintf(message, MessageLen, "Poll Results: %ld in favor, %ld oppose, %ld abstain", votingarbiter->getYesCount(), votingarbiter->getNoCount(), votingarbiter->getAbstentionCount());
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
		  else if (action == "kill")
	    pollAction = "killed";
	      else
		pollAction = action;
	      // a poll that exists and is closed has ended successfully
	      if(action != "flagreset")
		snprintf(message, MessageLen, "The poll is now closed and was successful.  %s is scheduled to be %s.", target.c_str(), pollAction.c_str());
	      else
		snprintf(message, MessageLen, "The poll is now closed and was successful.  Currently unused flags are scheduled to be reset.");
	      sendMessage(ServerPlayer, AllPlayers, message);
	      announcedClosure = true;
	    }
	  } else {
	    if (!announcedClosure) {
	      snprintf(message, MessageLen, "The poll to %s %s was not successful", action.c_str(), target.c_str());
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
		  pollAction += TextUtils::format("%d hour%s%s",
						    hours,
						    hours == 1 ? "." : "s",
						    minutes > 0 ? " and " : "");
		}
		if (minutes > 0) {
		  pollAction += TextUtils::format("%d minute%s",
						    minutes,
						    minutes > 1 ? "s" : "");
		}
		pollAction += ".";
	      } else if (action == "kick") {
		pollAction = std::string("kicked.");
	      } else if (action == "kill") {
		pollAction = std::string("killed.");
	      } else {
		pollAction = action;
	      }
	      if (action != "flagreset")
		snprintf(message, MessageLen, "%s has been %s", target.c_str(), pollAction.c_str());
	      else
		snprintf(message, MessageLen, "All unused flags have now been reset.");
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
		  if (otherData && (strncmp(otherData->player.getCallSign(),
			      target.c_str(), 256) == 0)) {
		    foundPlayer = true;
		    break;
		  }
		}
		// show the delinquent no mercy; make sure he is kicked even if he changed
		// his callsign by finding a corresponding IP and matching it to the saved one
		if (!foundPlayer) {
		  NetHandler *player = NetHandler::whoIsAtIP(realIP);
		  for (v = 0; v < curMaxPlayers; v++) {
		    GameKeeper::Player *otherData
		      = GameKeeper::Player::getPlayerByIndex(v);
		    if (otherData && (otherData->netHandler == player)) {
		      foundPlayer = true;
		      break;
		    }
		  }
		}
		if (foundPlayer) {
		  // notify the player
		  snprintf(message, MessageLen, "You have been %s due to sufficient votes to have you removed", action == "ban" ? "temporarily banned" : "kicked");
		  sendMessage(ServerPlayer, v, message);
		  snprintf(message,  MessageLen, "/poll %s", action.c_str());
		  removePlayer(v, message, true);
		}
	      } else if (action == "set") {
		std::vector<std::string> args = TextUtils::tokenize(target.c_str(), " ", 2, true);
		if (args.size() < 2) {
		  DEBUG1("Poll set taking action: no action taken, not enough parameters (%s).\n",
			 (args.size() > 0 ? args[0].c_str() : "No parameters."));
		}
		DEBUG1("Poll set taking action: setting %s to %s\n",
		       args[0].c_str(), args[1].c_str());
		BZDB.set(args[0], args[1], StateDatabase::Server);
	      } else if (action == "reset") {
		DEBUG1("Poll flagreset taking action: resetting unused flags.\n");
		for (int f = 0; f < numFlags; f++) {
		  FlagInfo &flag = *FlagInfo::get(f);
		  if (flag.player == -1)
		    resetFlag(flag);
		}
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
	      snprintf(message,  MessageLen, "Enough votes were collected to %s %s early.", action.c_str(), target.c_str());
	    else
	      snprintf(message,  MessageLen, "Enough votes were collected to reset all unused flags early.");

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
      static TimeKeeper lastbroadcast = tm;
      if (tm - lastbroadcast > 900) {
	// every 15 minutes
	char message[MessageLen];
	if (clOptions->advertisemsg != "") {
	  const std::string admsg = evaluateString(clOptions->advertisemsg);
	  // split the admsg into several lines if it contains '\n'
	  const char* c = admsg.c_str();
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
	    const std::string admsg = evaluateString((*adLines)[j]);
	    sendMessage(ServerPlayer, AllPlayers, admsg.c_str());
	  }
	}
	lastbroadcast = tm;
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
    if (!gameOver && clOptions->numExtraFlags > 0 && nextSuperFlagInsertion<=tm) {
      // randomly choose next flag respawn time; halflife distribution
      float r = float(bzfrand() + 0.01); // small offset, we do not want to wait forever
      nextSuperFlagInsertion += -logf(r) / flagExp;
      for (i = numFlags - clOptions->numExtraFlags; i < numFlags; i++) {
	FlagInfo &flag = *FlagInfo::get(i);
	if (flag.flag.type == Flags::Null) {
	  // flag in now entering game
	  flag.addFlag();
	  sendFlagUpdate(flag);
	  break;
	}
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

      // check if we have any UDP packets pending
      if (NetHandler::isUdpFdSet(&read_set)) {
	TimeKeeper receiveTime = TimeKeeper::getCurrent();
	while (true) {
	  struct sockaddr_in uaddr;
	  unsigned char ubuf[MaxPacketLen];

	  NetHandler* netHandler;

	  // interface to the UDP Receive routines
	  int id = NetHandler::udpReceive((char *) ubuf, &uaddr, &netHandler);
	  if (id == -1)
	    break;

	  uint16_t len, code;
	  void *buf = (char *)ubuf;
	  buf = nboUnpackUShort(buf, len);
	  buf = nboUnpackUShort(buf, code);

	  if (code == MsgPingCodeRequest) {
	    if (len != 2)
	      continue;
	    // if I'm ignoring pings
	    // then ignore the ping.
	    if (handlePings) {
	      respondToPing(Address(uaddr));
	      pingReply.write(NetHandler::getUdpSocket(), &uaddr);
	    }
	    continue;
	  }
	  if (!netHandler && (len == 1) && (code == MsgUDPLinkRequest)) {
	    // It is a UDP Link Request ... try to match it
	    uint8_t index;
	    buf = nboUnpackUByte(buf, index);
	    GameKeeper::Player *playerData
	      = GameKeeper::Player::getPlayerByIndex(index);
	    if (playerData) {
	      netHandler = playerData->netHandler;
	      if (netHandler->isMyUdpAddrPort(uaddr, false)) {
		netHandler->setUDPin(&uaddr);

		// send client the message that we are ready for him
		sendUDPupdate(netHandler);

		DEBUG2("Inbound UDP up %s:%d\n",
		       inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
	      } else {
		DEBUG2
		  ("Inbound UDP rejected %s:%d different IP than original\n",
		   inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
	      }
	      continue;
	    }
	  }
	  // handle the command for UDP
	  handleCommand(ubuf, true, netHandler);

	  // don't spend more than 250ms receiving udp
	  if (TimeKeeper::getCurrent() - receiveTime > 0.25f) {
	    DEBUG2("Too much UDP traffic, will hope to catch up later\n");
	    break;
	  }
	}
      }

      // process eventual resolver requests
      NetHandler::checkDNS(&read_set, &write_set);

      // now check messages from connected players and send queued messages
      GameKeeper::Player *playerData;
      NetHandler *netPlayer;
      for (i = 0; i < curMaxPlayers; i++) {
	playerData = GameKeeper::Player::getPlayerByIndex(i);
	if (!playerData)
	  continue;
	netPlayer = playerData->netHandler;
	// send whatever we have ... if any
	if (netPlayer->pflush(&write_set) == -1) {
	  removePlayer(i, "ECONNRESET/EPIPE", false);
	  continue;
	}
	playerData->handleTcpPacket(&read_set);
      }
    } else if (nfound < 0) {
      if (getErrno() != EINTR) {
	// test code - do not uncomment, will cause big stuttering
	// TimeKeeper::sleep(1.0f);
      }
    } else {
      if (NetHandler::anyUDPPending())
	NetHandler::flushAllUDP();
    }

    // Fire world weapons
    world->getWorldWeapons().fire();

    // fire off a tick event
    bz_TickEventData	tickData;
    tickData.time = TimeKeeper::getCurrent().getSeconds();
    worldEventManager.callEvents(bz_eTickEvent,&tickData);

    // Clean pending players
    bool resetGame = GameKeeper::Player::clean();

    if (resetGame && playerHadWorld) {
      playerHadWorld = false;
      (clOptions->worldFile == "") && !Replay::enabled() && defineWorld();
    }

    // cURLperform should be called in any case as we could incur in timeout
    dontWait = dontWait || cURLManager::perform();
  }

#ifdef _USE_BZ_API
  unloadPlugins();
#endif

  // print uptime
  DEBUG1("Shutting down server: uptime %s\n",
    TimeKeeper::printTime(TimeKeeper::getCurrent() - TimeKeeper::getStartTime()).c_str());

  GameKeeper::Player::freeTCPMutex();
  serverStop();

  // remove from list server and disconnect
  delete listServerLink;

  // free misc stuff
  delete clOptions; clOptions = NULL;
  FlagInfo::setSize(0);
  delete world; world = NULL;
  delete[] worldDatabase; worldDatabase = NULL;
  delete votingarbiter; votingarbiter = NULL;

  Record::kill();
  Replay::kill();
  Flags::kill();

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
