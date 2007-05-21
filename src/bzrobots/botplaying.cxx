
/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
#include <cmath>

// common headers
#include "AccessList.h"
#include "AnsiCodes.h"
#include "AresHandler.h"
#include "AutoHunt.h"
#include "BaseBuilding.h"
#include "BZDBCache.h"
#include "BzfMedia.h"
#include "bzsignal.h"
#include "CacheManager.h"
#include "CommandsStandard.h"
#include "DirectoryNames.h"
#include "ErrorHandler.h"
#include "FileManager.h"
#include "GameTime.h"
#include "KeyManager.h"
#include "md5.h"
#include "ObstacleList.h"
#include "ObstacleMgr.h"
#include "PhysicsDriver.h"
#include "PlatformFactory.h"
#include "ServerList.h"
#include "TextUtils.h"
#include "TimeBomb.h"
#include "version.h"
#include "WordFilter.h"

// local implementation headers
#include "RCLink.h"
#include "AutoPilot.h"
#include "bzflag.h"
#include "commands.h"
#include "daylight.h"
#include "FlashClock.h"
#include "ForceFeedback.h"
#include "HUDRenderer.h"
#include "LocalPlayer.h"
#include "motd.h"
#include "Roaming.h"
#include "RobotPlayer.h"
#include "Roster.h"
#include "ShotStats.h"
#include "RCRobotPlayer.h"
#include "World.h"
#include "WorldBuilder.h"
#include "SyncClock.h"

//#include "messages.h"

bool			headless = true;
static const float	FlagHelpDuration = 60.0f;
StartupInfo	startupInfo;
ServerLink*		serverLink = NULL;
static World	   *world = NULL;
LocalPlayer*		observerTank = NULL;
RCLink*		rcLink = NULL;
static Team*		teams = NULL;
int			numFlags = 0;
static bool		joinRequested = false;
static bool		waitingDNS    = false;
static bool		serverError = false;
static bool		serverDied = false;
static double		epochOffset;
static double		lastEpochOffset;
static std::vector<PlayingCallbackItem> playingCallbacks;
bool			gameOver = false;
static FlashClock		pulse;
static bool		justJoined = false;

float			roamDZoom = 0.0f;

static MessageOfTheDay		*motd = NULL;
DefaultCompleter	completer;

PlayerId		msgDestination;

static void		setTankFlags();
static void*		handleMsgSetVars(void *msg);
static void		handlePlayerMessage(uint16_t, uint16_t, void*);
static void		handleFlagTransferred(Player* fromTank, Player* toTank, int flagIndex);
static void		enteringServer(void *buf);
static void		joinInternetGame2();
static void		cleanWorldCache();
static void		markOld(std::string &fileName);
static void		setRobotTarget(RobotPlayer* robot);

static bool		gotBlowedUp(BaseLocalPlayer* tank,
				    BlowedUpReason reason,
				    PlayerId killer,
				    const ShotPath *hit = NULL,
				    int physicsDriver = -1);

static double		userTimeEpochOffset;

static bool		entered = false;
static bool		joiningGame = false;
static WorldBuilder	*worldBuilder = NULL;
static std::string	worldUrl;
static std::string	worldCachePath;
static std::string	md5Digest;
static uint32_t		worldPtr = 0;
static char		*worldDatabase = NULL;
static bool		isCacheTemp;
static std::ostream	*cacheOut = NULL;

static AresHandler      ares;

static AccessList	ServerAccessList("ServerAccess.txt", NULL);

ThirdPersonVars thirdPersonVars;

static void makeObstacleList();
static std::vector<BzfRegion*>	obstacleList;  // for robots


// Unused but linked to globally
HUDRenderer*		hud = NULL;
ControlPanel*		controlPanel = NULL;
MainWindow*		mainWindow = NULL;
float			clockAdjust = 0.0f;
bool			fireButton = false;
bool			roamButton = false;
float			pauseCountdown = 0.0f;
float			destructCountdown = 0.0f;
int			savedVolume = -1;
bool			pausedByUnmap = false;


//
// Unused Functions: included for linking only
//

BzfDisplay*		getDisplay()
{
  return NULL;
}

MainWindow*		getMainWindow()
{
  return NULL;
}

SceneRenderer*		getSceneRenderer()
{
  return NULL;
}

bool			setVideoFormat(int, bool)
{
  return false;
}

void			addShotExplosion(const float*)
{
}

void			addShotPuff(const float*, float, float)
{
}

void selectNextRecipient (bool, bool)
{
}

void warnAboutRadar()
{
}

void warnAboutConsole()
{
}

void warnAboutMainFlags()
{
}

void warnAboutRadarFlags()
{
}

void			notifyBzfKeyMapChanged()
{
}

void dumpResources()
{
}

void setTarget()
{
}

void drawFrame(float)
{
}

bool		shouldGrabMouse()
{
  return false;
}

void			setSceneDatabase()
{
}


void printout(const std::string& line)
{
#ifdef _WIN32
  FILE *fp = fopen ("stdout.txt", "a+");
  if (fp) {
    fprintf(fp,"%s\n", (stripAnsiCodes(line).c_str()));
    fclose(fp);
  }
#else
  if (echoAnsi) {
    std::cout << line << ColorStrings[ResetColor] << std::endl;
  } else {
    std::cout << stripAnsiCodes(line) << std::endl;
  }
  fflush(stdout);
#endif
}

// access silencePlayers from bzflag.cxx
std::vector<std::string>& getSilenceList()
{
  return silencePlayers;
}

StartupInfo*		getStartupInfo()
{
  return &startupInfo;
}


void			addPlayingCallback(PlayingCallback cb, void* data)
{
  PlayingCallbackItem item;
  item.cb = cb;
  item.data = data;
  playingCallbacks.push_back(item);
}

void			removePlayingCallback(PlayingCallback _cb, void* data)
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

static void		callPlayingCallbacks()
{
  const size_t count = playingCallbacks.size();
  for (size_t i = 0; i < count; i++) {
    const PlayingCallbackItem& cb = playingCallbacks[i];
    (*cb.cb)(cb.data);
  }
}

void			joinGame()
{
  if (joiningGame) {
    if (worldBuilder) {
      delete worldBuilder;
      worldBuilder = NULL;
    }
    if (worldDatabase) {
      delete[] worldDatabase;
      worldDatabase = NULL;
    }
    printError("Download stopped by user action");
    joiningGame = false;
  }
  joinRequested = true;
}

//
// handle signals that should kill me quickly
//

static void		dying(int sig)
{
  bzSignal(sig, SIG_DFL);
  raise(sig);
}

//
// handle signals that should kill me nicely
//

static void		suicide(int sig)
{
  bzSignal(sig, SIG_PF(suicide));
  CommandsStandard::quit();
}

//
// handle signals that should disconnect me from the server
//

static void		hangup(int sig)
{
  bzSignal(sig, SIG_PF(hangup));
  serverDied = true;
  serverError = true;
}


void		addMessage(const Player *_player, const std::string& msg,
			   int, bool highlight, const char* oldColor)
{
  std::string fullMessage;

  if (BZDB.isTrue("colorful")) {
    if (_player) {
      if (highlight) {
	if (BZDB.get("killerhighlight") == "1")
	  fullMessage += ColorStrings[PulsatingColor];
	else if (BZDB.get("killerhighlight") == "2")
	  fullMessage += ColorStrings[UnderlineColor];
      }
      const PlayerId pid = _player->getId();
      if (pid < 200) {
	int color = _player->getTeam();
	if (color < 0 || (color > 4 && color != HunterTeam)) {
	  // non-teamed, rabbit are white (same as observer)
	  color = WhiteColor;
	}
	fullMessage += ColorStrings[color];
      } else if (pid == ServerPlayer) {
	fullMessage += ColorStrings[YellowColor];
      } else {
	fullMessage += ColorStrings[CyanColor]; //replay observers
      }
      fullMessage += _player->getCallSign();

      if (highlight)
	fullMessage += ColorStrings[ResetColor];
#ifdef BWSUPPORT
      fullMessage += " (";
      fullMessage += Team::getName(_player->getTeam());
      fullMessage += ")";
#endif
      fullMessage += ColorStrings[DefaultColor] + ": ";
    }
    fullMessage += msg;
  } else {
    if (oldColor != NULL)
      fullMessage = oldColor;

    if (_player) {
      fullMessage += _player->getCallSign();

#ifdef BWSUPPORT
      fullMessage += " (";
      fullMessage += Team::getName(_player->getTeam());
      fullMessage += ")";
#endif
      fullMessage += ": ";
    }
    fullMessage += stripAnsiCodes(msg);
  }
  printout(fullMessage);
}

static void		updateNumPlayers()
{
  int i, numPlayers[NumTeams];
  for (i = 0; i < NumTeams; i++)
    numPlayers[i] = 0;
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i])
      numPlayers[player[i]->getTeam()]++;
}

static void		updateHighScores()
{
  /* check scores to see if my team and/or have the high score.  change
   * `>= bestScore' to `> bestScore' if you want to share the number
   * one spot. */
  bool anyPlayers = false;
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    if (player[i]) {
      anyPlayers = true;
      break;
    }
  if (!anyPlayers) {
    for (i = 0; i < numRobots; i++)
      if (robots[i]) {
	anyPlayers = true;
	break;
      }
  }
  if (!anyPlayers) {
    return;
  }
}


//
// server message handling
//
static Player*		addPlayer(PlayerId id, void* msg, int)
{
  uint16_t team, type, wins, losses, tks;
  char callsign[CallSignLen];
  char email[EmailLen];
  msg = nboUnpackUShort (msg, type);
  msg = nboUnpackUShort (msg, team);
  msg = nboUnpackUShort (msg, wins);
  msg = nboUnpackUShort (msg, losses);
  msg = nboUnpackUShort (msg, tks);
  msg = nboUnpackString (msg, callsign, CallSignLen);
  msg = nboUnpackString (msg, email, EmailLen);

  // Strip any ANSI color codes
  strncpy (callsign, stripAnsiCodes (std::string (callsign)).c_str (), 32);

  // id is slot, check if it's empty
  const int i = id;

  // sanity check
  if (i < 0) {
    printError (TextUtils::format ("Invalid player identification (%d)", i));
    std::
      cerr <<
      "WARNING: invalid player identification when adding player with id "
	   << i << std::endl;
    return NULL;
  }

  if (player[i]) {
    // we're not in synch with server -> help! not a good sign, but not fatal.
    printError ("Server error when adding player, player already added");
    std::cerr << "WARNING: player already exists at location with id "
	      << i << std::endl;
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
    player[i] = new RemotePlayer (id, TeamColor (team), callsign, email,
				  PlayerType (type));
    player[i]->changeScore (short (wins), short (losses), short (tks));
  }

  if (PlayerType (type) == ComputerPlayer)
    for (int j = 0; j < numRobots; j++)
      if (robots[j] && !strncmp (robots[j]->getCallSign (), callsign,
				 CallSignLen)) {
	robots[j]->setTeam (TeamColor (team));
	break;
      }

  // show the message if we don't have the playerlist
  // permission.  if * we do, MsgAdminInfo should arrive
  // with more info.
  if (!observerTank->hasPlayerList ()) {
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
    if (!player[i]) {
      std::string name (callsign);
      name += ": " + message;
      message = name;
    }
    addMessage (player[i], message);
  }
  completer.registerWord(callsign, true /* quote spaces */);

  return player[i];
}


