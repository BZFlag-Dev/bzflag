/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <errno.h>
#include <vector>
#include <string>
#include <time.h>
#include <stdio.h>
#include <sstream>

// common headers
#include "bzfsAPI.h"
#include "bz_md5.h"
#include "bzsignal.h"
#include "AnsiCodes.h"
#include "BaseBuilding.h"
#include "BZDBCache.h"
#include "BzDocket.h"
#include "BzVFS.h"
#include "CommandManager.h"
#include "ConfigFileManager.h"
#include "GameTime.h"
#include "NetHandler.h"
#include "NetMessage.h"
#include "Obstacle.h"
#include "ObstacleMgr.h"
#include "PhysicsDriver.h"
#include "ShotUpdate.h"
#include "TimeBomb.h"
#include "version.h"
#include "VotingArbiter.h"
#include "CollisionManager.h"
#include "Ray.h"
#ifdef DEBUG
#  include "MsgStrings.h"
#endif


// bzfs-specific headers
#include "RejoinList.h"
#include "ListServerConnection.h"
#include "WorldInfo.h"
#include "WorldWeapons.h"
#include "BZWReader.h"
#include "SendLagState.h"
#include "SpawnPosition.h"
#include "DropGeometry.h"
#include "commands.h"
#include "MasterBanList.h"
#include "Filter.h"
#include "WorldEventManager.h"
#include "WorldGenerators.h"
#include "bzfsMessages.h"
#include "bzfsClientMessages.h"
#include "bzfsPlayerStateVerify.h"
#include "AutoAllowTimer.h"
#include "ServerIntangibilityManager.h"
#include "lua/LuaServer.h"

#include "DirectoryNames.h"

#include "Stats.h"

StatsLink statsLink;

// only include this if we are going to use plugins and export the API
#ifdef BZ_PLUGINS
#  include "bzfsPlugins.h"
#endif

#ifndef BUFSIZE
#  define BUFSIZE 2048
#endif

AutoAllowTimerTickHandler autoAllowTimerTickHandler;

// pass through the SELECT loop
bool dontWait = true;

// every ListServerReAddTime server add ourself to the list
// server again.  this is in case the list server has reset
// or dropped us for some reason.
static const float ListServerReAddTime = 10.0f * 60.0f;

static const float FlagHalfLife = 10.0f;

// do NOT change
static const int InvalidPlayer = -1;

float speedTolerance = 1.125f;

// Command Line Options
CmdLineOptions *clOptions = NULL;

// server address to listen on
Address serverAddress;
// well known service socket
static int wksSocket = -1;
bool handlePings = true;
PingPacket pingReply;
// team info
TeamInfo teamInfos[NumTeams];
// num flags in flag list
int numFlags = 0;
bool serverDone = false;
// true if hit time/score limit
bool gameOver = true;
// "real" players, i.e. do not count observers
uint16_t maxRealPlayers = MaxPlayers;
// players + observers
uint16_t maxPlayers = MaxPlayers;
// highest active id
uint16_t curMaxPlayers = 0;

float maxWorldHeight = 0.0f;
CheatProtectionOptions cheatProtectionOptions;

bool publiclyDisconnected = false;

char hexDigest[50] = {0};

BzTime gameStartTime = BzTime::getNullTime();
BzTime countdownPauseStart = BzTime::getNullTime();
BzTime nextSuperFlagInsertion = BzTime::getNullTime();
bool countdownActive = false;
int countdownDelay = -1;
int countdownResumeTime = -1;

float flagExp = 0;

static ListServerLink *listServerLink = NULL;
static int listServerLinksCount = 0;

// FIXME: should be static, but needed by SpawnPosition
WorldInfo *world = NULL;
// FIXME: should be static, but needed by RecordReplay
char	  *worldDatabase = NULL;
uint32_t  worldDatabaseSize = 0;
char	  worldSettings[WorldSettingsSize];
float	  pluginWorldSize = -1;
float	  pluginWorldHeight = -1;
Filter	  filter;

VotingArbiter *votingArbiter = NULL;

BasesList bases;

// FIXME - define a well-known constant for a null playerid in address.h?
// might be handy in other players, too.
// Client does not check for rabbit to be 255, but it still works
// because 255 should be > curMaxPlayers and thus no matching player will
// be found.
// FIXME: should be static, but needed by RecordReplay
uint8_t rabbitIndex = NoPlayer;

RejoinList rejoinList;

static BzTime lastWorldParmChange = BzTime::getNullTime();
bool worldWasSentToAPlayer = false;

unsigned int maxNonPlayerDataChunk = 2048;
std::map<int,NetConnectedPeer> netConnectedPeers;


//============================================================================//

static void dropHandler(NetHandler *handler, const char *reason)
{
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData) {
      continue;
    }
    if (playerData->netHandler != handler) {
      continue;
    }
    removePlayer(i, reason, false);
  }
  netConnectedPeers[handler->getFD()].deleteMe = true;
}


static void bzfs_send(NetHandler *handler, const void *data, size_t size)
{
  int result = handler->pwrite(data, size);
  if (result == -1) {
    dropHandler(handler, "ECONNRESET/EPIPE");
  } else if (result == -2) {
    dropHandler(handler, "send queue too big");
  }
}


static void bzfs_broadcast(const void *data, size_t size, bool toTextClients)
{
  int result;
  std::list<NetHandler*>::const_iterator it;
  // send message to everyone
  for (it = NetHandler::netConnections.begin();
       it != NetHandler::netConnections.end();) {
    NetHandler *handler = *it;
    it++;
    if (toTextClients ||
        (handler->getClientKind() != NetHandler::clientBZAdmin)) {
      result = handler->pwrite(data, size);
      if (result == -1) {
	dropHandler(handler, "ECONNRESET/EPIPE");
      } else if (result == -2) {
	dropHandler(handler, "send queue too big");
      }
    }
  }

  //record the packet
  if (Record::enabled()) {
    const uint16_t code = ((uint16_t*)data)[1];
    const size_t lenCodeOffset = 2 * sizeof(uint16_t);
    Record::addPacket(code, size - lenCodeOffset, ((char*)data) + lenCodeOffset);
  }
}


static void initNetMessage()
{
  NetMessage::setSendFunc(bzfs_send);
  NetMessage::setBroadcastFunc(bzfs_broadcast);
}


//============================================================================//

class BZFSNetLogCB : NetworkDataLogCallback
{
public:
  BZFSNetLogCB() { addNetworkLogCallback(this); }
  virtual ~BZFSNetLogCB() { removeNetworkLogCallback(this); }

  virtual void networkDataLog(bool send, bool udp, const unsigned char* data,
                              unsigned int size, void* param = NULL)
  {

    // let any listeners know we got net data
    const bz_eEventType eventType = send ? bz_eNetDataSendEvent
                                         : bz_eNetDataReceiveEvent;

    if (!worldEventManager.getEventCount(eventType)) {
      return;
    }

    bz_NetTransferEventData_V1 eventData;
    eventData.eventType = eventType;
    eventData.send  = send;
    eventData.udp   = udp;
    eventData.iSize = size;

    NetHandler* h = (NetHandler*)param;
    if (h) {
      for (int i = 0; i < curMaxPlayers; i++) {
        GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
	if (playerData && (playerData->netHandler == h)) {
	  eventData.playerID = i;
	  break;
	}
      }
    }

    // make a copy of the data, just in case any plug-ins decide to MESS with it.
    eventData.data = (unsigned char*)malloc(size);
    memcpy(eventData.data, data, size);
    worldEventManager.callEvents(eventData.eventType, &eventData);
    free(eventData.data);
  }
};

BZFSNetLogCB netLogCB;


//============================================================================//

// Logging to the API
static void apiLoggingProc(int level, const std::string& msg, void* /*data*/)
{
  bz_LoggingEventData_V1 eventData;
  eventData.level = level;
  eventData.message = msg;
  worldEventManager.callEvents(bz_eLoggingEvent, &eventData);
}


int getCurMaxPlayers()
{
  return curMaxPlayers;
}


static bool realPlayer(const PlayerId& id)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(id);
  return playerData && playerData->player.isPlaying();
}


NetHandler* getPlayerNetHandler(int playerIndex)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return NULL;
  if (!playerData->netHandler || playerData->playerHandler)
    return NULL;
  if (playerData->isParting)
    return NULL;

  return playerData->netHandler;
}


//
// global variable callbacks
//

static void bzdbGlobalCallback(const std::string& name, void* /*data*/)
{
  const std::string value = BZDB.get(name);
  bz_BZDBChangeData_V1 eventData(name, value);
  worldEventManager.callEvents(eventData);

  // only send the network update if not replaying
  if (!Replay::enabled()) {
    NetMessage netMsg;
    netMsg.packUInt16(1);
    netMsg.packStdString(name);
    netMsg.packStdString(value);
    netMsg.broadcast(MsgSetVar);
  }
}


static void sendUDPupdate(NetHandler *handler)
{
  NetMessage netMsg;
  netMsg.send(handler, MsgUDPLinkEstablished);
  netMsg.send(handler, MsgUDPLinkRequest);
}


int lookupPlayer(const PlayerId& id)
{
  if (id == ServerPlayer)
    return id;

  if (!realPlayer(id))
    return InvalidPlayer;

  return id;
}


static float nextGameTime()
{
  float nextTime = +MAXFLOAT;
  const BzTime nowTime = BzTime::getCurrent();
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *gkPlayer = GameKeeper::Player::getPlayerByIndex(i);
    if ((gkPlayer != NULL) && gkPlayer->player.isHuman() && gkPlayer->netHandler) {
      const BzTime& pTime = gkPlayer->getNextGameTime();
      const float pNextTime = (float)(pTime - nowTime);
      if (pNextTime < nextTime) {
	nextTime = pNextTime;
      }
    }
  }
  return nextTime;
}


static void sendGameTime(GameKeeper::Player* gkPlayer)
{
  if (Replay::enabled() || (gkPlayer == NULL) || gkPlayer->playerHandler) {
    return;
  }

  NetMessage netMsg;
  GameTime::pack(netMsg, gkPlayer->lagInfo.getLagAvg());
  netMsg.send(gkPlayer->netHandler, MsgGameTime);

  gkPlayer->updateNextGameTime();

  return;
}


static void sendPendingGameTime()
{
  const BzTime nowTime = BzTime::getCurrent();
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *gkPlayer = GameKeeper::Player::getPlayerByIndex(i);
    if ((gkPlayer != NULL) && gkPlayer->player.isHuman() &&
        ((gkPlayer->getNextGameTime() - nowTime) < 0.0f)) {
      sendGameTime(gkPlayer);
    }
  }
  return;
}


static void sendPlayerUpdateB(GameKeeper::Player *playerData)
{
  if (!playerData->player.isPlaying())
    return;

  NetMessage netMsg;
  playerData->packPlayerUpdate(netMsg);
  netMsg.broadcast(MsgAddPlayer);
}


void sendPlayerInfo()
{
  NetMessage netMsg;

  int i, numPlayers = 0;
  for (i = 0; i < int(NumTeams); i++)
    numPlayers += teamInfos[i].team.size;

  netMsg.packUInt8(numPlayers);
  for (i = 0; i < curMaxPlayers; ++i) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData)
      continue;

    if (playerData->player.isPlaying()) {
      // see if any events want to update the playerInfo before it is sent out
      bz_GetPlayerInfoEventData_V1 playerInfoData;
      playerInfoData.playerID   = i;
      playerInfoData.callsign   = playerData->player.getCallSign();
      playerInfoData.team       = convertTeam(playerData->player.getTeam());
      playerInfoData.verified   = playerData->accessInfo.isVerified();
      playerInfoData.registered = playerData->accessInfo.isRegistered();
      playerInfoData.admin      = playerData->accessInfo.showAsAdmin();

      worldEventManager.callEvents(bz_eGetPlayerInfoEvent,&playerInfoData);

      PackPlayerInfo(netMsg, i, GetPlayerProperties(playerInfoData.registered,
                                                    playerInfoData.verified,
                                                    playerInfoData.admin));
    }
  }
  netMsg.broadcast(MsgPlayerInfo);
}


void sendIPUpdate(int targetPlayer, int playerIndex)
{
  // targetPlayer = -1: send to all players with the PLAYERLIST permission
  // playerIndex = -1: send info about all players

  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if ((playerIndex >= 0) && (!playerData || !playerData->player.isPlaying()))
    return;

  // send to who?
  std::vector<int> receivers
    = GameKeeper::Player::allowed(PlayerAccessInfo::playerList, targetPlayer);

  if (playerIndex >= 0) {
    for (unsigned int i = 0; i < receivers.size(); ++i)
      sendAdminInfoMessage(playerIndex,receivers[i]);

    if (Record::enabled())
      sendAdminInfoMessage(playerIndex,-1,true);
  } else {
    int i = 0;
    for (i = 0; i < curMaxPlayers; ++i) {
      playerData = GameKeeper::Player::getPlayerByIndex(i);

      if (playerData && playerData->player.isPlaying()) {
	for (unsigned int j = 0; j < receivers.size(); ++j)
	  sendAdminInfoMessage(i,receivers[j]);
      }
    }
  }
}


void pauseCountdown ( const char *pausedBy )
{
  if (clOptions->countdownPaused)
    return;

  clOptions->countdownPaused = true;
  countdownResumeTime = -1; // reset back to "unset"
  if (pausedBy)
    sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Countdown paused by %s",pausedBy).c_str());
  else
    sendMessage(ServerPlayer, AllPlayers, "Countdown paused");
}


void resumeCountdown ( const char *resumedBy )
{
  if (!clOptions->countdownPaused)
    return;

  clOptions->countdownPaused = false;
  countdownResumeTime = BZDB.evalInt(BZDBNAMES.COUNTDOWNRESTIME);

  if (countdownResumeTime <= 0) {
    // resume instantly
    countdownResumeTime = -1; // reset back to "unset"

    if (resumedBy)
      sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Countdown resumed by %s",resumedBy).c_str());
    else
      sendMessage(ServerPlayer, AllPlayers, "Countdown resumed");
  } else {
    // resume after number of seconds in countdownResumeTime
    if (resumedBy)
      sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Countdown is being resumed by %s",resumedBy).c_str());
    else
      sendMessage(ServerPlayer, AllPlayers, "Countdown is being resumed");
  }
}


void resetTeamScores()
{
  // reset team scores
  for (int i = RedTeam; i <= PurpleTeam; i++) {
    teamInfos[i].team.lost = teamInfos[i].team.won = 0;
  }
  sendTeamUpdateMessageBroadcast();
}


void startCountdown ( int delay, float limit, const char *byWho )
{
  sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Team scores reset, countdown started by %s.",byWho).c_str());

  clOptions->timeLimit = limit;
  countdownDelay = delay;

  // let everyone know what's going on
  long int timeArray[4];
  std::string matchBegins;
  if (countdownDelay == 0) {
    matchBegins = "Match begins now!";
  } else {
    BzTime::convertTime(countdownDelay, timeArray);
    std::string countdowntime = BzTime::printTime(timeArray);
    matchBegins = TextUtils::format("Match begins in about %s", countdowntime.c_str());
  }
  sendMessage(ServerPlayer, AllPlayers, matchBegins.c_str());

  BzTime::convertTime(clOptions->timeLimit, timeArray);
  std::string timelimit = BzTime::printTime(timeArray);
  matchBegins = TextUtils::format("Match duration is %s", timelimit.c_str());
  sendMessage(ServerPlayer, AllPlayers, matchBegins.c_str());

  // make sure the game always start unpaused
  clOptions->countdownPaused = false;
  countdownPauseStart = BzTime::getNullTime();
}


PingPacket getTeamCounts()
{
  pingReply.rogueCount = 0;
  pingReply.redCount = 0;
  pingReply.greenCount = 0;
  pingReply.blueCount = 0;
  pingReply.purpleCount = 0;
  pingReply.observerCount = 0;

  if (gameOver && clOptions->timeLimit > 0.0f && !clOptions->timeManualStart) {
    // pretend there are no players if the game is over, but only
    // for servers with automatic countdown because we want the server
    // to become empty, so a new countdown can start.
    // Servers with -timemanual (match servers) or plugins whch handle gameover
    // usually want people to join even when last game has just ended.
    // (FIXME: the countdown/gameover handling really needs a new concept,
    //         originally it was not possible to even join a server when gameover
    //         was reached, but this was changed for manual countdown (match) servers)
  } else {
    for (int i = 0; i < curMaxPlayers; i++) {
      GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
      if ((p == NULL))
	continue;

      if (p->player.isHuman()) {
	switch(p->player.getTeam()) {
	case HunterTeam:
	case RabbitTeam:
	case RogueTeam:
	  pingReply.rogueCount++;
	  break;

	case RedTeam:
	  pingReply.redCount++;
	  break;

	case GreenTeam:
	  pingReply.greenCount++;
	  break;

	case BlueTeam:
	  pingReply.blueCount++;
	  break;

	case PurpleTeam:
	  pingReply.purpleCount++;
	  break;

	default:
	  break;
	}
      }
    }
  }
  return pingReply;
}


//============================================================================//

static std::string publicOwner = "";


const std::string& getPublicOwner()
{
  return publicOwner;
}


void setPublicOwner(const std::string& owner)
{
  publicOwner = owner;
}


//============================================================================//

