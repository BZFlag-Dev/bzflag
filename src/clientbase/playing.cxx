/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "playing.h"

// system includes
#include <iostream>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#  include <shlobj.h>
#  include <direct.h>
#else
#  include <pwd.h>
#  include <dirent.h>
#  include <utime.h>
#endif

// common headers
#include "AccessList.h"
#include "AnsiCodes.h"
#include "AresHandler.h"
#include "BaseBuilding.h"
#include "BZDBCache.h"
#include "CacheManager.h"
#include "CommandsStandard.h"
#include "Downloads.h"
#include "FileManager.h"
#include "GameTime.h"
#include "ServerList.h"
#include "TextUtils.h"
#include "bzsignal.h"


// common client headers
#include "ClientIntangibilityManager.h"
#include "FlashClock.h"
#include "Region.h"
#include "Roster.h"
#include "SyncClock.h"
#include "World.h"
#include "WorldPlayer.h"


#include "motd.h"

#include "clientvars.h"


// FIXME: The following are referenced in some parts of the client base
//        when it should really be entirely in the GUI
//        note: many of them are from clientCommands.cxx and callbacks.cxx
#include "HUDRenderer.h"

BzfDisplay* display = NULL;
ControlPanel *controlPanel = NULL;
HUDRenderer *hud = NULL;
MainWindow *mainWindow = NULL;
CommandCompleter completer;
float roamDZoom = 0.0f;
int savedVolume = -1;
bool fireButton = false;
bool roamButton = false;
const float FlagHelpDuration = 60.0f;

ThirdPersonVars thirdPersonVars;
// END FIXME


StartupInfo startupInfo;
FlashClock pulse;
ServerLink *serverLink = NULL;
WordFilter *wordFilter = NULL;

LocalPlayer *myTank = NULL;
World *world = NULL;
Team *teams = NULL;

WorldBuilder *worldBuilder = NULL;
WorldDownLoader* worldDownLoader = NULL;
MessageOfTheDay *motd = NULL;
PlayerId msgDestination;

bool downloadingData = false;

static AresHandler ares;

std::vector<PlayingCallbackItem> playingCallbacks;

bool serverDied = false;
bool joinRequested = false;
bool waitingDNS = false;
bool serverError = false;
bool entered = false;
bool joiningGame = false;

double epochOffset;
double lastEpochOffset;
double userTimeEpochOffset;

bool justJoined = false;
bool canSpawn = true;
bool gameOver = false;

int numFlags = 0;

float clockAdjust = 0.0f;
bool pausedByUnmap = false;
float pauseCountdown = 0.0f;
float destructCountdown = 0.0f;

const char *blowedUpMessage[] = {
  NULL,
  "Got shot by ",
  "Got flattened by ",
  "Team flag was captured by ",
  "Teammate hit with Genocide by ",
  "Tank Self Destructed",
  "Tank Rusted"
};

static AccessList serverAccessList("ServerAccess.txt", NULL);

static const char AutoJoinContent[] = // FIXME
  "#\n"
  "# This file controls the auto-join confirmation requirements.\n"
  "# Patterns are attempted in order against both the hostname\n"
  "# and ip. The first matching pattern sets the state. If no\n"
  "# patterns are matched, then the server is authorized. There\n"
  "# are four types of matches:\n"
  "#\n"
  "#   simple globbing (* and ?)\n"
  "#     allow\n"
  "#     deny\n"
  "#\n"
  "#   regular expressions\n"
  "#     allow_regex\n"
  "#     deny_regex\n"
  "#\n"
  "\n"
  "#\n"
  "# To authorize all servers, remove the last 3 lines.\n"
  "#\n"
  "\n"
  "allow *.bzflag.bz\n"
  "allow *.bzflag.org\n"
  "deny *\n";
static AccessList autoJoinAccessList("AutoJoinAccess.txt", AutoJoinContent);


#ifdef ROBOT
std::vector<BzfRegion*> obstacleList;  // for robots
#endif


StartupInfo *getStartupInfo()
{
  return &startupInfo;
}


void addPlayingCallback(PlayingCallback cb, void *data)
{
  PlayingCallbackItem item;
  item.cb = cb;
  item.data = data;
  playingCallbacks.push_back(item);
}


void removePlayingCallback(PlayingCallback _cb, void *data)
{
  std::vector<PlayingCallbackItem>::iterator it = playingCallbacks.begin();
  while (it != playingCallbacks.end()) {
    if (it->cb == _cb && it->data == data) {
      playingCallbacks.erase(it);
      break;
    }
    it++;
  }
}


void callPlayingCallbacks()
{
  const size_t count = playingCallbacks.size();
  for (size_t i = 0; i < count; i++) {
    const PlayingCallbackItem &cb = playingCallbacks[i];
    (*cb.cb)(cb.data);
  }
}


//
// handle signals that should kill me nicely
//
void suicide(int sig)
{
  bzSignal(sig, SIG_PF(suicide));
  CommandsStandard::quit();
}


//
// handle signals that should disconnect me from the server
//
void hangup(int sig)
{
  bzSignal(sig, SIG_PF(hangup));
  serverDied = true;
  serverError = true;
}


void updateNumPlayers()
{
  int i, numPlayers[NumTeams];
  for (i = 0; i < NumTeams; i++)
    numPlayers[i] = 0;
  for (i = 0; i < curMaxPlayers; i++)
    if (remotePlayers[i])
      numPlayers[remotePlayers[i]->getTeam()]++;
  if (myTank)
    numPlayers[myTank->getTeam()]++;
}


static void sendFlagNegotiation()
{
  char msg[MaxPacketLen];
  FlagTypeMap::iterator i;
  char *buf = msg;

  /* Send MsgNegotiateFlags to the server with
   * the abbreviations for all the flags we support.
   */
  for (i = FlagType::getFlagMap().begin();
       i != FlagType::getFlagMap().end(); i++) {
    buf = (char*) i->second->pack(buf);
  }
  serverLink->send(MsgNegotiateFlags, (int)(buf - msg), msg);
}