static void printIpInfo (const Player *_player, const Address& addr,
			 const std::string note)
{
  if (_player == NULL) {
    return;
  }
  std::string colorStr;
  if (_player->getId() < 200) {
    int color = _player->getTeam();
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
  if (BZDBCache::colorful) message += colorStr;
  message += _player->getCallSign();
  if (BZDBCache::colorful) message += ColorStrings[CyanColor];
  message += "\t from: ";
  if (BZDBCache::colorful) message += colorStr;
  message += addrStr;

  message += ColorStrings[WhiteColor];
  for (int i = 0; i < (17 - (int)addrStr.size()); i++) {
    message += " ";
  }
  message += note;

  printout(message);

  return;
}


static bool removePlayer (PlayerId id)
{
  int playerIndex = lookupPlayerIndex(id);

  if (playerIndex < 0) {
    return false;
  }

  Player* p = getPlayerByIndex(playerIndex);

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

  completer.unregisterWord(p->getCallSign());

  delete player[playerIndex];
  player[playerIndex] = NULL;

  while ((playerIndex >= 0)
	 &&     (playerIndex+1 == curMaxPlayers)
	 &&     (player[playerIndex] == NULL)) {
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


static bool isCached(char *hexDigest)
{
  std::istream *cachedWorld;
  bool cached    = false;
  worldCachePath = getCacheDirName();
  worldCachePath += hexDigest;
  worldCachePath += ".bwc";
  if ((cachedWorld = FILEMGR.createDataInStream(worldCachePath, true))) {
    cached = true;
    delete cachedWorld;
  }
  return cached;
}


static void loadCachedWorld()
{
  // can't get a cache from nothing
  if (worldCachePath == std::string("")) {
    joiningGame = false;
    return;
  }

  // lookup the cached world
  std::istream *cachedWorld = FILEMGR.createDataInStream(worldCachePath, true);
  if (!cachedWorld) {
    printError("World cache files disappeared.  Join canceled");
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }

  // status update
  printout("Loading world into memory...");

  // get the world size
  cachedWorld->seekg(0, std::ios::end);
  std::streampos size = cachedWorld->tellg();
  unsigned long charSize = std::streamoff(size);

  // load the cached world
  cachedWorld->seekg(0);
  char *localWorldDatabase = new char[charSize];
  if (!localWorldDatabase) {
    printError("Error loading cached world.  Join canceled");
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }
  cachedWorld->read(localWorldDatabase, charSize);
  delete cachedWorld;

  // verify
  printout("Verifying world integrity...");
  MD5 md5;
  md5.update((unsigned char *)localWorldDatabase, charSize);
  md5.finalize();
  std::string digest = md5.hexdigest();
  if (digest != md5Digest) {
    if (worldBuilder) {
      delete worldBuilder;
      worldBuilder = NULL;
    }
    delete[] localWorldDatabase;
    printError("Error on md5. Removing offending file.");
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }

  // make world
  printout("Preparing world...");
  if (!worldBuilder->unpack(localWorldDatabase)) {
    // world didn't make for some reason
    if (worldBuilder) {
      delete worldBuilder;
      worldBuilder = NULL;
    }
    delete[] localWorldDatabase;
    printError("Error unpacking world database. Join canceled.");
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }
  delete[] localWorldDatabase;

  // return world
  if (worldBuilder) {
    world = worldBuilder->getWorld();
    delete worldBuilder;
    worldBuilder = NULL;
  }

  // Unlike playing.cxx, we aren't going to do startDownloads
  joinInternetGame2();
}

class WorldDownLoader : private cURLManager {
public:
  void	 start(char * hexDigest);
private:
  void	 askToBZFS();
  virtual void finalization(char *data, unsigned int length, bool good);
};

void WorldDownLoader::start(char * hexDigest)
{
  if (isCached(hexDigest)) {
    loadCachedWorld();
  } else if (worldUrl.size()) {
    printout(("Loading world from " + worldUrl).c_str());
    setURL(worldUrl);
    addHandle();
    worldUrl = ""; // clear the state
  } else {
    askToBZFS();
  }
}

void WorldDownLoader::finalization(char *data, unsigned int length, bool good)
{
  if (good) {
    worldDatabase = data;
    theData       = NULL;
    MD5 md5;
    md5.update((unsigned char *)worldDatabase, length);
    md5.finalize();
    std::string digest = md5.hexdigest();
    if (digest != md5Digest) {
      printError("Download from URL failed");
      askToBZFS();
    } else {
      std::ostream* cache =
	FILEMGR.createDataOutStream(worldCachePath, true, true);
      if (cache != NULL) {
	cache->write(worldDatabase, length);
	delete cache;
	loadCachedWorld();
      } else {
	printError("Problem writing cache");
	askToBZFS();
      }
    }
  } else {
    askToBZFS();
  }
}

void WorldDownLoader::askToBZFS()
{
  printout("Downloading World...");
  char message[MaxPacketLen];
  // ask for world
  nboPackUInt(message, 0);
  serverLink->send(MsgGetWorld, sizeof(uint32_t), message);
  worldPtr = 0;
  if (cacheOut)
    delete cacheOut;
  cacheOut = FILEMGR.createDataOutStream(worldCachePath, true, true);
}

static WorldDownLoader *worldDownLoader;

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

  std::vector<std::string> args;
  args.push_back(flags);
  printError(TextUtils::format("Flags not supported by this client: {1}",
		       &args).c_str());
}

static bool processWorldChunk(void *buf, uint16_t len, int bytesLeft)
{
  int totalSize = worldPtr + len + bytesLeft;
  int doneSize  = worldPtr + len;
  if (cacheOut)
    cacheOut->write((char *)buf, len);
  printError(TextUtils::format
     ("Downloading World (%2d%% complete/%d kb remaining)...",
      (100 * doneSize / totalSize), bytesLeft / 1024).c_str());
  return bytesLeft == 0;
}


static void handleSuperKill ( void *msg )
{
  uint8_t id;
  nboUnpackUByte(msg, id);
  serverError = true;
  printError("Server forced a disconnect");

  int i;
  for (i = 0; i < MAX_ROBOTS; i++)
    {
      if (robots[i] && robots[i]->getId() == id)
	break;
    }
  if (i >= MAX_ROBOTS)
    return;
  delete robots[i];
  robots[i] = NULL;
  numRobots--;
}

static void handleRejectMessage ( void *msg )
{
  void *buf;
  char buffer[MessageLen];
  uint16_t rejcode;
  std::string reason;

  buf = nboUnpackUShort (msg, rejcode); // filler for now
  buf = nboUnpackString (buf, buffer, MessageLen);
  buffer[MessageLen - 1] = '\0';
  reason = buffer;
  printError(reason);
}

static void handleFlagNegotiation ( void *msg, uint16_t len )
{
  if (len > 0)
    {
      dumpMissingFlag((char *)msg, len);
      return;
    }
  serverLink->send(MsgWantSettings, 0, NULL);
}

static void handleGameSettings ( void *msg )
{
  if (worldBuilder) {
    delete worldBuilder;
    worldBuilder = NULL;
  }
  worldBuilder = new WorldBuilder;
  worldBuilder->unpackGameSettings(msg);
  serverLink->send(MsgWantWHash, 0, NULL);
}

static void handleCacheURL ( void *msg, uint16_t len )
{
  char *cacheURL = new char[len];
  nboUnpackString(msg, cacheURL, len);
  worldUrl = cacheURL;
  delete [] cacheURL;
}

static void handleWantHash ( void* msg, uint16_t len )
{
  char *hexDigest = new char[len];
  nboUnpackString(msg, hexDigest, len);
  isCacheTemp = hexDigest[0] == 't';
  md5Digest = &hexDigest[1];

  worldDownLoader->start(hexDigest);
  delete [] hexDigest;
}

static void handleGetWorld ( void* msg, uint16_t len )
{
  // create world
  uint32_t bytesLeft;
  void *buf = nboUnpackUInt(msg, bytesLeft);
  bool last = processWorldChunk(buf, len - 4, bytesLeft);
  if (!last)
    {
      char message[MaxPacketLen];
      // ask for next chunk
      worldPtr += len - 4;
      nboPackUInt(message, worldPtr);
      serverLink->send(MsgGetWorld, sizeof(uint32_t), message);
      return;
    }
  if (cacheOut)
    delete cacheOut;
  cacheOut = NULL;
  loadCachedWorld();
  if (isCacheTemp)
    markOld(worldCachePath);
}

static void handleTimeUpdate ( void* msg, uint16_t /*len*/ )
{
  int32_t timeLeft;
  msg = nboUnpackInt(msg, timeLeft);
  hud->setTimeLeft(timeLeft);
  if (timeLeft == 0)
    {
      gameOver = true;
#ifdef ROBOT
      for (int i = 0; i < numRobots; i++)
	if (robots[i])
	  robots[i]->explodeTank();
#endif
    }
  else if (timeLeft < 0)
    hud->setAlert(0, "Game Paused", 10.0f, true);
}

static void handleScoreOver ( void *msg, uint16_t /*len*/ )
{
  // unpack packet
  PlayerId id;
  uint16_t team;
  msg = nboUnpackUByte(msg, id);
  msg = nboUnpackUShort(msg, team);
  Player* _player = lookupPlayer(id);

  // make a message
  std::string msg2;
  if (team == (uint16_t)NoTeam)
    {
      // a player won
      if (player)
	{
	  msg2 = _player->getCallSign();
	  msg2 += " (";
	  msg2 += Team::getName(_player->getTeam());
	  msg2 += ")";
	}
      else
	msg2 = "[unknown player]";
    }
  else
    msg2 = Team::getName(TeamColor(team));		// a team won

  msg2 += " won the game";

  gameOver = true;

#ifdef ROBOT
  for (int i = 0; i < numRobots; i++)
    {
      if (robots[i])
	robots[i]->explodeTank();
    }
#endif
}

static void handleAddPlayer ( void	*msg, uint16_t /*len*/, bool &checkScores )
{
  PlayerId id;
  msg = nboUnpackUByte(msg, id);

#if defined(FIXME) && defined(ROBOT)
  saveRobotInfo(id, msg);
#endif

  if (id == observerTank->getId())
    enteringServer(msg);		// it's me!  should be the end of updates
  else
    {
      addPlayer(id, msg, entered);
      updateNumPlayers();
      checkScores = true;
    }
}

static void handleRemovePlayer ( void	*msg, uint16_t /*len*/, bool &checkScores )
{
  PlayerId id;
  msg = nboUnpackUByte(msg, id);

  if (removePlayer (id))
    checkScores = true;
}

static void handleFlagUpdate ( void	*msg, uint16_t /*len*/ )
{
  uint16_t count;
  uint16_t flagIndex;
  msg = nboUnpackUShort(msg, count);
  for (int i = 0; i < count; i++)
    {
      msg = nboUnpackUShort(msg, flagIndex);
      msg = world->getFlag(int(flagIndex)).unpack(msg);
    }
}

static void handleTeamUpdate ( void	*msg, uint16_t /*len*/, bool &checkScores )
{
  uint8_t  numTeams;
  uint16_t team;

  msg = nboUnpackUByte(msg,numTeams);
  for (int i = 0; i < numTeams; i++)
    {
      msg = nboUnpackUShort(msg, team);
      msg = teams[int(team)].unpack(msg);
    }
  updateNumPlayers();
  checkScores = true;
}

static void handleAliveMessage ( void	*msg, uint16_t /*len*/ )
{
  PlayerId id;
  float pos[3], forward;

  msg = nboUnpackUByte(msg, id);
  msg = nboUnpackVector(msg, pos);
  msg = nboUnpackFloat(msg, forward);
  int playerIndex = lookupPlayerIndex(id);

  if ((playerIndex >= 0) || (playerIndex == -2))
    {
      static const float zero[3] = { 0.0f, 0.0f, 0.0f };
      Player* tank = getPlayerByIndex(playerIndex);
      if (tank->getPlayerType() == ComputerPlayer)
	{
	  for (int r = 0; r < numRobots; r++)
	    {
	      if (robots[r] && robots[r]->getId() == playerIndex)
		{
		  robots[r]->restart(pos,forward);
		  if (!rcLink)
		    {
		      setRobotTarget(robots[r]);
		    }
		  break;
		}
	    }
	}

      tank->setStatus(PlayerState::Alive);
      tank->move(pos, forward);
      tank->setVelocity(zero);
      tank->setAngularVelocity(0.0f);
      tank->setDeadReckoning((float)syncedClock.GetServerSeconds());
    }
}

static void handleAutoPilot ( void *msg, uint16_t /*len*/ )
{
  PlayerId id;
  msg = nboUnpackUByte(msg, id);

  uint8_t autopilot;
  nboUnpackUByte(msg, autopilot);

  Player* tank = lookupPlayer(id);
  if (!tank)
    return;

  tank->setAutoPilot(autopilot != 0);
  addMessage(tank, autopilot ? "Roger taking controls" : "Roger releasing controls");
}

static void handleAllow ( void *msg, uint16_t /*len*/ )
{
  PlayerId id;
  LocalPlayer *localtank = NULL;
  msg = nboUnpackUByte(msg, id);

  uint8_t allowMovement;
  nboUnpackUByte(msg, allowMovement);

  uint8_t allowShooting;
  nboUnpackUByte(msg, allowShooting);

  Player* tank = NULL;
  for (int i = 0; i < MAX_ROBOTS; i++) {
    if (robots[i] && robots[i]->getId() == id) {
      tank = localtank = robots[i];
    }
  }
  if (!tank) {
    tank = lookupPlayer(id);
  }
  if (!tank) return;

  if (localtank) {
    localtank->setDesiredSpeed(0.0);
    localtank->setDesiredAngVel(0.0);
    // drop any team flag we may have, as would happen if we paused
    const FlagType* flagd = localtank->getFlag();
    if (flagd->flagTeam != NoTeam)
      serverLink->sendDropFlag(localtank->getPosition());
  }

  tank->setAllowMovement(allowMovement != 0);
  tank->setAllowShooting(allowShooting != 0);
  addMessage(tank, allowMovement ? "Movement allowed" : "Movement forbidden");
  addMessage(tank, allowShooting ? "Shooting allowed" : "Shooting forbidden");
}

static void handleKilledMessage ( void *msg, uint16_t /*len*/, bool, bool &checkScores )
{
  PlayerId victim, killer;
  FlagType* flagType;
  int16_t shotId, reason;
  int phydrv = -1;
  msg = nboUnpackUByte(msg, victim);
  msg = nboUnpackUByte(msg, killer);
  msg = nboUnpackShort(msg, reason);
  msg = nboUnpackShort(msg, shotId);
  msg = FlagType::unpack(msg, flagType);
  if (reason == (int16_t)PhysicsDriverDeath)
    {
      int32_t inPhyDrv;
      msg = nboUnpackInt(msg, inPhyDrv);
      phydrv = int(inPhyDrv);
    }
  BaseLocalPlayer* victimLocal = getLocalPlayer(victim);
  BaseLocalPlayer* killerLocal = getLocalPlayer(killer);
  Player* victimPlayer = lookupPlayer(victim);
  Player* killerPlayer = lookupPlayer(killer);
  if (victimLocal)
    {
      // uh oh, local player is dead
      if (victimLocal->isAlive())
	gotBlowedUp(victimLocal, GotKilledMsg, killer);
    }
  else if (victimPlayer)
    {
      victimPlayer->setExplode(TimeKeeper::getTick());
    }

  if (killerLocal)
    {
      // local player did it
      if (shotId >= 0)
	killerLocal->endShot(shotId, true);				// terminate the shot
    }

#ifdef ROBOT
  // blow up robots on victim's team if shot was genocide
  if (killerPlayer && victimPlayer && shotId >= 0)
    {
      const ShotPath* shot = killerPlayer->getShot(int(shotId));
      if (shot && shot->getFlag() == Flags::Genocide)
	{
	  for (int i = 0; i < numRobots; i++)
	    {
	      if (robots[i] && victimPlayer != robots[i] && victimPlayer->getTeam() == robots[i]->getTeam() && robots[i]->getTeam() != RogueTeam)
		gotBlowedUp(robots[i], GenocideEffect, killerPlayer->getId());
	    }
	}
    }
#endif

  checkScores = true;
}

static void handleGrabFlag ( void *msg, uint16_t /*len*/ )
{
  PlayerId id;
  uint16_t flagIndex;

  msg = nboUnpackUByte(msg, id);
  msg = nboUnpackUShort(msg, flagIndex);
  msg = world->getFlag(int(flagIndex)).unpack(msg);

  Player* tank = lookupPlayer(id);
  if (!tank)
    return;

  // player now has flag
  tank->setFlag(world->getFlag(flagIndex).type);

  std::string message("grabbed ");
  message += tank->getFlag()->flagName;
  message += " flag";

  addMessage(tank, message);
}

static void handleDropFlag ( void *msg, uint16_t /*len*/)
{
  PlayerId id;
  uint16_t flagIndex;

  msg = nboUnpackUByte(msg, id);
  msg = nboUnpackUShort(msg, flagIndex);
  msg = world->getFlag(int(flagIndex)).unpack(msg);

  Player* tank = lookupPlayer(id);
  if (!tank)
    return;

  handleFlagDropped(tank);
}

static void handleCaptureFlag ( void *msg, uint16_t /*len*/, bool &checkScores )
{
  PlayerId id;
  uint16_t flagIndex, team;
  msg = nboUnpackUByte(msg, id);
  msg = nboUnpackUShort(msg, flagIndex);
  msg = nboUnpackUShort(msg, team);
  Player* capturer = lookupPlayer(id);

  if (flagIndex >= world->getMaxFlags())
    return;

  Flag capturedFlag = world->getFlag(int(flagIndex));

  if (capturedFlag.type == Flags::Null)
    return;

  int capturedTeam = capturedFlag.type->flagTeam;

  // player no longer has flag
  if (capturer)
    {
      capturer->setFlag(Flags::Null);

      // add message
      if (int(capturer->getTeam()) == capturedTeam)
	{
	  std::string message("took my flag into ");
	  message += Team::getName(TeamColor(team));
	  message += " territory";
	  addMessage(capturer, message);
	}
      else
	{
	  std::string message("captured ");
	  message += Team::getName(TeamColor(capturedTeam));
	  message += "'s flag";
	  addMessage(capturer, message);
	}
    }

#ifdef ROBOT
  //kill all my robots if they are on the captured team
  for (int r = 0; r < numRobots; r++)
    {
      if (robots[r] && robots[r]->getTeam() == capturedTeam)
	gotBlowedUp(robots[r], GotCaptured, robots[r]->getId());
    }
#endif

  checkScores = true;
}

static void handleNewRabbit ( void *msg, uint16_t /*len*/ )
{
  PlayerId id;
  msg = nboUnpackUByte(msg, id);
  Player *rabbit = lookupPlayer(id);

  // new mode option,
  unsigned char mode;
  msg = nboUnpackUByte(msg, mode);

  // mode 0 == swap the current rabbit with this rabbit
  // mode 1 == add this person as a rabbit
  // mode 2 == remove this person from the rabbit list

  if (mode == 0)	// we don't need to mod the hunters if we aren't swaping
    {
      for (int i = 0; i < curMaxPlayers; i++)
	{
	  if (player[i])
	    player[i]->setHunted(false);
	  if (i != id && player[i] && player[i]->getTeam() != RogueTeam && player[i]->getTeam() != ObserverTeam)
	    player[i]->changeTeam(HunterTeam);
	}
    }

  if (rabbit != NULL)
    {
      if (mode != 2)
	{
	  rabbit->changeTeam(RabbitTeam);

	  if (mode == 0)
	    addMessage(rabbit, "is now the rabbit", 3, true);
	  else
	    addMessage(rabbit, "is now a rabbit", 3, true);
	}
      else
	{
	  rabbit->changeTeam(HunterTeam);
	  addMessage(rabbit, "is no longer a rabbit", 3, true);
	}
    }

#ifdef ROBOT
  for (int r = 0; r < numRobots; r++)
    {
      if (robots[r])
	{
	  if (robots[r]->getId() == id)
	    robots[r]->changeTeam(RabbitTeam);
	  else
	    robots[r]->changeTeam(HunterTeam);
	}
    }
#endif
}

static void handleSetTeam ( void *msg, uint16_t len )
{
  if ( len < 2 )
    return;

  PlayerId id;
  msg = nboUnpackUByte(msg, id);

  uint8_t team;
  msg = nboUnpackUByte(msg, team);

  Player *p = lookupPlayer(id);

  p->changeTeam((TeamColor)team);
}
static void		handleServerMessage(bool human, uint16_t code,
					    uint16_t len, void* msg)
{
  std::vector<std::string> args;
  bool checkScores = false;

  switch (code)
  {
      case MsgNearFlag:
	//handleNearFlag(msg,len);
	break;

      case MsgSetTeam:
	handleSetTeam(msg,len);
	break;

      case MsgFetchResources:
	//handleResourceFetch(msg);
	break;

      case MsgCustomSound:
	//handleCustomSound(msg);
	break;

      case MsgUDPLinkEstablished:
	serverLink->enableOutboundUDP();      // server got our initial UDP packet
	break;

      case MsgUDPLinkRequest:
	serverLink->confirmIncomingUDP();      // we got server's initial UDP packet
	break;

      case MsgSuperKill:
	handleSuperKill(msg);
	break;

      case MsgAccept:
	break;

      case MsgReject:
	handleRejectMessage(msg);
	break;

      case MsgNegotiateFlags:
	handleFlagNegotiation(msg,len);
	break;

      case MsgGameSettings:
	handleGameSettings(msg);
	break;

      case MsgCacheURL:
	handleCacheURL(msg,len);
	break;

      case MsgWantWHash:
	handleWantHash(msg,len);
	break;

      case MsgGetWorld:
	handleGetWorld(msg,len);
	break;

      case MsgGameTime:
	GameTime::unpack(msg);
	GameTime::update();
	break;

      case MsgTimeUpdate:
	handleTimeUpdate(msg,len);
	break;

      case MsgScoreOver:
	handleScoreOver(msg,len);
	break;

      case MsgAddPlayer:
	handleAddPlayer(msg,len,checkScores);
	break;

      case MsgRemovePlayer:
	handleRemovePlayer(msg,len,checkScores);
	break;

      case MsgFlagUpdate:
	handleFlagUpdate(msg,len);
	break;

      case MsgTeamUpdate:
	handleTeamUpdate(msg,len,checkScores);
	break;

      case MsgAlive:
	handleAliveMessage(msg,len);
	break;;

      case MsgAutoPilot:
	handleAutoPilot(msg,len);
	break;;

      case MsgAllow:
	handleAllow(msg,len);
	break;

      case MsgKilled:
	handleKilledMessage(msg,len,human,checkScores);
	break;;

      case MsgGrabFlag:
	handleGrabFlag(msg,len);
	break;

      case MsgDropFlag:
	handleDropFlag(msg,len);
	break;

      case MsgCaptureFlag:
	handleCaptureFlag(msg,len,checkScores);
	break;

      case MsgNewRabbit:
	handleNewRabbit(msg,len);
	break;

      case MsgShotBegin: {
	FiringInfo firingInfo;

	PlayerId		shooterid;
	uint16_t		id;

	msg = nboUnpackUByte(msg, shooterid);
	msg = nboUnpackUShort(msg, id);

	firingInfo.shot.player = shooterid;
	firingInfo.shot.id     = id;

	if (shooterid >= playerSize)
	  break;

	RemotePlayer* shooter = player[shooterid];

	if (shooterid != ServerPlayer) {
	  if (shooter && player[shooterid]->getId() == shooterid) {
	    shooter->addShot(firingInfo);
	  }
	}
	break;
      }

      case MsgShotEnd: {
	PlayerId id;
	int16_t shotId;
	uint16_t reason;
	msg = nboUnpackUByte(msg, id);
	msg = nboUnpackShort(msg, shotId);
	msg = nboUnpackUShort(msg, reason);
	BaseLocalPlayer* localPlayer = getLocalPlayer(id);

	if (localPlayer)
	  localPlayer->endShot(int(shotId), false, reason == 0);
	else {
	  Player *pl = lookupPlayer(id);
	  if (pl)
	    pl->endShot(int(shotId), false, reason == 0);
	}
	break;
      }

      case MsgHandicap: {
	PlayerId id;
	uint8_t numHandicaps;
	int16_t handicap;
	msg = nboUnpackUByte(msg, numHandicaps);
	for (uint8_t s = 0; s < numHandicaps; s++) {
	  msg = nboUnpackUByte(msg, id);
	  msg = nboUnpackShort(msg, handicap);
	  Player *sPlayer = NULL;
	  int i = lookupPlayerIndex(id);
	  if (i >= 0)
	    sPlayer = getPlayerByIndex(i);
	  else
	    logDebugMessage(1, "Received handicap update for unknown player!\n");
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
      case MsgScore: {
	uint8_t numScores;
	PlayerId id;
	uint16_t wins, losses, tks;
	msg = nboUnpackUByte(msg, numScores);

	for (uint8_t s = 0; s < numScores; s++) {
	  msg = nboUnpackUByte(msg, id);
	  msg = nboUnpackUShort(msg, wins);
	  msg = nboUnpackUShort(msg, losses);
	  msg = nboUnpackUShort(msg, tks);

	  Player *sPlayer = NULL;
	  int i = lookupPlayerIndex(id);
	  if (i >= 0)
	    sPlayer = getPlayerByIndex(i);
	  else
	    logDebugMessage(1, "Recieved score update for unknown player!\n");
	  if (sPlayer)
	    sPlayer->changeScore(wins, losses, tks);
	}
	break;
      }

      case MsgSetVar: {
	msg = handleMsgSetVars(msg);
	break;
      }

      case MsgTeleport: {
	PlayerId id;
	uint16_t from, to;
	msg = nboUnpackUByte(msg, id);
	msg = nboUnpackUShort(msg, from);
	msg = nboUnpackUShort(msg, to);
	Player* tank = lookupPlayer(id);
	if (tank) {
	  tank->setTeleport(TimeKeeper::getTick(), short(from), short(to));
	}
	break;
      }

      case MsgTransferFlag:
	{
	  PlayerId fromId, toId;
	  unsigned short flagIndex;
	  msg = nboUnpackUByte(msg, fromId);
	  msg = nboUnpackUByte(msg, toId);
	  msg = nboUnpackUShort(msg, flagIndex);
	  msg = world->getFlag(int(flagIndex)).unpack(msg);
	  Player* fromTank = lookupPlayer(fromId);
	  Player* toTank = lookupPlayer(toId);
	  handleFlagTransferred( fromTank, toTank, flagIndex);
	  break;
	}


      case MsgMessage:
	break;

      case MsgReplayReset: {
	int i;
	unsigned char lastPlayer;
	msg = nboUnpackUByte(msg, lastPlayer);

	// remove players up to 'lastPlayer'
	// any PlayerId above lastPlayer is a replay observers
	for (i=0 ; i<lastPlayer ; i++) {
	  if (removePlayer (i)) {
	    checkScores = true;
	  }
	}

	// remove all of the flags
	for (i=0 ; i<numFlags; i++) {
	  Flag& flag = world->getFlag(i);
	  flag.owner = (PlayerId) -1;
	  flag.status = FlagNoExist;
	  world->initFlag (i);
	}
	break;
      }

      case MsgAdminInfo: {
	uint8_t numIPs;
	msg = nboUnpackUByte(msg, numIPs);

	/* if we're getting this, we have playerlist perm */
	observerTank->setPlayerList(true);

	// replacement for the normal MsgAddPlayer message
	if (numIPs == 1) {
	  uint8_t ipsize;
	  uint8_t index;
	  Address ip;
	  void* tmpMsg = msg; // leave 'msg' pointing at the first entry

	  tmpMsg = nboUnpackUByte(tmpMsg, ipsize);
	  tmpMsg = nboUnpackUByte(tmpMsg, index);
	  tmpMsg = ip.unpack(tmpMsg);
	  int playerIndex = lookupPlayerIndex(index);
	  Player* tank = getPlayerByIndex(playerIndex);
	  if (!tank) {
	    break;
	  }

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
	    msg = nboUnpackUByte(msg, addrlen);
	    msg = nboUnpackUByte(msg, playerId);
	    msg = addr.unpack(msg);

	    int playerIndex = lookupPlayerIndex(playerId);
	    Player *_player = getPlayerByIndex(playerIndex);
	    if (!_player) continue;
	    printIpInfo(_player, addr, "(join)");
	    _player->setIpAddress(addr); // save for the signoff message
	  } // end for loop
	}
	break;
      }

      case MsgPlayerInfo: {
	uint8_t numPlayers;
	int i;
	msg = nboUnpackUByte(msg, numPlayers);
	for (i = 0; i < numPlayers; ++i) {
	  PlayerId id;
	  msg = nboUnpackUByte(msg, id);
	  Player *p = lookupPlayer(id);
	  uint8_t info;
	  // parse player info bitfield
	  msg = nboUnpackUByte(msg, info);
	  if (!p)
	    continue;
	  p->setAdmin((info & IsAdmin) != 0);
	  p->setRegistered((info & IsRegistered) != 0);
	  p->setVerified((info & IsVerified) != 0);
	}
	break;
      }

      case MsgNewPlayer:
	uint8_t id;
	msg = nboUnpackUByte(msg, id);
#ifdef ROBOT
	int i;
	for (i = 0; i < MAX_ROBOTS; i++)
	  if (!robots[i])
	    break;
	if (i >= MAX_ROBOTS) {
	  logDebugMessage(1, "Too much bots requested\n");
	  break;
	}
	char callsign[CallSignLen];
	snprintf(callsign, CallSignLen, "%s%2.2d", startupInfo.callsign, i);
	if (rcLink) {
	  robots[i] = new RCRobotPlayer(id, callsign, serverLink,
				      rcLink, startupInfo.email);
	  fprintf(stderr, "new tank; type: %d\n", robots[i]->getPlayerType());
	} else {
	  robots[i] = new RobotPlayer(id, callsign, serverLink,
				      startupInfo.email);
	}
	robots[i]->setTeam(startupInfo.team);
	serverLink->sendEnter(id, ComputerPlayer, robots[i]->getTeam(),
			      robots[i]->getCallSign(),
			      robots[i]->getEmailAddress(), "");
	if (!numRobots) {
	  makeObstacleList();
	  RobotPlayer::setObstacleList(&obstacleList);
	}
	numRobots++;
#endif
	break;

	// inter-player relayed message
      case MsgPlayerUpdate:
      case MsgPlayerUpdateSmall:
      case MsgGMUpdate:
      case MsgLagPing:
	handlePlayerMessage(code, 0, msg);
	break;
    }

  if (checkScores) updateHighScores();
}

//
// player message handling
//

static void		handlePlayerMessage(uint16_t code, uint16_t,
					    void* msg)
{
  switch (code) {
    case MsgPlayerUpdate:
    case MsgPlayerUpdateSmall: {
      float timestamp; // could be used to enhance deadreckoning, but isn't for now
      PlayerId id;
      int32_t order;
      void *buf = msg;
      buf = nboUnpackUByte(buf, id);
      buf = nboUnpackFloat(buf, timestamp);
      Player* tank = lookupPlayer(id);
      if (!tank || tank == observerTank) break;
      nboUnpackInt(buf, order); // peek! don't update the msg pointer
      if (order <= tank->getOrder()) break;
      short oldStatus = tank->getStatus();
      tank->unpack(msg, code);
      short newStatus = tank->getStatus();
      if ((oldStatus & short(PlayerState::Paused)) !=
	  (newStatus & short(PlayerState::Paused)))
	addMessage(tank, (tank->getStatus() & PlayerState::Paused) ?
		   "Paused" : "Resumed");
      if ((oldStatus & short(PlayerState::Exploding)) == 0 &&
	  (newStatus & short(PlayerState::Exploding)) != 0) {
	// player has started exploding and we haven't gotten killed
	// message yet -- set explosion now, play sound later (when we
	// get killed message).  status is already !Alive so make player
	// alive again, then call setExplode to kill him.
	tank->setStatus(newStatus | short(PlayerState::Alive));
	tank->setExplode(TimeKeeper::getTick());
	// ROBOT -- play explosion now
      }
      break;
    }

    case MsgGMUpdate: {
      ShotUpdate shot;
      msg = shot.unpack(msg);
      Player* tank = lookupPlayer(shot.player);
      if (!tank || tank == observerTank) break;
      RemotePlayer* remoteTank = (RemotePlayer*)tank;
      RemoteShotPath* shotPath =
	(RemoteShotPath*)remoteTank->getShot(shot.id);
      if (shotPath) shotPath->update(shot, code, msg);
      break;
    }

      // just echo lag ping message
    case MsgLagPing:
      serverLink->sendLagPing((char *)msg);
      break;
  }
}

//
// message handling
//

static void		doMessages()
{
  char msg[MaxPacketLen];
  uint16_t code, len;
  int e = 0;

  // handle server messages
  if (serverLink) {
    while (!serverError && (e = serverLink->read(code, len, msg, 0)) == 1)
      handleServerMessage(true, code, len, msg);
    if (e == -2) {
      printError("Server communication error");
      serverError = true;
      return;
    }
  }
}


static void *handleMsgSetVars(void *msg)
{
  uint16_t numVars;
  uint8_t nameLen, valueLen;

  char name[MaxPacketLen];
  char value[MaxPacketLen];

  msg = nboUnpackUShort(msg, numVars);
  for (int i = 0; i < numVars; i++) {
    msg = nboUnpackUByte(msg, nameLen);
    msg = nboUnpackString(msg, name, nameLen);
    name[nameLen] = '\0';

    msg = nboUnpackUByte(msg, valueLen);
    msg = nboUnpackString(msg, value, valueLen);
    value[valueLen] = '\0';

    BZDB.set(name, value);
    BZDB.setPersistent(name, false);
    BZDB.setPermission(name, StateDatabase::Locked);
  }
  return msg;
}

void handleFlagDropped(Player* tank)
{
  // skip it if player doesn't actually have a flag
  if (tank->getFlag() == Flags::Null) return;

  // add message
  std::string message("dropped ");
  message += tank->getFlag()->flagName;
  message += " flag";
  addMessage(tank, message);

  // player no longer has flag
  tank->setFlag(Flags::Null);
}

static void	handleFlagTransferred( Player *fromTank, Player *toTank, int flagIndex)
{
  Flag f = world->getFlag(flagIndex);

  fromTank->setFlag(Flags::Null);
  toTank->setFlag(f.type);

  std::string message(toTank->getCallSign());
  message += " stole ";
  message += fromTank->getCallSign();
  message += "'s flag";
  addMessage(toTank, message);
}

static bool		gotBlowedUp(BaseLocalPlayer* tank,
				    BlowedUpReason reason,
				    PlayerId killer,
				    const ShotPath* hit, int phydrv)
{
  if (tank && (tank->getTeam() == ObserverTeam || !tank->isAlive()))
    return false;

  int shotId = -1;
  FlagType* flagType = Flags::Null;
  if (hit) {
    shotId = hit->getShotId();
    flagType = hit->getFlag();
  }

  // you can't take it with you
  const FlagType* flag = tank->getFlag();
  if (flag != Flags::Null) {

    // tell other players I've dropped my flag
    serverLink->sendDropFlag(tank->getPosition());

    // drop it
    handleFlagDropped(tank);
  }

  // take care of explosion business -- don't want to wait for
  // round trip of killed message.  waiting would simplify things,
  // but the delay (2-3 frames usually) can really fool and irritate
  // the player.  we have to be careful to ignore our own Killed
  // message when it gets back to us -- do this by ignoring killed
  // message if we're already dead.
  // don't die if we had the shield flag and we've been shot.
  if (reason != GotShot || flag != Flags::Shield) {
    // blow me up
    tank->explodeTank();

    // tell server I'm dead in case it doesn't already know
    if (reason == GotShot || reason == GotRunOver ||
	reason == GenocideEffect || reason == SelfDestruct ||
	reason == WaterDeath || reason == PhysicsDriverDeath)
      serverLink->sendKilled(tank->getId(), killer, reason, shotId, flagType,
			     phydrv);
  }

  // make sure shot is terminated locally (if not globally) so it can't
  // hit me again if I had the shield flag.  this is important for the
  // shots that aren't stopped by a hit and so may stick around to hit
  // me on the next update, making the shield useless.
  return (reason == GotShot && flag == Flags::Shield && shotId != -1);
}


static inline bool tankHasShotType(const Player* tank, const FlagType* ft)
{
  const int maxShots = tank->getMaxShots();
  for (int i = 0; i < maxShots; i++) {
    const ShotPath* sp = tank->getShot(i);
    if ((sp != NULL) && (sp->getFlag() == ft)) {
      return true;
    }
  }
  return false;
}


//
// some robot stuff
//

static void		addObstacle(std::vector<BzfRegion*>& rgnList, const Obstacle& obstacle)
{
  float p[4][2];
  const float* c = obstacle.getPosition();
  const float tankRadius = BZDBCache::tankRadius;

  if (BZDBCache::tankHeight < c[2])
    return;

  const float a = obstacle.getRotation();
  const float w = obstacle.getWidth() + tankRadius;
  const float h = obstacle.getBreadth() + tankRadius;
  const float xx =  w * cosf(a);
  const float xy =  w * sinf(a);
  const float yx = -h * sinf(a);
  const float yy =  h * cosf(a);
  p[0][0] = c[0] - xx - yx;
  p[0][1] = c[1] - xy - yy;
  p[1][0] = c[0] + xx - yx;
  p[1][1] = c[1] + xy - yy;
  p[2][0] = c[0] + xx + yx;
  p[2][1] = c[1] + xy + yy;
  p[3][0] = c[0] - xx + yx;
  p[3][1] = c[1] - xy + yy;

  unsigned int numRegions = (unsigned int)rgnList.size();
  for (unsigned int k = 0; k < numRegions; k++) {
    BzfRegion* region = rgnList[k];
    int side[4];
    if ((side[0] = region->classify(p[0], p[1])) == 1 ||
	(side[1] = region->classify(p[1], p[2])) == 1 ||
	(side[2] = region->classify(p[2], p[3])) == 1 ||
	(side[3] = region->classify(p[3], p[0])) == 1)
      continue;
    if (side[0] == -1 && side[1] == -1 && side[2] == -1 && side[3] == -1) {
      rgnList[k] = rgnList[numRegions-1];
      rgnList[numRegions-1] = rgnList[rgnList.size()-1];
      rgnList.pop_back();
      numRegions--;
      k--;
      delete region;
      continue;
    }
    for (int j = 0; j < 4; j++) {
      if (side[j] == -1) continue;		// to inside
      // split
      const float* p1 = p[j];
      const float* p2 = p[(j+1)&3];
      BzfRegion* newRegion = region->orphanSplitRegion(p2, p1);
      if (!newRegion) continue;		// no split
      if (region != rgnList[k]) rgnList.push_back(region);
      region = newRegion;
    }
    if (region != rgnList[k]) delete region;
  }
}

static void		makeObstacleList()
{
  const float tankRadius = BZDBCache::tankRadius;
  int i;
  for (std::vector<BzfRegion*>::iterator itr = obstacleList.begin();
       itr != obstacleList.end(); ++itr)
    delete (*itr);
  obstacleList.clear();

  // FIXME -- shouldn't hard code game area
  float gameArea[4][2];
  float worldSize = BZDBCache::worldSize;
  gameArea[0][0] = -0.5f * worldSize + tankRadius;
  gameArea[0][1] = -0.5f * worldSize + tankRadius;
  gameArea[1][0] =  0.5f * worldSize - tankRadius;
  gameArea[1][1] = -0.5f * worldSize + tankRadius;
  gameArea[2][0] =  0.5f * worldSize - tankRadius;
  gameArea[2][1] =  0.5f * worldSize - tankRadius;
  gameArea[3][0] = -0.5f * worldSize + tankRadius;
  gameArea[3][1] =  0.5f * worldSize - tankRadius;
  obstacleList.push_back(new BzfRegion(4, gameArea));

  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  const int numBoxes = boxes.size();
  for (i = 0; i < numBoxes; i++) {
    addObstacle(obstacleList, *boxes[i]);
  }
  const ObstacleList& pyramids = OBSTACLEMGR.getPyrs();
  const int numPyramids = pyramids.size();
  for (i = 0; i < numPyramids; i++) {
    addObstacle(obstacleList, *pyramids[i]);
  }
  const ObstacleList& teleporters = OBSTACLEMGR.getTeles();
  const int numTeleporters = teleporters.size();
  for (i = 0; i < numTeleporters; i++) {
    addObstacle(obstacleList, *teleporters[i]);
  }
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  const int numMeshes = meshes.size();
  for (i = 0; i < numMeshes; i++) {
    addObstacle(obstacleList, *meshes[i]);
  }
  if (World::getWorld()->allowTeamFlags()) {
    const ObstacleList& bases = OBSTACLEMGR.getBases();
    const int numBases = bases.size();
    for (i = 0; i < numBases; i++) {
      const BaseBuilding* base = (const BaseBuilding*) bases[i];
      if ((base->getHeight() != 0.0f) || (base->getPosition()[2] != 0.0f)) {
	addObstacle(obstacleList, *base);
      }
    }
  }
}

static void		setRobotTarget(RobotPlayer* robot)
{
  Player* bestTarget = NULL;
  float bestPriority = 0.0f;
  for (int j = 0; j < curMaxPlayers; j++)
    if (player[j] && player[j]->getId() != robot->getId() &&
	player[j]->isAlive() &&
	((robot->getTeam() == RogueTeam && !World::getWorld()->allowRabbit())
	 || player[j]->getTeam() != robot->getTeam())) {

      if (player[j]->isPhantomZoned() && !robot->isPhantomZoned())
	continue;

      if (World::getWorld()->allowTeamFlags() &&
	  (robot->getTeam() == RedTeam && player[j]->getFlag() == Flags::RedTeam) ||
	  (robot->getTeam() == GreenTeam && player[j]->getFlag() == Flags::GreenTeam) ||
	  (robot->getTeam() == BlueTeam && player[j]->getFlag() == Flags::BlueTeam) ||
	  (robot->getTeam() == PurpleTeam && player[j]->getFlag() == Flags::PurpleTeam)) {
	bestTarget = player[j];
	break;
      }

      const float priority = robot->getTargetPriority(player[j]);
      if (priority > bestPriority) {
	bestTarget = player[j];
	bestPriority = priority;
      }
    }
  robot->setTarget(bestTarget);
}

static void		updateRobots(float dt)
{
  static float newTargetTimeout = 1.0f;
  static float clock = 0.0f;
  bool pickTarget = false;
  int i;

  // see if we should look for new targets
  clock += dt;
  if (clock > newTargetTimeout) {
    while (clock > newTargetTimeout) {
      clock -= newTargetTimeout;
    }
    pickTarget = true;
  }

  // start dead robots
  for (i = 0; i < numRobots; i++) {
    if (!gameOver && robots[i]
	&& !robots[i]->isAlive() && !robots[i]->isExploding() && pickTarget) {
      serverLink->sendAlive(robots[i]->getId());
    }
  }

  if (!rcLink) {
    // retarget robots
    for (i = 0; i < numRobots; i++) {
      if (robots[i] && robots[i]->isAlive()
	  && (pickTarget || !robots[i]->getTarget()
	      || !robots[i]->getTarget()->isAlive())) {
	setRobotTarget(robots[i]);
      }
    }
  }

  // do updates
  for (i = 0; i < numRobots; i++)
    if (robots[i]) {
      robots[i]->update();
    }
}


static void		checkEnvironment(RobotPlayer* tank)
{
  // skip this if i'm dead or paused
  if (!tank->isAlive() || tank->isPaused()) return;

  // see if i've been shot
  const ShotPath* hit = NULL;
  float minTime = Infinity;
  int i;
  for (i = 0; i < curMaxPlayers; i++) {
    if (player[i]) {
      tank->checkHit(player[i], hit, minTime);
    }
  }

  // Check Server Shots
  tank->checkHit( World::getWorld()->getWorldWeapons(), hit, minTime);

  // Check if I've been tagged (freeze tag).  Note that we alternate the
  // direction that we go through the list to avoid a pathological case.
  static int upwards = 1;
  for (i = 0; i < curMaxPlayers; i ++) {
    int tankid;
    if (upwards) {
      tankid = i;
    } else {
      tankid = curMaxPlayers - 1 - i;
    }

    Player *othertank = lookupPlayer(tankid);

    if (othertank && othertank->getTeam() != ObserverTeam &&
	tankid != tank->getId()) {
      tank->checkCollision(othertank);
    }
  }
  // swap direction for next time:
  upwards = upwards ? 0 : 1;

  float waterLevel = World::getWorld()->getWaterLevel();

  if (hit) {
    // i got shot!  terminate the shot that hit me and blow up.
    // force shot to terminate locally immediately (no server round trip);
    // this is to ensure that we don't get shot again by the same shot
    // after dropping our shield flag.
    if (hit->isStoppedByHit())
      serverLink->sendHit(tank->getId(), hit->getPlayer(), hit->getShotId());

    FlagType* killerFlag = hit->getFlag();
    bool stopShot;

    if (killerFlag == Flags::Thief) {
      if (tank->getFlag() != Flags::Null) {
	serverLink->sendTransferFlag(tank->getId(), hit->getPlayer());
      }
      stopShot = true;
    }
    else {
      stopShot = gotBlowedUp(tank, GotShot, hit->getPlayer(), hit);
    }

    if (stopShot || hit->isStoppedByHit()) {
      Player* hitter = lookupPlayer(hit->getPlayer());
      if (hitter) hitter->endShot(hit->getShotId());
    }
  }
  // if not dead yet, see if i'm sitting on death
  else if (tank->getDeathPhysicsDriver() >= 0) {
    gotBlowedUp(tank, PhysicsDriverDeath, ServerPlayer, NULL,
		tank->getDeathPhysicsDriver());
  }
  // if not dead yet, see if the robot dropped below the death level
  else if ((waterLevel > 0.0f) && (tank->getPosition()[2] <= waterLevel)) {
    gotBlowedUp(tank, WaterDeath, ServerPlayer);
  }

  // if not dead yet, see if i got run over by the steamroller
  else {
    bool dead = false;
    const float* myPos = tank->getPosition();
    const float myRadius = tank->getRadius();
    for (i = 0; !dead && i < curMaxPlayers; i++) {
      if (player[i] && !player[i]->isPaused() &&
	  ((player[i]->getFlag() == Flags::Steamroller) ||
	   ((tank->getFlag() == Flags::Burrow) && player[i]->isAlive() &&
	    !player[i]->isPhantomZoned()))) {
	const float* pos = player[i]->getPosition();
	if (pos[2] < 0.0f) continue;
	const float radius = myRadius +
	  (BZDB.eval(StateDatabase::BZDB_SRRADIUSMULT) * player[i]->getRadius());
	const float distSquared =
	  hypotf(hypotf(myPos[0] - pos[0],
			myPos[1] - pos[1]), (myPos[2] - pos[2]) * 2.0f);
	if (distSquared < radius) {
	  gotBlowedUp(tank, GotRunOver, player[i]->getId());
	  dead = true;
	}
      }
    }
  }
}

static void		checkEnvironmentForRobots()
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i])
      checkEnvironment(robots[i]);
}

