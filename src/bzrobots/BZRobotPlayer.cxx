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
#include "BZRobotPlayer.h"

// common implementation headers
#include "BZDBCache.h"

// local implementation headers
#include "Roster.h"
#include "World.h"
#include "Intersect.h"
#include "TargetingUtils.h"


#define MIN_EXEC_TIME 0.05f // 1000ms * 0.05 = 50ms
#define DIST_THRESHOLD 0.001f
#define TURN_THRESHOLD 0.001f


// event priority sorting
static bool compareEventPriority(BZRobots::Event *a, BZRobots::Event *b)
{
  if(a->getPriority() < b->getPriority())
    return true;
  return false;
}

static void clearEventQueue(std::list<BZRobots::Event *> &eventQueue)
{
  while(!eventQueue.empty()) {
    BZRobots::Event *e = eventQueue.front();
    eventQueue.pop_front();
	delete e;
  }
}

static void runEventHandler(BZRobots::Robot *robot,BZRobots::Event *e)
{
  if(!robot)
	  return;
  switch(e->getEventID())
  {
    case BZRobots::BattleEndedEventID:
      robot->onBattleEnded(*((BZRobots::BattleEndedEvent *)e));
      break;
    case BZRobots::BulletHitEventID:
      robot->onBulletHit(*((BZRobots::BulletHitEvent *)e));
      break;
    case BZRobots::BulletMissedEventID:
      robot->onBulletMissed(*((BZRobots::BulletMissedEvent *)e));
      break;
    case BZRobots::DeathEventID:
      robot->onDeath(*((BZRobots::DeathEvent *)e));
      break;
    case BZRobots::HitByBulletEventID:
      robot->onHitByBullet(*((BZRobots::HitByBulletEvent *)e));
      break;
    case BZRobots::HitWallEventID:
      robot->onHitWall(*((BZRobots::HitWallEvent *)e));
      break;
    case BZRobots::RobotDeathEventID:
      robot->onRobotDeath(*((BZRobots::RobotDeathEvent *)e));
      break;
    case BZRobots::ScannedRobotEventID:
      robot->onScannedRobot(*((BZRobots::ScannedRobotEvent *)e));
      break;
    case BZRobots::SpawnEventID:
      robot->onSpawn(*((BZRobots::SpawnEvent *)e));
      break;
    case BZRobots::StatusEventID:
      robot->onStatus(*((BZRobots::StatusEvent *)e));
      break;
    case BZRobots::WinEventID:
      robot->onWin(*((BZRobots::WinEvent *)e));
      break;
    default:
      break;
  }
}


BZRobotPlayer::BZRobotPlayer(const PlayerId& _id,
			     const char* _name,
			     ServerLink* _server) :
  RobotPlayer(_id, _name, _server),
  lastExec(0.0f),
  inEvents(false),
  purgeQueue(false),
  didHitWall(false),
  robot(NULL),
  tsPlayerCount(0),
  tsName(_name),
  tsGunHeat(0.0),
  tsShoot(false),
  tsSpeed(BZDBCache::tankSpeed),
  tsNextSpeed(BZDBCache::tankSpeed),
  tsTurnRate(BZDBCache::tankAngVel),
  tsNextTurnRate(BZDBCache::tankAngVel),
  tsDistanceRemaining(0.0f),
  tsNextDistance(0.0f),
  tsTurnRemaining(0.0f),
  tsNextTurn(0.0f),
  tsHasStopped(false),
  tsStoppedDistance(0.0f),
  tsStoppedTurn(0.0f),
  tsStoppedForward(false),
  tsStoppedLeft(false),
  tsShotReloadTime(BZDB.eval(BZDBNAMES.RELOADTIME))
{
#if defined(HAVE_PTHREADS)
  pthread_mutex_init(&player_lock, NULL);
#elif defined(_WIN32) 
  InitializeCriticalSection(&player_lock);
#endif
  for (int i = 0; i < BZRobotPlayer::updateCount; ++i)
    tsPendingUpdates[i] = false;
  tsBattleFieldSize = BZDBCache::worldSize;
}

BZRobotPlayer::~BZRobotPlayer()
{
LOCK_PLAYER
  clearEventQueue(tsScanQueue);
  clearEventQueue(tsEventQueue);
UNLOCK_PLAYER
#if defined(_WIN32) 
  DeleteCriticalSection (&player_lock);
#endif
}