void publicize()
{
  /* // hangup any previous list server sockets
     if (listServerLinksCount)
     listServerLink.closeLink(); */

  if (!bz_getPublic())
    return;

  listServerLinksCount = 0;

  if (listServerLink)
    delete listServerLink;

  if (clOptions->publicizeServer) {
    // list server initialization
    for (std::vector<std::string>::const_iterator i = clOptions->listServerURL.begin(); i < clOptions->listServerURL.end(); i++) {
      listServerLink = new ListServerLink(i->c_str(),
					  clOptions->publicizedAddress, clOptions->publicizedTitle, clOptions->advertiseGroups, ListServerReAddTime*2.0f); /* recheck dns every other re-add */
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

  packWorldSettings();

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
    BzfNetwork::closeSocket(wksSocket);
    return false;
  }
#endif
  if (bind(wksSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
    if (!clOptions->useFallbackPort) {
      nerror("couldn't bind connect socket");
      BzfNetwork::closeSocket(wksSocket);
      return false;
    }

    // if we get here then try binding to any old port the system gives us
    addr.sin_port = htons(0);
    if (bind(wksSocket, (const struct sockaddr*)&addr, sizeof(addr)) == -1) {
      nerror("couldn't bind connect socket");
      BzfNetwork::closeSocket(wksSocket);
      return false;
    }

    // fixup ping reply
    socklen_t addrLen = sizeof(addr);
    if (getsockname(wksSocket, (struct sockaddr*)&addr, &addrLen) >= 0)
      pingReply.serverId.port = addr.sin_port;

    // fixup publicized name will want it here later
    clOptions->wksPort = ntohs(addr.sin_port);
  }

  if (listen(wksSocket, 5) == -1) {
    nerror("couldn't make connect socket queue");
    BzfNetwork::closeSocket(wksSocket);
    return false;
  }

  addr.sin_port = htons(clOptions->wksPort);
  if (!NetHandler::initHandlers(addr)) {
    BzfNetwork::closeSocket(wksSocket);
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
  shutdown(wksSocket, SHUT_RDWR);
  BzfNetwork::closeSocket(wksSocket);

  // tell all clients to quit
  NetMessage netMsg;
  netMsg.packUInt8(0xff);
  netMsg.broadcast(MsgSuperKill);
  BzTime::sleep(0.5);	// provide some time for message delivery

  // clean up Kerberos
  Authentication::cleanUp();
}


static bool allBasesDefined()
{
  if (clOptions->gameType == ClassicCTF) {
    for (int i = RedTeam; i <= PurpleTeam; i++) {
      if ((clOptions->maxTeam[i] > 0) && bases.find(i) == bases.end()) {
	// If this team has been set by some form of -mp, then warn.
	if (clOptions->maxTeam[i] < MaxPlayers)
	  std::cerr << "WARNING: A base was not defined for "
		    << Team::getName((TeamColor)i)
		    << ". This team has been disabled."
		    << std::endl;
	clOptions->maxTeam[i] = 0;
      }
    }
  }
  return true;
}


bool defineWorld()
{
  logDebugMessage(1,"defining world\n");

  // clean up old data
  if (world) {
    delete world;
    world = NULL;
  }
  if (worldDatabase) {
    delete[] worldDatabase;
    worldDatabase = NULL;
  }

  bz_GetWorldEventData_V1 worldData;
  worldData.ctf     = (clOptions->gameType == ClassicCTF);
  worldData.rabbit  = (clOptions->gameType == RabbitChase);
  worldData.openFFA = (clOptions->gameType == OpenFFA);
  worldData.worldFile = clOptions->worldFile;

  worldEventManager.callEvents(bz_eGetWorldEvent, &worldData);

  if (!worldData.generated && worldData.worldFile.size()) {
    clOptions->worldFile = worldData.worldFile.c_str();
    if (worldData.ctf)
      clOptions->gameType = ClassicCTF;
    else if (worldData.rabbit)
      clOptions->gameType = RabbitChase;
    else if (worldData.openFFA)
      clOptions->gameType = OpenFFA;
    else
      clOptions->gameType = TeamFFA;

    // todo.. load this map's options and vars and stuff.
  }

  // make world and add buildings
  if (worldData.worldBlob != NULL) {
    logDebugMessage(1, "reading worldfile from memory\n");
    std::istringstream in(worldData.worldBlob);
    BZWReader reader(in);
    world = reader.defineWorldFromFile();

    if (!allBasesDefined()) {
      delete world;
      world = NULL;
      return false;
    }
  }
  else if (clOptions->worldFile.size()) {
    logDebugMessage(1, "reading worldfile %s\n", clOptions->worldFile.c_str());
    BZWReader reader(clOptions->worldFile);
    world = reader.defineWorldFromFile();

    if (!allBasesDefined()) {
      delete world;
      world = NULL;
      return false;
    }
  }
  else {
    // check and see if anyone wants to define the world from an event
    if (!worldData.generated) {
      logDebugMessage(1, "building random map\n");
      delete world;
      world = (clOptions->gameType == ClassicCTF) ? defineTeamWorld()
                                                  : defineRandomWorld();
    }
    else {
      logDebugMessage(1, "loading plug-in map\n");
      float worldSize = BZDBCache::worldSize;
      if (pluginWorldSize > 0) {
	worldSize = pluginWorldSize;
      } else {
	pluginWorldSize = worldSize;
      }

      float wallHeight = BZDB.eval(BZDBNAMES.WALLHEIGHT);
      if (pluginWorldHeight > 0) {
	wallHeight = pluginWorldHeight;
      } else {
	pluginWorldHeight = wallHeight;
      }

      const float hws = 0.5f * worldSize;
      world->addWall(0.0f, +hws, 0.0f, (float)(1.5 * M_PI), hws, wallHeight);
      world->addWall(+hws, 0.0f, 0.0f, (float)(1.0 * M_PI), hws, wallHeight);
      world->addWall(0.0f, -hws, 0.0f, (float)(0.5 * M_PI), hws, wallHeight);
      world->addWall(-hws, 0.0f, 0.0f, (float)(0.0 * M_PI), hws, wallHeight);

      OBSTACLEMGR.makeWorld();
      world->finishWorld();
    }
  }

  if (world == NULL) {
    return false;
  }

  maxWorldHeight = world->getMaxWorldHeight();

  logDebugMessage(1,"packing world database\n");
  // package up world
  world->packDatabase();

  // now get world packaged for network transmission
  worldDatabaseSize =
    sizeof(uint16_t) + // WorldCodeHeaderSize
    sizeof(uint16_t) + // WorldCodeHeader
    WorldCodeHeaderSize +
    world->getDatabaseSize() +
    sizeof(uint16_t) + // WorldCodeEndSize
    WorldCodeEndSize + 
    sizeof(uint16_t);  // WorldCodeEnd

  worldDatabase = new char[worldDatabaseSize];
  // this should NOT happen but it does sometimes
  if (!worldDatabase)
    return false;

  memset(worldDatabase, 0, worldDatabaseSize);

  void *buf = worldDatabase;
  buf = nboPackUInt16(buf, WorldCodeHeaderSize);
  buf = nboPackUInt16(buf, WorldCodeHeader);
  buf = nboPackUInt16(buf, mapVersion);
  buf = nboPackUInt32(buf, world->getUncompressedSize());
  buf = nboPackUInt32(buf, world->getDatabaseSize());
  buf = nboPackString(buf, world->getDatabase(), world->getDatabaseSize());
  buf = nboPackUInt16(buf, WorldCodeEndSize);
  buf = nboPackUInt16(buf, WorldCodeEnd);

  const BzTime startTime = BzTime::getCurrent();
  {
    MD5 md5;
    md5.update((unsigned char *)worldDatabase, worldDatabaseSize);
    md5.finalize();
    if (clOptions->worldFile == "")
      strcpy(hexDigest, "t");
    else
      strcpy(hexDigest, "p");

    std::string digest = md5.hexdigest();
    strncat(hexDigest, digest.c_str(), 49);
  }
  const BzTime endTime = BzTime::getCurrent();
  logDebugMessage(3,"MD5 generation: %.3f seconds\n", endTime - startTime);

  // water levels probably require flags on buildings
  const float waterLevel = world->getWaterLevel();
  if (!clOptions->flagsOnBuildings && (waterLevel > 0.0f)) {
    clOptions->flagsOnBuildings = true;
    clOptions->respawnOnBuildings = true;
    logDebugMessage(1,"WARNING: enabling flag and tank spawns on buildings due to waterLevel\n");
  }

  // reset other stuff
  int i;
  for (i = 0; i < NumTeams; i++) {
    teamInfos[i].team.size = 0;
    teamInfos[i].team.won = 0;
    teamInfos[i].team.lost = 0;
  }
  FlagInfo::setNoFlagInAir();
  for (i = 0; i < numFlags; i++)
    resetFlag(*FlagInfo::get(i));

  bz_EventData eventData = bz_EventData(bz_eWorldFinalized);
  worldEventManager.callEvents(eventData);
  return true;
}


bool saveWorldCache ( const char* fileName )
{
  FILE* file;
  if (fileName)
    file = fopen (fileName, "wb");
  else
    file = fopen (clOptions->cacheOut.c_str(), "wb");
  if (file == NULL)
    return false;

  size_t written =
    fwrite (worldDatabase, sizeof(char), worldDatabaseSize, file);
  fclose (file);
  if (written != worldDatabaseSize)
    return false;

  return true;
}


TeamColor whoseBase(float x, float y, float z)
{
  if (clOptions->gameType!= ClassicCTF)
    return NoTeam;

  float highest = -1;
  int highestteam = -1;

  const fvec3 pos(x, y, z);
  for (BasesList::iterator it = bases.begin(); it != bases.end(); ++it) {
    const float baseZ = it->second.findBaseZ(pos);
    if (baseZ > highest) {
      highest = baseZ;
      highestteam = it->second.getTeam();
    }
  }

  if (highestteam == -1) {
    return NoTeam;
  }
  return TeamColor(highestteam);
}


static void dumpScore()
{
  if (!clOptions->printScore)
    return;

  if (clOptions->timeLimit > 0.0f)
    std::cout << "#time " << clOptions->timeLimit - clOptions->timeElapsed << std::endl;

  std::cout << "#teams";
  for (int i = int(RedTeam); i < NumTeams; i++)
    std::cout << ' ' << teamInfos[i].team.won << '-' << teamInfos[i].team.lost << ' ' << Team::getName(TeamColor(i));

  GameKeeper::Player::dumpScore();
  std::cout << "#end" << std::endl;
}


static void handleCommand(const void *rawbuf, bool udp, NetHandler *handler)
{
  if (!rawbuf) {
    std::cerr << "WARNING: handleCommand got a null rawbuf?!" << std::endl;
    return;
  }

  if (!handler)	{ // WTF?
    return;
  }

  // pull off the BZFS code and size that packs all BZFS fomat comunication
  uint16_t len, code;
  void *buf = (char *)rawbuf;
  getGeneralMessageInfo(&buf,code,len);

  // make sure it's not an attack
  if (udp && isUDPAttackMessage(code)) {
    logDebugMessage(1,
      "Received packet type (%x) via udp, possible attack from %s\n",
      code, handler->getTargetIP());
  }
#ifdef DEBUG
  else if (code != MsgPlayerUpdateSmall) {
    const MsgStringList msgList = MsgStrings::msgFromClient(len, code, buf);
    logDebugMessage(5, "%s from %s:%s at %f\n", msgList[0].text.c_str(),
      handler->getTargetIP(), (udp ? "udp" : "tcp"),
      BzTime::getCurrent().getSeconds());
  }
#endif

  // see if we have any registered handlers for this message type
  bool handled = false;

  std::map<uint16_t, PlayerNetworkMessageHandler*>::iterator playerItr =
    playerNetworkHandlers.find(code);

  if (playerItr != playerNetworkHandlers.end()) {
    PlayerNetworkMessageHandler* playerHandler = playerItr->second;
    // player messages all start with the player ID first
    // so get it, and verify that the sender IS the player.
    // TODO, punish the person who owns handler, as they are up to no good
    buf = playerHandler->unpackPlayer(buf,len);
    if (playerHandler->getPlayer() &&
        playerHandler->getPlayer()->netHandler == handler) {
      handled = playerHandler->execute(code, buf, len);
    }
  }
  else {
    // try a non player message
    // they don't start with a player
    std::map<uint16_t, ClientNetworkMessageHandler*>::iterator clientItr =
      clientNetworkHandlers.find(code);
    if (clientItr != clientNetworkHandlers.end()) {
      handled = clientItr->second->execute(handler,code,buf,len);
    }
  }

  if (!handled)	{ // someone got it, don't need to do the old way
    logDebugMessage(1,
      "Received an unknown packet type (%x), possible attack from %s\n",
      code, handler->getTargetIP());
  }
}


static void handleTcp(NetHandler &netPlayer, int i, const RxStatus e)
{
  if (e != ReadAll) {
    if (e == ReadReset) {
      dropHandler(&netPlayer, "ECONNRESET/EPIPE");
    } else if (e == ReadError) {
      // dump other errors and remove the player
      nerror("error on read");
      dropHandler(&netPlayer, "Read error");
    } else if (e == ReadDiscon) {
      // disconnected
      dropHandler(&netPlayer, "Disconnected");
    } else if (e == ReadHuge) {
      logDebugMessage(1,"Player [%d] sent huge packet length, possible attack\n", i);
      dropHandler(&netPlayer, "large packet recvd");
    }
    return;
  }

  uint16_t len, code;
  void *buf = netPlayer.getTcpBuffer();
  buf = nboUnpackUInt16(buf, len);
  buf = nboUnpackUInt16(buf, code);

  // trying to get the real player from the message: bots share tcp
  // connection with the player
  PlayerId t = i;
  switch (code) {
  case MsgShotBegin: {
    nboUnpackUInt8(buf, t);
    break;
  }
  case MsgPlayerUpdate:
  case MsgPlayerUpdateSmall: {
    double timestamp;
    buf = nboUnpackUInt8(buf, t);
    buf = nboUnpackDouble(buf, timestamp);
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
  if (true && playerData != NULL && !playerData->player.isBot()) {
    if (code == MsgShotBegin) {
      sendMessage(ServerPlayer, i, "Your end is not using UDP.");
      sendMessage(ServerPlayer, i, "Turn on UDP on your firewall or router.");
      removePlayer(i, "no UDP", true);
      return;
    }
  }

  // handle the command
  handleCommand(netPlayer.getTcpBuffer(), false, &netPlayer);
}


PlayerId getNewPlayer(NetHandler *netHandler)
{
  PlayerId playerIndex = getNewPlayerID();

  new GameKeeper::Player(playerIndex, netHandler, handleTcp);

  checkGameOn();

  return playerIndex;
}


PlayerId getNewBot(PlayerId hostPlayer, int botID)
{
  PlayerId playerIndex = getNewPlayerID();

  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(hostPlayer);
  if (!player || !player->netHandler)
    return 0xFF;

  GameKeeper::Player *newPlayer = new GameKeeper::Player(playerIndex, player->netHandler, handleTcp);

  // set bot data
  newPlayer->setBot(botID,hostPlayer);
  player->addBot(botID,playerIndex);

  checkGameOn();

  return playerIndex;
}


PlayerId getNewPlayerID()
{
  PlayerId playerIndex;

  // find open slot in players list
  PlayerId minPlayerId = 0, maxPlayerId = (PlayerId)MaxPlayers;
  if (Replay::enabled()) {
    minPlayerId = MaxPlayers;
    maxPlayerId = MaxPlayers + ReplayObservers;
  }
  playerIndex = GameKeeper::Player::getFreeIndex(minPlayerId, maxPlayerId);

  if (playerIndex >= maxPlayerId)
    return 0xff;

  if (playerIndex >= curMaxPlayers)
    curMaxPlayers = playerIndex + 1;

  return playerIndex;
}


void checkGameOn()
{
  // if game was over and this is the first player then game is on
  if (gameOver) {
    int count = GameKeeper::Player::count();
    if (count == 0) {
      gameOver = false;
      gameStartTime = BzTime::getCurrent();
      if (clOptions->timeLimit > 0.0f && !clOptions->timeManualStart) {
	clOptions->timeElapsed = 0.0f;
	countdownActive = true;
      }
    }
  }
}


static void acceptClient()
{
  // client (not a player yet) is requesting service.
  // accept incoming connection on our well known port
  struct sockaddr_in clientAddr;
  socklen_t addr_len = sizeof(clientAddr);
  int fd = accept(wksSocket, (struct sockaddr*)&clientAddr, &addr_len);
  if (fd == -1) {
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
  NetConnectedPeer peer;
  peer.netHandler = new NetHandler(clientAddr, fd);
  peer.apiHandler = NULL;
  peer.player = -1;
  peer.socket = fd;
  peer.deleteMe = false;
  peer.sent = false;
  peer.startTime = BzTime::getCurrent();

  netConnectedPeers[fd] = peer;
}


static void respondToPing(Address addr)
{
  // reply with current game info
  pingReply.sourceAddr    = addr;
  pingReply.rogueCount    = (uint8_t)teamInfos[0].team.size;
  pingReply.redCount      = (uint8_t)teamInfos[1].team.size;
  pingReply.greenCount    = (uint8_t)teamInfos[2].team.size;
  pingReply.blueCount     = (uint8_t)teamInfos[3].team.size;
  pingReply.purpleCount   = (uint8_t)teamInfos[4].team.size;
  pingReply.observerCount = (uint8_t)teamInfos[5].team.size;
}


void sendFilteredMessage(int sendingPlayer, PlayerId recipientPlayer, const char *message, uint8_t type)
{
  const char* msg = message;
  char filtered[MessageLen] = {0};
  char adminmsg[MessageLen] = {0};
  bool msgWasFiltered = false;

  if (clOptions->filterChat) {
    strncpy(filtered, message, MessageLen);

    clOptions->filter.filter(filtered, clOptions->filterSimple);

    msg = filtered;

    if (strcmp(message,filtered) != 0) {	// the filter did do something so barf a message
      bz_MessageFilteredEventData_V1	eventData;

      msgWasFiltered = true;
      eventData.playerID = sendingPlayer;
      eventData.rawMessage = message;
      eventData.filteredMessage = filtered;

      worldEventManager.callEvents(bz_eMessageFilteredEvent,&eventData);
    }
  }

  if (sendingPlayer != ServerPlayer) {
    // check that the player has the talk permission
    GameKeeper::Player *senderData = GameKeeper::Player::getPlayerByIndex(sendingPlayer);

    if (!senderData)
      return;

    if (!senderData->accessInfo.hasPerm(PlayerAccessInfo::talk)) {

      // if the player were sending to is an admin
      GameKeeper::Player *recipientData = GameKeeper::Player::getPlayerByIndex(recipientPlayer);

      // don't care if they're real, just care if they're an admin
      if ( !(recipientData && recipientData->accessInfo.isOperator()) && (recipientPlayer != AdminPlayers)) {
        sendMessage(ServerPlayer, sendingPlayer, "We're sorry, you are not allowed to talk!");
        return;
      }
    }
  }

  bz_ChatEventData_V1 chatData;
  chatData.eventType = bz_eFilteredChatMessageEvent;
  chatData.from = sendingPlayer;
  chatData.to = BZ_NULLUSER;

  if (recipientPlayer == AllPlayers) {
    chatData.to = BZ_ALLUSERS;
  } else if (recipientPlayer == AdminPlayers) {
    chatData.team = eAdministrators;
  } else if (recipientPlayer > LastRealPlayer) {
    chatData.team = convertTeam((TeamColor)(250 - recipientPlayer));
  } else {
    chatData.to = recipientPlayer;
  }

  chatData.message = msg;

  // send any events that want to watch the chat
  if (chatData.message.size())
    worldEventManager.callEvents(bz_eFilteredChatMessageEvent, &chatData);

  if (chatData.message.size()) {
    if (sendingPlayer == BZ_SERVER) {
      sendingPlayer = ServerPlayer;
    }
    sendMessage(sendingPlayer, recipientPlayer, chatData.message.c_str(), type);
  }

  // If the message was filtered report it on the admin channel
  if (msgWasFiltered && clOptions->filterAnnounce) {
    GameKeeper::Player *senderData = GameKeeper::Player::getPlayerByIndex(sendingPlayer);
    if (senderData) {
      snprintf(adminmsg, MessageLen, "Filtered Msg: %s said \"%s\"", senderData->player.getCallSign(), message);
      sendMessage(ServerPlayer, AdminPlayers, adminmsg);
    }
  }
}


void sendPlayerMessage(GameKeeper::Player *playerData, PlayerId dstPlayer,
		       const char *message)
{
  const PlayerId srcPlayer = playerData->getIndex();
  uint8_t type = ChatMessage;

  // reformat any '/me' action messages
  // this is here instead of in commands.cxx to allow player-player/player-channel targetted messages
  if (strncasecmp(message, "/me", 3) == 0) {

    // don't bother with empty messages
    if (message[3] == '\0' || (isspace(message[3]) && message[4] == '\0')) {
      char reply[MessageLen] = {0};
      snprintf(reply, MessageLen, "%s, the /me command requires an argument", playerData->player.getCallSign());
      sendMessage(ServerPlayer, srcPlayer, reply);
      return;
    }

    // don't intercept other messages beginning with /me...
    if (!isspace(message[3])) {
      parseServerCommand(message, srcPlayer);
      return;
    }

    // check for permissions
    if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::actionMessage)) {
      char reply[MessageLen] = {0};
      snprintf(reply, MessageLen, "%s, you are not presently authorized to perform /me actions", playerData->player.getCallSign());
      sendMessage(ServerPlayer, srcPlayer, reply);
      return;
    }

	// Trim off the command to leave the player's message
    message = message + 4;

	// Set the message type to an action message
	type = ActionMessage;
  }

  // check for a server command
  else if ((message[0] == '/') && (isalpha(message[1]) || message[1] == '?')) {
    // record server commands
    if (Record::enabled()) {
      NetMessage netMsg;
      netMsg.packUInt8(srcPlayer);
      netMsg.packUInt8(dstPlayer);
      netMsg.packUInt8(type);
      netMsg.packString(message, strlen(message) + 1);
      Record::addPacket(MsgMessage, netMsg, HiddenPacket);
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

  sendChatMessage(srcPlayer, dstPlayer, message, type);
}


void sendChatMessage(PlayerId srcPlayer, PlayerId dstPlayer, const char *message, uint8_t type)
{
  bz_ChatEventData_V1 chatData;
  chatData.from = BZ_SERVER;
  if (srcPlayer != ServerPlayer)
    chatData.from = srcPlayer;

  chatData.to = BZ_NULLUSER;

  if (dstPlayer == AllPlayers)
    chatData.to = BZ_ALLUSERS;
  else if ( dstPlayer == AdminPlayers )
    chatData.team = eAdministrators;
  else if ( dstPlayer > LastRealPlayer )
    chatData.team = convertTeam((TeamColor)(250-dstPlayer));
  else
    chatData.to = dstPlayer;

  chatData.message = message;

  // send any events that want to watch the chat
  if (chatData.message.size())
    worldEventManager.callEvents(bz_eRawChatMessageEvent,&chatData);

  if (chatData.message.size())
    sendFilteredMessage(srcPlayer, dstPlayer, chatData.message.c_str(), type);
}


void sendMessage(int playerIndex, PlayerId dstPlayer, const char *message, uint8_t type)
{
  long int msglen = strlen(message) + 1; // include null terminator
  const char *msg = message;

  if (message[0] == '/' && message[1] == '/')
    msg = &message[1];

  // Should cut the message
  if (msglen > MessageLen) {
    logDebugMessage(1,"WARNING: Network message being sent is too long! "
		    "(message is %d, cutoff at %d)\n", msglen, MessageLen);
    msglen = MessageLen;
  }

  // TODO: Add the message type to the API
  // Notify any plugins
  if (playerIndex == ServerPlayer) {
    bz_ServerMsgEventData_V1 serverMsgData;
    switch (dstPlayer) {
    case AdminPlayers:
      serverMsgData.to = BZ_NULLUSER;
      serverMsgData.team = eAdministrators;
      break;
    case AllPlayers:
      serverMsgData.to = BZ_ALLUSERS;
      break;
    default:
      if (dstPlayer <= LastRealPlayer) {
	serverMsgData.to = dstPlayer;
      } else {
	serverMsgData.to = BZ_NULLUSER;
	// FIXME this teamcolor <-> player id conversion is in several files now
	serverMsgData.team = convertTeam(TeamColor(250 - dstPlayer));
      }
    }
    serverMsgData.message = message;
    worldEventManager.callEvents(bz_eServerMsgEvent, &serverMsgData);
  }

  sendTextMessage(dstPlayer, playerIndex, msg, msglen, type);

  if (Record::enabled() && !(dstPlayer == AllPlayers)) // don't record twice
    sendTextMessage(-1, playerIndex, msg, msglen, type, true);
}


void rejectPlayer(int playerIndex, uint16_t code, const char *reason)
{
  char message[MessageLen];

  // tell them they were rejected
  sendRejectPlayerMessage(playerIndex,code,reason);
  // remove player so he can not ignore the reject message and, then can avoid a ban, hostban...
  snprintf(message, MessageLen, "rejected: %s", reason); 
  removePlayer(playerIndex, message, true);
  return;
}


// FIXME this is a workaround for a bug, still needed?
// Team Size is wrong at some time
static void fixTeamCount() {
  int playerIndex, teamNum;
  for (teamNum = RogueTeam; teamNum < NumTeams; teamNum++) {
    teamInfos[teamNum].team.size = 0;
  }
  for (playerIndex = 0; playerIndex < curMaxPlayers; playerIndex++) {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(playerIndex);
    if (p && p->player.isPlaying()) {
      teamNum = p->player.getTeam();
      if (teamNum == RabbitTeam)
	teamNum = HunterTeam;
      teamInfos[teamNum].team.size++;
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
      numplayers += teamInfos[i].team.size;

  // if no player are available, join as Observer
  if (numplayers == maxRealPlayers)
    return ObserverTeam;

  // if we're running rabbit chase, all non-observers start as hunters
  if (clOptions->gameType == RabbitChase)
    return HunterTeam;

  // If tank ask for rogues, and rogues are allowed, give it
  if ((t == RogueTeam)
      && teamInfos[RogueTeam].team.size < clOptions->maxTeam[RogueTeam])
    return RogueTeam;

  // If no auto-team, server or client, go back with client choice
  if (!clOptions->autoTeam && t != AutomaticTeam)
    return t;

  // Fill a vector with teams status, not putting in not enabled teams
  std::vector<TeamSize> teams;

  for (i = (int)RedTeam; i < (int)ObserverTeam; i++) {
    TeamSize currTeam = {(TeamColor)i,
			 teamInfos[i].team.size,
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
	    const float uptime = (float)(BzTime::getCurrent() - BzTime::getStartTime());
	    snprintf(buffer, 16, "%i", (int)uptime);
	    eval += buffer;
	  }
	  else {
	    eval += "*BADVAR*";
	  }
	}
	break;
      }
      default:
	break;
      }
    }
  }
  return eval;
}


static bool spawnSoon = false;


bool validPlayerCallsign ( int playerIndex )
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (clOptions->filterCallsigns) {
    int filterIndex = 0;
    Filter::Action filterAction = filter.check(*playerData, filterIndex);
    if (filterAction == Filter::DROP) {
      rejectPlayer(playerIndex, RejectBadCallsign, "Unacceptable Callsign");
      return false;
    }
  }
  return true;
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


void addPlayer(int playerIndex, GameKeeper::Player *playerData)
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

      GameKeeper::Player *otherPlayer = GameKeeper::Player::getPlayerByIndex(i);
      if (!otherPlayer)
	continue;

      if (TextUtils::compare_nocase(otherPlayer->player.getCallSign(),
				    playerData->player.getCallSign()) == 0) {
	if ( !otherPlayer->accessInfo.regAtJoin ) {
	  // if the player was not reged at join they have done nothing wrong
	  rejectPlayer(playerIndex, RejectBadCallsign, "Ghostie was not registered on join");
	  return;
	} else {
	  sendMessage(ServerPlayer, i ,
		      "Another client has demonstrated ownership of your "
		      "callsign with the correct password.  You have been "
		      "ghosted.");
	  removePlayer(i, "Ghost", true);
	}
	break;
      }
    }
  }
  if (!validPlayerCallsign(playerIndex))
    return;

  const bool playerIsAntiBanned =
    playerData->accessInfo.hasPerm(PlayerAccessInfo::antiban);

  // no need to ban local players, if they arn't suposed to be here, don't load them
  if (playerData->playerHandler) {
    playerData->setNeedThisHostbanChecked(false);
  }
  else {
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
      if (idInfo.reason.size())
	rejectionMessage += idInfo.reason;
      else
	rejectionMessage += "General Ban";

      rejectionMessage += ColorStrings[WhiteColor];
      if (idInfo.bannedBy.size()) {
	rejectionMessage += " by ";
	rejectionMessage += ColorStrings[BlueColor];
	rejectionMessage += idInfo.bannedBy;
      }

      rejectionMessage += ColorStrings[GreenColor];
      if (idInfo.fromMaster)
	rejectionMessage += " [from the master server]";

      rejectPlayer(playerIndex, RejectIDBanned, rejectionMessage.c_str());
      return;
    }

    // check against id and hostname ban lists (on the next cycle)
    playerData->setNeedThisHostbanChecked(true);
  }

  // see if any watchers don't want this guy
  bz_AllowPlayerEventData_V1 allowData;
  allowData.callsign = playerData->player.getCallSign();
  if (playerData->netHandler)
    allowData.ipAddress = playerData->netHandler->getTargetIP();
  else
    allowData.ipAddress = "local.player";

  allowData.playerID = playerIndex;

  worldEventManager.callEvents(bz_eAllowPlayer,&allowData);
  if (!allowData.allow) {
    rejectPlayer(playerIndex, RejectBadRequest, allowData.reason.c_str());
    return;
  }

  // pick a team
  TeamColor t = autoTeamSelect(playerData->player.getTeam());

  bz_GetAutoTeamEventData_V1 autoTeamData;
  autoTeamData.eventType = bz_eGetAutoTeamEvent;
  autoTeamData.playerID = playerIndex;
  autoTeamData.team = convertTeam(t);
  autoTeamData.callsign = playerData->player.getCallSign();
  worldEventManager.callEvents(bz_eGetAutoTeamEvent,&autoTeamData);

  playerData->setTeam(convertTeam(autoTeamData.team), false);
  playerData->player.endShotCredit = 0;	// reset shotEndCredit

  // count current number of players and players+observers
  int numplayers = 0;
  for (int i = 0; i < int(ObserverTeam); i++)
    numplayers += teamInfos[i].team.size;
  const int numplayersobs = numplayers + teamInfos[ObserverTeam].team.size;

  if (!playerData->playerHandler) {	// locals can rejoin as fast as they want
    // no quick rejoining, make 'em wait
    // you can switch to observer immediately, or switch from observer
    // to regular player immediately, but only if last time time you
    // were a regular player isn't in the rejoin list. As well, this all
    // only applies if the game isn't currently empty.
    if ((playerData->player.getTeam() != ObserverTeam) &&
	(GameKeeper::Player::count() >= 0)) {
      float waitTime = rejoinList.waitTime(playerIndex);
      if (waitTime > 0.0f) {
	char buffer[MessageLen] = {0};
	logDebugMessage(2,"Player %s [%d] rejoin wait of %.1f seconds\n",
			playerData->player.getCallSign(), playerIndex, waitTime);
	snprintf(buffer, MessageLen, "You are unable to begin playing for %.1f seconds.", waitTime);
	sendMessage(ServerPlayer, playerIndex, buffer);
      }
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
  } else if (teamInfos[int(t)].team.size >= clOptions->maxTeam[int(t)]) {
    rejectPlayer(playerIndex, RejectTeamFull,
		 "This team is full.  Try another team.");
    return ;
  }

  if (!sendAcceptPlayerMessage(playerIndex)) {
    return;
  }

  // abort if we hung up on the client
  if (!GameKeeper::Player::getPlayerByIndex(playerIndex)) {
    return;
  }

  // player is signing on (has already connected via addClient).
  playerData->signingOn(clOptions->gameType == ClassicCTF);

  // update team state and if first player on team, reset it's score
  int teamIndex = int(playerData->player.getTeam());
  teamInfos[teamIndex].team.size++;
  if (teamInfos[teamIndex].team.size == 1
      && Team::isColorTeam((TeamColor)teamIndex)) {
    teamInfos[teamIndex].team.won = 0;
    teamInfos[teamIndex].team.lost = 0;
  }

  // send new player updates on each player, all existing flags, and all teams.
  // don't send robots any game info.  watch out for connection being closed
  // because of an error.
  if (playerData->player.isHuman()) {
    if (!sendTeamUpdateMessage(playerIndex))
      return;
    sendFlagUpdateMessage(playerIndex);
  }
  if (!playerData->player.isBot()) {
    sendExistingPlayerUpdates(playerIndex);
    sendHandicapInfoUpdate(playerIndex);
  }

  // if new player connection was closed (because of an error) then stop here
  if (!GameKeeper::Player::getPlayerByIndex(playerIndex)) {
    return;
  }

  // send MsgAddPlayer to everybody -- this concludes MsgEnter response
  // to joining player
  sendPlayerUpdateB(playerData);

  // send update of info for team just joined
  sendTeamUpdateMessageBroadcast(teamIndex);

  // again check if player was disconnected
  if (!GameKeeper::Player::getPlayerByIndex(playerIndex)) {
    return;
  }

  // send (possibly resend), all of the player's custom data
  broadcastPlayerCustomDataMap(playerData);

  // send IP update to everyone with PLAYERLIST permission
  sendIPUpdate(-1, playerIndex);
  sendIPUpdate(playerIndex, -1);

  // send rabbit information
  if (clOptions->gameType == RabbitChase) {
    NetMessage netMsg;
    netMsg.packUInt8(rabbitIndex);
    netMsg.packUInt8(0); // swap mode
    netMsg.send(playerData->netHandler, MsgNewRabbit);
  }

  // again check if player was disconnected
  if (!GameKeeper::Player::getPlayerByIndex(playerIndex))
    return;

  // if first player on team add team's flag
  if (teamInfos[teamIndex].team.size == 1
      && Team::isColorTeam((TeamColor)teamIndex)) {
    if (clOptions->gameType == ClassicCTF) {
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
  if(listServerLink)
    listServerLink->queueMessage(ListServerLink::ADD);

  dumpScore();

  char message[MessageLen] = {0};

#ifdef SERVERLOGINMSG
  snprintf(message, MessageLen, "BZFlag server %s, http://BZFlag.org/", getAppVersion());
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
  if (lines != NULL) {
    for (int i = 0; i < (int)lines->size(); i ++) {
      const std::string srvmsg = evaluateString((*lines)[i]);
      sendMessage(ServerPlayer, playerIndex, srvmsg.c_str());
    }
  }

  if (playerData->player.isObserver())
    sendMessage(ServerPlayer, playerIndex, "You are in observer mode.");
#endif

  // send time update to new player if we're counting down
  if (countdownActive && clOptions->timeLimit > 0.0f
      && !playerData->player.isBot()) {
    if (countdownPauseStart) {
      gameStartTime += (BzTime::getCurrent() - countdownPauseStart);
      countdownPauseStart = BzTime::getNullTime();
    }
    float timeLeft = clOptions->timeLimit - (float)(BzTime::getCurrent() - gameStartTime);
    if (timeLeft < 0.0f)
      // oops
      timeLeft = 0.0f;

    NetMessage netMsg;
    netMsg.packInt32((int32_t)timeLeft);
    netMsg.send(playerData->netHandler, MsgTimeUpdate);
    // players should know when the countdown is paused
    if (clOptions->countdownPaused) {
      long int timeArray[4];
      BzTime::convertTime(timeLeft, timeArray);
      std::string remainingTime = BzTime::printTime(timeArray);
      char reply[MessageLen] = {0};
      snprintf(reply, MessageLen, "Countdown is paused. %s remaining.", remainingTime.c_str());
      sendMessage(ServerPlayer, playerData->getIndex(), reply);
      netMsg.clear();
      netMsg.packInt32(-1);
      netMsg.send(playerData->netHandler, MsgTimeUpdate);
    }
  }

  playerData->accessInfo.regAtJoin = playerData->accessInfo.isRegistered();

  if (GameKeeper::Player::getPlayerByIndex(playerIndex) &&
      playerData->accessInfo.isRegistered() &&
      playerData->_LSAState != GameKeeper::Player::verified) {
    // nick is in the DB send him a message to identify.
    sendMessage(ServerPlayer, playerIndex, "This callsign is registered. You must use global registration.");
  }

  dropAssignedFlag(playerIndex);
  sendOSRequest(playerIndex);

  sendPlayerInfo();
  ServerIntangibilityManager::instance().sendNewPlayerWorldTangibility(playerIndex);

  // call any on join events
  bz_PlayerJoinPartEventData_V1	joinEventData;
  joinEventData.eventType = bz_ePlayerJoinEvent;
  joinEventData.playerID = playerIndex;
  joinEventData.record = bz_getPlayerByIndex(playerIndex);

  // don't give events if we don't have a real player slot
  if ((playerData->player.getTeam() != NoTeam) && strlen(playerData->player.getCallSign())) {
    worldEventManager.callEvents(bz_ePlayerJoinEvent,&joinEventData);
  }

  if (spawnSoon)
    playerAlive(playerIndex);

  playerData->player.setCompletelyAdded();
}


void resetFlag(FlagInfo &flag)
{
  // NOTE -- must not be called until world is defined
  assert(world != NULL);

  float baseSize = BZDB.eval(BZDBNAMES.BASESIZE);

  // reposition flag (middle of the map might be a bad idea)
  fvec3 flagPos(0.0f, 0.0f, 0.0f);

  int teamIndex = flag.teamIndex();
  if ((teamIndex >= ::RedTeam) &&  (teamIndex <= ::PurpleTeam)
      && (bases.find(teamIndex) != bases.end())) {
    if (!world->getFlagSpawnPoint(&flag, flagPos)) {
      // return the flag to the center of the top of one of the team
      // bases.. we assume it'll fit.
      TeamBases& teamBases = bases[teamIndex];
      const TeamBase& base = teamBases.getRandomBase();
      base.getTopCenter(flagPos);
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
	flagPos.x = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	flagPos.y = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	flagPos.z = world->getMaxWorldHeight() * (float)bzfrand();
      }

      if (DropGeometry::dropFlag(flagPos, minZ, maxZ)) {
	break;
      }
    }

    if (i == 10000) {
      std::cerr << "Unable to position flags on this world." << std::endl;
    }
  }

  bool teamIsEmpty = true;
  if (teamIndex != ::NoTeam)
    teamIsEmpty = (teamInfos[teamIndex].team.size == 0);

  bz_FlagResetEventData_V1 eventData;
  memcpy(eventData.pos,flagPos,sizeof(float)*3);
  eventData.teamIsEmpty = teamIsEmpty;
  eventData.flagID = flag.getIndex();
  eventData.flagType = flag.flag.type->label().c_str();

  worldEventManager.callEvents(bz_eFlagResetEvent,&eventData);

  // reset a flag's info
  const fvec3 pos(eventData.pos[0], eventData.pos[1], eventData.pos[2]);
  flag.resetFlag(pos, teamIsEmpty);

  sendFlagUpdateMessage(flag);
}


void dropFlag(FlagInfo &flag)
{
  // see if someone had grabbed flag.  tell 'em to drop it.
  const int playerIndex = flag.player;
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  flag.player = -1;
  playerData->player.resetFlag();

  sendDropFlagMessage(playerIndex,flag);
}


void zapFlag(FlagInfo &flag)
{
  // called when a flag must just disappear -- doesn't fly
  // into air, just *poof* vanishes.

  dropFlag(flag);

  // if flag was flying then it flies no more
  flag.landing(BzTime::getSunExplodeTime());

  flag.flag.status = FlagNoExist;

  // reset flag status
  resetFlag(flag);
}


void anointNewRabbit( int killerId )
{
  GameKeeper::Player *killerData = GameKeeper::Player::getPlayerByIndex(killerId);
  GameKeeper::Player *oldRabbitData = GameKeeper::Player::getPlayerByIndex(rabbitIndex);
  int oldRabbit = rabbitIndex;
  rabbitIndex = NoPlayer;

  if (clOptions->rabbitSelection == KillerRabbitSelection) {
    // check to see if the rabbit was just killed by someone; if so, make them the rabbit if they're still around.
    if (killerId != oldRabbit && killerData && killerData->player.isPlaying() && killerData->player.canBeRabbit())
      rabbitIndex = killerId;
  }

  if (rabbitIndex == NoPlayer)
    rabbitIndex = GameKeeper::Player::anointRabbit(oldRabbit);

  // pass the rabbits to the API let it mod it if it wants
  bz_AnointRabbitEventData_V1	anoitData;
  anoitData.newRabbit = rabbitIndex;
  if (anoitData.newRabbit == NoPlayer)
    anoitData.newRabbit = -1;

  anoitData.swap = true;

  worldEventManager.callEvents(bz_eAnointRabbitEvent,&anoitData);

  if (anoitData.newRabbit != oldRabbit) {
    logDebugMessage(3,"rabbitIndex is set to %d\n", anoitData.newRabbit);
    if (oldRabbitData && anoitData.swap)
      oldRabbitData->player.wasARabbit();

    if (anoitData.newRabbit != -1) {
      GameKeeper::Player *rabbitData = GameKeeper::Player::getPlayerByIndex(anoitData.newRabbit);
      rabbitData->setTeam(RabbitTeam);

      sendRabbitUpdate(rabbitIndex,anoitData.swap ? 0 : 1);
    }

    bz_NewRabbitEventData_V1	newRabbitData;

    newRabbitData.newRabbit = anoitData.newRabbit;
    worldEventManager.callEvents(bz_eNewRabbitEvent,&newRabbitData);
  } else {
    logDebugMessage(3,"no other than old rabbit to choose from, rabbitIndex is %d\n", rabbitIndex);
  }
}


void pausePlayer(int playerIndex, bool paused)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData) {
    return;
  }

  logDebugMessage(2, "pausePlayer %s %s\n", playerData->player.getCallSign(),
                  paused ? "true" : "false");

  // always reset these parameters
  playerData->pauseRequested = false;
  playerData->pauseActiveTime = BzTime::getNullTime();

  // if the state is not being changed, leave
  if (paused == playerData->player.isPaused()) {
    return;
  }

  // change the PlayerInfo state
  playerData->player.setPaused(paused);

  if (clOptions->gameType == RabbitChase) {
    if (paused && (rabbitIndex == playerIndex)) {
      anointNewRabbit();
    } else if (!paused && (rabbitIndex == NoPlayer)) {
      anointNewRabbit();
    }
  }

  // drop flags
  FlagInfo* playerFlag = FlagInfo::get(playerData->player.getFlag());
  if (playerFlag && (playerFlag->flag.type->flagQuality != FlagBad)) {
    zapFlag(*playerFlag);
  }

  NetMessage netMsg;
  netMsg.packUInt8(playerIndex);
  netMsg.packUInt8(paused ? PauseCodeEnable : PauseCodeDisable);
  netMsg.broadcast(MsgPause);

  bz_PlayerPausedEventData_V1 pauseEventData;
  pauseEventData.playerID = playerIndex;
  pauseEventData.pause = paused;

  worldEventManager.callEvents(bz_ePlayerPausedEvent, &pauseEventData);
}