void joinInternetGame(const struct in_addr *inAddress)
{
  // get server address
  Address serverAddress(*inAddress);
  if (serverAddress.isAny()) {
    showError("Server not found");
    return;
  }

  // check for a local server block
  serverAccessList.reload();
  std::vector<std::string> nameAndIp;
  nameAndIp.push_back(startupInfo.serverName);
  nameAndIp.push_back(serverAddress.getDotNotation());
  if (!serverAccessList.authorized(nameAndIp)) {
    showError("Server Access Denied Locally");
    std::string msg = ColorStrings[WhiteColor];
    msg += "NOTE: ";
    msg += ColorStrings[GreyColor];
    msg += "server access is controlled by ";
    msg += ColorStrings[YellowColor];
    msg += serverAccessList.getFileName();
    addMessage(NULL, msg);
    return;
  }

  syncedClock.update(NULL);
  // open server
  ServerLink *_serverLink = new ServerLink(serverAddress,
    startupInfo.serverPort);

  serverLink = _serverLink;

  serverLink->sendVarRequest();

  // assume everything's okay for now
  serverDied = false;
  serverError = false;

  if (!serverLink) {
    showError("Memory error");
    return;
  }

  // check server
  if (serverLink->getState() != ServerLink::Okay) {
    switch (serverLink->getState()) {
      case ServerLink::BadVersion: {
        static char versionError[] = "Incompatible server version XXXXXXXX";
        strncpy(versionError + strlen(versionError) - 8,
          serverLink->getVersion(), 8);
        showError(versionError);
        break;
      }
      case ServerLink::Refused: {
        // you got banned
        const std::string &rejmsg = serverLink->getRejectionMessage();

        // add to the HUD
        std::string msg = ColorStrings[RedColor];
        msg += "You have been banned from this server";
        showError(msg.c_str());

        // add to the console
        msg = ColorStrings[RedColor];
        msg += "You have been banned from this server:";
        addMessage(NULL, msg);
        msg = ColorStrings[YellowColor];
        msg += rejmsg;
        addMessage(NULL, msg);

        break;
      }
      case ServerLink::Rejected: {
        // the server is probably full or the game is over.  if not then
        // the server is having network problems.
        showError("Game is full or over.  Try again later.");
        break;
      }
      case ServerLink::SocketError: {
        showError("Error connecting to server.");
        break;
      }
      case ServerLink::CrippledVersion: {
        // can't connect to (otherwise compatible) non-crippled server
        showError("Cannot connect to full version server.");
        break;
      }
      default: {
        showError(TextUtils::format
          ("Internal error connecting to server (error code %d).",
          serverLink->getState()).c_str());
        break;
      }
    }
    return;
  }
  // use parallel UDP if desired and using server relay
  serverLink->sendUDPlinkRequest();

  showError("Connection Established...");

  sendFlagNegotiation();
  joiningGame = true;
  GameTime::reset();
  syncedClock.update(serverLink);
}



void getAFastToken(void)
{
  // get token if we need to (have a password but no token)
  if ((startupInfo.token[0] == '\0') && (startupInfo.password[0] != '\0')) {
    ServerList *serverList = new ServerList;
    serverList->startServerPings(&startupInfo);
    // wait no more than 10 seconds for a token
    for (int j = 0; j < 40; j++) {
      serverList->checkEchos(getStartupInfo());
      cURLManager::perform();
      if (startupInfo.token[0] != '\0')
	break;
      TimeKeeper::sleep(0.25f);
    }
    delete serverList;
  }

  // don't let the bad token specifier slip through to the server,
  // just erase it
  if (strcmp(startupInfo.token, "badtoken") == 0)
    startupInfo.token[0] = '\0';
}


void handlePendingJoins(void)
{
  // try to join a game if requested.  do this *before* handling
  // events so we do a redraw after the request is posted and
  // before we actually try to join.
  if (!joinRequested) {
    return;
  }

  // if already connected to a game then first sign off
  if (myTank)
    leaveGame();

  getAFastToken();

  ares.queryHost(startupInfo.serverName);
  waitingDNS = true;

  // don't try again
  joinRequested = false;
}


// check for Steamroller and Burrow kills
bool checkSquishKill(const Player* victim, const Player* killer, bool localKiller)
{
  static BZDB_float srRadiusMult(StateDatabase::BZDB_SRRADIUSMULT);

  if (!victim || !victim->isAlive() || victim->isPaused() ||
      !killer || !killer->isAlive() || killer->isPaused()) {
    return false;
  }

  const fvec3& victimPos = victim->getPosition();
  const fvec3& killerPos = killer->getPosition();

  if (killer->getFlag() != Flags::Steamroller) {
    if (victim->getFlag() != Flags::Burrow) {
      return false;
    }
    else if ((victimPos.z >= 0.0f) || (killerPos.z < 0.0f)) {
      return false;
    }
  }

  if (victim->isPhantomZoned() || (victim->getTeam() == ObserverTeam) ||
      killer->isPhantomZoned() || (killer->getTeam() == ObserverTeam)) {
    return false;
  }
  if (!localKiller && killer->isNotResponding()) {
    return false;
  }

  fvec3 diff = (victimPos - killerPos);
  diff.z *= 2.0f; // oblate spheroids

  const float victimRadius = victim->getRadius();
  const float killerRadius = killer->getRadius();

  const float radius = victimRadius + (killerRadius * srRadiusMult);

  return (diff.lengthSq() < (radius * radius));
}


bool dnsLookupDone(struct in_addr &inAddress)
{
  if (!waitingDNS)
    return false;

  fd_set readers, writers;
  int nfds = -1;
  struct timeval timeout;
  timeout.tv_sec  = 0;
  timeout.tv_usec = 0;
  FD_ZERO(&readers);
  FD_ZERO(&writers);
  ares.setFd(&readers, &writers, nfds);
  nfds = select(nfds + 1, (fd_set*)&readers, (fd_set*)&writers, 0, &timeout);
  ares.process(&readers, &writers);

  AresHandler::ResolutionStatus status = ares.getHostAddress(&inAddress);
  if (status == AresHandler::Failed) {
    showError("Server not found");
    waitingDNS = false;
  } else if (status == AresHandler::HbNSucceeded) {
    waitingDNS = false;
    return true;
  }
  return false;
}


void updateShots(const float dt)
{
  // update other tank's shots
  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i])
      remotePlayers[i]->updateShots(dt);
  }

  // update servers shots
  const World *_world = World::getWorld();
  if (_world)
    _world->getWorldWeapons()->updateShots(dt);
}


void setTankFlags()
{
  // scan through flags and, for flags on
  // tanks, tell the tank about its flag.
  const int maxFlags = world->getMaxFlags();
  for (int i = 0; i < maxFlags; i++) {
    const Flag &flag = world->getFlag(i);
    if (flag.status == FlagOnTank) {
      for (int j = 0; j < curMaxPlayers; j++) {
	if (remotePlayers[j] && remotePlayers[j]->getId() == flag.owner) {
	  remotePlayers[j]->setFlag(flag.type);
	  break;
	}
      }
    }
  }
}


#ifdef ROBOT

//
// some robot stuff
//