// Called by bzrobots client thread
void BZRobotPlayer::setRobot(BZRobots::Robot *_robot)
{
  robot = _robot;
}

// Called by bzrobots client thread
void BZRobotPlayer::explodeTank()
{
  LocalPlayer::explodeTank();
  BZRobots::DeathEvent *e = new BZRobots::DeathEvent();
  e->setTime(TimeKeeper::getCurrent().getSeconds());
  LOCK_PLAYER
  clearEventQueue(tsScanQueue);
  clearEventQueue(tsEventQueue);
  tsEventQueue.push_back(e);
  purgeQueue = true;
  UNLOCK_PLAYER
}

// Called by bzrobots client thread
void BZRobotPlayer::restart(const fvec3& pos, float azimuth)
{
  BZRobots::SpawnEvent *e = new BZRobots::SpawnEvent();
  e->setTime(TimeKeeper::getCurrent().getSeconds());
  LOCK_PLAYER
  tsEventQueue.push_back(e);
  tsTankSize = getDimensions();
  tsGunHeat = 0.0;
  tsPosition = pos;
  tsCurrentHeading = azimuth;
  tsCurrentSpeed = 0.0;
  tsCurrentTurnRate = 0.0;
  UNLOCK_PLAYER
  LocalPlayer::restart(pos, azimuth);
}

// Called by bzrobots client thread
void BZRobotPlayer::update(float inputDT)
{
  std::string robotName = this->getCallSign();
  LOCK_PLAYER
  // Fire a shot if requested
  if (tsShoot) {
    tsShoot = false;
    fireShot();
	// TODO: Use ShotPath *shot = fireShot();
	// Then track the shot live
  }
  // Update player count (better way to do this?)
  int i;
  tsPlayerCount = 0;
  for (i = 0; i < curMaxPlayers; i++)
    if (remotePlayers[i])
	  if(remotePlayers[i]->getTeam() >= 0 && !remotePlayers[i]->isObserver())
		if(robotName != std::string(remotePlayers[i]->getCallSign()))
	      tsPlayerCount++;
  for (i = 0; i < numRobots; i++)
    if (robots[i] && robots[i] != this)
	  tsPlayerCount++;
  // Check for wall hit
  if (hasHitWall()) {
    if (!didHitWall) {
      BZRobots::HitWallEvent *hitWallEvent = new BZRobots::HitWallEvent(0.0f); // Get real angle to wall?
      hitWallEvent->setTime(TimeKeeper::getCurrent().getSeconds());
      tsEventQueue.push_back(hitWallEvent);
      didHitWall = true;
    }
  } else {
    didHitWall = false;
  }
  // Update scanned player queue
  //double cpa = getAngle();
  fvec3 cpp = getPosition();
  clearEventQueue(tsScanQueue);
  for (i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i] == NULL)
      continue;
    if (remotePlayers[i]->isAlive() == false)
      continue;
    if (remotePlayers[i]->getId() == getId())
      continue;
    if (remotePlayers[i]->isObserver())
      continue;

	fvec3 rpp = remotePlayers[i]->getPosition();
	fvec3 rpv = remotePlayers[i]->getVelocity();
	double remotePlayerVelocity = sqrt(rpv.x*rpv.x + rpv.y*rpv.y); // exclude z vector
	fvec3 rpd(rpp.x-cpp.x,rpp.y-cpp.y,rpp.z-cpp.z);
	double remotePlayerDistance = sqrt(rpd.x*rpd.x + rpd.y*rpd.y); // exclude z vector
	double headingToPlayer = -atan2(rpd.x,rpd.y) + (M_PI/2.0f);
        if(headingToPlayer < 0.0f) headingToPlayer += (M_PI + M_PI);
        double bearingToPlayer = -(headingToPlayer - getAngle());
        if(bearingToPlayer < -M_PI) bearingToPlayer += (M_PI + M_PI);
        if(bearingToPlayer > M_PI) bearingToPlayer -= (M_PI + M_PI);
	BZRobots::ScannedRobotEvent *sre = new BZRobots::ScannedRobotEvent(
          remotePlayers[i]->getCallSign(),
          bearingToPlayer * 180.0f/M_PI,
	  remotePlayerDistance,
	  rpp.x, rpp.y, rpp.z,
	  remotePlayers[i]->getAngle(),
	  remotePlayerVelocity);
        sre->setTime(TimeKeeper::getCurrent().getSeconds());
	tsScanQueue.push_back(sre);
  }
  /*
  // Get Obstacles
    unsigned int i;
  link->send(ObstaclesBeginReply());
  const ObstacleList &boxes = OBSTACLEMGR.getBoxes();
  for (i = 0; i < boxes.size(); i++) {
    Obstacle *obs = boxes[i];
    link->send(ObstacleReply(obs, boxType));
  }

  const ObstacleList &pyrs = OBSTACLEMGR.getPyrs();
  for (i = 0; i < pyrs.size(); i++) {
    Obstacle *obs = pyrs[i];
    link->send(ObstacleReply(obs, pyrType));
  }

  const ObstacleList &bases = OBSTACLEMGR.getBases();
  for (i = 0; i < bases.size(); i++) {
    Obstacle *obs = bases[i];
    link->send(ObstacleReply(obs, baseType));
  }

  const ObstacleList &meshes = OBSTACLEMGR.getMeshes();
  for (i = 0; i < meshes.size(); i++) {
    Obstacle *obs = meshes[i];
    link->send(ObstacleReply(obs, meshType));
  }

  const ObstacleList &walls = OBSTACLEMGR.getWalls();
  for (i = 0; i < walls.size(); i++) {
    Obstacle *obs = walls[i];
    link->send(ObstacleReply(obs, wallType));
  }
  */
  BaseLocalPlayer::update(inputDT); // There is no LocalPlayer::update
  UNLOCK_PLAYER
}