void getObsCorners(const Obstacle *obstacle, bool addTankRadius, float corners[4][2])
{
  const float* c = obstacle->getPosition();
  const float a = obstacle->getRotation();
  float w;
  float h;

  if (addTankRadius) {
    const float tankRadius = BZDBCache::tankRadius;
    w = obstacle->getWidth() + tankRadius;
    h = obstacle->getBreadth() + tankRadius;
  } else {
    w = obstacle->getWidth();
    h = obstacle->getBreadth();
  }
  const float xx =  w * cosf(a);
  const float xy =  w * sinf(a);
  const float yx = -h * sinf(a);
  const float yy =  h * cosf(a);

  // TODO, make sure these go consistently in clockwise or counter-clockwise
  // order.
  corners[0][0] = c[0] - xx - yx;
  corners[0][1] = c[1] - xy - yy;
  corners[1][0] = c[0] + xx - yx;
  corners[1][1] = c[1] + xy - yy;
  corners[2][0] = c[0] + xx + yx;
  corners[2][1] = c[1] + xy + yy;
  corners[3][0] = c[0] - xx + yx;
  corners[3][1] = c[1] - xy + yy;
}

// Gaussian RVS using Box-Muller Transform
static float		gauss(float mu, float sigma)
{
  float x, y, z;

  // z is sampled from a gaussian with mu=0 and sigma=1
  x = (float)bzfrand();
  y = (float)bzfrand();
  z = cos(x * 2 * M_PI) * sqrt(-2 * log(1 - y));
  return mu + z * sigma;
}

