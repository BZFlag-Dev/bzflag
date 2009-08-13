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

#ifndef	__RCROBOTPLAYER_H__
#define	__RCROBOTPLAYER_H__

#include "common.h"

/* system interface headers */
#include <vector>

/* interface header */
#include "BZRobotPlayer.h"

/* common interface headers */
#include "ServerLink.h"

/* local interface headers */
#include "BZRobot.h"
#include "Region.h"
#include "RegionPriorityQueue.h"
#include "RobotPlayer.h"

#if defined(HAVE_PTHREADS)
# define LOCK_PLAYER   pthread_mutex_lock(&player_lock);
# define UNLOCK_PLAYER pthread_mutex_unlock(&player_lock);
#elif defined(_WIN32) 
# define LOCK_PLAYER   EnterCriticalSection(&player_lock);
# define UNLOCK_PLAYER LeaveCriticalSection(&player_lock);
#else
# define LOCK_PLAYER
# define UNLOCK_PLAYER
#endif

#ifdef _WIN32
# define SLEEP_PLAYER(ms) Sleep(ms)
#else
# define SLEEP_PLAYER(ms) (ms > 1000 ? sleep(ms/1000) : usleep(ms*1000))
#endif

/**
 * BZFlag Robot Player
 */
class BZRobotPlayer : public RobotPlayer
{
public:
  BZRobotPlayer(const PlayerId&, const char* name, ServerLink*);
  ~BZRobotPlayer();

  typedef enum {
    speedUpdate,
    turnRateUpdate,
    distanceUpdate,
    turnUpdate,
    updateCount
  } variableUpdates;
  
  void setRobot(BZRobot *_robot);
  void pushEvent(BZRobotEvent e);
  void explodeTank();
  void restart(const fvec3& pos, float azimuth);
  void update(float inputDT);
  void doUpdate(float dt);
  void doUpdateMotion(float dt);

  void botAhead(double distance);
  void botBack(double distance);
  void botDoNothing();
  void botExecute();
  void botFire();
  double botGetDistanceRemaining();
  const char * botGetName();
  double botGetGunCoolingRate();
  double botGetBattleFieldSize();
  double botGetGunHeat();
  double botGetVelocity();
  double botGetHeading();
  double botGetWidth();
  double botGetLength();
  double botGetHeight();
  double botGetTime();
  double botGetX();
  double botGetY();
  double botGetZ();
  double botGetTurnRemaining();
  void botResume();
  void botScan();
  void botSetAhead(double distance);
  void botSetFire();
  void botSetTurnRate(double rate);
  void botSetMaxVelocity(double speed);
  void botSetResume();
  void botStop(bool overwrite);
  void botSetStop(bool overwrite);
  void botSetTurnLeft(double turn);
  void botTurnLeft(double turn);
  void botTurnRight(double turn);
  
private:
  double lastExec;

// Begin shared thread-safe variables
  BZRobot *robot;

  double tsBattleFieldSize;

  const std::string tsName;

  fvec3 tsTankSize;

  bool tsPendingUpdates[updateCount];
  std::list<BZRobotEvent> tsEventQueue; // non-prioritized

  double tsGunHeat;
  bool tsShoot;

  fvec3 tsPosition;
  double tsCurrentHeading;
  double tsCurrentSpeed, tsSpeed, tsNextSpeed;
  double tsCurrentTurnRate, tsTurnRate, tsNextTurnRate;
 
  double tsDistanceRemaining, tsNextDistance;
  bool tsDistanceForward, tsTurnLeft;
  double tsTurnRemaining, tsNextTurn;
 
  bool tsHasStopped;
  double tsStoppedDistance, tsStoppedTurn;
  bool tsStoppedForward, tsStoppedLeft;

  double tsShotReloadTime;

#if defined(HAVE_PTHREADS)
  pthread_mutex_t player_lock;
#elif defined(_WIN32) 
  CRITICAL_SECTION player_lock;
#endif
// End shared thread-safe variables
};

#else
class BZRobotPlayer;
#endif // __RCROBOTPLAYER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