void addObstacle(std::vector<BzfRegion*> &rgnList, const Obstacle &obstacle)
{
  fvec2 p[4];
  const fvec3& c = obstacle.getPosition();
  const float tankRadius = BZDBCache::tankRadius;

  if (BZDBCache::tankHeight < c.z) {
    return;
  }

  const float a = obstacle.getRotation();
  const float w = obstacle.getWidth() + tankRadius;
  const float h = obstacle.getBreadth() + tankRadius;
  const float xx =  w * cosf(a);
  const float xy =  w * sinf(a);
  const float yx = -h * sinf(a);
  const float yy =  h * cosf(a);
  p[0].x = c.x - xx - yx;
  p[0].y = c.y - xy - yy;
  p[1].x = c.x + xx - yx;
  p[1].y = c.y + xy - yy;
  p[2].x = c.x + xx + yx;
  p[2].y = c.y + xy + yy;
  p[3].x = c.x - xx + yx;
  p[3].y = c.y - xy + yy;

  size_t numRegions = rgnList.size();
  for (size_t k = 0; k < numRegions; k++) {
    BzfRegion *region = rgnList[k];
    int side[4];
    if ((side[0] = region->classify(p[0], p[1])) == 1 ||
        (side[1] = region->classify(p[1], p[2])) == 1 ||
        (side[2] = region->classify(p[2], p[3])) == 1 ||
        (side[3] = region->classify(p[3], p[0])) == 1) {
      continue;
    }
    if (side[0] == -1 && side[1] == -1 && side[2] == -1 && side[3] == -1) {
      rgnList[k] = rgnList[numRegions - 1];
      rgnList[numRegions-1] = rgnList[rgnList.size() - 1];
      rgnList.pop_back();
      numRegions--;
      k--;
      delete region;
      continue;
    }
    for (size_t j = 0; j < 4; j++) {
      if (side[j] == -1) {
        continue; // to inside
      }
      // split
      const fvec2& p1 = p[j];
      const fvec2& p2 = p[(j + 1) & 3];
      BzfRegion *newRegion = region->orphanSplitRegion(p2, p1);
      if (!newRegion) continue; // no split
      if (region != rgnList[k]) rgnList.push_back(region);
      region = newRegion;
    }
    if (region != rgnList[k]) {
      delete region;
    }
  }
}


void makeObstacleList()
{
  const float tankRadius = BZDBCache::tankRadius;
  int i;
  for (std::vector<BzfRegion*>::iterator itr = obstacleList.begin();
    itr != obstacleList.end(); ++itr)
    delete (*itr);
  obstacleList.clear();

  // FIXME -- shouldn't hard code game area
  fvec2 gameArea[4];
  float worldSize = BZDBCache::worldSize;
  gameArea[0].x = -0.5f * worldSize + tankRadius;
  gameArea[0].y = -0.5f * worldSize + tankRadius;
  gameArea[1].x =  0.5f * worldSize - tankRadius;
  gameArea[1].y = -0.5f * worldSize + tankRadius;
  gameArea[2].x =  0.5f * worldSize - tankRadius;
  gameArea[2].y =  0.5f * worldSize - tankRadius;
  gameArea[3].x = -0.5f * worldSize + tankRadius;
  gameArea[3].y =  0.5f * worldSize - tankRadius;
  obstacleList.push_back(new BzfRegion(4, gameArea));

  const ObstacleList &boxes = OBSTACLEMGR.getBoxes();
  const int numBoxes = boxes.size();
  for (i = 0; i < numBoxes; i++)
    addObstacle(obstacleList, *boxes[i]);

  const ObstacleList &pyramids = OBSTACLEMGR.getPyrs();
  const int numPyramids = pyramids.size();
  for (i = 0; i < numPyramids; i++)
    addObstacle(obstacleList, *pyramids[i]);

  const ObstacleList &meshes = OBSTACLEMGR.getMeshes();
  const int numMeshes = meshes.size();
  for (i = 0; i < numMeshes; i++)
    addObstacle(obstacleList, *meshes[i]);

  if (World::getWorld()->allowTeamFlags()) {
    const ObstacleList &bases = OBSTACLEMGR.getBases();
    const int numBases = bases.size();
    for (i = 0; i < numBases; i++) {
      const BaseBuilding *base = (const BaseBuilding*) bases[i];
      if ((base->getHeight() != 0.0f) || (base->getPosition().z != 0.0f)) {
	addObstacle(obstacleList, *base);
      }
    }
  }
}


void setRobotTarget(RobotPlayer *robot)
{
  Player *bestTarget = NULL;
  float bestPriority = 0.0f;
  for (int j = 0; j < curMaxPlayers; j++)
    if (remotePlayers[j] && remotePlayers[j]->getId() != robot->getId() &&
      remotePlayers[j]->isAlive() &&
      ((robot->getTeam() == RogueTeam) || remotePlayers[j]->getTeam() != robot->getTeam())) {

	if (remotePlayers[j]->isPhantomZoned() && !robot->isPhantomZoned())
	  continue;

        const TeamColor robotTeam = robot->getTeam();
        const FlagType* flagType = remotePlayers[j]->getFlag();
	if (World::getWorld()->allowTeamFlags() &&
	    (((robotTeam == RedTeam)    && (flagType == Flags::RedTeam))   ||
	     ((robotTeam == GreenTeam)  && (flagType == Flags::GreenTeam)) ||
	     ((robotTeam == BlueTeam)   && (flagType == Flags::BlueTeam))  ||
	     ((robotTeam == PurpleTeam) && (flagType == Flags::PurpleTeam)))) {
          bestTarget = remotePlayers[j];
          break;
	}

	const float priority = robot->getTargetPriority(remotePlayers[j]);
	if (priority > bestPriority) {
	  bestTarget = remotePlayers[j];
	  bestPriority = priority;
	}
    }
    if (myTank->isAlive() &&
      ((robot->getTeam() == RogueTeam) || myTank->getTeam() != robot->getTeam())) {
	const float priority = robot->getTargetPriority(myTank);
	if (priority > bestPriority) {
	  bestTarget = myTank;
	  bestPriority = priority;
	}
    }
    robot->setTarget(bestTarget);
}


void updateRobots(float dt)
{
  static float newTargetTimeout = 1.0f;
  static float clock = 0.0f;
  bool pickTarget = false;
  int i;

  // see if we should look for new targets
  clock += dt;
  if (clock > newTargetTimeout) {
    while (clock > newTargetTimeout)
      clock -= newTargetTimeout;
    pickTarget = true;
  }

  // start dead robots
  for (i = 0; i < numRobots; i++)
    if (!gameOver && robots[i] && !robots[i]->isAlive() &&
      !robots[i]->isExploding() && pickTarget)
      serverLink->sendAlive(robots[i]->getId());

  // retarget robots
  for (i = 0; i < numRobots; i++) {
    if (robots[i] && robots[i]->isAlive() &&
      (pickTarget || !robots[i]->getTarget() ||
      !robots[i]->getTarget()->isAlive())) {
	setRobotTarget(robots[i]);
    }
  }

  // do updates
  for (i = 0; i < numRobots; i++)
    if (robots[i])
      robots[i]->update();
}

void sendRobotUpdates()
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i] && robots[i]->isDeadReckoningWrong()) {
      serverLink->sendPlayerUpdate(robots[i]);
    }
}