static void		sendBase(BaseBuilding *base, const char *teamname)
{
  float corners[4][2];
  getObsCorners(base, false, corners);

  rcLink->respondf("base %s", teamname);

  for (int i=0; i < 4; i++) {
    float* point = corners[i];
    rcLink->respondf(" %f %f", point[0], point[1]);
  }

  rcLink->respond("\n");
}

static void		sendBasesList()
{
  //if (World::getWorld()->allowTeamFlags()) {
  const ObstacleList& bases = OBSTACLEMGR.getBases();
  const int numBases = bases.size();

  rcLink->respond("begin\n");

  for (int i = 0; i < numBases; i++) {
    BaseBuilding* base = (BaseBuilding*) bases[i];
    TeamColor color = (TeamColor)base->getTeam();
    sendBase(base, Team::getShortName(color));
  }

  rcLink->respond("end\n");
}

static void		sendObstacle(Obstacle *obs)
{
  float corners[4][2];
  float posnoise = atof(BZDB.get("bzrcPosNoise").c_str());

  getObsCorners(obs, true, corners);

  rcLink->respond("obstacle");

  for (int i=0; i < 4; i++) {
    float* point = corners[i];
    rcLink->respondf(" %f %f", gauss(point[0], posnoise), \
	gauss(point[1], posnoise));
  }

  rcLink->respond("\n");
}