void zapFlagByPlayer(int playerIndex)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  int flagid = playerData->player.getFlag();
  if (flagid < 0)
    return;

  FlagInfo &flag = *FlagInfo::get(flagid);
  // do not simply zap team flag
  Flag &carriedflag = flag.flag;
  if (carriedflag.type->flagTeam != ::NoTeam)
    dropPlayerFlag(*playerData, playerData->currentPos);
  else
    zapFlag(flag);
}


void removePlayer(int playerIndex, const char *reason, bool notify)
{
  // player is signing off or sent a bad packet.  since the
  // bad packet can come before MsgEnter, we must be careful
  // not to undo operations that haven't been done.
  // first shutdown connection

  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData)
    return;

  bool  isBot, isBotHost, netHandlerIsSafe;
  isBot = playerData->botHost != -1;
  isBotHost = playerData->childBots.size() != 0;
  netHandlerIsSafe = !isBot;

  // if it's a bot host, kill all it's bots first so they are clean when we kill this guy's net handler
  if (isBotHost)
    // if it's a bot host, we need to  kill all the other bots too
    for ( size_t b = 0; b < playerData->childBots.size(); b++)
      removePlayer(playerData->childBots[b],reason, notify);

  // flush all pending messages for the player immediatley

  if (netHandlerIsSafe && playerData->netHandler) {
    playerData->netHandler->flushUDP();            // FIXME ?
    playerData->netHandler->bufferedSend(NULL, 0); // FIXME ?
  }

  playerData->isParting = true;

  // call any on part events
  bz_PlayerJoinPartEventData_V1 partEventData;
  partEventData.eventType = bz_ePlayerPartEvent;
  partEventData.playerID = playerIndex;
  partEventData.record = bz_getPlayerByIndex(playerIndex);
  if (reason)
    partEventData.reason = reason;

  if ((playerData->player.getTeam() != NoTeam) && strlen(playerData->player.getCallSign()))
    // don't give events if we don't have a real player slot
    worldEventManager.callEvents(bz_ePlayerPartEvent,&partEventData);

  if (netHandlerIsSafe && notify) {
    // send a super kill to be polite
    // send message to one player
    if (playerData->playerHandler) {
      playerData->playerHandler->removed();
    } else {
      // do not use message system, remove the player NOW
      char tempBuf[5];
      void *buf  = tempBuf;
      buf	= nboPackUInt16(buf, 1);
      buf	= nboPackUInt16(buf, MsgSuperKill);
      buf	= nboPackUInt8(buf, uint8_t(playerIndex));
      playerData->netHandler->pwrite(tempBuf, 5);
    }
  }

  // flush the connection
  if(netHandlerIsSafe) {
    if (playerData->netHandler)
      netConnectedPeers[playerData->netHandler->getFD()].deleteMe = true;
    playerData->netHandler->flushAllUDP();
  }

  // if there is an active poll, cancel any vote this player may have made
  if ((votingArbiter != NULL) && (votingArbiter->knowsPoll()))
    votingArbiter->retractVote(std::string(playerData->player.getCallSign()));

  // status message
  std::string timeStamp = BzTime::timestamp();
  logDebugMessage(1,"Player %s [%d] removed at %s: %s\n",
		  playerData->player.getCallSign(),
		  playerIndex, timeStamp.c_str(), reason);
  bool wasPlaying = playerData->player.isPlaying();

  zapFlagByPlayer(playerIndex);

  // player is outta here.  if player never joined a team then
  // don't count as a player.

  if (wasPlaying) {
    // make them wait from the time they left, but only if they are
    // not already waiting, and they are not currently an observer.
    if ((playerData->player.getTeam() != ObserverTeam) &&
	(rejoinList.waitTime(playerIndex) <= 0.0f) &&
	!playerData->accessInfo.hasPerm(PlayerAccessInfo::rejoin)) {
      rejoinList.add(playerIndex);
    }

    sendRemovePlayerMessage(playerIndex);

    // decrease team size
    int teamNum = int(playerData->player.getTeam());
    --teamInfos[teamNum].team.size;

    // if last active player on team then remove team's flag if no one
    // is carrying it
    if (Team::isColorTeam((TeamColor)teamNum)
	&& teamInfos[teamNum].team.size == 0 &&
	(clOptions->gameType == ClassicCTF)) {
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
    sendTeamUpdateMessageBroadcast(teamNum);
  }

  playerData->close();

  if (wasPlaying) {
    // 'fixing' the count after deleting player
    fixTeamCount();

    // tell the list server the new number of players
    if(listServerLink)
      listServerLink->queueMessage(ListServerLink::ADD);
  }

  if (clOptions->gameType == RabbitChase)
    if (playerIndex == rabbitIndex)
      anointNewRabbit();

  // recompute curMaxPlayers
  if (playerIndex + 1 == curMaxPlayers)
    while (true) {
      curMaxPlayers--;
      if (curMaxPlayers <= 0 ||
	  GameKeeper::Player::getPlayerByIndex(curMaxPlayers - 1))
	break;
    }

  if (wasPlaying) {
    // if everybody left then reset world
    if (GameKeeper::Player::count() == 0) {
      if (clOptions->worldFile == "")
	bases.clear();

      if (clOptions->oneGameOnly) {
	serverDone = true;
      } else {
	// republicize ourself.  this dereferences the URL chain
	// again so we'll notice any pointer change when any game
	// is over (i.e. all players have quit).
	publicize();
      }
    }
  }
}


