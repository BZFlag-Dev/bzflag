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
#include "botplaying.h"

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
#include "AnsiCodes.h"
#include "AccessList.h"
#include "AutoHunt.h"
#include "BaseBuilding.h"
#include "BZDBCache.h"
#include "BzfMedia.h"
#include "bzsignal.h"
#include "CacheManager.h"
#include "CollisionManager.h"
#include "CommandsStandard.h"
#include "DirectoryNames.h"
#include "ErrorHandler.h"
#include "FileManager.h"
#include "GameTime.h"
#include "KeyManager.h"
#include "LinkManager.h"
#include "ObstacleList.h"
#include "ObstacleMgr.h"
#include "PhysicsDriver.h"
#include "PlatformFactory.h"
#include "ServerList.h"
#include "TextUtils.h"
#include "TimeBomb.h"
#include "TimeKeeper.h"
#include "WordFilter.h"
#include "bz_md5.h"
#include "vectors.h"
#include "version.h"

// common client headers
#include "ClientIntangibilityManager.h"
#include "FlashClock.h"
#include "LocalPlayer.h"
#include "Roaming.h"
#include "RobotPlayer.h"
#include "Roster.h"
#include "ShotStats.h"
#include "GameTime.h"
#include "WorldBuilder.h"
#include "WorldPlayer.h"
#include "World.h"

// local implementation headers
#include "Downloads.h"
#include "Frontend.h"
#include "Logger.h"
#include "RCLinkBackend.h"
#include "RCMessageFactory.h"
#include "RCRobotPlayer.h"
#include "ScriptLoaderFactory.h"

#include "bzflag.h"
#include "commands.h"
#include "motd.h"

#include "clientvars.h"


// FIXME: Any code surrounded by "if (!headless)" is unsafely assuming that
// it's operating in a context where graphics and sound are available.
bool headless = true;

RCLinkBackend *rcLink = NULL;


// to simplify code shared between bzrobots and bzflag
// - in bzrobots, this just goes to the error console
void showError(const char *msg, bool flush)
{
  printError(msg);
  if(flush) {
    ;
  }
}


// - in bzrobots, this shows the error on the console
void showMessage(const std::string& line)
{
  BACKENDLOGGER << stripAnsiCodes(line.c_str()) << std::endl;
}


void showMessage(const std::string& line, ControlPanel::MessageModes)
{
  BACKENDLOGGER << stripAnsiCodes(line.c_str()) << std::endl;
}
// access silencePlayers from bzflag.cxx
std::vector<std::string> &getSilenceList()
{
  return silencePlayers;
}

void selectNextRecipient (bool, bool)
{
}
bool shouldGrabMouse()
{
  return false;
}

void warnAboutMainFlags()
{
}


void warnAboutRadarFlags()
{
}


void warnAboutRadar()
{
}


void warnAboutConsole()
{
}


BzfDisplay*		getDisplay()
{
  return NULL;
}

MainWindow*		getMainWindow()
{
  return NULL;
}

RadarRenderer* getRadarRenderer()
{
  return NULL;
}


void forceControls(bool, float, float)
{
}


void setSceneDatabase()
{
}



bool setVideoFormat(int, bool)
{
  return false;
}

void addShotExplosion(const fvec3&)
{
}

void addShotPuff(const fvec3&, const fvec3&)
{
}


void notifyBzfKeyMapChanged()
{
}

void dumpResources()
{
}


void setTarget()
{
}



void joinGame()
{
  if (joiningGame) {
    if(worldDownLoader) {
      worldDownLoader->stop();
      joiningGame = false;
      showError("Download stopped by user action");
    }
  }
  joinRequested = true;
}

//
// handle signals that should kill me quickly
//

static void dying(int sig)
{
  bzSignal(sig, SIG_DFL);
  raise(sig);
}