static void		sendObsList()
{
  int i;
  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  const int numBoxes = boxes.size();
  const ObstacleList& pyramids = OBSTACLEMGR.getPyrs();
  const int numPyramids = pyramids.size();
  const ObstacleList& teleporters = OBSTACLEMGR.getTeles();
  const int numTeleporters = teleporters.size();
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  const int numMeshes = meshes.size();

  rcLink->respond("begin\n");

  for (i = 0; i < numBoxes; i++) {
    sendObstacle(boxes[i]);
  }
  for (i = 0; i < numPyramids; i++) {
    sendObstacle(pyramids[i]);
  }
  for (i = 0; i < numTeleporters; i++) {
    sendObstacle(teleporters[i]);
  }
  for (i = 0; i < numMeshes; i++) {
    sendObstacle(meshes[i]);
  }

  rcLink->respond("end\n");
}

static void		sendTeamList()
{
  const Team& redteam = world->getTeam(RedTeam);
  const Team& greenteam = world->getTeam(GreenTeam);
  const Team& blueteam = world->getTeam(BlueTeam);
  const Team& purpleteam = world->getTeam(PurpleTeam);

  rcLink->respond("begin\n");

  // Note that we only look at the first base

  if (redteam.size > 0) {
    rcLink->respondf("team %s %d\n", Team::getShortName(RedTeam),
			  redteam.size);
  }
  if (greenteam.size > 0) {
    rcLink->respondf("team %s %d\n", Team::getShortName(GreenTeam),
			  greenteam.size);
  }
  if (blueteam.size > 0) {
    rcLink->respondf("team %s %d\n", Team::getShortName(BlueTeam),
			  blueteam.size);
  }
  if (purpleteam.size > 0) {
    rcLink->respondf("team %s %d\n", Team::getShortName(PurpleTeam),
			  purpleteam.size);
  }

  rcLink->respond("end\n");
}