void spawnPlayer ( int playerIndex )
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData || !playerData->isSpawnable())
    return;

  // player is coming alive.
  dropAssignedFlag(playerIndex);

  // get the spawn position
  SpawnPosition spawnPosition(playerIndex,
    (!clOptions->respawnOnBuildings) || (playerData->player.isBot()),
    clOptions->gameType == ClassicCTF);

  // see if there is anyone to handle the spawn event, and if they want to change it.
  bz_GetPlayerSpawnPosEventData_V1 spawnData;
  spawnData.playerID = playerIndex;
  spawnData.team     = convertTeam(playerData->player.getTeam());
  spawnData.pos[0]   = spawnPosition.getX();
  spawnData.pos[1]   = spawnPosition.getY();
  spawnData.pos[2]   = spawnPosition.getZ();
  spawnData.rot      = spawnPosition.getAzimuth();

  worldEventManager.callEvents(bz_eGetPlayerSpawnPosEvent,&spawnData);

  // update last position immediately
  const fvec3 pos(spawnData.pos[0], spawnData.pos[1], spawnData.pos[2]);
  playerData->setPlayerState(pos, spawnData.rot);

  sendMessageAlive(playerIndex, playerData->currentPos, playerData->currentRot);

  playerData->effectiveShotType = StandardShot;
  playerData->player.setAllow(AllowAll);
  sendMessageAllow(playerIndex, AllowAll);

  // call any events for a playerspawn
  bz_PlayerSpawnEventData_V1 spawnEvent;
  spawnEvent.playerID = playerIndex;
  spawnEvent.team = convertTeam(playerData->player.getTeam());

  playerStateToAPIState(spawnEvent.state, playerData->lastState);

  worldEventManager.callEvents(bz_ePlayerSpawnEvent, &spawnEvent);

  if (clOptions->gameType == RabbitChase) {
    playerData->player.wasNotARabbit();
    if (rabbitIndex == NoPlayer)
      anointNewRabbit();
  }

}


void playerAlive(int playerIndex)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData || !playerData->isSpawnable())
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

  bz_AllowSpawnData_V1	spawnAllowData;
  spawnAllowData.playerID = playerIndex;
  spawnAllowData.team = convertTeam(playerData->player.getTeam());

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::spawn)) {
    sendMessage(ServerPlayer, playerIndex, "You do not have permission to spawn on this server.");
    sendMessage(ServerPlayer, playerIndex, "This server may require identification before you can join.");
    sendMessage(ServerPlayer, playerIndex, "Please register on http://my.bzflag.org/bb/ and use that callsign/password,");
    spawnAllowData.allow = false;
  }

  if (playerData->player.isBot()) {
    if (BZDB.isTrue(BZDBNAMES.DISABLEBOTS) || (clOptions->botsPerIP == 0)) {
      sendMessage(ServerPlayer, playerIndex, "I'm sorry, we do not allow bots on this server.");
      removePlayer(playerIndex, "ComputerPlayer", true);
      return;
    }

    /* count how many bots are active from this IP address */
    in_addr playerIP = playerData->netHandler->getIPAddress();
    int botsFound = 0;
    for (int i = 0; i < curMaxPlayers; i++) {
      GameKeeper::Player *pbData = GameKeeper::Player::getPlayerByIndex(i);
      if (pbData == NULL)
	continue;

      if ((pbData->netHandler->getIPAddress().s_addr == playerIP.s_addr) && pbData->player.isBot()) {
	botsFound++;
	if (botsFound > clOptions->botsPerIP)
	  break;
      }
    }
    if (botsFound > clOptions->botsPerIP) {
      sendMessage(ServerPlayer, playerData->botHost,
		  "So sorry.  You are attempting to connect with too many bots.");
      sendMessage(ServerPlayer, playerData->botHost,
		  TextUtils::format("This server only allows a maximum of %d %s per IP address.",
				    clOptions->botsPerIP,
				    clOptions->botsPerIP==1?"bot":"bots").c_str());
      removePlayer(playerIndex, "Too many bots", true);
    }
  }


  // check for any spawn allow events
  worldEventManager.callEvents(bz_eAllowSpawn,&spawnAllowData);

  if (!spawnAllowData.allow) {
    // client won't send another enter so kick em =(
    removePlayer(playerIndex, "Not allowed to spawn", true);
    return;
  }

  spawnPlayer(playerIndex);
}


static void checkTeamScore(int playerIndex, int teamIndex)
{
  if (clOptions->maxTeamScore == 0 || !Team::isColorTeam(TeamColor(teamIndex)))
    return;

  if (teamInfos[teamIndex].team.won - teamInfos[teamIndex].team.lost >= clOptions->maxTeamScore) {
    sendScoreOverMessage(playerIndex, (TeamColor)teamIndex);

    gameOver = true;
    if (clOptions->oneGameOnly) {
      sendMessage(ServerPlayer, AllPlayers,
		  "automatically shutting down now that the game is over");
      serverDone = true;
    }
  }
}


bool checkForTeamKill ( GameKeeper::Player* killer,  GameKeeper::Player* victim, bool &teamkill  )
{
  if (clOptions->gameType == OpenFFA || !victim || !killer)
    return false;

  // killing rabbit or killing anything when I am a dead ex-rabbit is allowed
  teamkill = !Team::areFoes(victim->player.getTeam(), killer->player.getTeam(), clOptions->gameType) && !killer->player.isARabbitKill(victim->player);

  // update tk-score
  if ((victim->getIndex() != killer->getIndex()) && teamkill) {
    killer->score.tK();
    char message[MessageLen];
    if (clOptions->tkAnnounce) {
      snprintf(message, MessageLen, "Team kill: %s killed %s", killer->player.getCallSign(), victim->player.getCallSign());
      sendMessage(ServerPlayer, AdminPlayers, message);
    }
    if (killer->score.isTK()) {
      strcpy(message, "You have been automatically kicked for team killing" );
      sendMessage(ServerPlayer, killer->getIndex(), message);
      snprintf(message, MessageLen, "Player %s removed: team killing", killer->player.getCallSign());
      sendMessage(ServerPlayer, AdminPlayers, message);
      removePlayer(killer->getIndex(), "teamkilling", true);
      return true;
    }
  }
  return false;
}


void updateScoresForKill(GameKeeper::Player* victim,
                         GameKeeper::Player* killer, bool teamkill)
{
  // change the player score
  if (victim) {
    victim->score.killedBy();
    sendPlayerScoreUpdate(victim);
  }
  if (killer) {
    if (!victim || (victim->getIndex() != killer->getIndex())) {
      if (teamkill && !clOptions->teamKillerDies)
	killer->score.killedBy();
      else if (!teamkill)
	killer->score.kill();
    }
    sendPlayerScoreUpdate(killer);
  }
}


void updateHandicaps(GameKeeper::Player* victim, GameKeeper::Player* killer)
{
  if (!(clOptions->gameOptions & HandicapGameStyle))
    return;

  if (killer)
    sendSingleHandicapInfoUpdate(killer);

  if (victim)
    sendSingleHandicapInfoUpdate(victim);
}


void checkForScoreLimit(GameKeeper::Player* killer)
{
  // see if the player reached the score limit
  if (clOptions->maxPlayerScore != 0 && killer && killer->score.reached()) {
    sendScoreOverMessage(killer->getIndex(), NoTeam);

    gameOver = true;
    if (clOptions->oneGameOnly) {
      sendMessage(ServerPlayer, AllPlayers,
		  "automatically shutting down now that the game is over");
      serverDone = true;
    }
  }
}


const fvec3* closestBase(TeamColor color, const fvec3& position)
{
  float bestDistSq = Infinity;
  const fvec3* bestBase = NULL;

  if (bases.find(color) == bases.end()) {
    return NULL;
  }

  TeamBases &teamBases = bases[color];
  int count = teamBases.size();
  for (int i=0; i<count; i++) {
    const fvec3& basepos = teamBases.getBasePosition(i);
    const float distSq = (position.xy() - basepos.xy()).lengthSq();
    if (distSq < bestDistSq) {
      bestBase = &basepos;
      bestDistSq = distSq;
    }
  }

  return bestBase;
}