void updateHighScores()
{
  /* check scores to see if my team and/or have the high score.  change
   * `>= bestScore' to `> bestScore' if you want to share the number
   * one spot. */
  bool anyPlayers = false;
  int i;
  for (i = 0; i < curMaxPlayers; i++)
    if (remotePlayers[i]) {
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





int curlProgressFunc(void * /*clientp*/,
		     double dltotal, double dlnow,
		     double /*ultotal*/, double /*ulnow*/)
{
  // FIXME: beaucoup cheeze here in the aborting style
  // we should be using async dns and multi-curl

  // run an inner main loop to check if we should abort
  /*
  BzfEvent event;
  if (display->isEventPending() && display->peekEvent(event)) {
    switch (event.type) {
    case BzfEvent::Quit:
      return 1; // terminate the curl call
    case BzfEvent::KeyDown:
      display->getEvent(event); // flush the event
      if (event.keyDown.chr == 27)
	return 1; // terminate the curl call

      break;
    case BzfEvent::KeyUp:
      display->getEvent(event); // flush the event
      break;
    case BzfEvent::MouseMove:
      display->getEvent(event); // flush the event
      break;
    case BzfEvent::Unset:
    case BzfEvent::Map:
    case BzfEvent::Unmap:
    case BzfEvent::Redraw:
    case BzfEvent::Resize:
      // leave the event, it might be important
      break;
    }
  }
  */
  
  // update the status
  double percentage = 0.0;
  if ((int)dltotal > 0)
    percentage = 100.0 * dlnow / dltotal;

  showError(TextUtils::format("%2.1f%% (%i/%i)", percentage, (int) dlnow, (int) dltotal).c_str());

  return 0;
}





void handleResourceFetch(void*)
{
}


void handleQueryOS(void*)
{
}


void handleCustomSound(void *)
{
}


void handleTimeUpdate(void *msg)
{
  int32_t timeLeft;
  msg = nboUnpackInt32(msg, timeLeft);
  //hud->setTimeLeft(timeLeft);
  if (timeLeft == 0) {
    gameOver = true;
    myTank->explodeTank();
    showMessage("Time Expired");
    //hud->setAlert(0, "Time Expired", 10.0f, true);
#ifdef ROBOT
    for (int i = 0; i < numRobots; i++)
      if (robots[i])
	robots[i]->explodeTank();
#endif
  } else if (timeLeft < 0) {
    //hud->setAlert(0, "Game Paused", 10.0f, true);
  }
}


void handleScoreOver(void *msg)
{
  // unpack packet
  PlayerId id;
  int16_t team;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackInt16(msg, team);
  Player *player = lookupPlayer(id);

  // make a message
  std::string msg2;
  if (team == (int16_t)NoTeam) {
    // a player won
    if (player) {
      msg2 = TextUtils::format("%s (%s) won the game",
      player->getCallSign(),
      Team::getName(player->getTeam()));
    } else {
      msg2 = "[unknown player] won the game";
    }
  } else {
    msg2 = TextUtils::format("%s won the game",
      Team::getName(TeamColor(team)));
  }

  gameOver = true;
  myTank->explodeTank();

#ifdef ROBOT
  for (int i = 0; i < numRobots; i++) {
    if (robots[i])
      robots[i]->explodeTank();
  }
#endif
}


void handleAliveMessage(void *msg)
{
  PlayerId id;
  fvec3 pos;
  float forward;

  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackFVec3(msg, pos);
  msg = nboUnpackFloat(msg, forward);
  int playerIndex = lookupPlayerIndex(id);

  if ((playerIndex < 0) && (playerIndex != -2)) {
    return;
  }
  Player *tank = getPlayerByIndex(playerIndex);
  if (tank == NULL) {
    return;
  }

  if (tank == myTank) {
    //wasRabbit = tank->getTeam() == RabbitTeam;
    myTank->restart(pos, forward);
    myTank->setShotType(StandardShot);
    //firstLife = false;
    justJoined = false;

    //if (!myTank->isAutoPilot())
    //  mainWindow->warpMouse();

    //hud->setAltitudeTape(myTank->canJump());
  }
#ifdef ROBOT
  else if (tank->getPlayerType() == ComputerPlayer) {
    for (int r = 0; r < numRobots; r++) {
      if (robots[r] && robots[r]->getId() == playerIndex) {
        robots[r]->restart(pos, forward);
        setRobotTarget(robots[r]);
        break;
      }
    }
  }
#endif

  static const fvec3 zero(0.0f, 0.0f, 0.0f);
  tank->setStatus(tank->getStatus() | PlayerState::Alive);
  tank->move(pos, forward);
  tank->setVelocity(zero);
  tank->setAngularVelocity(0.0f);
  tank->setDeadReckoning(GameTime::getDRTime());
  tank->spawnEffect();
  if (tank == myTank) {
    myTank->setSpawning(false);
  }

  //SOUNDSYSTEM.play(SFX_POP, pos, true, isViewTank(tank));
}


void handleAutoPilot(void *msg)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);

  uint8_t autopilot;
  nboUnpackUInt8(msg, autopilot);

  Player *tank = lookupPlayer(id);
  if (!tank)
    return;

  tank->setAutoPilot(autopilot != 0);
  addMessage(tank, autopilot ? "Roger taking controls" : "Roger releasing controls");
}




void handleKilledMessage(void *msg, bool /*human*/, bool &checkScores)
{
  PlayerId victim, killer;
  FlagType *flagType;
  int16_t shotId, reason;
  int phydrv = -1;
  msg = nboUnpackUInt8(msg, victim);
  msg = nboUnpackUInt8(msg, killer);
  msg = nboUnpackInt16(msg, reason);
  msg = nboUnpackInt16(msg, shotId);
  msg = FlagType::unpack(msg, flagType);
  if (reason == (int16_t)PhysicsDriverDeath) {
    int32_t inPhyDrv;
    msg = nboUnpackInt32(msg, inPhyDrv);
    phydrv = int(inPhyDrv);
  }
  BaseLocalPlayer *victimLocal = getLocalPlayer(victim);
  BaseLocalPlayer *killerLocal = getLocalPlayer(killer);
  Player *victimPlayer = lookupPlayer(victim);
  Player *killerPlayer = lookupPlayer(killer);

  if (victimPlayer == myTank) {
    // uh oh, i'm dead
    if (myTank->isAlive()) {
      serverLink->sendDropFlag(myTank->getPosition());
      myTank->setShotType(StandardShot);
      //handleMyTankKilled(reason);
    }
  }

  if (victimLocal) {
    // uh oh, local player is dead
    if (victimLocal->isAlive()) {
      gotBlowedUp(victimLocal, GotKilledMsg, killer);
      rcLink->pushEvent(new DeathEvent());
    }
  }
  else if (victimPlayer) {
    victimPlayer->setExplode(TimeKeeper::getTick());
  }

  if (killerLocal) {
    // local player did it
    if (shotId >= 0)
      killerLocal->endShot(shotId, true);	// terminate the shot
  }

#ifdef ROBOT
  // blow up robots on victim's team if shot was genocide
  if (killerPlayer && victimPlayer && shotId >= 0) {
    const ShotPath *shot = killerPlayer->getShot(int(shotId));
    if (shot && shot->getFlag() == Flags::Genocide) {
      for (int i = 0; i < numRobots; i++) {
	if (robots[i] && victimPlayer != robots[i] && victimPlayer->getTeam() == robots[i]->getTeam() && robots[i]->getTeam() != RogueTeam)
	  gotBlowedUp(robots[i], GenocideEffect, killerPlayer->getId());
      }
    }
  }
#endif

  checkScores = true;
}