void addRobots()
{
  int  j;
  for (j = 0; j < MAX_ROBOTS; j++)
    robots[j] = NULL;
}

#endif


void doNetworkStuff(void)
{
  // send my data
  if (myTank && myTank->isDeadReckoningWrong() &&
    (myTank->getTeam() != ObserverTeam))
    serverLink->sendPlayerUpdate(myTank); // also calls setDeadReckoning()

#ifdef ROBOT
  if (entered)
    sendRobotUpdates();
#endif

  cURLManager::perform();
}


//
// server message handling
//
static Player* addPlayer(PlayerId id, void* msg, int showMessage)
{
  uint16_t team, type, wins, losses, tks;
  float rank;
  char callsign[CallSignLen];
  msg = nboUnpackUInt16 (msg, type);
  msg = nboUnpackUInt16 (msg, team);
  msg = nboUnpackFloat (msg, rank);
  msg = nboUnpackUInt16 (msg, wins);
  msg = nboUnpackUInt16 (msg, losses);
  msg = nboUnpackUInt16 (msg, tks);
  msg = nboUnpackString (msg, callsign, CallSignLen);

  // Strip any ANSI color codes
  strncpy(callsign, stripAnsiCodes(callsign), 32);

  // id is slot, check if it's empty
  const int i = id;

  // sanity check
  if (i < 0) {
    showError(TextUtils::format ("Invalid player identification (%d)", i).c_str());
    return NULL;
  }

  if (remotePlayers[i]) {
    // we're not in synch with server -> help! not a good sign, but not fatal.
    showError("Server error when adding player, player already added");
    return NULL;
  }

  if (i >= curMaxPlayers) {
    curMaxPlayers = i + 1;
    if (World::getWorld()) {
      World::getWorld()->setCurMaxPlayers(curMaxPlayers);
    }
  }

  // add player
  if (PlayerType (type) == TankPlayer
    || PlayerType (type) == ComputerPlayer
    || PlayerType (type) == ChatPlayer) {
    remotePlayers[i] = new RemotePlayer (id, TeamColor (team), callsign, PlayerType (type));
    remotePlayers[i]->changeScore (rank, short (wins), short (losses), short (tks));
  }

#ifdef ROBOT
  if (PlayerType (type) == ComputerPlayer) {
    for (int j = 0; j < numRobots; j++) {
      if (robots[j] && !strncmp (robots[j]->getCallSign (), callsign, CallSignLen)) {
	robots[j]->setTeam (TeamColor (team));
	robots[j]->changeScore(rank, wins, losses, tks);
	break;
      }
    }
  }
#endif

  // show the message if we don't have the playerlist
  // permission.  if * we do, MsgAdminInfo should arrive
  // with more info.
  if (showMessage && !myTank->hasPlayerList ()) {
    std::string message ("joining as ");
    if (team == ObserverTeam) {
      message += "an observer";
    } else {
      switch (PlayerType (type)) {
      case TankPlayer:
	message += "a tank";
	break;
      case ComputerPlayer:
	message += "a robot tank";
	break;
      default:
	message += "an unknown type";
	break;
      }
    }

    if (!remotePlayers[i]) {
      std::string name (callsign);
      name += ": " + message;
      message = name;
    }
    addMessage (remotePlayers[i], message);
  }
  completer.registerWord(callsign, true /* quote spaces */);

  return remotePlayers[i];
}


static void printIpInfo(const Player *player,
                        const Address &addr,
                        const std::string note)
{
  if (player == NULL) {
    return;
  }

  std::string colorStr;
  if (player->getId() < 200) {
    int color = player->getTeam();
    if (color == RabbitTeam || color < 0 || color > LastColor) {
      // non-teamed, rabbit are white (same as observer)
      color = WhiteColor;
    }
    colorStr = ColorStrings[color];
  } else {
    colorStr = ColorStrings[CyanColor]; // replay observers
  }

  const std::string addrStr = addr.getDotNotation();
  std::string message = ColorStrings[CyanColor]; // default color
  message += "IPINFO: ";
  if (BZDBCache::colorful) { message += colorStr; }
  message += player->getCallSign();
  if (BZDBCache::colorful) { message += ColorStrings[CyanColor]; }
  message += "\t from: ";
  if (BZDBCache::colorful) { message += colorStr; }
  message += addrStr;

  message += ColorStrings[WhiteColor];
  for (int i = 0; i < (17 - (int)addrStr.size()); i++) {
    message += " ";
  }

  message += note;

  // print into the Server Menu
  showMessage(message, ControlPanel::MessageServer);

  return;
}


static bool removePlayer (PlayerId id)
{
  int playerIndex = lookupPlayerIndex(id);

  if (playerIndex < 0)
    return false;

  Player *p = getPlayerByIndex(playerIndex);

  Address addr;
  std::string msg = "signing off";
  if (!p->getIpAddress(addr)) {
    addMessage(p, "signing off");
  } else {
    msg += " from ";
    msg += addr.getDotNotation();
    p->setIpAddress(addr);
    addMessage(p, msg);
    if (BZDB.evalInt("showips") > 1) {
      printIpInfo (p, addr, "(leave)");
    }
  }

  if (myTank->getRecipient() == p)
    myTank->setRecipient(0);
  if (myTank->getNemesis() == p)
    myTank->setNemesis(0);

  completer.unregisterWord(p->getCallSign());

  delete remotePlayers[playerIndex];
  remotePlayers[playerIndex] = NULL;

  while ((playerIndex >= 0) &&
         (playerIndex+1 == curMaxPlayers) &&
         (remotePlayers[playerIndex] == NULL)) {
    playerIndex--;
    curMaxPlayers--;
  }

  World *_world = World::getWorld();
  if (!_world) {
    return false;
  }

  _world->setCurMaxPlayers(curMaxPlayers);

  updateNumPlayers();

  return true;
}


#if defined(FIXME) && defined(ROBOT)
static void saveRobotInfo(Playerid id, void *msg)
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i]) {
      void *tmpbuf = msg;
      uint16_t team, type, wins, losses, tks;
      char callsign[CallSignLen];
      tmpbuf = nboUnpackUInt16(tmpbuf, type);
      tmpbuf = nboUnpackUInt16(tmpbuf, team);
      tmpbuf = nboUnpackUInt16(tmpbuf, wins);
      tmpbuf = nboUnpackUInt16(tmpbuf, losses);
      tmpbuf = nboUnpackUInt16(tmpbuf, tks);
      tmpbuf = nboUnpackString(tmpbuf, callsign, CallSignLen);
      std::cerr << "id " << id.port << ':' <<
	id.number << ':' <<
	callsign << ' ' <<
	robots[i]->getId().port << ':' <<
	robots[i]->getId().number << ':' <<
	robots[i]->getCallsign() << std::endl;
      if (strncmp(robots[i]->getCallSign(), callsign, CallSignLen)) {
	// check for real robot id
	char buffer[100];
	snprintf(buffer, 100, "id test %p %p %p %8.8x %8.8x\n",
	  robots[i], tmpbuf, msg, *(int *)tmpbuf, *((int *)tmpbuf + 1));
	std::cerr << buffer;
	if (tmpbuf < (char *)msg + len) {
	  PlayerId id;
	  tmpbuf = nboUnpackUInt8(tmpbuf, id);
	  robots[i]->id.serverHost = id.serverHost;
	  robots[i]->id.port = id.port;
	  robots[i]->id.number = id.number;
	  robots[i]->server->send(MsgIdAck, 0, NULL);
	}
      }
    }
}
#endif