static void		sendFlagList()
{
  rcLink->respond("begin\n");

  for (int i=0; i<numFlags; i++) {
    Flag& flag = world->getFlag(i);
    FlagType *flagtype = flag.type;
    Player *possessplayer = lookupPlayer(flag.owner);

    const char *flagteam = Team::getShortName(flagtype->flagTeam);
    const char *possessteam;

    if (possessplayer != NULL) {
      possessteam = Team::getShortName(possessplayer->getTeam());
    } else {
      possessteam = Team::getShortName(NoTeam);
    }

    if (flag.status != FlagNoExist) {
      rcLink->respondf("flag %s %s %f %f\n",
	  flagteam, possessteam,
	  flag.position[0], flag.position[1]);
    }
  }

  rcLink->respond("end\n");
}

static void		sendShotList()
{
  rcLink->respond("begin\n");

  for (int i=0; i<curMaxPlayers; i++) {
    Player* tank = player[i];
    if (!tank) continue;

    int count = tank->getMaxShots();
    for (int j=0; j<count; j++) {
      ShotPath* shot = tank->getShot(j);
      if (!shot || shot->isExpired() || shot->isExpiring()) continue;

      const float* position = shot->getPosition();
      const float* velocity = shot->getVelocity();

      rcLink->respondf("shot %f %f %f %f\n", position[0], position[1],
	    velocity[0], velocity[1]);
    }

  }

  rcLink->respond("end\n");
}

static void		sendMyTankList()
{
  RobotPlayer* bot;
  float posnoise = atof(BZDB.get("bzrcPosNoise").c_str());
  float angnoise = atof(BZDB.get("bzrcAngNoise").c_str());
  float velnoise = atof(BZDB.get("bzrcVelNoise").c_str());

  rcLink->respond("begin\n");

  const int numShots = World::getWorld()->getMaxShots();

  for (int i=0; i<numRobots; i++) {
    bot = robots[i];
    const char* callsign = bot->getCallSign();
    char *statstr;
    if (bot->isPaused()) {
      statstr = "paused";
    } else if (bot->isFlagActive()) {
      statstr = "super";
    } else if (bot->isAlive()) {
      if (bot->canMove()) {
	statstr = "normal";
      } else {
	statstr = "frozen";
      }
    } else {
      statstr = "dead";
    }

    int shots_avail = numShots;
    for (int shot=0; shot<numShots; shot++) {
      if (bot->getShot(shot)) shots_avail--;
    }

    float reloadtime = bot->getReloadTime();

    FlagType* flagtype = bot->getFlag();
    char *flagname;
    if (flagtype == Flags::Null) {
      flagname = "none";
    } else if (flagtype == Flags::RedTeam) {
      flagname = "red";
    } else if (flagtype == Flags::GreenTeam) {
      flagname = "green";
    } else if (flagtype == Flags::BlueTeam) {
      flagname = "blue";
    } else if (flagtype == Flags::PurpleTeam) {
      flagname = "purple";
    } else {
      flagname = "other";
    }

    const float *pos = bot->getPosition();
    const float angle = bot->getAngle();
    const float *vel = bot->getVelocity();
    const float angvel = bot->getAngularVelocity();

    float noisy_angle = gauss(angle, angnoise);
    if (noisy_angle > M_PI) {
      noisy_angle -= 2 * M_PI;
    } else if (noisy_angle <= -M_PI) {
      noisy_angle += 2 * M_PI;
    }

    rcLink->respondf("mytank %d %s %s %d %f %s %f %f %f %f %f %f\n",
	i, callsign, statstr, shots_avail, reloadtime, flagname,
	gauss(pos[0], posnoise), gauss(pos[1], posnoise),
	noisy_angle, gauss(vel[0], velnoise),
	gauss(vel[1], velnoise), gauss(angvel, angnoise));
  }

  rcLink->respond("end\n");
}