void handleGrabFlag(void *msg)
{
  PlayerId id;
  uint16_t flagIndex;
  unsigned char shot;

  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, flagIndex);
  if (flagIndex >= world->getMaxFlags()) {
    return;
  }
  Flag& flag = world->getFlag(int(flagIndex));
  msg = flag.unpack(msg);
  msg = nboUnpackUInt8(msg, shot);

  Player *tank = lookupPlayer(id);
  if (!tank)
    return;

  // player now has flag
  tank->setFlag(flag.type);
  tank->setShotType((ShotType)shot);

  std::string message("grabbed ");
  message += tank->getFlag()->flagName;
  message += " flag";

  addMessage(tank, message);
}


void handleCaptureFlag(void *msg, bool &checkScores)
{
  PlayerId id;
  uint16_t flagIndex;
  int16_t team;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, flagIndex);
  if (flagIndex >= world->getMaxFlags()) {
    return;
  }
  msg = nboUnpackInt16(msg, team);
  Player *capturer = lookupPlayer(id);

  Flag& capturedFlag = world->getFlag(int(flagIndex));

  if (capturedFlag.type == Flags::Null)
    return;

  int capturedTeam = capturedFlag.type->flagTeam;

  // player no longer has flag
  if (capturer) {
    capturer->setFlag(Flags::Null);
    //if (capturer == myTank)
    //  updateFlag(Flags::Null);

    // add message
    if (int(capturer->getTeam()) == capturedTeam) {
      std::string message("took my flag into ");
      message += Team::getName(TeamColor(team));
      message += " territory";
      addMessage(capturer, message);
    } else {
      std::string message("captured ");
      message += Team::getName(TeamColor(capturedTeam));
      message += "'s flag";
      addMessage(capturer, message);
    }
  }

#ifdef ROBOT
  //kill all my robots if they are on the captured team
  for (int r = 0; r < numRobots; r++) {
    if (robots[r] && robots[r]->getTeam() == capturedTeam)
      gotBlowedUp(robots[r], GotCaptured, robots[r]->getId());
  }
#endif

  checkScores = true;
}



// changing the rabbit

void handleNewRabbit(void *msg)
{
  PlayerId id;
  msg = nboUnpackUInt8(msg, id);
  Player *rabbit = lookupPlayer(id);

  // new mode option,
  unsigned char mode;
  msg = nboUnpackUInt8(msg, mode);

  // mode 0 == swap the current rabbit with this rabbit
  // mode 1 == add this person as a rabbit
  // mode 2 == remove this person from the rabbit list

  // we don't need to mod the hunters if we aren't swaping
  if (mode == 0) {
    for (int i = 0; i < curMaxPlayers; i++) {
      if (remotePlayers[i])
	remotePlayers[i]->setHunted(false);
      if (i != id && remotePlayers[i] &&
          remotePlayers[i]->getTeam() != RogueTeam &&
          remotePlayers[i]->getTeam() != ObserverTeam)
	remotePlayers[i]->changeTeam(HunterTeam);
    }
  }

  if (rabbit != NULL) {
    if (mode != 2) {
      rabbit->changeTeam(RabbitTeam);

      if (mode == 0)
	addMessage(rabbit, "is now the rabbit", ControlPanel::MessageMisc, true);
      else
	addMessage(rabbit, "is now a rabbit", ControlPanel::MessageMisc, true);
    } else {
      rabbit->changeTeam(HunterTeam);
      addMessage(rabbit, "is no longer a rabbit", ControlPanel::MessageMisc, true);
    }
  }

#ifdef ROBOT
  for (int r = 0; r < numRobots; r++) {
    if (robots[r]) {
      if (robots[r]->getId() == id) {
	robots[r]->changeTeam(RabbitTeam);
      } else {
	robots[r]->changeTeam(HunterTeam);
      }
    }
  }
#endif
}


void handleNearFlag(void *msg)
{
  fvec3 pos;
  std::string flagName;
  msg = nboUnpackFVec3(msg, pos);
  msg = nboUnpackStdString(msg, flagName);

  std::string fullMessage = "Closest Flag: " + flagName;
  std::string colorMsg;
  colorMsg += ColorStrings[YellowColor];
  colorMsg += fullMessage;
  colorMsg += ColorStrings[DefaultColor];

  addMessage(NULL, colorMsg, ControlPanel::MessageServer, false, NULL);

  if (myTank) {
    //hud->setAlert(0, fullMessage.c_str(), 5.0f, false);
  }
}