static void dumpMissingFlag(char *buf, uint16_t len)
{
  int i;
  int nFlags = len/2;
  std::string flags;

  for (i = 0; i < nFlags; i++) {
    /* We can't use FlagType::unpack() here, since it counts on the
     * flags existing in our flag database.
     */
    if (i)
      flags += ", ";
    flags += buf[0];
    if (buf[1])
      flags += buf[1];
    buf += 2;
  }

  showError(TextUtils::format("Flags not supported by this client: %s",flags.c_str()).c_str());
}


//
// message handlers
//

void handleSetShotType(BufferedNetworkMessage *msg)
{
  PlayerId id = msg->unpackUInt8();
  unsigned char shotType = msg->unpackUInt8();

  Player *p = lookupPlayer(id);
  if (!p)
    return;
  p->setShotType((ShotType)shotType);
}


void handleWhatTimeIsIt(void *msg)
{
  double time = -1;
  unsigned char tag = 0;

  msg = nboUnpackUInt8(msg, tag);
  msg = nboUnpackDouble(msg, time);
  syncedClock.timeMessage(tag, time);
}


void handleSetTeam(void *msg, uint16_t len)
{
  if (len < 2) {
    return;
  }

  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

  uint8_t team;
  msg = nboUnpackUInt8(msg, team);

  Player *p = lookupPlayer(id);

  p->changeTeam((TeamColor)team);
}


void handleJoinServer(void *msg)
{
  // FIXME: MsgJoinServer notes ...
  //        - fix whatever is broken
  //        - add notifications
  //        - add an AutoJoinAccess.txt file to decided
  //          if confirmation queries or required

  std::string addr;
  int32_t port;
  int32_t team;
  std::string referrer;
  std::string message;

  msg = nboUnpackStdString(msg, addr);
  msg = nboUnpackInt32(msg, port);
  msg = nboUnpackInt32(msg, team);
  msg = nboUnpackStdString(msg, referrer);
  msg = nboUnpackStdString(msg, message);

  if (addr.empty()) {
    return;
  }
  if ((port < 0) || (port > 65535)) {
    return;
  }

  if (!BZDB.isTrue("autoJoin")) {
    addMessage(NULL,
      TextUtils::format("ignored autoJoin to %s:%i", addr.c_str(), port)
    );
    return;
  }

  autoJoinAccessList.reload();
  std::vector<std::string> nameAndIp;
  nameAndIp.push_back(addr);
//FIXME  nameAndIp.push_back(serverAddress.getDotNotation());
  if (!autoJoinAccessList.authorized(nameAndIp)) {
    showError("Auto Join Denied Locally");
    std::string warn = ColorStrings[WhiteColor];
    warn += "NOTE: ";
    warn += ColorStrings[GreyColor];
    warn += "auto joining is controlled by ";
    warn += ColorStrings[YellowColor];
    warn += autoJoinAccessList.getFileName();
    addMessage(NULL, warn);
    return;
  }

  StartupInfo& info = startupInfo;

  strncpy(info.serverName, addr.c_str(), ServerNameLen - 1);
  info.serverName[ServerNameLen - 1] = 0;

  strncpy(info.referrer, referrer.c_str(), ReferrerLen - 1);
  info.referrer[ReferrerLen - 1] = 0;

  info.serverPort = port;
  if (team == NoTeam) {
    // leave it alone, player can select using the menu
  } else {
    info.team = (TeamColor)team;
  }

  printf("AutoJoin: %s %u %i \"%s\" \"%s\"\n", // FIXME
         addr.c_str(), port, team, referrer.c_str(), message.c_str());

  joinGame();
}


void handleSuperKill(void *msg)
{
  uint8_t id;
  nboUnpackUInt8(msg, id);
  if (!myTank || myTank->getId() == id || id == 0xff) {
    serverError = true;
    showError("Server forced a disconnect");
#ifdef ROBOT
  } else {
    int i;
    for (i = 0; i < MAX_ROBOTS; i++) {
      if (robots[i] && robots[i]->getId() == id)
	break;
    }
    if (i >= MAX_ROBOTS)
      return;
    delete robots[i];
    robots[i] = NULL;
    numRobots--;
#endif
  }
}


void handleRejectMessage(void *msg)
{
  void *buf;
  char buffer[MessageLen];
  uint16_t rejcode;
  std::string reason;

  buf = nboUnpackUInt16 (msg, rejcode); // filler for now
  buf = nboUnpackString (buf, buffer, MessageLen);
  buffer[MessageLen - 1] = '\0';
  reason = buffer;
  showError(reason.c_str());
}


void handleFlagNegotiation(void *msg, uint16_t len)
{
  if (len > 0) {
    dumpMissingFlag((char *)msg, len);
    return;
  }
  serverLink->send(MsgWantSettings, 0, NULL);
}


void handleFlagType(void *msg)
{
FlagType* typ = NULL;
      FlagType::unpackCustom(msg, typ);
      logDebugMessage(1, "Got custom flag type from server: %s\n",
                         typ->information().c_str());
}


void handleGameSettings(void *msg)
{
  if (worldBuilder) {
    delete worldBuilder;
    worldBuilder = NULL;
  }
  worldBuilder = new WorldBuilder;
  worldBuilder->unpackGameSettings(msg);
  serverLink->send(MsgWantWHash, 0, NULL);
  showError("Requesting World Hash...");
}


void handleCacheURL(void *msg, uint16_t len)
{
  char *cacheURL = new char[len];
  nboUnpackString(msg, cacheURL, len);
  worldDownLoader->setCacheURL(cacheURL);
  delete [] cacheURL;
}


void handleWantHash(void *msg, uint16_t len)
{
  char *hexDigest = new char[len];
  nboUnpackString(msg, hexDigest, len);
  worldDownLoader->setCacheTemp(hexDigest[0] == 't');
  worldDownLoader->start(hexDigest);
  delete [] hexDigest;
}