// FIXME - needs extra checks for killerIndex=ServerPlayer (world weapons)
// (was broken before); it turns out that killerIndex=-1 for world weapon?
// No need to check on victimIndex.
//   It is taken as the index of the udp table when called by incoming message
//   It is taken by killerIndex when autocalled, but only if != -1
// killer could be InvalidPlayer or a number within [0 curMaxPlayer)
void playerKilled(int victimIndex, int killerIndex, BlowedUpReason reason,
		  int16_t shotIndex, const FlagType* flagType, int phydrv,
		  bool respawnOnBase )
{
  GameKeeper::Player *killerData = NULL;
  GameKeeper::Player *victimData = GameKeeper::Player::getPlayerByIndex(victimIndex);

  if (!victimData || !victimData->player.isPlaying())
    return;

  if (killerIndex != InvalidPlayer && killerIndex != ServerPlayer)
    killerData = GameKeeper::Player::getPlayerByIndex(killerIndex);

  // aliases for convenience
  // Warning: killer should not be used when killerIndex == InvalidPlayer or ServerPlayer
  PlayerInfo *killer = realPlayer(killerIndex) ? &killerData->player : 0, *victim = &victimData->player;

  // victim was already dead. keep score.
  if (!victim->isAlive())
    return;

  victim->setRestartOnBase(respawnOnBase);
  victim->setDead();

  // call any events for a playerdeath
  bz_PlayerDieEventData_V1	dieEvent;
  dieEvent.playerID = victimIndex;
  dieEvent.team = convertTeam(victim->getTeam());
  dieEvent.killerID = killerIndex;
  dieEvent.shotID = shotIndex;

  if (killer)
    dieEvent.killerTeam = convertTeam(killer->getTeam());

  dieEvent.flagKilledWith = flagType->flagAbbv;

  playerStateToAPIState(dieEvent.state, victimData->lastState);

  worldEventManager.callEvents(bz_ePlayerDieEvent,&dieEvent);

  // If a plugin changed the killer, we need to update the data.
  if (dieEvent.killerID != killerIndex) {
    killerIndex = dieEvent.killerID;
    if (killerIndex != InvalidPlayer && killerIndex != ServerPlayer)
      killerData = GameKeeper::Player::getPlayerByIndex(killerIndex);
    killer = realPlayer(killerIndex) ? &killerData->player : 0;
  }

  sendPlayerKilledMessage(victimIndex,killerIndex,reason,shotIndex,flagType,phydrv);

  bool teamkill = false;
  if (checkForTeamKill(killerData,victimData,teamkill))
    killerData = NULL;

  // zap flag player was carrying.  clients should send a drop flag
  // message before sending a killed message, so this shouldn't happen.
  zapFlagByPlayer(victimIndex);

  // if weTKed, and we didn't suicide, and we are killing TKers, then kill that bastard
  if (teamkill &&  (victimIndex != killerIndex) && clOptions->teamKillerDies)
    playerKilled(killerIndex, killerIndex, reason, -1, Flags::Null, -1);

  if (!respawnOnBase) {
    updateScoresForKill(victimData,killerData,teamkill);
    updateHandicaps(victimData,killerData);
    checkForScoreLimit(killerData);
  }

  if (clOptions->gameType == RabbitChase) {
    if (victimIndex == rabbitIndex)
      anointNewRabbit(killerIndex);
  } else {
    // Change the team scores -- rogues don't have team scores.
    // Change team scores for individual players' kills only in
    // the free-for-all modes.
    // Team score is not used at all on RabbitChase.
    int winningTeam = (int)NoTeam;
    if ( clOptions->gameType == OpenFFA || clOptions->gameType == TeamFFA ) {
      int killerTeam = -1;
      if (killer && victim->getTeam() == killer->getTeam()) {
	if (!killer->isTeam(RogueTeam)) {
	  if (killerIndex == victimIndex)
	    teamInfos[int(victim->getTeam())].team.lost += 1;
	  else
	    teamInfos[int(victim->getTeam())].team.lost += 2;
	}
      } else {
	if (killer && !killer->isTeam(RogueTeam)) {
	  winningTeam = int(killer->getTeam());
	  teamInfos[winningTeam].team.won++;
	}
	if (!victim->isTeam(RogueTeam))
	  teamInfos[int(victim->getTeam())].team.lost++;
	if (killer)
	  killerTeam = killer->getTeam();
      }
      sendTeamUpdateMessageBroadcast(int(victim->getTeam()), killerTeam);
    }

    dumpScore();

    if (winningTeam != (int)NoTeam)
      checkTeamScore(killerIndex, winningTeam);

    victimData->player.setPaused(false);
    victimData->pauseRequested = false;
    victimData->pauseActiveTime = BzTime::getNullTime();
  }
}


void searchFlag(GameKeeper::Player &playerData)
{
  if (!playerData.player.isAlive() || playerData.player.isPaused()) {
    return;
  }

  bool identify = false;
  float radius = BZDBCache::tankRadius + BZDBCache::flagRadius;

  int flagId = playerData.player.getFlag();
  if (flagId >= 0) {
    FlagInfo &playerFlag = *FlagInfo::get(flagId);
    if (playerFlag.flag.type != Flags::Identify || playerData.player.isBot()) {
      return;
    }
    identify = true;
    radius = BZDB.eval(BZDBNAMES.IDENTIFYRANGE);
  }

  const PlayerId playerIndex = playerData.getIndex();

  const fvec3& tpos = playerData.lastState.pos;

  float radius2 = radius * radius;

  int closestFlag = -1;
  for (int i = 0; i < numFlags; i++) {
    FlagInfo &flag = *FlagInfo::get(i);
    if (!flag.exist() || (flag.flag.status != FlagOnGround)) {
      continue;
    }
    const fvec3& fpos = flag.flag.position;

    if (!identify) {
      // z separation check
      if (fabsf(tpos.z - fpos.z) >= 0.1f) {
        continue;
      }
    }

    // spherical separation check
    const float distSq = (tpos - fpos).lengthSq();
    if (distSq >= radius2) {
      continue;
    }

    // check if there is an obstacle between the tank and the flag
    if (!identify) {
      fvec3 flagCenter = fpos;
      fvec3 tankCenter = tpos;
      flagCenter.z += 0.5f * BZDBCache::flagPoleSize;
      tankCenter.z += BZDBCache::muzzleHeight;
      const Ray ray(flagCenter, tankCenter - flagCenter);
      const ObsList* oList = COLLISIONMGR.rayTest(&ray, 1.0f);
      const int count = oList->count;
      int o;
      for (o = 0; o < count; o++) {
        const Obstacle* obs = oList->list[o];
        // FIXME -- check for passthru and link characteristics ?
        const float t = obs->intersect(ray);
        if ((t >= 0.0f) && (t < 1.0f)) {
          break;
        }
      }
      if (o != count) {
        continue; // there is a blocking obstacle
      }
    }

    radius2 = distSq;
    closestFlag = i;

    if (!identify) {
      break;
    }
  }

  if (closestFlag < 0) {
    if (identify) {
      playerData.setLastIdFlag(-1);
    }
    return;
  }

  FlagInfo &flag = *FlagInfo::get(closestFlag);
  if (identify) {
    if (closestFlag != playerData.getLastIdFlag()) {
      sendClosestFlagMessage(playerIndex, flag.flag.type, flag.flag.position);
      playerData.setLastIdFlag(closestFlag);
    }
  }
  else {
    if (!BZDB.isTrue(BZDBNAMES.GRABOWNFLAG)) {
      if (flag.flag.type->flagTeam != playerData.player.getTeam()) {
	sendGrabFlagMessage(playerIndex, flag);
      }
    }
    else {
      sendGrabFlagMessage(playerIndex, flag);
    }
  }
}


void dropFlag(FlagInfo& flagInfo, const fvec3& dropPos)
{
  assert(world != NULL);

  // maximum X or Y coordinate is 1/2 of worldsize
  const float size = BZDBCache::worldSize * 0.5f;
  const fvec3 pos(
    ((dropPos.x < -size) || (dropPos.x > size)) ? 0.0f : dropPos.x,
    ((dropPos.y < -size) || (dropPos.y > size)) ? 0.0f : dropPos.y,
    (dropPos.z > maxWorldHeight) ? maxWorldHeight : dropPos.z
  );

  // player wants to drop flag.  we trust that the client won't tell
  // us to drop a sticky flag until the requirements are satisfied.
  const int flagIndex = flagInfo.getIndex();
  if (flagInfo.flag.status != FlagOnTank)
    return;

  int flagTeam = flagInfo.flag.type->flagTeam;
  bool isTeamFlag = (flagTeam != ::NoTeam);

  // limited flags that have been fired should be disposed of
  bool limited = clOptions->flagLimit[flagInfo.flag.type] != -1;
  if (limited && flagInfo.numShots > 0) flagInfo.grabs = 0;


  const float waterLevel = world->getWaterLevel();
  float minZ = 0.0f;
  if (waterLevel > minZ) {
    minZ = waterLevel;
  }

  const float maxZ = MAXFLOAT;

  fvec3 landing = pos;
  bool safelyDropped = DropGeometry::dropTeamFlag(landing, minZ, maxZ, flagTeam);

  bool vanish;

  if (isTeamFlag) {
    vanish = false;
  } else if (--flagInfo.grabs <= 0) {
    vanish = true;
    flagInfo.grabs = 0;
  } else if (!clOptions->flagsOnBuildings && (landing.z > 0.0f)) {
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
    if (!world->getFlagDropPoint(&flagInfo, pos, landing)) {
      // try the center
      landing = fvec3(0.0f, 0.0f, 0.0f);
      safelyDropped = DropGeometry::dropTeamFlag(landing, minZ, maxZ, flagTeam);
      if (!safelyDropped) {
	// ok, we give up, send it home
	TeamBases& teamBases = bases[flagTeam];
	const TeamBase& base = teamBases.getRandomBase();
	base.getTopCenter(landing);
      }
    }
  }

  if (isTeamFlag) {
    // if it is a team flag, check if there are any players left in
    // that team - if not, start the flag timeout
    if (teamInfos[flagInfo.flag.type->flagTeam].team.size == 0) {
      teamInfos[flagIndex + 1].flagTimeout = BzTime::getCurrent();
      teamInfos[flagIndex + 1].flagTimeout += (float)clOptions->teamFlagTimeout;
    }
  }

  flagInfo.dropFlag(pos, landing, vanish);

  // player no longer has flag -- send MsgDropFlag
  dropFlag(flagInfo);

  // notify of new flag state
  sendFlagUpdateMessage(flagInfo);
}


void dropPlayerFlag(GameKeeper::Player &playerData, const fvec3& dropPos)
{
  const int flagIndex = playerData.player.getFlag();
  if (flagIndex < 0)
    return;
  dropFlag(*FlagInfo::get(flagIndex), dropPos);
  playerData.effectiveShotType = StandardShot;

  bz_FlagDroppedEventData_V1 data;
  data.playerID = playerData.getIndex();
  data.flagID = flagIndex;
  data.flagType = FlagInfo::get(flagIndex)->flag.type->flagAbbv.c_str();
  memcpy(data.pos, dropPos, sizeof(float)*3);

  worldEventManager.callEvents(bz_eFlagDroppedEvent,&data);
}


void captureFlag(int playerIndex, TeamColor teamCaptured)
{
  GameKeeper::Player *playerData  = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!playerData || invalidPlayerAction(playerData->player, playerIndex, "capture a flag"))
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

  if (checkFlagCheats(playerData,teamIndex))
    return;

  bz_AllowCTFCaptureEventData_V1  allowData;

  fvec3 posTmp;

  allowData.teamCapped = convertTeam((TeamColor)teamIndex);
  allowData.teamCapping = convertTeam(teamCaptured);
  allowData.playerCapping = playerIndex;
  playerData->getPlayerCurrentPosRot(posTmp, allowData.rot);
  memcpy(allowData.pos, posTmp, sizeof(float[3]));
  allowData.allow = true;
  allowData.killTeam = true;

  worldEventManager.callEvents(bz_eAllowCTFCaptureEvent,&allowData);

  if (!allowData.allow)
    return;

  // player no longer has flag and put flag back at it's base
  playerData->player.resetFlag();
  resetFlag(flag);

  sendFlagCaptureMessage(playerIndex,flagIndex,teamCaptured);

  // find any events for capturing the flags on the capped team or events for ANY team
  bz_CTFCaptureEventData_V1	eventData;
  eventData.teamCapped = convertTeam((TeamColor)teamIndex);
  eventData.teamCapping = convertTeam(teamCaptured);
  eventData.playerCapping = playerIndex;
  playerData->getPlayerCurrentPosRot(posTmp, eventData.rot);
  memcpy(eventData.pos, posTmp, sizeof(float[3]));

  worldEventManager.callEvents(bz_eCaptureEvent,&eventData);

  if (allowData.killTeam) {
    // everyone on losing team is dead, KILL THEM.
    for (int i = 0; i < curMaxPlayers; i++) {
      GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
      if ((p == NULL) || (teamIndex != (int)p->player.getTeam()))
	continue;
      playerKilled(i, playerIndex, GotCaptured, -1, Flags::Null, -1, true);
    }
  }

  // update score (rogues can't capture flags)
  int winningTeam = (int)NoTeam;
  if (teamIndex != int(playerData->player.getTeam())) {
    // player captured enemy flag
    winningTeam = int(playerData->player.getTeam());
    teamInfos[winningTeam].team.won++;
  }
  teamInfos[teamIndex].team.lost++;
  sendTeamUpdateMessageBroadcast(winningTeam, teamIndex);

  dumpScore();

  if (winningTeam != (int)NoTeam)
    checkTeamScore(playerIndex, winningTeam);
}


bool updatePlayerState(GameKeeper::Player *playerData,
                       PlayerState &state, BzTime const& timeStamp, bool shortState)
{
  // observer updates are not relayed, or checked
  if (playerData->player.isObserver()) {
    // skip all of the checks
    playerData->setPlayerState(state, timeStamp);
    return true;
  }

  // bz_ePlayerUpdateEvent
  bz_PlayerUpdateEventData_V1 eventData;
  playerStateToAPIState(eventData.state,state);
  eventData.stateTime = timeStamp.getSeconds();
  eventData.playerID = playerData->getIndex();
  worldEventManager.callEvents(bz_ePlayerUpdateEvent, &eventData);

  // ignore out of order packet
  if (state.order <= playerData->lastState.order) {
    logDebugMessage(5, "[%d] ignoring out of order update (%ld <= %ld)\n",
      playerData->player.getPlayerIndex(),
      state.order, playerData->lastState.order);
    return true;
  }

  if (!validatePlayerState(playerData, state)) {
    return false;
  }

  playerData->setPlayerState(state, timeStamp);

  // bz_ePlayerUpdateDoneEvent
  bz_PlayerUpdateDoneEventData_V1 doneEventData;
  doneEventData.stateTime = timeStamp.getSeconds();
  doneEventData.playerID = playerData->getIndex();
  worldEventManager.callEvents(bz_ePlayerUpdateDoneEvent, &doneEventData);

  // Player might already be dead and did not know it yet (e.g. teamkill)
  // do not propogate
  if (!playerData->player.isAlive() &&
      (state.status & short(PlayerState::Alive))) {
    return true;
  }

  searchFlag(*playerData);

  sendPlayerStateMessage(playerData, shortState);

  return true;
}


/** observers and paused players should not be sending updates.. punish the
 * ones that are paused since they are probably cheating.
 */
bool invalidPlayerAction(PlayerInfo &p, int t, const char *action) {
  if (p.isObserver() || p.isPaused()) {
    if (p.isPaused()) {
      char buffer[MessageLen];
      logDebugMessage(1,"Player \"%s\" tried to %s while paused\n", p.getCallSign(), action);
      snprintf(buffer, MessageLen, "Autokick: Looks like you tried to %s while paused.", action);
      sendMessage(ServerPlayer, t, buffer);
      snprintf(buffer, MessageLen, "Invalid attempt to %s while paused", action);
      removePlayer(t, buffer, true);
    } else {
      logDebugMessage(1,"Player %s tried to %s as an observer\n", p.getCallSign(), action);
    }
    return true;
  }
  return false;
}


void lagKick(int playerIndex)
{
  char message[MessageLen];
  sprintf(message,
	  "You have been kicked due to excessive lag (you have been warned %d times).",
	  clOptions->maxlagwarn);
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (playerData != NULL) {
    sendMessage(ServerPlayer, playerIndex, message);
    snprintf(message, MessageLen,"Lagkick: %s", playerData->player.getCallSign());
    sendMessage(ServerPlayer, AdminPlayers, message);
    removePlayer(playerIndex, "lag", true);
  }
}


void jitterKick(int playerIndex)
{
  char message[MessageLen];
  sprintf(message,
	  "You have been kicked due to excessive jitter (you have been warned %d times).",
	  clOptions->maxjitterwarn);
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (playerData != NULL) {
    sendMessage(ServerPlayer, playerIndex, message);
    snprintf(message, MessageLen,"Jitterkick: %s",
	     playerData->player.getCallSign());
    sendMessage(ServerPlayer, AdminPlayers, message);
    removePlayer(playerIndex, "jitter", true);
  }
}


void packetLossKick(int playerIndex)
{
  char message[MessageLen];
  sprintf(message,
	  "You have been kicked due to excessive packetloss (you have been warned %d times).",
	  clOptions->maxpacketlosswarn);
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (playerData != NULL) {
    sendMessage(ServerPlayer, playerIndex, message);
    snprintf(message, MessageLen,"Packetlosskick: %s",
	     playerData->player.getCallSign());
    sendMessage(ServerPlayer, AdminPlayers, message);
    removePlayer(playerIndex, "packetloss", true);
  }
}


static void adjustTolerances()
{
  // check for handicap adjustment
  if ((clOptions->gameOptions & HandicapGameStyle) != 0) {
    const float speedAdj = BZDB.eval(BZDBNAMES.HANDICAPVELAD);
    speedTolerance *= speedAdj * speedAdj;
  }

  // check for physics driver disabling
  cheatProtectionOptions.doHeightChecks = true;
  cheatProtectionOptions.doSpeedChecks = true;

  int i = 0;
  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(i);
  while (phydrv) {
    if (!phydrv->getIsDeath()) {
      const fvec3& lv = phydrv->getLinearVel();
      const float  av = phydrv->getAngularVel();
      const float  rv = phydrv->getRadialVel();
      if (!phydrv->getIsSlide() &&
	  ((lv != fvec3(0.0f, 0.0f, 0.0f)) || (av != 0.0f) || (rv != 0.0f))) {
	cheatProtectionOptions.doSpeedChecks = false;
      }
      if (lv.z > 0.0f) {
	cheatProtectionOptions.doHeightChecks = false;
      }
    }
    i++;
    phydrv = PHYDRVMGR.getDriver(i);
  }

  if (!cheatProtectionOptions.doSpeedChecks) {
    speedTolerance = MAXFLOAT;
    logDebugMessage(1,"Warning: disabling speed checking due to physics drivers\n");
  }
  if (!cheatProtectionOptions.doHeightChecks)
    logDebugMessage(1,"Warning: disabling height checking partly due to physics drivers\n");

  return;
}