void handleShotBegin(bool /*human*/, void *msg)
{
  PlayerId shooterid;
  uint16_t id;

  msg = nboUnpackUInt8(msg, shooterid);
  msg = nboUnpackUInt16(msg, id);

  FiringInfo firingInfo;
  msg = firingInfo.unpack(msg);
  firingInfo.shot.player = shooterid;
  firingInfo.shot.id     = id;

  if (shooterid >= playerSize) {
    return;
  }

  if (shooterid == myTank->getId()) {
    // the shot is ours, find the shot we made, and kill it
    // then rebuild the shot with the info from the server
    myTank->updateShot(firingInfo, id, firingInfo.timeSent);
  }
  else {
    RemotePlayer *shooter = remotePlayers[shooterid];

    if (shooterid != ServerPlayer) {
      if (shooter && remotePlayers[shooterid]->getId() == shooterid) {
	shooter->addShot(firingInfo);
      }
    }
  }
}


void handleWShotBegin (void *msg)
{
  FiringInfo firingInfo;
  msg = firingInfo.unpack(msg);

  WorldPlayer *worldWeapons = world->getWorldWeapons();

  worldWeapons->addShot(firingInfo);
  //playShotSound(firingInfo, false);
}


void handleScore(void *msg)
{
  uint8_t numScores;
  PlayerId id;
  float rank;
  uint16_t wins, losses, tks;
  msg = nboUnpackUInt8(msg, numScores);

  for (uint8_t s = 0; s < numScores; s++) {
    msg = nboUnpackUInt8(msg, id);
    msg = nboUnpackFloat(msg, rank);
    msg = nboUnpackUInt16(msg, wins);
    msg = nboUnpackUInt16(msg, losses);
    msg = nboUnpackUInt16(msg, tks);

    Player *sPlayer = NULL;
    if (id == myTank->getId()) {
      sPlayer = myTank;
    } else {
      int i = lookupPlayerIndex(id);
      if (i >= 0)
	sPlayer = getPlayerByIndex(i);
      else
	logDebugMessage(1, "Received score update for unknown player!\n");
    }
    if (sPlayer) {
      if (sPlayer == myTank) {
	/*ExportInformation &ei = ExportInformation::instance();
	ei.setInformation("Score",
	  TextUtils::format("%d (%d-%d) [%d]",
	  wins - losses, wins, losses, tks),
	  ExportInformation::eitPlayerStatistics,
	  ExportInformation::eipPrivate);*/
      }
      sPlayer->changeScore(rank, wins, losses, tks);
    }
  }
}


void handleTeleport(void *msg)
{
  PlayerId id;
  uint16_t srcID, dstID;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt16(msg, srcID);
  msg = nboUnpackUInt16(msg, dstID);
  Player *tank = lookupPlayer(id);
  if (tank) {
    if (tank != myTank) {
      tank->setTeleport(TimeKeeper::getTick(), short(srcID), short(dstID));
      /*const MeshFace* linkDst = linkManager.getLinkDstFace(dstID);
      const MeshFace* linkSrc = linkManager.getLinkSrcFace(srcID);
      if (linkDst && linkSrc) {
        const fvec3& pos = linkDst->getPosition();
        if (!linkSrc->linkSrcNoSound()) {
          SOUNDSYSTEM.play(SFX_TELEPORT, pos, false, false);
        }
      }*/
    }
  }
}


void handleMessage(void* /*msg*/)
{
}


void handleNewPlayer(void *msg)
{
  uint8_t id;
  uint8_t botID;
  int16_t team;
  msg = nboUnpackUInt8(msg, id);
  msg = nboUnpackUInt8(msg, botID);
  msg = nboUnpackInt16(msg, team);
  
#ifdef ROBOT
  int i;
  for (i = 0; i < MAX_ROBOTS; i++)
    if (!robots[i])
      break;
  if (i >= MAX_ROBOTS) {
    logDebugMessage(1, "Too many bots requested\n");
    return;
  }
  robots[i] = new RCRobotPlayer(id,
    TextUtils::format("%s%2.2d", myTank->getCallSign(),i).c_str(),
    serverLink);
  robots[i]->setTeam((TeamColor)team);
  serverLink->sendEnter(id, ComputerPlayer, NoUpdates, robots[i]->getTeam(),
    robots[i]->getCallSign(), "", "");
  if (!numRobots) {
    makeObstacleList();
    RobotPlayer::setObstacleList(&obstacleList);
  }
  numRobots++;
#endif
}


void handleFlagTransferred(Player *fromTank, Player *toTank, int flagIndex, ShotType shotType)
{
  Flag& f = world->getFlag(flagIndex);

  fromTank->setShotType(StandardShot);
  fromTank->setFlag(Flags::Null);
  toTank->setShotType(shotType);
  toTank->setFlag(f.type);

  std::string message(toTank->getCallSign());
  message += " stole ";
  message += fromTank->getCallSign();
  message += "'s flag";
  addMessage(toTank, message);
}


void handleQueryGL(void*)
{
}