void handleGetWorld(void *msg, uint16_t len)
{
  int32_t bytesLeft;
  msg = nboUnpackInt32(msg, bytesLeft);
  const uint32_t worldPtr =
    worldDownLoader->processChunk(msg, len - 4, bytesLeft);
  if (worldPtr > 0) {
    // ask for more
    char message[MaxPacketLen];
    nboPackUInt32(message, worldPtr);
    serverLink->send(MsgGetWorld, sizeof(uint32_t), message);
  }
}


void handleGameTime(void *msg)
{
  GameTime::unpack(msg);
  GameTime::update();
}


void handleAddPlayer(void* msg, bool& checkScores)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

#if defined(FIXME) && defined(ROBOT)
  saveRobotInfo(id, msg);
#endif

  if (id == myTank->getId()) {
    enteringServer(msg); // it's me!  should be the end of updates
  } else {
    addPlayer(id, msg, entered);
    updateNumPlayers();
    checkScores = true;

    if (myTank->getId() >= 200) {
      setTankFlags(); // update the tank flags when in replay mode.
    }
  }
}


void handleRemovePlayer(void *msg, bool &checkScores)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

  if (removePlayer(id)) {
    checkScores = true;
  }
}


void handleFlagUpdate(void *msg, size_t len)
{
  uint16_t count = 0;
  uint16_t flagIndex;
  if (len >= 2)
    msg = nboUnpackUInt16(msg, count);

  size_t perFlagSize = 2 + 55;

  if (len >= (2 + (perFlagSize*count)))
    for (int i = 0; i < count; i++) {
      msg = nboUnpackUInt16(msg, flagIndex);
      msg = world->getFlag(int(flagIndex)).unpack(msg);
      world->initFlag(int(flagIndex));
    }
}


void handleTeamUpdate(void *msg, bool &checkScores)
{
  uint8_t  numTeams;
  uint16_t team;

  msg = nboUnpackUInt8(msg, numTeams);
  for (int i = 0; i < numTeams; i++) {
    msg = nboUnpackUInt16(msg, team);
    msg = teams[int(team)].unpack(msg);
  }
  updateNumPlayers();
  checkScores = true;
}


void handleAllow(void *msg)
{
  PlayerId id;
  LocalPlayer *localtank = NULL;
  msg = nboUnpackUInt8(msg, id);

  uint8_t allow;
  nboUnpackUInt8(msg, allow);

  Player *tank = NULL;
  if (myTank && myTank->getId() == id) {
    tank = localtank = myTank;
  } else {
#ifdef ROBOT
    for (int i = 0; i < MAX_ROBOTS; i++) {
      if (robots[i] && robots[i]->getId() == id) {
	tank = localtank = robots[i];
      }
    }
#endif
    if (!tank)
      tank = lookupPlayer(id);

  }
  if (!tank) return;

  if (localtank) {
    localtank->setDesiredSpeed(0.0);
    localtank->setDesiredAngVel(0.0);
    // drop any team flag we may have, as would happen if we paused
    const FlagType *flagd = localtank->getFlag();
    if (flagd->flagTeam != NoTeam) {
      serverLink->sendDropFlag(localtank->getPosition());
      localtank->setShotType(StandardShot);
    }
  }

  // FIXME - this is currently too noisy
  tank->setAllow(allow);
  addMessage(tank, allow & AllowShoot ? "Shooting allowed" : "Shooting forbidden");
  addMessage(tank, allow & (AllowMoveForward  |
                            AllowMoveBackward |
                            AllowTurnLeft     |
                            AllowTurnRight) ? "Movement allowed"
                                            : "Movement restricted");
  addMessage(tank, allow & AllowJump ? "Jumping allowed" : "Jumping forbidden");
}


void handleDropFlag(void *msg)
{
  PlayerId id;
  uint16_t flagIndex;

  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, flagIndex);
  if (flagIndex >= world->getMaxFlags()) {
    return;
  }
  Flag& flag = world->getFlag(int(flagIndex));
  msg = flag.unpack(msg);

  Player *tank = lookupPlayer(id);
  if (!tank)
    return;

  handleFlagDropped(tank);
}


void handleShotEnd(void *msg)
{
  PlayerId id;
  int16_t shotId;
  uint16_t reason;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackInt16(msg, shotId);
  msg = nboUnpackUInt16(msg, reason);
  BaseLocalPlayer *localPlayer = getLocalPlayer(id);

  if (localPlayer) {
    localPlayer->endShot(int(shotId), false, reason == 0);
  } else {
    Player *pl = lookupPlayer(id);
    if (pl)
      pl->endShot(int(shotId), false, reason == 0);
  }
}


void handleHandicap(void *msg)
{
  PlayerId id;
  uint8_t numHandicaps;
  int16_t handicap;
  msg = nboUnpackUInt8(msg, numHandicaps);
  for (uint8_t s = 0; s < numHandicaps; s++) {
    msg = nboUnpackUInt8(msg, id);
    msg = nboUnpackInt16(msg, handicap);
    Player *sPlayer = NULL;
    if (id == myTank->getId()) {
      sPlayer = myTank;
    } else {
      int i = lookupPlayerIndex(id);
      if (i >= 0)
	sPlayer = getPlayerByIndex(i);
      else
	logDebugMessage(1, "Received handicap update for unknown player!\n");
    }
    if (sPlayer) {
      // a relative score of -50 points will provide maximum handicap
      float normalizedHandicap = float(handicap)
	/ BZDB.eval(StateDatabase::BZDB_HANDICAPSCOREDIFF);

      /* limit how much of a handicap is afforded, and only provide
      * handicap advantages instead of disadvantages.
      */
      if (normalizedHandicap > 1.0f)
	// advantage
	normalizedHandicap  = 1.0f;
      else if (normalizedHandicap < 0.0f)
	// disadvantage
	normalizedHandicap  = 0.0f;

      sPlayer->setHandicap(normalizedHandicap);
    }
  }
}


void handleReplayReset(void *msg, bool &checkScores)
{
  int i;
  unsigned char lastPlayer;
  msg = nboUnpackUInt8(msg, lastPlayer);

  // remove players up to 'lastPlayer'
  // any PlayerId above lastPlayer is a replay observers
  for (i = 0 ; i < lastPlayer ; i++)
    if (removePlayer (i))
      checkScores = true;

  // remove all of the flags
  for (i = 0 ; i < numFlags; i++) {
    Flag &flag = world->getFlag(i);
    flag.owner = (PlayerId) -1;
    flag.status = FlagNoExist;
    world->initFlag(i);
  }
}