// Called by bzrobots client thread
// Note that LOCK_PLAYER is already set by BZRobotPlayer::update
void BZRobotPlayer::doUpdate(float dt)
{
  LocalPlayer::doUpdate(dt);
  // Copy data accessed by both threads
  const fvec3& vvec = getVelocity();
  tsTankSize = getDimensions();
  tsGunHeat = getReloadTime();
  tsPosition = getPosition();
  tsCurrentHeading = getAngle();
  tsCurrentSpeed = sqrt(vvec.x*vvec.x + vvec.y*vvec.y);
  tsCurrentTurnRate = getAngularVelocity();
}

// Called by bzrobots client thread
// Note that LOCK_PLAYER is already set by BZRobotPlayer::update
void BZRobotPlayer::doUpdateMotion(float dt)
{
  if(!tsHasStopped) {
    const fvec3& vvec = getVelocity();
    float dist = dt *sqrt(vvec.x*vvec.x + vvec.y*vvec.y); // no z vector
    tsDistanceRemaining -= dist;
    if (tsDistanceRemaining > DIST_THRESHOLD) {
      setDesiredSpeed((float)(tsDistanceForward ? tsSpeed : -tsSpeed));
    } else {
      setDesiredSpeed(0);
      tsDistanceRemaining = 0.0f;
    }
    if (tsTurnRemaining > TURN_THRESHOLD) {
      double turnAdjust = getAngularVelocity() * dt;
      if (tsTurnLeft) {
	tsTurnRemaining -= turnAdjust;
	if (tsTurnRemaining <= TURN_THRESHOLD)
	  setDesiredAngVel(0);
	else if (tsTurnRate * dt > tsTurnRemaining)
	  setDesiredAngVel((float)tsTurnRemaining/dt);
	else
	  setDesiredAngVel((float)tsTurnRate);
      } else {
	tsTurnRemaining += turnAdjust;
	if (tsTurnRemaining <= TURN_THRESHOLD)
	  setDesiredAngVel(0);
	else if (tsTurnRate * dt > tsTurnRemaining)
	  setDesiredAngVel((float)-tsTurnRemaining/dt);
	else
	  setDesiredAngVel((float)-tsTurnRate);
      }
    }
    if (tsTurnRemaining <= TURN_THRESHOLD) {
      setDesiredAngVel(0);
      tsTurnRemaining = 0.0f;
    }
  } else {
    setDesiredSpeed(0);
    setDesiredAngVel(0);
  }
  LocalPlayer::doUpdateMotion(dt);
}