void handleGMUpdate(void *msg)
{
  ShotUpdate shot;
  msg = shot.unpack(msg);
  Player *tank = lookupPlayer(shot.player);
  if (!tank || tank == myTank)
    return;

  RemotePlayer *remoteTank = (RemotePlayer*)tank;
  RemoteShotPath *shotPath = (RemoteShotPath*)remoteTank->getShot(shot.id);
  if (shotPath)
    shotPath->update(shot, msg);
  PlayerId targetId;
  msg = nboUnpackUInt8(msg, targetId);
  Player *targetTank = getPlayerByIndex(targetId);

  if (targetTank && (targetTank == myTank) && (myTank->isAlive()))
  {
    static TimeKeeper lastLockMsg;
    if (TimeKeeper::getTick() - lastLockMsg > 0.75)
    {
      //SOUNDSYSTEM.play(SFX_LOCK, shot.pos, false, false);
      lastLockMsg = TimeKeeper::getTick();
      addMessage(tank, "locked on me");
    }
  }
}




void handleLimboMessage(void *msg)
{
  std::string limboMessage;
  nboUnpackStdString(msg, limboMessage);
  //hud->setLimboMessage(limboMessage);
}


void handleFlagDropped(Player *tank)
{

  if (tank->getPlayerType() == ComputerPlayer) {
    RobotPlayer *robot = lookupRobotPlayer(tank->getId());
    if (!robot)
      return;
    robot->setShotType(StandardShot);
    robot->setFlag(Flags::Null);
  } else {
    tank->setShotType(StandardShot);

    // skip it if player doesn't actually have a flag
    if (tank->getFlag() == Flags::Null) return;

    if (tank == myTank) {
      // make sure the player must reload after theft
      if (tank->getFlag() == Flags::Thief)
	myTank->forceReload(BZDB.eval(BZDBNAMES.THIEFDROPTIME));

      // update display and play sound effects
      //SOUNDSYSTEM.play(SFX_DROP_FLAG);
      //updateFlag(Flags::Null);
    }

    // add message
    std::string message("dropped ");
    message += tank->getFlag()->flagName;
    message += " flag";
    addMessage(tank, message);

    // player no longer has flag
    tank->setFlag(Flags::Null);
  }
}


bool gotBlowedUp(BaseLocalPlayer *tank, BlowedUpReason reason, PlayerId killer,
			const ShotPath *hit, int phydrv)
{
  if (!tank || (tank->getTeam() == ObserverTeam || !tank->isAlive()))
    return false;

  int shotId = -1;
  FlagType *flagType = Flags::Null;
  if (hit) {
    shotId = hit->getShotId();
    flagType = hit->getFlag();
  }

  // you can't take it with you
  const FlagType *flag = tank->getFlag();
  if (flag != Flags::Null) {


    // tell other players I've dropped my flag
    serverLink->sendDropFlag(tank->getPosition());
    tank->setShotType(StandardShot);

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





//
// some robot stuff
//

static void checkEnvironment(RobotPlayer *tank)
{
  // skip this if i'm dead or paused
  if (!tank->isAlive() || tank->isPaused()) return;

  // see if i've been shot
  const ShotPath *hit = NULL;
  float minTime = Infinity;
  tank->checkHit(myTank, hit, minTime);
  int i;
  for (i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i] && remotePlayers[i]->getId() != tank->getId()) {
      tank->checkHit(remotePlayers[i], hit, minTime);
    }
  }

  // Check Server Shots
  tank->checkHit(World::getWorld()->getWorldWeapons(), hit, minTime);

  float waterLevel = World::getWorld()->getWaterLevel();

  if (hit) {
    // i got shot!  terminate the shot that hit me and blow up.
    // force shot to terminate locally immediately (no server round trip);
    // this is to ensure that we don't get shot again by the same shot
    // after dropping our shield flag.
    if (hit->isStoppedByHit())
      serverLink->sendHit(tank->getId(), hit->getPlayer(), hit->getShotId());

    FlagType *killerFlag = hit->getFlag();
    bool stopShot;

    if (killerFlag == Flags::Thief) {
      if (tank->getFlag() != Flags::Null)
	serverLink->sendTransferFlag(tank->getId(), hit->getPlayer());
      stopShot = true;
    } else {
      stopShot = gotBlowedUp(tank, GotShot, hit->getPlayer(), hit);
    }

    if (stopShot || hit->isStoppedByHit()) {
      Player *hitter = lookupPlayer(hit->getPlayer());
      if (hitter) hitter->endShot(hit->getShotId());
    }
  }
  else if (tank->getDeathPhysicsDriver() >= 0) {
    // if not dead yet, see if i'm sitting on death
    gotBlowedUp(tank, PhysicsDriverDeath, ServerPlayer, NULL,
      tank->getDeathPhysicsDriver());
  }
  else if ((waterLevel > 0.0f) && (tank->getPosition().z <= waterLevel)) {
    // if not dead yet, see if the robot dropped below the death level
    gotBlowedUp(tank, WaterDeath, ServerPlayer);
  }
  else { // if not dead yet, see if i got run over by the steamroller
    // robot vs. myTank
    if (checkSquishKill(tank, myTank, true)) {
      gotBlowedUp(tank, GotRunOver, myTank->getId());
    }
    else {
      // robot vs. remote tanks
      for (i = 0; i < curMaxPlayers; i++) {
        const Player* remote = remotePlayers[i];
        if (checkSquishKill(tank, remote)) {
          gotBlowedUp(tank, GotRunOver, remotePlayers[i]->getId());
          break;
        }
      }
    }
  }
}


static void checkEnvironmentForRobots()
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