void handleAdminInfo(void *msg)
{
  uint8_t numIPs;
  msg = nboUnpackUInt8(msg, numIPs);

  /* if we're getting this, we have playerlist perm */
  myTank->setPlayerList(true);

  // replacement for the normal MsgAddPlayer message
  if (numIPs == 1) {
    uint8_t ipsize;
    uint8_t index;
    Address ip;
    void *tmpMsg = msg; // leave 'msg' pointing at the first entry

    tmpMsg = nboUnpackUInt8(tmpMsg, ipsize);
    tmpMsg = nboUnpackUInt8(tmpMsg, index);
    tmpMsg = ip.unpack(tmpMsg);
    int playerIndex = lookupPlayerIndex(index);
    Player *tank = getPlayerByIndex(playerIndex);
    if (!tank)
      return;

    std::string name(tank->getCallSign());
    std::string message("joining as ");
    if (tank->getTeam() == ObserverTeam) {
      message += "an observer";
    } else {
      switch (tank->getPlayerType()) {
case TankPlayer:
  message += "a tank";
  break;
case ComputerPlayer:
  message += "a robot tank";
  break;
default:
  message += "an unknown type";
  break;
      }
    }
    message += " from " + ip.getDotNotation();
    tank->setIpAddress(ip);
    addMessage(tank, message);
  }

  // print fancy version to be easily found
  if ((numIPs != 1) || (BZDB.evalInt("showips") > 0)) {
    uint8_t playerId;
    uint8_t addrlen;
    Address addr;

    for (int i = 0; i < numIPs; i++) {
      msg = nboUnpackUInt8(msg, addrlen);
      msg = nboUnpackUInt8(msg, playerId);
      msg = addr.unpack(msg);

      int playerIndex = lookupPlayerIndex(playerId);
      Player *player = getPlayerByIndex(playerIndex);
      if (!player) continue;
      printIpInfo(player, addr, "(join)");
      player->setIpAddress(addr); // save for the signoff message
    } // end for loop
  }
}


void handlePlayerInfo(void *msg)
{
  uint8_t numPlayers;
  int i;
  msg = nboUnpackUInt8(msg, numPlayers);
  for (i = 0; i < numPlayers; ++i) {
    PlayerId id;
    msg = nboUnpackUInt8(msg, id);
    Player *p = lookupPlayer(id);
    uint8_t info;
    // parse player info bitfield
    msg = nboUnpackUInt8(msg, info);
    if (!p)
      continue;
    p->setAdmin((info & IsAdmin) != 0);
    p->setRegistered((info & IsRegistered) != 0);
    p->setVerified((info & IsVerified) != 0);
  }
}


void handleNewPlayer(void *msg)
{
  uint8_t id;
  msg = nboUnpackUInt8(msg, id);
#ifdef ROBOT
  int i;
  for (i = 0; i < MAX_ROBOTS; i++)
    if (!robots[i])
      break;
  if (i >= MAX_ROBOTS) {
    logDebugMessage(1, "Too many bots requested\n");
    return;
  }
  robots[i] = new RobotPlayer(id,
    TextUtils::format("%s%2.2d", myTank->getCallSign(),
    i).c_str(),
    serverLink);
  robots[i]->setTeam(AutomaticTeam);
  serverLink->sendEnter(id, ComputerPlayer, NoUpdates, robots[i]->getTeam(),
    robots[i]->getCallSign(), "", "");
  if (!numRobots) {
    makeObstacleList();
    RobotPlayer::setObstacleList(&obstacleList);
  }
  numRobots++;
#endif
}


void handleMovementUpdate(uint16_t code, void *msg)
{
  double timestamp;
  PlayerId id;
  int32_t order;
  void *buf = msg;

  buf = nboUnpackUInt8(buf, id); // peek! don't update the msg pointer
  buf = nboUnpackDouble(buf, timestamp); // peek

  Player *tank = lookupPlayer(id);
  if (!tank || tank == myTank)
    return;

  nboUnpackInt32(buf, order); // peek
  if (order <= tank->getOrder())
    return;
  short oldStatus = tank->getStatus();

  tank->unpack(msg, code); // now read
  short newStatus = tank->getStatus();

  if ((oldStatus & short(PlayerState::Paused)) != (newStatus & short(PlayerState::Paused)))
    addMessage(tank, (tank->getStatus() & PlayerState::Paused) ? "Paused" : "Resumed");

  if ((oldStatus & short(PlayerState::Exploding)) == 0 && (newStatus & short(PlayerState::Exploding)) != 0) {
    // player has started exploding and we haven't gotten killed
    // message yet -- set explosion now, play sound later (when we
    // get killed message).  status is already !Alive so make player
    // alive again, then call setExplode to kill him.
    tank->setStatus(newStatus | short(PlayerState::Alive));
    tank->setExplode(TimeKeeper::getTick());
    // ROBOT -- play explosion now
  }
}


void handleTangUpdate(uint16_t len, void *msg)
{
  if (len >= 5) {
    unsigned int objectID = 0;
    msg = nboUnpackUInt32(msg, objectID);
    unsigned char tang = 0;
    msg = nboUnpackUInt8(msg, tang);

    ClientIntangibilityManager::instance().setWorldObjectTangibility(objectID, tang);
  }
}


void handleTangReset(void)
{
  ClientIntangibilityManager::instance().resetTangibility();
}


void handleAllowSpawn(uint16_t len, void *msg)
{
  if (len >= 1) {
    unsigned char allow = 0;
    msg = nboUnpackUInt8(msg, allow);

    canSpawn = allow != 0;
  }
}


void handlePlayerData(void *msg)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

  std::string key, value;
  msg = nboUnpackStdString(msg, key);
  msg = nboUnpackStdString(msg, value);

  Player *p = lookupPlayer(id);
  if (p && key.size())
    p->customData[key] = value;
}


bool handleServerMessage(bool /*human*/, BufferedNetworkMessage *msg)
{
  switch (msg->getCode()) {
default:
  return false;

case MsgSetShot:
  handleSetShotType(msg);
  break;
  }
  return true;
}