void BZRobotPlayer::shotFired(const ShotPath *shot, const Player *shooter)
{
  if(shot == NULL || shooter == NULL)
    return;
  std::string robotName = this->getCallSign();
  std::string shooterName = shooter->getCallSign();
  LOCK_PLAYER
  if(robotName != shooterName) {
    // TODO: Create a Bullet based on the shot and place it in the event
    BZRobots::BulletFiredEvent *bfe = new BZRobots::BulletFiredEvent(NULL);
	bfe->setTime(TimeKeeper::getCurrent().getSeconds());
	tsEventQueue.push_back(bfe);
  }
  UNLOCK_PLAYER
}

void BZRobotPlayer::shotKilled(const ShotPath *shot, const Player *killer, const Player *victim)
{
  if(shot == NULL || killer == NULL || victim == NULL)
    return;
  std::string robotName = this->getCallSign();
  std::string killerName = killer->getCallSign();
  std::string victimName = victim->getCallSign();
  LOCK_PLAYER
  if(robotName == killerName) {
    // TODO: Create a Bullet based on the shot and place it in the event
	BZRobots::BulletHitEvent *bhe = new BZRobots::BulletHitEvent(victimName,NULL);
	bhe->setTime(TimeKeeper::getCurrent().getSeconds());
	tsEventQueue.push_back(bhe);
  }
  if(robotName == victimName) {
    // TODO: Create a Bullet based on the shot and place it in the event
    BZRobots::HitByBulletEvent *hbbe = new BZRobots::HitByBulletEvent(0.0f,NULL);
    hbbe->setTime(TimeKeeper::getCurrent().getSeconds());
    tsEventQueue.push_back(hbbe);
  }
  if(robotName != victimName) {
    BZRobots::RobotDeathEvent *rde = new BZRobots::RobotDeathEvent(victimName);
    rde->setTime(TimeKeeper::getCurrent().getSeconds());
    tsEventQueue.push_back(rde);
  }
  UNLOCK_PLAYER
}

void BZRobotPlayer::botAhead(double distance)
{
  botSetAhead(distance);
  botExecute();
  while(botGetDistanceRemaining() > 0.0f)
    botDoNothing();
}

void BZRobotPlayer::botBack(double distance)
{
  botSetBack(distance);
  botExecute();
  while(botGetDistanceRemaining() > 0.0f)
    botDoNothing();
}