static void		sendOtherTankList()
{
  Player* tank;
  float posnoise = atof(BZDB.get("bzrcPosNoise").c_str());
  float angnoise = atof(BZDB.get("bzrcAngNoise").c_str());

  rcLink->respond("begin\n");

  for (int i=0; i<curMaxPlayers; i++) {
    tank = player[i];
    if (!tank) continue;

    TeamColor team = tank->getTeam();
    if (team == ObserverTeam) continue;
    if (team == startupInfo.team && startupInfo.team != AutomaticTeam) {
      continue;
    }

    const char* callsign = tank->getCallSign();

    const char* colorname = Team::getShortName(team);

    char *statstr;
    if (tank->isPaused()) {
      statstr = "paused";
    } else if (tank->isFlagActive()) {
      statstr = "super";
    } else if (tank->isAlive()) {
      if (tank->canMove()) {
	statstr = "normal";
      } else {
	statstr = "frozen";
      }
    } else {
      statstr = "dead";
    }

    FlagType* flagtype = tank->getFlag();
    char *flagname;
    if (flagtype == Flags::Null) {
      flagname = "none";
    } else if (flagtype == Flags::RedTeam) {
      flagname = "red";
    } else if (flagtype == Flags::GreenTeam) {
      flagname = "green";
    } else if (flagtype == Flags::BlueTeam) {
      flagname = "blue";
    } else if (flagtype == Flags::PurpleTeam) {
      flagname = "purple";
    } else {
      flagname = "other";
    }

    const float *pos = tank->getPosition();
    const float angle = tank->getAngle();

    rcLink->respondf("othertank %s %s %s %s %f %f %f\n",
	callsign, colorname, statstr, flagname,
	gauss(pos[0], posnoise), gauss(pos[1], posnoise),
	gauss(angle, angnoise));
  }

  rcLink->respond("end\n");
}

static void		sendConstList()
{
  rcLink->respond("begin\n");

  rcLink->respondf("constant team %s\n", Team::getShortName(startupInfo.team));
  rcLink->respondf("constant worldsize %f\n", BZDBCache::worldSize);
  rcLink->respond("constant hoverbot 0\n");

  rcLink->respond("end\n");
}

static void		doBotRequests()
{
  RCRequest* req;
  RCRobotPlayer* bot;
  int tankindex;

  while ((req=rcLink->poprequest()) != NULL) {
    req->sendack(rcLink);
    if (req->fail) {
      req->sendfail(rcLink);
      break;
    }

    switch (req->get_request_type()) {
      case Speed:
      case AngularVel:
      case Shoot:
	tankindex = req->get_robotindex();
	if (tankindex == -1) {
	  rcLink->respondf("fail Invalid tank index.\n");
	} else {
	  bot = (RCRobotPlayer*)robots[tankindex];
	  bot->processrequest(req, rcLink);
	}
	break;
      case TeamListRequest:
	sendTeamList();
	break;
      case BasesListRequest:
	sendBasesList();
	break;
      case ObstacleListRequest:
	sendObsList();
	break;
      case FlagListRequest:
	sendFlagList();
	break;
      case ShotListRequest:
	sendShotList();
	break;
      case MyTankListRequest:
	sendMyTankList();
	break;
      case OtherTankListRequest:
	sendOtherTankList();
	break;
      case ConstListRequest:
	sendConstList();
	break;
      default:
	break;
    }
  }
}

static void		sendRobotUpdates()
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i] && robots[i]->isDeadReckoningWrong()) {
      serverLink->sendPlayerUpdate(robots[i]);
    }
}

static void		addRobots()
{
  int  j;
  for (j = 0; j < MAX_ROBOTS; j++)
    robots[j] = NULL;
}

static void enteringServer(void *buf)
{
#if defined(ROBOT)
  int i;
  for (i = 0; i < numRobotTanks; i++)
    serverLink->sendNewPlayer();
  numRobots = 0;
#endif
  // the server sends back the team the player was joined to
  void *tmpbuf = buf;
  uint16_t team, type;
  tmpbuf = nboUnpackUShort(tmpbuf, type);
  tmpbuf = nboUnpackUShort(tmpbuf, team);

  setTankFlags();

  // clear now invalid token
  startupInfo.token[0] = '\0';

  // add robot tanks
#if defined(ROBOT)
  addRobots();
#endif

  // start listening on remote control port
  if (rcLink) {
    rcLink->startListening();
  }

  // initialize some other stuff
  updateNumPlayers();
  updateHighScores();

  entered = true;
}


static void setTankFlags()
{
  // scan through flags and, for flags on
  // tanks, tell the tank about its flag.
  const int maxFlags = world->getMaxFlags();
  for (int i = 0; i < maxFlags; i++) {
    const Flag& flag = world->getFlag(i);
    if (flag.status == FlagOnTank) {
      for (int j = 0; j < curMaxPlayers; j++) {
	if (player[j] && player[j]->getId() == flag.owner) {
	  player[j]->setFlag(flag.type);
	  break;
	}
      }
    }
  }
}


static void cleanWorldCache()
{
  // setup the world cache limit
  int cacheLimit = (10 * 1024 * 1024);
  if (BZDB.isSet("worldCacheLimit")) {
    const int dbCacheLimit = BZDB.evalInt("worldCacheLimit");
    // the old limit was 100 Kbytes, too small
    if (dbCacheLimit == (100 * 1024)) {
      BZDB.setInt("worldCacheLimit", cacheLimit);
    } else {
      cacheLimit = dbCacheLimit;
    }
  } else {
    BZDB.setInt("worldCacheLimit", cacheLimit);
  }

  const std::string worldPath = getCacheDirName();

  while (true) {
    char *oldestFile = NULL;
    int oldestSize = 0;
    int totalSize = 0;

#ifdef _WIN32
    std::string pattern = worldPath + "*.bwc";
    WIN32_FIND_DATA findData;
    HANDLE h = FindFirstFile(pattern.c_str(), &findData);
    if (h != INVALID_HANDLE_VALUE) {
      FILETIME oldestTime;
      while (FindNextFile(h, &findData)) {
	if ((oldestFile == NULL) ||
	    (CompareFileTime(&oldestTime, &findData.ftLastAccessTime) > 0)) {
	  if (oldestFile) {
	    free(oldestFile);
	  }
	  oldestFile = strdup(findData.cFileName);
	  oldestSize = findData.nFileSizeLow;
	  oldestTime = findData.ftLastAccessTime;
	}
	totalSize += findData.nFileSizeLow;
      }
      FindClose(h);
    }
#else
    DIR *directory = opendir(worldPath.c_str());
    if (directory) {
      struct dirent* contents;
      struct stat statbuf;
      time_t oldestTime = time(NULL);
      while ((contents = readdir(directory))) {
	const std::string filename = contents->d_name;
	const std::string fullname = worldPath + filename;
	stat(fullname.c_str(), &statbuf);
	if (S_ISREG(statbuf.st_mode) && (filename.size() > 4) &&
	    (filename.substr(filename.size() - 4) == ".bwc")) {
	  if ((oldestFile == NULL) || (statbuf.st_atime < oldestTime)) {
	    if (oldestFile) {
	      free(oldestFile);
	    }
	    oldestFile = strdup(contents->d_name);
	    oldestSize = statbuf.st_size;
	    oldestTime = statbuf.st_atime;
	  }
	  totalSize += statbuf.st_size;
	}
      }
      closedir(directory);
    }
#endif

    // any valid cache files?
    if (oldestFile == NULL) {
      return;
    }

    // is the cache small enough?
    if (totalSize < cacheLimit) {
      if (oldestFile != NULL) {
	free(oldestFile);
	oldestFile = NULL;
      }
      return;
    }

    // remove the oldest file
    logDebugMessage(1, "cleanWorldCache: removed %s\n", oldestFile);
    remove((worldPath + oldestFile).c_str());
    free(oldestFile);
    totalSize -= oldestSize;
  }
}


static void markOld(std::string &fileName)
{
#ifdef _WIN32
  FILETIME ft;
  HANDLE h = CreateFile(fileName.c_str(),
			FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h != INVALID_HANDLE_VALUE) {
    SYSTEMTIME st;
    memset(&st, 0, sizeof(st));
    st.wYear = 1900;
    st.wMonth = 1;
    st.wDay = 1;
    SystemTimeToFileTime(&st, &ft);
    SetFileTime(h, &ft, &ft, &ft);
    GetLastError();
    CloseHandle(h);
  }
#else
  struct utimbuf times;
  times.actime = 0;
  times.modtime = 0;
  utime(fileName.c_str(), &times);
#endif
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


#if defined(FIXME) && defined(ROBOT)
static void saveRobotInfo(Playerid id, void *msg)
{
  for (int i = 0; i < numRobots; i++)
    if (robots[i]) {
      void *tmpbuf = msg;
      uint16_t team, type, wins, losses, tks;
      char callsign[CallSignLen];
      char email[EmailLen];
      tmpbuf = nboUnpackUShort(tmpbuf, type);
      tmpbuf = nboUnpackUShort(tmpbuf, team);
      tmpbuf = nboUnpackUShort(tmpbuf, wins);
      tmpbuf = nboUnpackUShort(tmpbuf, losses);
      tmpbuf = nboUnpackUShort(tmpbuf, tks);
      tmpbuf = nboUnpackString(tmpbuf, callsign, CallSignLen);
      tmpbuf = nboUnpackString(tmpbuf, email, EmailLen);
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
	  tmpbuf = nboUnpackUByte(tmpbuf, id);
	  robots[i]->id.serverHost = id.serverHost;
	  robots[i]->id.port = id.port;
	  robots[i]->id.number = id.number;
	  robots[i]->server->send(MsgIdAck, 0, NULL);
	}
      }
    }
}
#endif

static void resetServerVar(const std::string& name, void*)
{
  // reset server-side variables
  if (BZDB.getPermission(name) == StateDatabase::Locked) {
    const std::string defval = BZDB.getDefault(name);
    BZDB.set(name, defval);
  }
}

void		leaveGame()
{
  entered = false;
  joiningGame = false;

  // shut down robot connections
  int i;
  for (i = 0; i < numRobots; i++) {
    if (robots[i])
      serverLink->sendExit();
    delete robots[i];
    robots[i] = NULL;
  }
  numRobots = 0;

  for (std::vector<BzfRegion*>::iterator itr = obstacleList.begin();
       itr != obstacleList.end(); ++itr)
    delete (*itr);
  obstacleList.clear();

  // delete world
  World::setWorld(NULL);
  delete world;
  world = NULL;
  teams = NULL;
  curMaxPlayers = 0;
  numFlags = 0;
  player = NULL;

  // shut down server connection
  serverLink->sendExit();
  ServerLink::setServer(NULL);
  delete serverLink;
  serverLink = NULL;

  // reset some flags
  gameOver = false;
  serverError = false;
  serverDied = false;

  // reset the BZDB variables
  BZDB.iterate(resetServerVar, NULL);

  return;
}