void handleServerMessage(bool human, uint16_t code, uint16_t len, void *msg)
{
  std::vector<std::string> args;
  bool checkScores = false;

  switch (code) {
    case MsgWhatTimeIsIt: {
      handleWhatTimeIsIt(msg);
      break;
    }
    case MsgNearFlag: {
      handleNearFlag(msg);
      break;
    }
    case MsgSetTeam: {
      handleSetTeam(msg, len);
      break;
    }
    case MsgFetchResources: {
      handleResourceFetch(msg);
      break;
    }
    case MsgCustomSound: {
      handleCustomSound(msg);
      break;
    }
    case MsgUDPLinkEstablished: {
      serverLink->enableOutboundUDP();  // server got our initial UDP packet
      break;
    }
    case MsgUDPLinkRequest: {
      serverLink->confirmIncomingUDP();  // we got server's initial UDP packet
      break;
    }
    case MsgJoinServer: {
      handleJoinServer(msg);
      break;
    }
    case MsgSuperKill: {
      handleSuperKill(msg);
      break;
    }
    case MsgAccept: {
      break;
    }
    case MsgReject: {
      handleRejectMessage(msg);
      break;
    }
    case MsgNegotiateFlags: {
      handleFlagNegotiation(msg, len);
      break;
    }
    case MsgFlagType: {
      handleFlagType(msg);
      break;
    }
    case MsgGameSettings: {
      handleGameSettings(msg);
      break;
    }
    case MsgCacheURL: {
      handleCacheURL(msg, len);
      break;
    }
    case MsgWantWHash: {
      handleWantHash(msg, len);
      break;
    }
    case MsgGetWorld: {
      handleGetWorld(msg, len);
      break;
    }
    case MsgGameTime: {
      handleGameTime(msg);
      break;
    }
    case MsgTimeUpdate: {
      handleTimeUpdate(msg);
      break;
    }
    case MsgScoreOver: {
      handleScoreOver(msg);
      break;
    }
    case MsgAddPlayer: {
      handleAddPlayer(msg, checkScores);
      break;
    }
    case MsgRemovePlayer: {
      handleRemovePlayer(msg, checkScores);
      break;
    }
    case MsgFlagUpdate: {
      handleFlagUpdate(msg, len);
      break;
    }
    case MsgTeamUpdate: {
      handleTeamUpdate(msg, checkScores);
      break;
    }
    case MsgAlive: {
      handleAliveMessage(msg);
      break;
    }
    case MsgAutoPilot: {
      handleAutoPilot(msg);
      break;
    }
    case MsgAllow: {
      handleAllow(msg);
      break;
    }
    case MsgKilled: {
      handleKilledMessage(msg, human, checkScores);
      break;
    }
    case MsgGrabFlag: {
      handleGrabFlag(msg);
      break;
    }
    case MsgDropFlag: {
      handleDropFlag(msg);
      break;
    }
    case MsgCaptureFlag: {
      handleCaptureFlag(msg, checkScores);
      break;
    }
    case MsgNewRabbit: {
      handleNewRabbit(msg);
      break;
    }
    case MsgShotBegin: {
      handleShotBegin(human, msg);
      break;
    }
    case MsgWShotBegin: {
      handleWShotBegin(msg);
      break;
    }
    case MsgShotEnd: {
      handleShotEnd(msg);
      break;
    }
    case MsgHandicap: {
      handleHandicap(msg);
      break;
    }
    case MsgScore: {
      handleScore(msg);
      break;
    }
    case MsgSetVar: {
      handleMsgSetVars(msg);
      break;
    }
    case MsgTeleport: {
      handleTeleport(msg);
      break;
    }
    case MsgTransferFlag: {
      handleTransferFlag(msg);
      break;
    }
    case MsgMessage: {
      handleMessage(msg);
      break;
    }
    case MsgReplayReset: {
      handleReplayReset(msg, checkScores);
      break;
    }
    case MsgAdminInfo: {
      handleAdminInfo(msg);
      break;
    }
    case MsgPlayerInfo: {
      handlePlayerInfo(msg);
      break;
    }
    case MsgNewPlayer: {
      handleNewPlayer(msg);
      break;
    }
    // inter-player relayed message
    case MsgPlayerUpdate:
    case MsgPlayerUpdateSmall: {
      handleMovementUpdate(code, msg);
      break;
    }
    case MsgGMUpdate: {
      handleGMUpdate(msg);
      break;
    }
    case MsgLagPing: {
      serverLink->sendLagPing((char *)msg);
      break;
    }
    case MsgTangibilityUpdate: {
      handleTangUpdate(len, msg);
      break;
    }
    case MsgTangibilityReset: {
      handleTangReset();
      break;
    }
    case MsgAllowSpawn: {
      handleAllowSpawn(len, msg);
      break;
    }
    case MsgLimboMessage: {
      handleLimboMessage(msg);
      break;
    }
    case MsgPlayerData: {
      handlePlayerData(msg);
      break;
    }
    case MsgPause: {
      printf("MsgPause(FIXME) %s:%i\n", __FILE__, __LINE__);
      break;
    }
  }

  if (checkScores) {
    updateHighScores();
  }
}


//
// message handling
//

class ClientReceiveCallback : public NetworkMessageTransferCallback
{
  public:
    virtual size_t receive(BufferedNetworkMessage *message)
    {
      if (!serverLink || serverError) {
        return 0;
      }

      int e = 0;

      e = serverLink->read(message, 0);

      if (e == -2) {
        showError("Server communication error");
        serverError = true;
        return 0;
      }

      if (e == 1) {
        return message->size() + 4;
      }

      return 0;
    }
};


void doMessages()
{
  static ClientReceiveCallback clientMessageCallback;

  BufferedNetworkMessageManager::MessageList messageList;

  MSGMGR.update();
  MSGMGR.receiveMessages(&clientMessageCallback, messageList);

  BufferedNetworkMessageManager::MessageList::iterator itr = messageList.begin();

  while (itr != messageList.end()) {
    if (!handleServerMessage(true, *itr))
      handleServerMessage(true, (*itr)->getCode(), (uint16_t)(*itr)->size(), (*itr)->buffer());

    delete *itr;
    itr++;
  }
}


void addMessage(const Player *player, const std::string &msg,
		ControlPanel::MessageModes mode, bool highlight,
		const char *oldColor)
{
  std::string prefix;
  const char *message;

  if (BZDB.isTrue("colorful")) {
    if (player) {
      if (highlight) {
	if (BZDB.get("killerhighlight") == "1")
	  prefix += ColorStrings[PulsatingColor];
	else if (BZDB.get("killerhighlight") == "2")
	  prefix += ColorStrings[UnderlineColor];
      }
      const PlayerId pid = player->getId();
      if (pid < 200) {
	int color = player->getTeam();
	if (color < 0 || (color > 4 && color != HunterTeam))
	  // non-teamed, rabbit are white (same as observer)
	  color = WhiteColor;

	prefix += ColorStrings[color];
      } else if (pid == ServerPlayer) {
	prefix += ColorStrings[YellowColor];
      } else {
	prefix += ColorStrings[CyanColor]; //replay observers
      }
      prefix += player->getCallSign();

      if (highlight)
	prefix += ColorStrings[ResetColor];
#ifdef BWSUPPORT
      prefix += " (";
      prefix += Team::getName(player->getTeam());
      prefix += ")";
#endif
      prefix += std::string(ColorStrings[DefaultColor]) + ": ";
    }
    message = msg.c_str();
  } else {
    if (oldColor != NULL)
      prefix = oldColor;

    if (player) {
      prefix += player->getCallSign();

#ifdef BWSUPPORT
      prefix += " (";
      prefix += Team::getName(player->getTeam());
      prefix += ")";
#endif
      prefix += ": ";
    }
    message = stripAnsiCodes(msg.c_str());
  }
  const std::string msgf = TextUtils::format("%s%s", prefix.c_str(), message);
  showMessage(msgf, mode);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