void BZRobotPlayer::botClearAllEvents()
{
  LOCK_PLAYER
  clearEventQueue(tsScanQueue);
  clearEventQueue(tsEventQueue);
  purgeQueue = true;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botDoNothing()
{
  botExecute();
}

// This does three things:
// 1) makes the setXXX calls "live"
// 2) runs any pending events ("end of turn")
// 3) may sleep for a time
// 4) send status event ("start of next turn")
void BZRobotPlayer::botExecute()
{
  std::list<BZRobots::Event *> eventQueue;

  LOCK_PLAYER
  if (tsPendingUpdates[BZRobotPlayer::speedUpdate])
    tsSpeed = tsNextSpeed;
  if (tsPendingUpdates[BZRobotPlayer::turnRateUpdate])
    tsTurnRate = tsNextTurnRate;

  if (tsPendingUpdates[BZRobotPlayer::distanceUpdate]) {
    if (tsNextDistance < 0.0f)
      tsDistanceForward = false;
    else
      tsDistanceForward = true;
    tsDistanceRemaining = (tsDistanceForward ? 1 : -1) * tsNextDistance;
  }
  if (tsPendingUpdates[BZRobotPlayer::turnUpdate]) {
    if (tsNextTurn < 0.0f)
      tsTurnLeft = false;
    else
      tsTurnLeft = true;
    tsTurnRemaining = (tsTurnLeft ? 1 : -1) * tsNextTurn;
  }

  for (int i = 0; i < BZRobotPlayer::updateCount; ++i)
    tsPendingUpdates[i] = false;

  // Copy the event queues, since LOCK_PLAYER must not
  // be locked while executing the various events
  if(!inEvents) {
    eventQueue.splice(eventQueue.begin(),tsScanQueue);
    eventQueue.splice(eventQueue.begin(),tsEventQueue);
  }

  UNLOCK_PLAYER

  // It's possible we might call execute
  // from within an event handler, so return
  // here if we're flushing an event queue
  // (to prevent infinite recursion)
  if(!inEvents) {
    inEvents = true;
    eventQueue.sort(compareEventPriority);
    purgeQueue = false;
    while(!purgeQueue && !eventQueue.empty()) {
      BZRobots::Event *e = eventQueue.front();
	  eventQueue.pop_front();
	  runEventHandler(robot,e);
	  delete e;
    }
    if(purgeQueue) {
	  purgeQueue = false;
	  clearEventQueue(eventQueue);
    }
    inEvents = false;
  }

  double thisExec = TimeKeeper::getCurrent().getSeconds();
  double diffExec = (thisExec - lastExec);
  if(diffExec < MIN_EXEC_TIME) {
    TimeKeeper::sleep(MIN_EXEC_TIME - diffExec);
    lastExec = TimeKeeper::getCurrent().getSeconds();
  } else {
    lastExec = thisExec;
  }

  if(!inEvents) {
    inEvents = true;
	BZRobots::RobotStatus robotStatus(
		tsDistanceRemaining,
		16,
		tsCurrentHeading * 180.0f/M_PI,
		tsCurrentHeading,
		tsGunHeat,
		0.0f,
		0.0f,
		tsCurrentHeading * 180.0f/M_PI,
		tsCurrentHeading,
		1,
		0, // TODO: count other robots
		tsCurrentHeading * 180.0f/M_PI,
		tsCurrentHeading,
		0.0f,
		0.0f,
		1,
		TimeKeeper::getCurrent().getSeconds(),
		tsTurnRemaining * 180.0f/M_PI,
		tsTurnRemaining,
		tsCurrentSpeed,
		tsPosition.x,
		tsPosition.y,
		tsPosition.z
	);
    BZRobots::StatusEvent *statusEvent = new BZRobots::StatusEvent(robotStatus);
    statusEvent->setTime(robotStatus.getTime());
    runEventHandler(robot,statusEvent);
	delete statusEvent;
    inEvents = false;
  }
}
void BZRobotPlayer::botFire(double power)
{
  botSetFire(power);
  botExecute();
}

BZRobots::Bullet* BZRobotPlayer::botFireBullet(double power)
{
  BZRobots::Bullet *bullet;
  bullet = botSetFireBullet(power);
  botExecute();
  return bullet;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetAllEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

double BZRobotPlayer::botGetBattleFieldLength()
{
  LOCK_PLAYER
  double battleFieldSize = tsBattleFieldSize;
  UNLOCK_PLAYER
  return battleFieldSize;
}

double BZRobotPlayer::botGetBattleFieldWidth()
{
  LOCK_PLAYER
  double battleFieldSize = tsBattleFieldSize;
  UNLOCK_PLAYER
  return battleFieldSize;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetBulletHitBulletEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetBulletHitEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetBulletMissedEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

double BZRobotPlayer::botGetDistanceRemaining()
{
  double distanceRemaining = 0.0f;
  LOCK_PLAYER
  if (tsPendingUpdates[BZRobotPlayer::distanceUpdate])
    distanceRemaining = tsNextDistance;
  else
    distanceRemaining = tsDistanceRemaining;
  UNLOCK_PLAYER
  return distanceRemaining;
}

double BZRobotPlayer::botGetEnergy()
{
  return 16.0f;
}

double BZRobotPlayer::botGetGunCoolingRate()
{
  return tsShotReloadTime;
}

double BZRobotPlayer::botGetGunHeading()
{
  return botGetHeading();
}

double BZRobotPlayer::botGetGunHeadingRadians()
{
  return botGetHeadingRadians();
}

double BZRobotPlayer::botGetGunTurnRemaining()
{
  return 0.0f;
}

double BZRobotPlayer::botGetGunTurnRemainingRadians()
{
  return 0.0f;
}

double BZRobotPlayer::botGetGunHeat()
{
  LOCK_PLAYER
  double gunHeat = tsGunHeat;
  UNLOCK_PLAYER
  return gunHeat;
}

double BZRobotPlayer::botGetHeading()
{
  LOCK_PLAYER
  double heading = tsCurrentHeading * 180.0f/M_PI;
  UNLOCK_PLAYER
  return heading;
}

double BZRobotPlayer::botGetHeadingRadians()
{
  LOCK_PLAYER
  double heading = tsCurrentHeading;
  UNLOCK_PLAYER
  return heading;
}

double BZRobotPlayer::botGetHeight()
{
  LOCK_PLAYER
  double height = tsTankSize.z;
  UNLOCK_PLAYER
  return height;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetHitByBulletEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetHitRobotEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetHitWallEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

double BZRobotPlayer::botGetLength()
{
  LOCK_PLAYER
  double length = tsTankSize.y;
  UNLOCK_PLAYER
  return length;
}

std::string BZRobotPlayer::botGetName()
{
  std::string playerName;
  LOCK_PLAYER
  playerName = tsName;
  UNLOCK_PLAYER
  return playerName;
}

int BZRobotPlayer::botGetNumRounds()
{
  return 1;
}

int BZRobotPlayer::botGetOthers()
{
  int othersCount = 0;
  LOCK_PLAYER
  othersCount = tsPlayerCount;
  UNLOCK_PLAYER
  return othersCount;
}

double BZRobotPlayer::botGetRadarHeading()
{
  return botGetHeading();
}

double BZRobotPlayer::botGetRadarHeadingRadians()
{
  return botGetHeadingRadians();
}

double BZRobotPlayer::botGetRadarTurnRemaining()
{
  return 0.0f;
}

double BZRobotPlayer::botGetRadarTurnRemainingRadians()
{
  return 0.0f;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetRobotDeathEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

int BZRobotPlayer::botGetRoundNum()
{
  return 1;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetScannedRobotEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

std::list<BZRobots::Event> BZRobotPlayer::botGetStatusEvents()
{
  std::list<BZRobots::Event> queue;
  return queue;
}

double BZRobotPlayer::botGetTime()
{
  return TimeKeeper::getCurrent().getSeconds();
}

double BZRobotPlayer::botGetTurnRemaining()
{
  double turnRemaining = 0.0f;
  LOCK_PLAYER
  if (tsPendingUpdates[BZRobotPlayer::turnUpdate])
    turnRemaining = tsNextTurn * 180.0f/M_PI;
  else
    turnRemaining = tsTurnRemaining * 180.0f/M_PI;
  UNLOCK_PLAYER
  return turnRemaining;
}

double BZRobotPlayer::botGetTurnRemainingRadians()
{
  double turnRemaining = 0.0f;
  LOCK_PLAYER
  if (tsPendingUpdates[BZRobotPlayer::turnUpdate])
    turnRemaining = tsNextTurn;
  else
    turnRemaining = tsTurnRemaining;
  UNLOCK_PLAYER
  return turnRemaining;
}

double BZRobotPlayer::botGetVelocity()
{
  LOCK_PLAYER
  double velocity = tsCurrentSpeed;
  UNLOCK_PLAYER
  return velocity;
}

double BZRobotPlayer::botGetWidth()
{
  LOCK_PLAYER
  double width = tsTankSize.x;
  UNLOCK_PLAYER
  return width;
}

double BZRobotPlayer::botGetX()
{
  LOCK_PLAYER
  double xPos = tsPosition.x;
  UNLOCK_PLAYER
  return xPos;
}

double BZRobotPlayer::botGetY()
{
  LOCK_PLAYER
  double yPos = tsPosition.y;
  UNLOCK_PLAYER
  return yPos;
}

double BZRobotPlayer::botGetZ()
{
  LOCK_PLAYER
  double zPos = tsPosition.z;
  UNLOCK_PLAYER
  return zPos;
}

bool BZRobotPlayer::botIsAdjustGunForRobotTurn()
{
  return true;
}

bool BZRobotPlayer::botIsAdjustRadarForGunTurn()
{
  return true;
}

bool BZRobotPlayer::botIsAdjustRadarForRobotTurn()
{
  return true;
}

void BZRobotPlayer::botResume()
{
  botSetResume();
  botExecute();
}

void BZRobotPlayer::botScan()
{
  botExecute();
}

void BZRobotPlayer::botSetAdjustGunForRobotTurn(bool /*independent*/)
{
}

void BZRobotPlayer::botSetAdjustRadarForGunTurn(bool /*independent*/)
{
}

void BZRobotPlayer::botSetAdjustRadarForRobotTurn(bool /*independent*/)
{
}

void BZRobotPlayer::botSetAhead(double distance)
{
  LOCK_PLAYER
  tsNextDistance = distance;
  tsPendingUpdates[BZRobotPlayer::distanceUpdate] = true;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetBack(double distance)
{
  LOCK_PLAYER
  tsNextDistance = -distance;
  tsPendingUpdates[BZRobotPlayer::distanceUpdate] = true;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetFire(double /*power*/)
{
  LOCK_PLAYER
  tsShoot = true;
  UNLOCK_PLAYER
}

BZRobots::Bullet* BZRobotPlayer::botSetFireBullet(double /*power*/)
{
  LOCK_PLAYER
  tsShoot = true;
  UNLOCK_PLAYER
  // TODO: Make a bullet and return it
  return NULL;
}

void BZRobotPlayer::botSetMaxTurnRate(double turnRate)
{
  LOCK_PLAYER
  tsNextTurnRate = turnRate * M_PI/180.0f;
  tsPendingUpdates[BZRobotPlayer::turnRateUpdate] = true;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetMaxVelocity(double speed)
{
  LOCK_PLAYER
  tsNextSpeed = speed;
  tsPendingUpdates[BZRobotPlayer::speedUpdate] = true;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetResume()
{
  LOCK_PLAYER
  if (tsHasStopped) {
    tsHasStopped = false;
    tsDistanceRemaining = tsStoppedDistance;
    tsTurnRemaining = tsStoppedTurn;
    tsDistanceForward = tsStoppedForward;
    tsTurnLeft = tsStoppedLeft;
  }
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetStop(bool overwrite)
{
  LOCK_PLAYER
  if (!tsHasStopped || overwrite) {
    tsHasStopped = true;
    tsStoppedDistance = tsDistanceRemaining;
    tsStoppedTurn = tsTurnRemaining;
    tsStoppedForward = tsDistanceForward;
    tsStoppedLeft = tsTurnLeft;
  }
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetTurnLeft(double turn)
{
  LOCK_PLAYER
  tsPendingUpdates[BZRobotPlayer::turnUpdate] = true;
  tsNextTurn = turn * M_PI/180.0f;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetTurnLeftRadians(double turn)
{
  LOCK_PLAYER
  tsPendingUpdates[BZRobotPlayer::turnUpdate] = true;
  tsNextTurn = turn;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetTurnRight(double turn)
{
  LOCK_PLAYER
  tsPendingUpdates[BZRobotPlayer::turnUpdate] = true;
  tsNextTurn = -turn * M_PI/180.0f;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetTurnRightRadians(double turn)
{
  LOCK_PLAYER
  tsPendingUpdates[BZRobotPlayer::turnUpdate] = true;
  tsNextTurn = -turn;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botStop(bool overwrite)
{
  botSetStop(overwrite);
  botExecute();
}

void BZRobotPlayer::botTurnGunLeft(double /*turn*/)
{
}

void BZRobotPlayer::botTurnGunLeftRadians(double /*turn*/)
{
}

void BZRobotPlayer::botTurnGunRight(double /*turn*/)
{
}

void BZRobotPlayer::botTurnGunRightRadians(double /*turn*/)
{
}

void BZRobotPlayer::botTurnLeft(double turn)
{
  botSetTurnLeft(turn);
  botExecute();
  while(botGetTurnRemaining() > 0.0f)
    botDoNothing();
}

void BZRobotPlayer::botTurnLeftRadians(double turn)
{
  botSetTurnLeftRadians(turn);
  botExecute();
  while(botGetTurnRemaining() > 0.0f)
    botDoNothing();
}

void BZRobotPlayer::botTurnRadarLeft(double /*turn*/)
{
}

void BZRobotPlayer::botTurnRadarLeftRadians(double /*turn*/)
{
}

void BZRobotPlayer::botTurnRadarRight(double /*turn*/)
{
}

void BZRobotPlayer::botTurnRadarRightRadians(double /*turn*/)
{
}

void BZRobotPlayer::botTurnRight(double turn)
{
  botSetTurnRight(turn);
  botExecute();
  while(botGetTurnRemaining() < 0.0f)
    botDoNothing();
}

void BZRobotPlayer::botTurnRightRadians(double turn)
{
  botSetTurnRightRadians(turn);
  botExecute();
  while(botGetTurnRemaining() < 0.0f)
    botDoNothing();
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