static void terminateServer(int /*sig*/)
{
  bzSignal(SIGINT, SIG_PF(terminateServer));
  bzSignal(SIGTERM, SIG_PF(terminateServer));
  sendMessage(ServerPlayer, AllPlayers, "shutting down now");
  serverDone = true;
}


static std::string cmdSet(const std::string&, const CommandManager::ArgList& args, bool *worked)
{
  if (worked)
    *worked = true;
  switch (args.size()) {
  case 2:
    if (BZDB.isSet(args[0])) {
      StateDatabase::Permission permission = BZDB.getPermission(args[0]);
      if ((permission == StateDatabase::ReadWrite) || (permission == StateDatabase::Locked)) {
	BZDB.set(args[0], args[1], StateDatabase::Server);
	lastWorldParmChange = BzTime::getCurrent();
	return args[0] + " set";
      }
      if (worked)*worked = false;
      return "variable " + args[0] + " is not writeable";
    } else {
      if (worked)*worked = false;
      return "variable " + args[0] + " does not exist";
    }
  case 1:
    if (BZDB.isSet(args[0])) {
      return args[0] + " is " + BZDB.get(args[0]);
    } else {
      if (worked)*worked = false;
      return "variable " + args[0] + " does not exist";
    }
  default:
    if (worked)*worked = false;
    return "usage: set <name> [<value>]";
  }
}


static void resetAllCallback(const std::string &name, void*)
{
  StateDatabase::Permission permission = BZDB.getPermission(name);
  if ((permission == StateDatabase::ReadWrite) || (permission == StateDatabase::Locked))
    BZDB.set(name, BZDB.getDefault(name), StateDatabase::Server);
}


static std::string cmdReset(const std::string&, const CommandManager::ArgList& args, bool*)
{
  if (args.size() == 1) {
    if (args[0] == "*") {
      BZDB.iterate(resetAllCallback,NULL);
      lastWorldParmChange = BzTime::getCurrent();
      return "all variables reset";
    } else if (BZDB.isSet(args[0])) {
      StateDatabase::Permission permission = BZDB.getPermission(args[0]);
      if ((permission == StateDatabase::ReadWrite) || (permission == StateDatabase::Locked)) {
	BZDB.set(args[0], BZDB.getDefault(args[0]), StateDatabase::Server);
	lastWorldParmChange = BzTime::getCurrent();
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
    if ((!playerData.accessInfo.hasPerm(PlayerAccessInfo::antikick)) &&
	(playerData.player.isTooMuchIdling(clOptions->idlekickthresh))) {
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
  }
  else if (!playerData.netHandler &&
           (playerData._LSAState != GameKeeper::Player::done)) {
    addPlayer(p, &playerData);
    playerData._LSAState = GameKeeper::Player::done;
  }
  else if (playerData.netHandler) {
    if (playerData.netHandler->reverseDNSDone()) {
      if ((playerData._LSAState == GameKeeper::Player::verified) ||
          (playerData._LSAState == GameKeeper::Player::timedOut) ||
          (playerData._LSAState == GameKeeper::Player::failed)	 ||
          (playerData._LSAState == GameKeeper::Player::notRequired)) {
        addPlayer(p, &playerData);
        playerData._LSAState = GameKeeper::Player::done;
      }
    }
  }

  // Check host bans
  if (!playerData.netHandler) {
    playerData.setNeedThisHostbanChecked(false);
  } else {
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
	dropPlayerFlag(playerData, playerData.currentPos);
	// Should recheck if player is still available
	if (!GameKeeper::Player::getPlayerByIndex(p))
	  return;
      }
    }
  }

  if (playerData.netHandler) {
    // send lag pings
    bool warn;
    bool kick;
    int nextPingSeqno = playerData.lagInfo.getNextPingSeqno(warn, kick);
    if (nextPingSeqno > 0) {
      NetMessage netMsg;
      netMsg.packUInt16(nextPingSeqno);
      netMsg.send(playerData.netHandler, MsgLagPing);

      if (warn) {
	sendMessage(ServerPlayer, p, "*** Server Warning: your lag is too high (failed to return ping) ***");
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
}


void rescanForBans ( const char* callsign, int playerID )
{
  // Validate all of the current players

  std::string banner = "SERVER";
  if (callsign && strlen(callsign))
    banner = callsign;

  std::string reason;
  char kickmessage[MessageLen];

  // Check host bans
  GameKeeper::Player::setAllNeedHostbanChecked(true);

  // Check IP bans
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *otherPlayer = GameKeeper::Player::getPlayerByIndex(i);
    if (otherPlayer && !clOptions->acl.validate(otherPlayer->netHandler->getIPAddress())) {
      // make sure this player isn't protected
      GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
      if ((p != NULL) && (p->accessInfo.hasPerm(PlayerAccessInfo::antiban))) {
	if (playerID != -1) {
	  snprintf(kickmessage, MessageLen, "%s is protected from being banned (skipped).", p->player.getCallSign());
	  sendMessage(ServerPlayer, playerID, kickmessage);
	}
	continue;
      }

      snprintf(kickmessage, MessageLen, "You were banned from this server by %s", banner.c_str());
      sendMessage(ServerPlayer, i, kickmessage);
      if (reason.length() > 0) {
	snprintf(kickmessage, MessageLen, "Reason given: %s", reason.c_str());
	sendMessage(ServerPlayer, i, kickmessage);
      }
      removePlayer(i, "/ban");
    }
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
  info.explicitAllows[PlayerAccessInfo::listPlugins] = true;
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
  // do not add pollSet permission here because it is easy to abuse and
  // most server owners don't want that perm given to verified
  info.explicitAllows[PlayerAccessInfo::pollBan] = true;
  info.explicitAllows[PlayerAccessInfo::pollKick] = true;
  info.explicitAllows[PlayerAccessInfo::pollFlagReset] = true;
  info.groupState[PlayerAccessInfo::isGroup] = true;
  info.groupState[PlayerAccessInfo::isDefault] = true;
  groupAccess["VERIFIED"] = info;

  //  LOCAL.ADMIN
  info.explicitAllows.reset();
  info.groupState.reset();
  for (int i = 0; i < PlayerAccessInfo::lastPerm; i++) {
    info.explicitAllows[i] = true;
  }
  info.groupState[PlayerAccessInfo::isGroup] = true;
  info.groupState[PlayerAccessInfo::isDefault] = true;
  info.explicitAllows[PlayerAccessInfo::hideAdmin] = false;
  groupAccess["LOCAL.ADMIN"] = info;

  // load databases
  if (groupsFile.size())
    PlayerAccessInfo::readGroupsFile(groupsFile);
}


static void updatePlayerPositions()
{
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(i);
    if (player)
      player->doPlayerDR();
  }
}


static void checkForWorldDeaths()
{
  float waterLevel = world->getWaterLevel();

  if (waterLevel > 0.0f) {
    for (int i = 0; i < curMaxPlayers; i++) {
      // kill anyone under the water level
      GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(i);
      if (player && player->player.isAlive() &&
          player->lastState.pos.z <= waterLevel) {
	playerKilled(player->getIndex(), ServerPlayer,
	             WaterDeath, -1, Flags::Null, -1);
      }
    }
  }
}


static bool initNet()
{
  // initialize
#if defined(_WIN32)
  static const int major = 2, minor = 2;
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(major, minor), &wsaData)) {
    logDebugMessage(2,"Failed to initialize Winsock.  Terminating.\n");
    return false;
  }
  if (LOBYTE(wsaData.wVersion) != major || HIBYTE(wsaData.wVersion) != minor) {
    logDebugMessage(2,"Version mismatch in Winsock; got %d.%d, expected %d.%d.	Terminating.\n",
		    (int)LOBYTE(wsaData.wVersion),
		    (int)HIBYTE(wsaData.wVersion),
		    major,
		    minor);
    WSACleanup();
    return false;
  }
  return true;
#else
  // don't die on broken pipe
  bzSignal(SIGPIPE, SIG_IGN);
  return true;
#endif /* defined(_WIN32) */
}


static void initStartupParameters(int argc, char **argv)
{
  Flags::init();

  clOptions = new CmdLineOptions(argv[0]);

  // set default DB entries
  for (unsigned int gi = 0; gi < numGlobalDBItems; ++gi) {
    assert(globalDBItems[gi].name != NULL);
    if (globalDBItems[gi].value != NULL) {
      BZDB.set(globalDBItems[gi].name, globalDBItems[gi].value);
      BZDB.setDefault(globalDBItems[gi].name, globalDBItems[gi].value);
    }

    BZDB.setPersistent(globalDBItems[gi].name, globalDBItems[gi].persistent);
    BZDB.setPermission(globalDBItems[gi].name, globalDBItems[gi].permission);
  }

  // add the global callback for worldEventManager
  BZDB.addGlobalCallback(bzdbGlobalCallback, NULL);

  CMDMGR.add("set", cmdSet, "set <name> [<value>]");
  CMDMGR.add("reset", cmdReset, "reset <name>");

  BZDBCache::init();

  // any set in parse this is a default value
  BZDB.setSaveDefault(true);

  // parse arguments  (finalized later)
  clOptions->parseArgOptions(argc, argv);
  setDebugTimestamp (clOptions->timestampLog, clOptions->timestampMicros, clOptions->timestampUTC);


  if (clOptions->bzdbVars.length() > 0) {
    logDebugMessage(1,"Loading variables from %s\n", clOptions->bzdbVars.c_str());
    bool success = CFGMGR.read(clOptions->bzdbVars);
    if (success)
      logDebugMessage(1,"Successfully loaded variable(s)\n");
    else
      logDebugMessage(1,"WARNING: unable to load the variable file\n");
  }

  if (clOptions->publicizeServer && clOptions->publicizedKey.empty()) {
    logDebugMessage(0,
      "\n"
      "WARNING:\n"
      "  Publicly listed bzfs servers must register using the '-publickey <key>'\n"
      "  option. A web page describing list-server policies and procedures can\n"
      "  be found at the following location:\n"
      "\n"
      "    http://my.bzflag.org/w/ServerAuthentication\n"
      "\n");
  }

  // no more defaults
  BZDB.setSaveDefault(false);
}


static void setupPlugins()
{
#ifdef BZ_PLUGINS
  // see if we are going to load any plugins
  initPlugins();
  // check for python by default
  //  loadPlugin(std::string("python"),std::string(""));
  for (unsigned int plugin = 0; plugin < clOptions->pluginList.size(); plugin++) {
    if (!loadPlugin(clOptions->pluginList[plugin].plugin, clOptions->pluginList[plugin].command)) {
      std::string text = "WARNING: unable to load the plugin; ";
      text += clOptions->pluginList[plugin].plugin + "\n";
      logDebugMessage(0,text.c_str());
    }
  }
#endif
}


static bool prepareWorld()
{
  // start listening and prepare world database
  if (!defineWorld()) {
#if defined(_WIN32)
    WSACleanup();
#endif /* defined(_WIN32) */
    std::cerr << "ERROR: A world was not specified" << std::endl;
    return false;
  } else if (clOptions->cacheOut != "") {
    if (!saveWorldCache())
      std::cerr << "ERROR: could not save world cache file: " << clOptions->cacheOut << std::endl;
    serverDone = true;
  }
  return true;
}


static void enableReplayServer()
{
  // enable replay server mode
  if (clOptions->replayServer) {

    Replay::init();

    // we don't send flags to a client that isn't expecting them
    numFlags = 0;
    clOptions->numExtraFlags = 0;

    // maxPlayers is sent in the world data to the client.
    // the client then uses this to setup it's players
    // data structure, so we need to send it the largest
    // PlayerId it might see.
    maxPlayers = MaxPlayers + ReplayObservers;

    if (clOptions->maxTeam[ObserverTeam] == 0) {
      std::cerr << "replay needs at least 1 observer, set to 1" << std::endl;
      clOptions->maxTeam[ObserverTeam] = 1;
    } else if (clOptions->maxTeam[ObserverTeam] > ReplayObservers) {
      std::cerr << "observer count limited to " << ReplayObservers << " for replay" << std::endl;
      clOptions->maxTeam[ObserverTeam] = ReplayObservers;
    }

    if (clOptions->botsPerIP != 0) {
      std::cerr << "bots are not allowed in replay mode" << std::endl;
      clOptions->botsPerIP = 0;
    }
  }
}


static void setupBadWordFilter()
{
  /* load the bad word filter if it was set */
  if (clOptions->filterFilename.length() != 0) {
    if (clOptions->filterChat || clOptions->filterCallsigns) {
      if (debugLevel >= 1) {
	unsigned int count;
	logDebugMessage(1,"Loading %s\n", clOptions->filterFilename.c_str());
	count = clOptions->filter.loadFromFile(clOptions->filterFilename, true);
	logDebugMessage(1,"Loaded %u words\n", count);
      } else {
	clOptions->filter.loadFromFile(clOptions->filterFilename, false);
      }
    } else {
      logDebugMessage(1,"Bad word filter specified without -filterChat or -filterCallsigns\n");
    }
  }
}


static void setupVoteArbiter()
{
  /* initialize the poll arbiter for voting if necessary */
  if (clOptions->voteTime > 0) {
    votingArbiter =
      new VotingArbiter(clOptions->voteTime, clOptions->vetoTime, clOptions->votesRequired, clOptions->votePercentage, clOptions->voteRepeatTime);
    logDebugMessage(1,"There is a voting arbiter with the following settings:\n");
    logDebugMessage(1,"\tvote time is %d seconds\n", clOptions->voteTime);
    logDebugMessage(1,"\tveto time is %d seconds\n", clOptions->vetoTime);
    logDebugMessage(1,"\tvotes required are %d\n", clOptions->votesRequired);
    logDebugMessage(1,"\tvote percentage necessary is %f\n", clOptions->votePercentage);
    logDebugMessage(1,"\tvote repeat time is %d seconds\n", clOptions->voteRepeatTime);
    logDebugMessage(1,"\tavailable voters is initially set to %d\n", maxPlayers);

    // override the default voter count to the max number of players possible
    votingArbiter->setAvailableVoters(maxPlayers);
  }
}


static void setupPublicInterfaces()
{
  if (clOptions->pingInterface != "")
    serverAddress = Address::getHostAddress(clOptions->pingInterface);

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
    logDebugMessage(1,"Running a public server with the following settings:\n");
    logDebugMessage(1,"\tpublic address is %s\n", clOptions->publicizedAddress.c_str());
  } else {
    logDebugMessage(1,"Running a private server with the following settings:\n");
  }
}


static void setupMasterBanList()
{
  // get the master ban list
  if (clOptions->publicizeServer && !clOptions->suppressMasterBanList) {
    MasterBanList banList;
    std::vector<std::string>::const_iterator it;
    for (it = clOptions->masterBanListURL.begin(); it != clOptions->masterBanListURL.end(); it++) {
      clOptions->acl.merge(banList.get(it->c_str()));
      logDebugMessage(1,"Loaded master ban list from %s\n", it->c_str());
    }
  }
}


static void setupScoringOptions()
{
  Score::setTeamKillRatio(clOptions->teamKillerKickRatio);
  Score::setWinLimit(clOptions->maxPlayerScore);

  if (clOptions->rabbitSelection == RandomRabbitSelection)
    Score::setRandomRanking();
}


static void setupPingReply()
{
  // prep ping reply
  pingReply.serverId.serverHost = serverAddress;
  pingReply.serverId.port       = htons(clOptions->wksPort);
  pingReply.serverId.number     = 0;
  pingReply.gameType            = clOptions->gameType;
  pingReply.gameOptions         = clOptions->gameOptions;
  pingReply.maxPlayers          = (uint8_t)maxRealPlayers;
  pingReply.maxShots            = clOptions->maxShots;
  pingReply.rogueMax            = (uint8_t)clOptions->maxTeam[0];
  pingReply.redMax              = (uint8_t)clOptions->maxTeam[1];
  pingReply.greenMax            = (uint8_t)clOptions->maxTeam[2];
  pingReply.blueMax             = (uint8_t)clOptions->maxTeam[3];
  pingReply.purpleMax           = (uint8_t)clOptions->maxTeam[4];
  pingReply.observerMax         = (uint8_t)clOptions->maxTeam[5];
  pingReply.shakeWins           = clOptions->shakeWins;
  pingReply.shakeTimeout        = clOptions->shakeTimeout;
  pingReply.maxTime             = (uint16_t)clOptions->timeLimit;
  pingReply.maxPlayerScore      = clOptions->maxPlayerScore;
  pingReply.maxTeamScore        = clOptions->maxTeamScore;
}


static void setupPermissions()
{
  // load up the access permissions & stuff
  initGroups();
  if (userDatabaseFile.size())
    PlayerAccessInfo::readPermsFile(userDatabaseFile);

  // warn noisily if nobody has SPAWN permission
  bool serverAllowsSpawn = false;
  // check groups (usually the EVERYONE or VERIFIED group has SPAWN)
  for (PlayerAccessMap::iterator group = groupAccess.begin();
       group != groupAccess.end();
       ++group) {
    if (!group->second.explicitDenys.test(PlayerAccessInfo::spawn) &&
	group->second.explicitAllows.test(PlayerAccessInfo::spawn)) {
      serverAllowsSpawn = true;
      break;
    }
  }
  // hmm, no groups have it.	check all registered users...
  if (!serverAllowsSpawn) {
    for (PlayerAccessMap::iterator user = userDatabase.begin();
	 user != userDatabase.end();
	 ++user) {
      if (!user->second.explicitDenys.test(PlayerAccessInfo::spawn) &&
	  user->second.explicitAllows.test(PlayerAccessInfo::spawn)) {
	serverAllowsSpawn = true;
	break;
      }
    }
  }

  // eek, nobody can spawn!!
  if (!serverAllowsSpawn)
    std::cout << "WARNING: No players have the SPAWN permission!" << std::endl;

  // if requested, make it so AllowMovement and AllowShooting eventually
  // get reenabled automatically
  if (BZDB.eval(BZDBNAMES.AUTOALLOWTIME) > 0)
    bz_registerEvent(bz_eTickEvent, &autoAllowTimerTickHandler);
}