/*
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

  rcLink->sendf("base %s", teamname);

  for (int i=0; i < 4; i++) {
    float* point = corners[i];
    rcLink->sendf(" %f %f", point[0], point[1]);
  }

  rcLink->send("\n");
}

static void		sendBasesList()
{
  //if (World::getWorld()->allowTeamFlags()) {
  const ObstacleList& bases = OBSTACLEMGR.getBases();
  const int numBases = bases.size();

  rcLink->send("begin\n");

  for (int i = 0; i < numBases; i++) {
    BaseBuilding* base = (BaseBuilding*) bases[i];
    TeamColor color = (TeamColor)base->getTeam();
    sendBase(base, Team::getShortName(color));
  }

  rcLink->send("end\n");
}

static void		sendObstacle(Obstacle *obs)
{
  float corners[4][2];
  float posnoise = atof(BZDB.get("bzrcPosNoise").c_str());

  getObsCorners(obs, true, corners);

  rcLink->send("obstacle");

  for (int i=0; i < 4; i++) {
    float* point = corners[i];
    rcLink->sendf(" %f %f", gauss(point[0], posnoise), \
		  gauss(point[1], posnoise));
  }

  rcLink->send("\n");
}

static void		sendObsList()
{
  int i;
  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  const int numBoxes = boxes.size();
  const ObstacleList& pyramids = OBSTACLEMGR.getPyrs();
  const int numPyramids = pyramids.size();
  const int numTeleporters = teleporters.size();
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  const int numMeshes = meshes.size();

  rcLink->send("begin\n");

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

  rcLink->send("end\n");
}

static void		sendTeamList()
{
  const Team& redteam = world->getTeam(RedTeam);
  const Team& greenteam = world->getTeam(GreenTeam);
  const Team& blueteam = world->getTeam(BlueTeam);
  const Team& purpleteam = world->getTeam(PurpleTeam);

  rcLink->send("begin\n");

  // Note that we only look at the first base

  if (redteam.size > 0) {
    rcLink->sendf("team %s %d\n", Team::getShortName(RedTeam),
		  redteam.size);
  }
  if (greenteam.size > 0) {
    rcLink->sendf("team %s %d\n", Team::getShortName(GreenTeam),
		  greenteam.size);
  }
  if (blueteam.size > 0) {
    rcLink->sendf("team %s %d\n", Team::getShortName(BlueTeam),
		  blueteam.size);
  }
  if (purpleteam.size > 0) {
    rcLink->sendf("team %s %d\n", Team::getShortName(PurpleTeam),
		  purpleteam.size);
  }

  rcLink->send("end\n");
}

static void		sendFlagList()
{
  rcLink->send("begin\n");

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
      rcLink->sendf("flag %s %s %f %f\n",
		    flagteam, possessteam,
		    flag.position[0], flag.position[1]);
    }
  }

  rcLink->send("end\n");
}

static void		sendShotList()
{
  rcLink->send("begin\n");

  for (int i=0; i<curMaxPlayers; i++) {
    Player* tank = remotePlayers[i];
    if (!tank) continue;

    int count = tank->getMaxShots();
    for (int j=0; j<count; j++) {
      ShotPath* shot = tank->getShot(j);
      if (!shot || shot->isExpired() || shot->isExpiring()) continue;

      const float* position = shot->getPosition();
      const float* velocity = shot->getVelocity();

      rcLink->sendf("shot %f %f %f %f\n", position[0], position[1],
		    velocity[0], velocity[1]);
    }

  }

  rcLink->send("end\n");
}

static void		sendMyTankList()
{
  RobotPlayer* bot;
  float posnoise = atof(BZDB.get("bzrcPosNoise").c_str());
  float angnoise = atof(BZDB.get("bzrcAngNoise").c_str());
  float velnoise = atof(BZDB.get("bzrcVelNoise").c_str());

  rcLink->send("begin\n");

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

    rcLink->sendf("mytank %d %s %s %d %f %s %f %f %f %f %f %f\n",
		  i, callsign, statstr, shots_avail, reloadtime, flagname,
		  gauss(pos[0], posnoise), gauss(pos[1], posnoise),
		  noisy_angle, gauss(vel[0], velnoise),
		  gauss(vel[1], velnoise), gauss(angvel, angnoise));
  }

  rcLink->send("end\n");
}

static void		sendOtherTankList()
{
  Player* tank;
  float posnoise = atof(BZDB.get("bzrcPosNoise").c_str());
  float angnoise = atof(BZDB.get("bzrcAngNoise").c_str());

  rcLink->send("begin\n");

  for (int i=0; i<curMaxPlayers; i++) {
    tank = remotePlayers[i];
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

    rcLink->sendf("othertank %s %s %s %s %f %f %f\n",
		  callsign, colorname, statstr, flagname,
		  gauss(pos[0], posnoise), gauss(pos[1], posnoise),
		  gauss(angle, angnoise));
  }

  rcLink->send("end\n");
}

static void		sendConstList()
{
  rcLink->send("begin\n");

  rcLink->sendf("constant team %s\n", Team::getShortName(startupInfo.team));
  rcLink->sendf("constant worldsize %f\n", BZDBCache::worldSize);
  rcLink->send("constant hoverbot 0\n");

  rcLink->send("end\n");
}
*/