static void joinInternetGame(const struct in_addr *inAddress)
{
  // get server address
  Address serverAddress(*inAddress);
  if (serverAddress.isAny()) {
    printError("Server not found");
    return;
  }

  // check for a local server block
  ServerAccessList.reload();
  std::vector<std::string> nameAndIp;
  nameAndIp.push_back(startupInfo.serverName);
  nameAndIp.push_back(serverAddress.getDotNotation());
  if (!ServerAccessList.authorized(nameAndIp)) {
    printError("Server Access Denied Locally");
    std::string msg = ColorStrings[WhiteColor];
    msg += "NOTE: ";
    msg += ColorStrings[GreyColor];
    msg += "server access is controlled by ";
    msg += ColorStrings[YellowColor];
    msg += ServerAccessList.getFileName();
    addMessage(NULL, msg);
    return;
  }

  // open server
  ServerLink* _serverLink = new ServerLink(serverAddress,
					   startupInfo.serverPort);

  serverLink = _serverLink;

  serverLink->sendVarRequest();

  // assume everything's okay for now
  serverDied = false;
  serverError = false;

  if (!serverLink) {
    printError("Memory error");
    return;
  }

  // printError("Join Game");
  // check server
  if (serverLink->getState() != ServerLink::Okay) {
    switch (serverLink->getState()) {
      case ServerLink::BadVersion: {
	static char versionError[] = "Incompatible server version XXXXXXXX";
	strncpy(versionError + strlen(versionError) - 8,
		serverLink->getVersion(), 8);
	printError(versionError);
	break;
      }

	// you got banned
      case ServerLink::Refused:{
	const std::string& rejmsg = serverLink->getRejectionMessage();

	// add to the console
	std::string msg = ColorStrings[RedColor];
	msg += "You have been banned from this server:";
	addMessage(NULL, msg);
	msg = ColorStrings[YellowColor];
	msg += rejmsg;
	addMessage(NULL, msg);

	break;
      }

      case ServerLink::Rejected:
	// the server is probably full or the game is over.  if not then
	// the server is having network problems.
	printError("Game is full or over.  Try again later.");
	break;

      case ServerLink::SocketError:
	printError("Error connecting to server.");
	break;

      case ServerLink::CrippledVersion:
	// can't connect to (otherwise compatible) non-crippled server
	printError("Cannot connect to full version server.");
	break;

      default:
	printError(TextUtils::format
	   ("Internal error connecting to server (error code %d).",
	    serverLink->getState()).c_str());
	break;
    }
    return;
  }

  // use parallel UDP if desired and using server relay
  serverLink->sendUDPlinkRequest();

  printout("Connection Established...");

  sendFlagNegotiation();
  joiningGame = true;
  GameTime::reset();
}


static void addVarToAutoComplete(const std::string& name, void* /*userData*/)
{
  if ((name.size() <= 0) || (name[0] != '_')) {
    return; // we're skipping "poll"
  }
  if (BZDB.getPermission(name) == StateDatabase::Server) {
    completer.registerWord(name);
  }
  return;
}


static void joinInternetGame2()
{
  justJoined = true;

  printout("Entering game...");

  ServerLink::setServer(serverLink);
  World::setWorld(world);

  // prep teams
  teams = world->getTeams();

  // prep players
  curMaxPlayers = 0;
  player = world->getPlayers();
  playerSize = world->getPlayersSize();

  // reset the autocompleter
  completer.setDefaults();
  BZDB.iterate(addVarToAutoComplete, NULL);

  // prep flags
  numFlags = world->getMaxFlags();

  // create observer tank.  This is necessary because the server won't
  // send messages to a bot, but they will send them to an observer.
  observerTank = new LocalPlayer(serverLink->getId(), startupInfo.callsign,
				  startupInfo.email);
  observerTank->setTeam(ObserverTeam);
  LocalPlayer::setMyTank(observerTank);

  // tell the server that the observer tank wants to join
  serverLink->sendEnter(observerTank->getId(), TankPlayer,
			observerTank->getTeam(), observerTank->getCallSign(),
			observerTank->getEmailAddress(),
			startupInfo.token);
  startupInfo.token[0] = '\0';

  joiningGame = false;
}

void getAFastToken ( void )
{
  // get token if we need to (have a password but no token)
  if ((startupInfo.token[0] == '\0') && (startupInfo.password[0] != '\0'))
    {
      ServerList* serverList = new ServerList;
      serverList->startServerPings(&startupInfo);
      // wait no more than 10 seconds for a token
      for (int j = 0; j < 40; j++)
	{
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

void handlePendingJoins ( void )
{
  // try to join a game if requested.  do this *before* handling
  // events so we do a redraw after the request is posted and
  // before we actually try to join.
  if (!joinRequested)
    return;

  // if already connected to a game then first sign off
  if (observerTank)
    leaveGame();

  getAFastToken();

  ares.queryHost(startupInfo.serverName);
  waitingDNS = true;

  // don't try again
  joinRequested = false;
}

bool dnsLookupDone ( struct in_addr &inAddress )
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
  if (status == AresHandler::Failed)
    {
      printError("Server not found.");
      waitingDNS = false;
    }
  else if (status == AresHandler::HbNSucceeded)
    {
      waitingDNS = false;
      return true;
    }
  return false;
}

void checkForServerBail ( void )
{
  // if server died then leave the game (note that this may cause
  // further server errors but that's okay).
  if (serverError || (serverLink && serverLink->getState() == ServerLink::Hungup))
    {
      // if we haven't reported the death yet then do so now
      if (serverDied || (serverLink && serverLink->getState() == ServerLink::Hungup))
	printError("Server has unexpectedly disconnected");
      leaveGame();
    }
}

void updateShots ( const float dt )
{
  // update other tank's shots
  for (int i = 0; i < curMaxPlayers; i++)
    {
      if (player[i])
	player[i]->updateShots(dt);
    }

  // update servers shots
  const World *_world = World::getWorld();
  if (_world)
    _world->getWorldWeapons()->updateShots(dt);
}

void doTankMotions ( const float /*dt*/ )
{
  // do dead reckoning on remote players
  for (int i = 0; i < curMaxPlayers; i++)
    {
      if (player[i])
	{
	  const bool wasNotResponding = player[i]->isNotResponding();
	  player[i]->doDeadReckoning();
	  const bool isNotResponding = player[i]->isNotResponding();

	  if (!wasNotResponding && isNotResponding)
	    addMessage(player[i], "not responding");
	  else if (wasNotResponding && !isNotResponding)
	    addMessage(player[i], "okay");
	}
    }
}

void updatePostions ( const float dt )
{
  updateShots(dt);

  doTankMotions(dt);

  if (entered)
    updateRobots(dt);
}

void checkEnvironment ( const float )
{
  if (entered)
    checkEnvironmentForRobots();
}

void doUpdates ( const float dt )
{
  updatePostions(dt);
  checkEnvironment(dt);

  // update AutoHunt
  AutoHunt::update();
}

void doNetworkStuff ( void )
{
  if (entered)
    sendRobotUpdates();

  cURLManager::perform();
}

void doEnergySaver ( void )
{
  static TimeKeeper lastTime = TimeKeeper::getCurrent();
  const float fpsLimit = BZDB.eval("fpsLimit");

  if ((fpsLimit >= 1.0f) && !isnan(fpsLimit))
    {
      const float elapsed = float(TimeKeeper::getCurrent() - lastTime);
      if (elapsed > 0.0f)
	{
	  const float period = (1.0f / fpsLimit);
	  const float remaining = (period - elapsed);

	  if (remaining > 0.0f)
	    TimeKeeper::sleep(remaining);
	}
    }
  lastTime = TimeKeeper::getCurrent();
}

//
// main playing loop
//

static void		playingLoop()
{
  // start timing
  TimeKeeper::setTick();

  worldDownLoader = new WorldDownLoader;

  // main loop
  while (!CommandsStandard::isQuit())
    {
      BZDBCache::update();

      // set this step game time
      GameTime::setStepTime();

      // get delta time
      TimeKeeper prevTime = TimeKeeper::getTick();
      TimeKeeper::setTick();
      const float dt = float(TimeKeeper::getTick() - prevTime);

      doMessages();    // handle incoming packets

      if (world)
	world->checkCollisionManager();    // see if the world collision grid needs to be updated

      handlePendingJoins();

      struct in_addr inAddress;
      if (dnsLookupDone(inAddress))
	joinInternetGame(&inAddress);

      // Communicate with remote agent if necessary
      if (rcLink) {
	rcLink->tryAccept();
	rcLink->update();
	doBotRequests();
      }

      callPlayingCallbacks();    // invoke callbacks

      if (CommandsStandard::isQuit())     // quick out
	break;

      checkForServerBail();

      doUpdates(dt);

      doNetworkStuff();

      doEnergySaver();

      if (serverLink)
	serverLink->flush();

    } // end main client loop


  delete worldDownLoader;
}


static void		defaultErrorCallback(const char* msg)
{
  std::string message = ColorStrings[RedColor];
  message += msg;
  printout(message);
}


void			botStartPlaying()
{
  // register some commands
  for (unsigned int c = 0; c < countof(commandList); ++c) {
    CMDMGR.add(commandList[c].name, commandList[c].func, commandList[c].help);
  }

  // normal error callback
  setErrorCallback(defaultErrorCallback);

  // initialize epoch offset (time)
  userTimeEpochOffset = (double)mktime(&userTime);
  epochOffset = userTimeEpochOffset;
  lastEpochOffset = epochOffset;

  // catch kill signals before changing video mode so we can
  // put it back even if we die.  ignore a few signals.
  bzSignal(SIGILL, SIG_PF(dying));
  bzSignal(SIGABRT, SIG_PF(dying));
  bzSignal(SIGSEGV, SIG_PF(dying));
  bzSignal(SIGTERM, SIG_PF(suicide));
#if !defined(_WIN32)
  if (bzSignal(SIGINT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGINT, SIG_PF(suicide));
  bzSignal(SIGPIPE, SIG_PF(hangup));
  bzSignal(SIGHUP, SIG_IGN);
  if (bzSignal(SIGQUIT, SIG_IGN) != SIG_IGN)
    bzSignal(SIGQUIT, SIG_PF(dying));
#ifndef GUSI_20
  bzSignal(SIGBUS, SIG_PF(dying));
#endif
  bzSignal(SIGUSR1, SIG_IGN);
  bzSignal(SIGUSR2, SIG_IGN);
#endif /* !defined(_WIN32) */

  updateNumPlayers();

  // windows version can be very helpful in debug logs
#ifdef _WIN32
  if (debugLevel >= 1) {
    OSVERSIONINFO info;
    ZeroMemory(&info, sizeof(OSVERSIONINFO));
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&info);
    logDebugMessage(1, "Running on Windows %s%d.%d %s\n",
	   (info.dwPlatformId == VER_PLATFORM_WIN32_NT) ? "NT " : "",
	   info.dwMajorVersion, info.dwMinorVersion,
	   info.szCSDVersion);
  }
#endif

  // print expiration
  if (timeBombString()) {
    // add message about date of expiration
    char bombMessage[80];
    sprintf(bombMessage, "This release will expire on %s", timeBombString());
    printError(bombMessage);
  }

  // get current MOTD
  //if (!BZDB.isTrue("disableMOTD")) {
  //  motd = new MessageOfTheDay;
  //  motd->getURL(BZDB.get("motdServer"));
  //}

  // inform user of silencePlayers on startup
  for (unsigned int j = 0; j < silencePlayers.size(); j ++){
    std::string aString = silencePlayers[j];
    aString += " Silenced";
    if (silencePlayers[j] == "*") {
      aString = "Silenced All Msgs";
    }
    printError(aString);
  }

  // startup an RCLink if requested
  if (BZDB.isSet("rcPort")) {
    int port = atoi(BZDB.get("rcPort").c_str());
    rcLink = new RCLink(port);
  }

  // enter game if we have all the info we need, otherwise
  joinRequested    = true;
  printout("Trying...");

  // start game loop
  playingLoop();

  // clean up
  delete motd;
  leaveGame();
  setErrorCallback(NULL);
  World::done();
  cleanWorldCache();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