static bool initServer(int argc, char **argv)
{
  initNetMessage();

  registerLoggingProc(apiLoggingProc, NULL);

  /* line buffered output to console */
  setvbuf(stdout, (char *)NULL, _IOLBF, BUFSIZE);
  setvbuf(stderr, (char *)NULL, _IOLBF, BUFSIZE);

  Record::init();

  // check time bomb
  if (timeBombBoom()) {
    std::cerr << "This release expired on " << timeBombString() << "." << std::endl;
    std::cerr << "Please upgrade to the latest release." << std::endl;
    exit(0);
  }

  // print expiration date
  if (timeBombString()) {
    std::cerr << "This release will expire on " << timeBombString() << "." << std::endl;
    std::cerr << "Version " << getAppVersion() << std::endl;
  }

  if (!initNet())
    return false;

  bzfsrand((unsigned int)time(0));

  initStartupParameters(argc, argv);

  setupPermissions();

  setupPlugins();

  // start up a stats link class, it'll take care of itself.
  statsLink.init();

  if (!prepareWorld())
    return false;

  // make flags, check sanity, etc...
  // (do this after the world has been loaded)
  clOptions->finalizeParsing(world->getEntryZones());
  FlagInfo::setNoFlagInAir();
  for (int i = 0; i < numFlags; i++)
    resetFlag(*FlagInfo::get(i));

  // loading extra flag number
  FlagInfo::setExtra(clOptions->numExtraFlags);

  // loading lag thresholds
  LagInfo::setThreshold(clOptions->lagwarnthresh,(float)clOptions->maxlagwarn);

  // loading jitter thresholds
  LagInfo::setJitterThreshold(clOptions->jitterwarnthresh, (float)clOptions->maxjitterwarn);

  // loading packetloss thresholds
  LagInfo::setPacketLossThreshold(clOptions->packetlosswarnthresh, (float)clOptions->maxpacketlosswarn);

  // loading player callsign filters
  PlayerInfo::setFilterParameters(clOptions->filterCallsigns, clOptions->filter,clOptions->filterSimple);

  GameKeeper::Player::setMaxShots(clOptions->maxShots);

  enableReplayServer();
  setupBadWordFilter();

  // loading authentication parameters
  Authentication::init(clOptions->publicizedAddress.c_str(), clOptions->wksPort, clOptions->password.c_str());

  setupVoteArbiter();
  setupPublicInterfaces();
  setupMasterBanList();
  setupScoringOptions();

  // print networking info
  logDebugMessage(1,"\tlistening on %s:%i\n", serverAddress.getDotNotation().c_str(), clOptions->wksPort);
  logDebugMessage(1,"\twith title of \"%s\"\n", clOptions->publicizedTitle.c_str());

  setupPingReply();

  // adjust speed and height checking as required
  adjustTolerances();

  // no original world weapons in replay mode
  if (Replay::enabled())
    world->getWorldWeapons().clear();

  nextSuperFlagInsertion = BzTime::getCurrent();
  flagExp = -logf(0.5f) / FlagHalfLife;

  if (clOptions->startRecording)
    Record::start(AllPlayers);

  // trap some signals
  if (bzSignal(SIGINT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGINT, SIG_PF(terminateServer));

  bzSignal(SIGTERM, SIG_PF(terminateServer));

  return true;
}


static void doTickEvent()
{
  // fire off a tick event
  bz_TickEventData_V1 tickData;
  worldEventManager.callEvents(bz_eTickEvent,&tickData);
}


static float getAPIMaxWaitTime()
{
  float minTime = 100.0f;
  std::map<std::string, float>::const_iterator it;
  for (it = APIWaitTimes.begin(); it != APIWaitTimes.end(); ++it) {
    if (minTime > it->second) {
      minTime = it->second;
    }
  }
  return minTime;
}


static void checkWaitTime(BzTime &tm, float &waitTime)
{
  if ((countdownDelay >= 0) || (countdownResumeTime >= 0))
    waitTime = 0.5f; // 3 seconds too slow for match countdowns
  else if (countdownActive && clOptions->timeLimit > 0.0f)
    waitTime = 1.0f;

  // get time for next flag drop
  float dropTime;
  while ((dropTime = FlagInfo::getNextDrop(tm)) <= 0.0f) {
    // if any flags were in the air, see if they've landed
    for (int i = 0; i < numFlags; i++) {
      FlagInfo &flag = *FlagInfo::get(i);
      if (flag.landing(tm)) {
	if (flag.flag.status == FlagOnGround) {
	  sendFlagUpdateMessage(flag);
	} else {
	  resetFlag(flag);
        }
      }
    }
  }
  if (waitTime > dropTime) {
    waitTime = dropTime;
  }

  // get time for next Player internal action
  GameKeeper::Player::updateLatency(waitTime);

  // get time for the next world weapons shot
  if (world->getWorldWeapons().count() > 0) {
    float nextTime = world->getWorldWeapons().nextTime ();
    if (waitTime > nextTime) {
      waitTime = nextTime;
    }
  }

  // get time for the next replay packet (if active)
  if (Replay::enabled()) {
    float nextTime = Replay::nextTime ();
    if (waitTime > nextTime) {
      waitTime = nextTime;
    }
  } else {
    // game time updates
    const float nextGT = nextGameTime();
    if (waitTime > nextGT) {
      waitTime = nextGT;
    }
  }

  // see if we are within the plug requested max wait time
  const float plugInWait = getAPIMaxWaitTime();
  if (waitTime > plugInWait) {
    waitTime = plugInWait;
  }

  const float sendLagWaitTime = SendLagState::getWaitTime();
  if (waitTime > sendLagWaitTime) {
    waitTime = sendLagWaitTime;
  }

  // minmal waitTime
  if (waitTime < 0.0f) {
    waitTime = 0.0f;
  }

  // if there are buffered UDP, no wait at all
  if (NetHandler::anyUDPPending()) {
    waitTime = 0.0f;
  }

  // don't wait (used by CURL and MsgEnter)
  if (dontWait) {
    waitTime = 0.0f;
    dontWait = false;
  }
}


static void doCountdown(int &readySetGo, BzTime &tm)
{
  // players see a countdown
  if (countdownDelay >= 0) {
    static BzTime timePrevious = tm;
    if (readySetGo == -1)
      readySetGo = countdownDelay;

    if (tm - timePrevious > 1.0f) {
      timePrevious = tm;

      if (readySetGo == 0) {
	sendMessage(ServerPlayer, AllPlayers, "The match has started!...Good Luck Teams!");
	countdownDelay = -1; // reset back to "unset"
	countdownResumeTime = -1; // reset back to "unset"
	readySetGo = -1; // reset back to "unset"
	countdownActive = true;
	gameOver = false;

	// start server's clock
	gameStartTime = tm;
	clOptions->timeElapsed = 0.0f;

	sendMsgTimeUpdate((int32_t)clOptions->timeLimit);

	// kill any players that are playing already
	GameKeeper::Player *player;

	if (clOptions->gameType == ClassicCTF) {
	  // cap all the flags
	  sendFlagCaptureMessage((uint8_t)curMaxPlayers,FlagInfo::lookupFirstTeamFlag(RedTeam),RedTeam);
	  sendFlagCaptureMessage((uint8_t)curMaxPlayers,FlagInfo::lookupFirstTeamFlag(GreenTeam),GreenTeam);
	  sendFlagCaptureMessage((uint8_t)curMaxPlayers,FlagInfo::lookupFirstTeamFlag(BlueTeam),BlueTeam);
	  sendFlagCaptureMessage((uint8_t)curMaxPlayers,FlagInfo::lookupFirstTeamFlag(PurpleTeam),PurpleTeam);

	  for (int j = 0; j < curMaxPlayers; j++) {
	    player = GameKeeper::Player::getPlayerByIndex(j);

	    if (!player || player->player.isObserver() || !player->player.isPlaying())
	      continue;

	    // kick 'em while they're down
	    playerKilled(j, curMaxPlayers, GotKilledMsg, -1, Flags::Null, -1);

	    // be sure to reset the player!
	    player->player.setDead();
	    zapFlagByPlayer(j);
	    player->player.setPlayedEarly(false);
	  }
	}

	// reset all flags
	for (int j = 0; j < numFlags; j++)
	  zapFlag(*FlagInfo::get(j));

	resetTeamScores();

	// fire off a game start event
	bz_GameStartEndEventData_V1 gameData;
	gameData.eventType = bz_eGameStartEvent;
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

  // players see the announce of resuming the countdown
  if (countdownResumeTime >= 0) {
    static BzTime timePrevious = tm;
    if (tm - timePrevious > 1.0f) {
      timePrevious = tm;
      if (gameOver)
	countdownResumeTime = -1; // reset back to "unset"
      else if (countdownResumeTime == 0) {
	countdownResumeTime = -1; // reset back to "unset"
	clOptions->countdownPaused = false;
	sendMessage(ServerPlayer, AllPlayers, "Countdown resumed");
      } else {
	sendMessage(ServerPlayer, AllPlayers, TextUtils::format("%i...", countdownResumeTime).c_str());
	--countdownResumeTime;
      }
    } // end check if second has elapsed
  } // end check if countdown resuming delay is active

  // see if game time ran out or if we are paused
  if (!gameOver && countdownActive && clOptions->timeLimit > 0.0f) {
    float newTimeElapsed = (float)(tm - gameStartTime);
    float timeLeft = clOptions->timeLimit - newTimeElapsed;
    if (timeLeft <= 0.0f && !countdownPauseStart) {
      timeLeft = 0.0f;
      gameOver = true;
      countdownActive = false;
      countdownPauseStart = BzTime::getNullTime ();
      clOptions->countdownPaused = false;

      // fire off a game end event
      bz_GameStartEndEventData_V1 gameData;
      gameData.eventType = bz_eGameEndEvent;
      gameData.duration = clOptions->timeLimit;
      worldEventManager.callEvents(bz_eGameEndEvent,&gameData);
    }

    if (countdownActive && clOptions->countdownPaused && !countdownPauseStart) {
      // we have a new pause
      countdownPauseStart = tm;
      sendMsgTimeUpdate(-1);
    }

    if (countdownActive && !clOptions->countdownPaused && (countdownResumeTime < 0) && countdownPauseStart) {
      // resumed
      gameStartTime += (tm - countdownPauseStart);
      countdownPauseStart = BzTime::getNullTime ();
      newTimeElapsed = (float)(tm - gameStartTime);
      timeLeft = clOptions->timeLimit - newTimeElapsed;
      sendMsgTimeUpdate((int32_t)timeLeft);
    }

    if ((timeLeft == 0.0f || newTimeElapsed - clOptions->timeElapsed >= 30.0f || clOptions->addedTime != 0.0f) &&
	!clOptions->countdownPaused &&
	(countdownResumeTime < 0)) {
      // send update every 30 seconds, when the game is over, or when time adjusted
      if (clOptions->addedTime != 0.0f) {
	if (timeLeft + clOptions->addedTime <= 0.0f)
	  timeLeft = 0.0f;
	else
	  clOptions->timeLimit += clOptions->addedTime;

	if (timeLeft > 0.0f)
	  timeLeft += clOptions->addedTime;

	//inform visitors about the change
	sendMessage(ServerPlayer, AllPlayers,
		    TextUtils::format("Adjusting the countdown by %f seconds",
				      clOptions->addedTime).c_str());

	clOptions->addedTime = 0.0f; //reset
      }

      sendMsgTimeUpdate((int32_t)timeLeft);
      clOptions->timeElapsed = newTimeElapsed;
      if (clOptions->oneGameOnly && timeLeft == 0.0f) {
	sendMessage(ServerPlayer, AllPlayers,
		    "automatically shutting down now that the game is over");
	serverDone = true;
      }
    }
  }
}


static void doPlayerStuff()
{
  requestAuthentication = false;

  for (int p = 0; p < curMaxPlayers; p++) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(p);
    if (!playerData)
      continue;

    doStuffOnPlayer(*playerData);
  }

  if (requestAuthentication)
    listServerLink->queueMessage(ListServerLink::ADD);	// Request the listserver authentication
}


static void doVoteArbiter(BzTime &tm)
{
  // manage voting poll for collective kicks/bans/sets
  if ((clOptions->voteTime > 0) && (votingArbiter != NULL)) {
    if (votingArbiter->knowsPoll()) {
      char message[MessageLen];

      std::string pollRequester = votingArbiter->getPollRequestor();
      std::string target = votingArbiter->getPollTarget();
      std::string action = votingArbiter->getPollAction();
      std::string realIP = votingArbiter->getPollTargetIP();

      static unsigned short int voteTime = 0;

      /* flags to only blather once */
      static bool announcedOpening = false;
      static bool announcedClosure = false;
      static bool announcedResults = false;

      /* once a poll begins, announce its commencement */
      if (!announcedOpening) {
	voteTime = votingArbiter->getVoteTime();
	snprintf(message, MessageLen, "A poll to %s %s has begun.  Players have up to %d seconds to vote.",
		 action.c_str(), target.c_str(), voteTime);
	sendMessage(ServerPlayer, AllPlayers, message);
	announcedOpening = true;
      }

      static BzTime lastAnnounce = BzTime::getNullTime();

      /* make a heartbeat announcement every 15 seconds */
      if (((voteTime - (int)(tm - votingArbiter->getStartTime()) - 1) % 15 == 0) &&
	  ((int)(tm - lastAnnounce) != 0) &&
	  (votingArbiter->timeRemaining() > 0)) {
	snprintf(message, MessageLen, "%d seconds remain in the poll to %s %s.",
		 votingArbiter->timeRemaining(), action.c_str(), target.c_str());
	sendMessage(ServerPlayer, AllPlayers, message);
	lastAnnounce = tm;
      }

      if (votingArbiter->isPollClosed()) {
	if (!announcedResults) {
	  snprintf(message, MessageLen,
		   "Poll Results: %ld in favor, %ld oppose, %ld abstain",
		   votingArbiter->getYesCount(), votingArbiter->getNoCount(),
		   votingArbiter->getAbstentionCount());
	  sendMessage(ServerPlayer, AllPlayers, message);
	  announcedResults = true;
	}

	if (votingArbiter->isPollSuccessful()) {
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
	    if (action != "flagreset")
	      snprintf(message, MessageLen,
		       "The poll is now closed and was successful.  %s is scheduled to be %s.",
		       target.c_str(), pollAction.c_str());
	    else
	      snprintf(message, MessageLen,
		       "The poll is now closed and was successful.  Currently unused flags are scheduled to be reset.");

	    sendMessage(ServerPlayer, AllPlayers, message);
	    announcedClosure = true;
	  }
	} else {
	  if (!announcedClosure) {
	    snprintf(message, MessageLen,
		     "The poll to %s %s was not successful", action.c_str(),
		     target.c_str());
	    sendMessage(ServerPlayer, AllPlayers, message);
	    announcedClosure = true;

	    // go ahead and reset the poll (don't bother waiting for veto timeout)
	    votingArbiter->forgetPoll();
	    announcedClosure = false;
	  }
	}

	/* the poll either terminates by itself or via a veto command */
	if (votingArbiter->isPollExpired()) {
	  /* maybe successful, maybe not */
	  if (votingArbiter->isPollSuccessful()) {
	    // perform the action of the poll, if any
	    std::string pollAction;

	    if (action == "ban") {
	      int hours = 0;
	      int minutes = clOptions->banTime % 60;

	      if (clOptions->banTime > 60)
		hours = clOptions->banTime / 60;

	      pollAction = std::string("banned for ");

	      if (hours > 0)
		pollAction += TextUtils::format("%d hour%s%s", hours, hours == 1 ? "." : "s", minutes > 0 ? " and " : "");

	      if (minutes > 0)
		pollAction += TextUtils::format("%d minute%s", minutes, minutes > 1 ? "s" : "");

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
	      std::string reason = TextUtils::format("%s from a poll ban",target.c_str());
	      clOptions->acl.ban(realIP.c_str(), pollRequester.c_str(), clOptions->banTime, reason.c_str());
	    }

	    if ((action == "ban") || (action == "kick")) {
	      // lookup the player id
	      bool foundPlayer = false;
	      int v;

	      for (v = 0; v < curMaxPlayers; v++) {
		GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(v);
		if (otherData && (strncmp(otherData->player.getCallSign(), target.c_str(), 256) == 0)) {
		  foundPlayer = true;
		  break;
		}
	      }
	      // show the delinquent no mercy; make sure he is kicked even if he changed
	      // his callsign by finding a corresponding IP and matching it to the saved one
	      if (!foundPlayer) {
		NetHandler *player = NetHandler::whoIsAtIP(realIP);
		for (v = 0; v < curMaxPlayers; v++) {
		  GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(v);
		  if (otherData && (otherData->netHandler == player)) {
		    foundPlayer = true;
		    break;
		  }
		}
	      }
	      if (foundPlayer) {
		// notify the player
		snprintf(message, MessageLen,
			 "You have been %s due to sufficient votes to have you removed",
			 action == "ban" ? "temporarily banned" : "kicked");
		sendMessage(ServerPlayer, v, message);
		snprintf(message,  MessageLen, "/poll %s", action.c_str());
		removePlayer(v, message, true);
	      }
	    } else if (action == "set") {
	      std::vector<std::string> args = TextUtils::tokenize(target.c_str(), " ", 2, true);
	      if ( args.size() < 2 ) {
		logDebugMessage(1, "Poll set taking action: no action taken, not enough parameters (%s).\n",
				(args.size() > 0 ? args[0].c_str() : "No parameters."));
	      } else {
		StateDatabase::Permission permission = BZDB.getPermission(args[0]);
		if (!(BZDB.isSet(args[0]) &&
		      (permission == StateDatabase::ReadWrite || permission == StateDatabase::Locked))) {
		  logDebugMessage(1,"Poll set taking action: no action taken, variable cannot be set\n");
		} else {
		  logDebugMessage(1,"Poll set taking action: setting %s to %s\n", args[0].c_str(), args[1].c_str());
		  BZDB.set(args[0], args[1], StateDatabase::Server);
		}
	      }
	    } else if (action == "reset") {
	      logDebugMessage(1,"Poll flagreset taking action: resetting unused flags.\n");
	      for (int f = 0; f < numFlags; f++) {
		FlagInfo &flag = *FlagInfo::get(f);
		if (flag.player == -1)
		  resetFlag(flag);
	      }
	    }
	  } /* end if poll is successful */

	  // get ready for the next poll
	  votingArbiter->forgetPoll();

	  announcedClosure = false;
	  announcedOpening = false;
	  announcedResults = false;

	} // the poll expired

      } else {
	// the poll may get enough votes early
	if (votingArbiter->isPollSuccessful()) {
	  if (action != "flagreset")
	    snprintf(message,  MessageLen, "Enough votes were collected to %s %s early.", action.c_str(), target.c_str());
	  else
	    snprintf(message,  MessageLen, "Enough votes were collected to reset all unused flags early.");

	  sendMessage(ServerPlayer, AllPlayers, message);

	  // close the poll since we have enough votes (next loop will kick off notification)
	  votingArbiter->closePoll();

	} // the poll is over
      } // is the poll closed
    } // knows of a poll
  } // voting is allowed and an arbiter exists
}


static void doTextBroadcasts ( BzTime &tm )
{
  // periodic advertising broadcast
  static const std::vector<std::string>* adLines = clOptions->textChunker.getTextChunk("admsg");

  if ((clOptions->advertisemsg != "") || adLines != NULL) {
    static BzTime lastbroadcast = tm;

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
}


static void doTeamFlagTimeouts ( BzTime &tm )
{
  // check team flag timeouts
  if (clOptions->gameType == ClassicCTF) {
    for (int i = RedTeam; i < CtfTeams; ++i) {
      if (teamInfos[i].flagTimeout - tm < 0 && teamInfos[i].team.size == 0) {
	int flagid = FlagInfo::lookupFirstTeamFlag(i);
	if (flagid >= 0) {
	  for (int n = 0; n < clOptions->numTeamFlags[i]; n++) {
	    FlagInfo &flag = *FlagInfo::get(flagid + n);
	    if (flag.exist() && flag.player == -1) {
	      logDebugMessage(1,"Flag timeout for team %d\n", i);
	      zapFlag(flag);
	    }
	  }
	}
      }
    }
  }
}


static void doSuperFlags ( BzTime &tm )
{
  // maybe add a super flag (only if game isn't over)
  if (!gameOver && clOptions->numExtraFlags > 0 && nextSuperFlagInsertion <= tm) {
    // randomly choose next flag respawn time; halflife distribution
    float r = float(bzfrand() + 0.01); // small offset, we do not want to wait forever

    nextSuperFlagInsertion += -logf(r) / flagExp;

    for (int i = numFlags - clOptions->numExtraFlags; i < numFlags; i++) {
      FlagInfo &flag = *FlagInfo::get(i);

      if (flag.flag.type == Flags::Null) {
	// flag in now entering game
	flag.addFlag();
	sendFlagUpdateMessage(flag);
	break;
      }
    }
  }
}


static void doListServerUpdate ( BzTime &tm )
{
  // occasionally add ourselves to the list again (in case we were dropped for some reason).
  if ((clOptions->publicizeServer) && (tm - listServerLink->lastAddTime > ListServerReAddTime)) {
    // if there are no list servers and nobody is playing then
    // try publicizing again because we probably failed to get
    // the list last time we published, and if we don't do it
    // here then unless somebody stumbles onto this server then
    // quits we'll never try publicizing ourself again.
    // if nobody playing then publicize
    if (listServerLinksCount == 0 && GameKeeper::Player::count() == 0)
      publicize();

    // send add request
    listServerLink->queueMessage(ListServerLink::ADD);
  }
}


void sendBufferedNetDataForPeer (NetConnectedPeer &peer )
{
  if (peer.sendChunks.empty()) {
    return;
  }

  const std::string& chunk = peer.sendChunks.front();
  peer.netHandler->bufferedSend(chunk.data(), chunk.size());

  peer.sendChunks.pop_front();
}


bool updateCurl()
{
  // let curl do it's own select
  fd_set curlReadSet,curlWriteSet;
  FD_ZERO(&curlReadSet);
  FD_ZERO(&curlWriteSet);

  struct timeval timeout;

  timeout.tv_sec = long(floorf(0.01f));
  timeout.tv_usec = long(1.0e+6f * (0.01f - floorf(0.01f)));

  int curlMaxFile = cURLManager::fdset(curlReadSet,curlWriteSet);

  select(curlMaxFile+1, (fd_set*)&curlReadSet, (fd_set*)&curlWriteSet, 0, &timeout);

  return cURLManager::perform();
}


static void processConnectedPeer(NetConnectedPeer& peer, int sockFD,
                                 fd_set& read_set, fd_set& write_set)
{
  if (peer.deleteMe) {
    return; // skip it, it's dead to us, we'll close and purge it later
  }

  NetHandler* netHandler = peer.netHandler;

  if (peer.player != -1) {
    peer.sent = true;
    // it's a player now, so treat them with the respect they deserve
    GameKeeper::Player* playerData =
      GameKeeper::Player::getPlayerByIndex(peer.player);

    if (playerData && netHandler) {
      // send whatever we have ... if any
      if (netHandler->pflush(&write_set) == -1) {
        removePlayer(peer.player, "ECONNRESET/EPIPE", false);
      } else {
        playerData->handleTcpPacket(&read_set);
      }
    }
    // check for any async data to send out
    sendBufferedNetDataForPeer(peer);
    return; // player connection has been handled
  }

  // it's not a player yet (but may be), check for any data to send out
  sendBufferedNetDataForPeer(peer);

  if (netHandler->pflush(&write_set) == -1) {
    peer.deleteMe = true; // kill it later
    return; // disconnected
  }

  if (!netHandler->isFdSet(&read_set)) {
    // there is no data, so delete us or move along
    // ... and it's not a player for god's sake
    if ((peer.player == -1) && !peer.apiHandler && peer.sendChunks.empty()) {
      if (peer.sent) {
        // he has sent some data, but now is dead, let em go.
        peer.deleteMe = true;
      }
    }
    return;
  }

  // there is some data for us
  if (!peer.apiHandler) {
    // no apiHandler, we are probably new, just read in the first N bits
    RxStatus e = netHandler->receive(strlen(BZ_CONNECT_HEADER));

    if ((e != ReadAll) && (e != ReadPart)) {
      // there was an error but we aren't a player yet
      if (e == ReadError) {
        nerror("error on read");
      }
      else {
        if (e == ReadHuge) {
          logDebugMessage(1,
            "socket [%d] sent huge packet length, possible attack\n", sockFD);
        }
        peer.deleteMe = true;
      }
    }
    else {
      // the dude has sent SOME data
      peer.sent = true;

      unsigned int readSize = netHandler->getTcpReadSize();
      void *buf = netHandler->getTcpBuffer();
      int fd = sockFD;

      // check for the magic header string
      const char*  header = BZ_CONNECT_HEADER;
      const size_t headerLen = strlen(header);

      if ((e == ReadAll) && (strncmp((char*)buf, header, headerLen) == 0)) {
        // it's a player, it sent us the magic string, get it setup
        netHandler->flushData();

        // send server version and playerid
        char buffer[9];
        memcpy(buffer, getServerVersion(), 8);

        // send 0xff if list is full
        buffer[8] = (char)0xff;

        PlayerId playerIndex = getNewPlayer(netHandler);
        peer.player = playerIndex;

        if (playerIndex < 0xff) {
          logDebugMessage(1,
            "Player [%d] accept() from %s on %i\n",
            playerIndex, inet_ntoa(netHandler->getIPAddress()), fd);

          buffer[8] = (uint8_t)playerIndex;
          send(fd, (const char*)buffer, sizeof(buffer), 0);
        }
        else {
          // full? reject by closing socket
          logDebugMessage(1,
            "all slots occupied, rejecting accept() from %s on %i\n",
            inet_ntoa(netHandler->getIPAddress()), fd);

          // send back 0xff before closing
          send(fd, (const char*)buffer, sizeof(buffer), 0);
          peer.deleteMe = true; // kill/close later
        }
      }
      else { // not the BZFS header
        // it's NOT a player but it sent us data,
        // see if anyone wants to deal with it

        // ok read in all the data we may have waiting
        void* data = malloc(readSize);
        memcpy(data, buf, readSize);
        unsigned int totalSize = readSize;

        while (e == ReadAll) {
          netHandler->flushData();

          e = netHandler->receive(256);
          readSize = netHandler->getTcpReadSize();
          buf = netHandler->getTcpBuffer();

          unsigned char *temp = (unsigned char*)malloc(totalSize + readSize);
          memcpy(temp, data, totalSize);
          memcpy(temp + totalSize, buf, readSize);
          free(data);
          data = temp;
          totalSize += readSize;
        }

        // we have a copy of all the data, so we can flush now
        netHandler->flushData();

        // call an event to let people know we got a new connect
        bz_NewNonPlayerConnectionEventData_V1 eventData;

        eventData.data = data;
        eventData.size = totalSize;
        eventData.connectionID = sockFD;

        worldEventManager.callEvents(bz_eNewNonPlayerConnection, &eventData);
        free(data);

        if (!peer.apiHandler && peer.sendChunks.empty()) {
          // nobody wanted it and it's got nothing to send so mark it for purge
          peer.deleteMe = true;
        }
      }
    }
  }
  else { // (peer.apiHandler != NULL)
    // we have an API listener, so lets get all our data,
    // and send it to him so he can do what he wants with it
    RxStatus e = netHandler->receive(256);

    if ((e != ReadAll) && (e != ReadPart)) {
      // we could not read, it must have disconnected, or there was an error
      if (peer.apiHandler) {
        peer.apiHandler->disconnect(sockFD);
      }

      peer.deleteMe = true;

      if (e == ReadError) {
        nerror("error on read");
      }

      if (e == ReadHuge) {
        logDebugMessage(1,
          "socket [%d] sent huge packet length, possible attack\n", sockFD);
      }
    }
    else {
      // the dude has sent SOME data
      peer.sent = true;

      unsigned int readSize = netHandler->getTcpReadSize();
      void* buf = netHandler->getTcpBuffer();

      void* data = malloc(readSize);
      memcpy(data, buf, readSize);
      unsigned int totalSize = readSize;

      while (e == ReadAll) {
        netHandler->flushData();

        e = netHandler->receive(256);
        readSize = netHandler->getTcpReadSize();
        buf = netHandler->getTcpBuffer();

        unsigned char* temp = (unsigned char*)malloc(totalSize + readSize);
        memcpy(temp, data, totalSize);
        memcpy(temp + totalSize, buf, readSize);
        free(data);
        data = temp;
        totalSize += readSize;
      }

      netHandler->flushData();

      if (peer.apiHandler) {
        peer.apiHandler->pending(sockFD, data, totalSize);
      }

      free(data);
    }
  }
}


static void runMainLoop()
{
  /* MAIN SERVER RUN LOOP
   *
   * the main loop runs at approximately 2 iterations per 5 seconds
   * when there are no players on the field.  this can increase to
   * about 100 iterations per 5 seconds with a single player, though
   * average is about 20-40 iterations per five seconds.  Adding
   * world weapons will increase the number of iterations
   * substantially (about x10)
   **/

  int readySetGo = -1; // match countdown timer
  int nfound;
  while (!serverDone) {
    doTickEvent();
    updatePlayerPositions();
    checkForWorldDeaths();

    // see if the octree needs to be reloaded
    world->checkCollisionManager();

    // highest fd used
    int maxFileDescriptor = 0;

    // prepare select set
    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    NetHandler::setFd(&read_set, &write_set, maxFileDescriptor);

    // always listen for connections
    FD_SET((unsigned int)wksSocket, &read_set);
    if (wksSocket > maxFileDescriptor)
      maxFileDescriptor = wksSocket;

    // find timeout when next flag would hit ground
    BzTime tm = BzTime::getCurrent();

    // lets start by waiting 3 sec
    float waitTime = 3.0f;
    checkWaitTime(tm,waitTime);

    const BzTime nowTime = BzTime::getCurrent();
    double tcpTimeout = BZDB.eval(BZDBNAMES.TCPTIMEOUT);

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
    nfound = select(maxFileDescriptor + 1,
                    (fd_set*)&read_set, (fd_set*)&write_set, 0, &timeout);
    //if (nfound)
    //	logDebugMessage(1,"nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);

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
    tm = BzTime::getCurrent();
    PlayerInfo::setCurrentTime(tm);
    NetHandler::setCurrentTime(tm);

    doCountdown(readySetGo,tm);
    doPlayerStuff();

    GameKeeper::Player::setAllNeedHostbanChecked(false);

    doVoteArbiter(tm);
    doTextBroadcasts(tm);
    doTeamFlagTimeouts(tm);
    doSuperFlags(tm);
    doListServerUpdate(tm);
    SendLagState::update();

    // check messages
    if (nfound > 0) {
      //logDebugMessage(1,"chkmsg nfound,read,write %i,%08lx,%08lx\n", nfound, read_set, write_set);
      // first check initial contacts
      if (FD_ISSET(wksSocket, &read_set)) {
	acceptClient();
      }

      // check if we have any UDP packets pending
      if (NetHandler::isUdpFdSet(&read_set)) {
	BzTime receiveTime = BzTime::getCurrent();
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
	  getGeneralMessageInfo(&buf,code,len);

	  if (code == MsgPingCodeRequest) {
	    if (len != 2) {
	      continue;
            }
	    // if I'm ignoring pings
	    // then ignore the ping.
	    if (handlePings) {
	      respondToPing(Address(uaddr));
	      pingReply.write(NetHandler::getUdpSocket(), &uaddr);
	    }
	    continue;
	  }
	  else if (code == MsgEchoRequest) {
	    // Handle pings of the server from a client
	    // This could be factored into it's own function
	    // Also, Maybe have an option to ignore pings
	    unsigned char tag = 0;
	    buf = nboUnpackUInt8(buf, tag);

	    sendEchoResponse(&uaddr, tag);

	    continue;
	  }

	  if (!netHandler && (len == 1) && (code == MsgUDPLinkRequest)) {
	    // It is a UDP Link Request ... try to match it
	    uint8_t index;
	    buf = nboUnpackUInt8(buf, index);
	    GameKeeper::Player *playerData =
	      GameKeeper::Player::getPlayerByIndex(index);

	    if (playerData) {
	      netHandler = playerData->netHandler;
	      if (netHandler->isMyUdpAddrPort(uaddr, false)) {
		netHandler->setUDPin(&uaddr);

		// send client the message that we are ready for him
		sendUDPupdate(netHandler);

		logDebugMessage(2,"Inbound UDP up %s:%d\n",
				inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
	      } else {
		logDebugMessage(2,
		  "Inbound UDP rejected %s:%d different IP than original\n",
		  inet_ntoa(uaddr.sin_addr), ntohs(uaddr.sin_port));
	      }
	      continue;
	    }
	  }

	  // handle the command for UDP
	  handleCommand(ubuf, true, netHandler);

	  // don't spend more than 250ms receiving udp
	  if (BzTime::getCurrent() - receiveTime > 0.25f) {
	    logDebugMessage(2,
	      "Too much UDP traffic, will hope to catch up later\n");
	    break;
	  }
	}
      }

      // process eventual resolver requests
      NetHandler::checkDNS(&read_set, &write_set);

      // now check our connected peer list, and see if we have any data pending.
      std::map<int,NetConnectedPeer>::iterator peerItr;

      // get a list of connections to purge, then purge them
      std::vector<int> toKill;

      for (peerItr  = netConnectedPeers.begin();
           peerItr != netConnectedPeers.end(); ++peerItr) {
	if (peerItr->second.deleteMe) {
	  toKill.push_back(peerItr->first);
        }
      }

      for (unsigned int i = 0; i < toKill.size(); i++) {
	if (netConnectedPeers.find(toKill[i]) != netConnectedPeers.end()) {
	  NetConnectedPeer &peer = netConnectedPeers[toKill[i]];
	  if (peer.netHandler) {
	    delete(peer.netHandler);
          }
	  peer.netHandler = NULL;
	  netConnectedPeers.erase(netConnectedPeers.find(toKill[i]));
	}
      }

      // process the connections
      for (peerItr  = netConnectedPeers.begin();
           peerItr != netConnectedPeers.end(); ++peerItr) {
	processConnectedPeer(peerItr->second, peerItr->first,
	                     read_set, write_set);
      }
    }
    else if (nfound < 0) {
      if (getErrno() != EINTR) {
	// test code - do not uncomment, will cause big stuttering
	// BzTime::sleep(1.0f);
      }
    }
    else {
      if (NetHandler::anyUDPPending()) {
	NetHandler::flushAllUDP();
      }
    }

    // go thru all the connected users, see if they need any automatic handling
    std::map<int, NetConnectedPeer>::iterator peerItr;
    for (peerItr  = netConnectedPeers.begin();
         peerItr != netConnectedPeers.end(); ++peerItr) {
      NetConnectedPeer& peer = peerItr->second;
      if (!peer.sent) { // little guy hasn't sent us a thing
	// check for any waiting conenctions if they are timed,
	// then see if anyone wants them
	if ((nowTime - peer.startTime) > tcpTimeout) {
	  // see if anyone wants him
	  bz_NewNonPlayerConnectionEventData_V1 eventData;
	  eventData.eventType = bz_eIdleNewNonPlayerConnection;
	  eventData.connectionID = peerItr->first;

	  worldEventManager.callEvents(bz_eIdleNewNonPlayerConnection, &eventData);
	  peer.sent = true;

	  // no love for our little connection, let him go.
	  if (!peer.apiHandler && peer.sendChunks.empty()) {
	    peer.deleteMe = true;
          }
	}
      }
    }

    // Fire world weapons
    world->getWorldWeapons().fire();

    // clean any pending players and rebuild the world if necessary
    if (GameKeeper::Player::clean() && worldWasSentToAPlayer) {
      worldWasSentToAPlayer = false;
      if ((clOptions->worldFile == "") && !Replay::enabled()) {
	defineWorld();
      }
    }

    dontWait = dontWait || updateCurl();
  }
}


static void cleanupServer()
{
#ifdef BZ_PLUGINS
  unloadPlugins();
#endif

  LuaServer::kill();

  // print uptime
  logDebugMessage(1,"Shutting down server: uptime %s\n", BzTime::printTime(BzTime::getCurrent() - BzTime::getStartTime()).c_str());

  GameKeeper::Player::freeTCPMutex();
  serverStop();

  // remove from list server and disconnect
  delete listServerLink;

  // free misc stuff
  delete clOptions; clOptions = NULL;
  FlagInfo::setSize(0);
  delete world; world = NULL;
  delete[] worldDatabase; worldDatabase = NULL;
  delete votingArbiter; votingArbiter = NULL;

  Record::kill();
  Replay::kill();
  Flags::kill();

#if defined(_WIN32)
  WSACleanup();
#endif /* defined(_WIN32) */
}


void saveStartupInfo()
{
  std::string conf = getConfigDirName();
  conf += "bzfs.dir";
  FILE *fp = fopen(conf.c_str(),"wt");
  if (fp) {
    std::string exepath = getModuleName();
    exepath += "\n";
    fwrite(exepath.c_str(),exepath.size(),1,fp);
    fclose(fp);
  }
}


/** main parses command line options and then enters an event and activity
 * dependant main loop.  once inside the main loop, the server is up and
 * running and should be ready to process connections and activity.
 */
int main(int argc, char **argv)
{
  // setup all the data for the server
  if (!initServer(argc,argv))
    return 1;

  registerDefaultHandlers();

  saveStartupInfo();

  // start the server
  if (!serverStart()) {
#if defined(_WIN32)
    WSACleanup();
#endif /* defined(_WIN32) */
    std::cerr << "ERROR: Unable to start the server, perhaps one is already running?" << std::endl;
    return 2;
  }

  GameKeeper::Player::passTCPMutex();

  runMainLoop();

  cleanupServer();
  cleanupDefaultHandlers();

  // done
  return 0;
}


bool worldStateChanging()
{
  return (BzTime::getCurrent() - lastWorldParmChange) <= 10.0f;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