static void doBotRequests()
{
  RCRequest* req;

  if (numRobots < 1)
    return;

  while ((req = rcLink->peekRequest()) != NULL) {
    if (!req->process((RCRobotPlayer*)robots[0]))
      return;

    rcLink->popRequest(); // Discard it, we're done with this one.
    rcLink->sendAck(req);
  }
}

void enteringServer(void* buf)
{
#if defined(ROBOT)
  int maxBots = world->getBotsPerIP();
  if (numRobotTanks > maxBots) {
    showMessage(
	TextUtils::format("The server only allows %d robot %s.",
	  maxBots, (maxBots == 1 ? "player" : "players")));
    showMessage("Additional robot players have been removed.");
    numRobotTanks = maxBots;
  }
  int i;
  for (i = 0; i < numRobotTanks; i++)
    serverLink->sendNewPlayer(i,AutomaticTeam);
  numRobots = 0;
#endif
  // the server sends back the team the player was joined to
  void *tmpbuf = buf;
  int16_t team;
  uint16_t type, wins, losses, tks;
  float rank;
  tmpbuf = nboUnpackUInt16(tmpbuf, type);
  tmpbuf = nboUnpackInt16(tmpbuf, team);
  tmpbuf = nboUnpackFloat(tmpbuf, rank);
  tmpbuf = nboUnpackUInt16(tmpbuf, wins);
  tmpbuf = nboUnpackUInt16(tmpbuf, losses);
  tmpbuf = nboUnpackUInt16(tmpbuf, tks);

  myTank->changeScore(rank, wins, losses, tks);

  const TeamColor teamColor = (TeamColor)team;
  const char* teamName = Team::getName(teamColor);

  // if server assigns us a different team, display a message
  std::string teamMsg;
  if (myTank->getTeam() != AutomaticTeam) {
    teamMsg = TextUtils::format("%s team was unavailable, you were joined ",
                                Team::getName(myTank->getTeam()));
    if (teamColor == ObserverTeam) {
      teamMsg += "as an Observer";
    } else {
      teamMsg += TextUtils::format("to the %s", teamName);
    }
  }
  else {
    if (teamColor == ObserverTeam) {
      teamMsg = "You were joined as an observer";
    }
    else {
      if (team != RogueTeam) {
	teamMsg = TextUtils::format("You joined the %s", teamName);
      } else {
	teamMsg = TextUtils::format("You joined as a %s", teamName);
      }
    }
  }

  if (myTank->getTeam() != teamColor) {
    myTank->setTeam(teamColor);
    //hud->setAlert(1, teamMsg.c_str(), 8.0f, teamColor == ObserverTeam);
    addMessage(NULL, teamMsg.c_str(), ControlPanel::MessageMisc, true);
  }

  setTankFlags();

  // clear now invalid token
  startupInfo.token[0] = '\0';

  // add robot tanks
#if defined(ROBOT)
  addRobots();
#endif

  // initialize some other stuff
  updateNumPlayers();
  updateHighScores();

  entered = true;
}


static void resetServerVar(const std::string &name, void*)
{
  // reset server-side variables
  if (BZDB.getPermission(name) == StateDatabase::Locked) {
    const std::string defval = BZDB.getDefault(name);
    BZDB.set(name, defval);
  }
}


void leaveGame()
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
  remotePlayers = NULL;

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


static void addVarToAutoComplete(const std::string &name, void* /*userData*/)
{
  if ((name.size() <= 0) || (name[0] != '_'))
    return; // we're skipping "poll"

  if (BZDB.getPermission(name) == StateDatabase::Server)
    completer.registerWord(name);

  return;
}


void joinInternetGame2()
{
  justJoined = true;

  showMessage("Entering game...");

  ServerLink::setServer(serverLink);
  World::setWorld(world);

  canSpawn = true;

  // prep teams
  teams = world->getTeams();

  // prep players
  curMaxPlayers = 0;
  remotePlayers = world->getPlayers();
  playerSize = world->getPlayersSize();

  // reset the autocompleter
  completer.reset();
  BZDB.iterate(addVarToAutoComplete, NULL);

  // prep flags
  numFlags = world->getMaxFlags();

  // create observer tank.  This is necessary because the server won't
  // send messages to a bot, but they will send them to an observer.
  myTank = new LocalPlayer(serverLink->getId(), startupInfo.callsign);
  myTank->setTeam(ObserverTeam);
  LocalPlayer::setMyTank(myTank);

  // tell the server that the observer tank wants to join
  serverLink->sendEnter(myTank->getId(), TankPlayer, AllUpdates, 
			myTank->getTeam(), myTank->getCallSign(),
			startupInfo.token, startupInfo.referrer);
  startupInfo.token[0] = '\0';
  startupInfo.referrer[0] = '\0';
  joiningGame = false;
}


void checkForServerBail(void )
{
  // if server died then leave the game (note that this may cause
  // further server errors but that's okay).
  if (serverError || (serverLink && serverLink->getState() == ServerLink::Hungup)) {
    // if we haven't reported the death yet then do so now
    if (serverDied || (serverLink && serverLink->getState() == ServerLink::Hungup))
      printError("Server has unexpectedly disconnected");
    else
      printError("We were disconencted from the server, quitting.");
    CommandsStandard::quit();
  }
}


void doTankMotions(const float /*dt*/)
{
  // do dead reckoning on remote players
  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i]) {
      const bool wasNotResponding = remotePlayers[i]->isNotResponding();
      remotePlayers[i]->doDeadReckoning();
      const bool isNotResponding = remotePlayers[i]->isNotResponding();

      if (!wasNotResponding && isNotResponding)
	addMessage(remotePlayers[i], "not responding");
      else if (wasNotResponding && !isNotResponding)
	addMessage(remotePlayers[i], "okay");
    }
  }
  
  // do motion
  if (myTank) {
    if (myTank->isAlive() && !myTank->isPaused()) {
      /*doMotion();
      if (scoreboard->getHuntState()==ScoreboardRenderer::HUNT_ENABLED)
	setHuntTarget(); //spot hunt target

      if (myTank->getTeam() != ObserverTeam &&
	((fireButton && myTank->getFlag() == Flags::MachineGun) ||
	(myTank->getFlag() == Flags::TriggerHappy)))
	myTank->fireShot();

      setLookAtMarker();

      // see if we have a target, if so lock on to the bastage
      if (myTank->getTarget())
	hud->AddLockOnMarker(myTank->getTarget()->getPosition(),
	myTank->getTarget()->getCallSign(),
	!isKillable(myTank->getTarget()));
	
    } else {
      int mx, my;
      mainWindow->getMousePosition(mx, my);*/
    }
    myTank->update();
  }
}

void updatePositions ( const float dt )
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
  float doneDT = dt;
  float dtLimit = MAX_DT_LIMIT;
  float realDT = dt;

  if (doneDT > dtLimit) {
    realDT = dtLimit;
    doneDT -= dtLimit;
  }
  
  while (doneDT > 0) {
    updatePositions(dt);
    checkEnvironment(dt);

    doneDT -= dtLimit;

    if (doneDT < dtLimit) // if we only have a nubby left, don't do a full dt.
      realDT = doneDT;
  }
  
  // update AutoHunt
  AutoHunt::update();
}




bool checkForCompleteDownloads(void)
{
  // check if we are waiting for initial texture downloading
  if (!Downloads::instance().requestFinalized())
    return false;

  // downloading is terminated. go!
  Downloads::instance().finalizeDownloads();
  if (downloadingData) {
    downloadingData = false;
    return true;
  } else {
    //setSceneDatabase();
  }

  return false;
}

void doEnergySaver(void )
{
  static TimeKeeper lastTime = TimeKeeper::getCurrent();
  const float fpsLimit = 10000;

  if ((fpsLimit >= 1.0f) && !isnan(fpsLimit)) {
    const float elapsed = float(TimeKeeper::getCurrent() - lastTime);
    if (elapsed > 0.0f) {
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

static void playingLoop()
{
  // start timing
  TimeKeeper::setTick();

  worldDownLoader = new WorldDownLoader;

  // main loop
  while (!CommandsStandard::isQuit()) {
    BZDBCache::update();

    canSpawn = true;

    // set this step game time
    GameTime::setStepTime();

    // get delta time
    TimeKeeper prevTime = TimeKeeper::getTick();
    TimeKeeper::setTick();
    const float dt = float(TimeKeeper::getTick() - prevTime);

    doMessages();    // handle incoming packets

    if (world)
      world->checkCollisionManager(); // see if the world collision grid needs to be updated

    handlePendingJoins();

    struct in_addr inAddress;
    if (dnsLookupDone(inAddress))
      joinInternetGame(&inAddress);

    // Communicate with remote agent if necessary
    if (rcLink) {
      if (numRobots >= numRobotTanks)
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

    if (checkForCompleteDownloads())
      joinInternetGame2(); // we did the inital downloads, so we should join

    doEnergySaver();

    if (serverLink)
      serverLink->flush();

  } // end main client loop


  delete worldDownLoader;
}


static void defaultErrorCallback(const char *msg)
{
  std::string message = ColorStrings[RedColor];
  message += msg;
  showMessage(message);
}


void			botStartPlaying()
{
  // register some commands
  const std::vector<CommandListItem>& commandList = getCommandList();
  for (size_t c = 0; c < commandList.size(); ++c) {
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
    snprintf(bombMessage, 80, "This release will expire on %s", timeBombString());
    showMessage(bombMessage);
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
    if (silencePlayers[j] == "*")
      aString = "Silenced All Msgs";

    printError(aString);
  }

  int port;
  if (!BZDB.isSet("rcPort")) // Generate a random port between 1024 & 65536.
    port = (int)(bzfrand() * (65536 - 1024)) + 1024;
  else
    port = atoi(BZDB.get("rcPort").c_str());

  // here we register the various RCRequest-handlers for commands
  // that RCLinkBackend receives. :-)
  RCMessageFactory<RCRequest>::initialize();

  rcLink = new RCLinkBackend();
  rcLink->startListening(port);
  RCREQUEST.setLink(rcLink);

  if (!BZDB.isSet("robotScript")) {
    BACKENDLOGGER << "Missing script on commandline!\n" << std::endl;
    exit(EXIT_FAILURE);
  }

  ScriptLoaderFactory::initialize();

  if (!Frontend::run(BZDB.get("robotScript"), "localhost", port))
    return;

  // enter game if we have all the info we need, otherwise
  joinRequested    = true;
  showMessage("Trying...");

  // start game loop
  playingLoop();

  // clean up
  delete motd;
  leaveGame();
  setErrorCallback(NULL);
  World::done();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
