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
#include "RobotPlayer.h"

/* common interface headers */
#include "ServerLink.h"

/* local interface headers */
#include "Region.h"
#include "RegionPriorityQueue.h"

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

/**
 * BZFlag Robot Player
 */
class BZRobotPlayer : public RobotPlayer
{
public:
  BZRobotPlayer(const PlayerId&, const char* name, ServerLink*);

  typedef enum {
    speedUpdate,
    turnRateUpdate,
    distanceUpdate,
    turnUpdate,
    updateCount
  } variableUpdates;
  
  float           getReloadTime();
  void            addShot(ShotPath *shot, const FiringInfo &info);
  void            updateShot(FiringInfo &info, int shotID, double time );

  void            restart(const double* pos, double azimuth);
  void            explodeTank();
  
private:
  void            doUpdate(float dt);
  void            doUpdateMotion(float dt);
public:
  bool            pendingUpdates[updateCount];
  
  double          lastTickAt;
  double          tickDuration;
  double          speed, nextSpeed;
  double          turnRate, nextTurnRate;
  bool            shoot;
  
  double          distanceRemaining, nextDistance;
  bool            distanceForward, turnLeft;
  double          turnRemaining, nextTurn;
  
  bool            hasStopped;
  double          stoppedDistance, stoppedTurn;
  bool            stoppedForward, stoppedLeft;
  
private:
#if defined(HAVE_PTHREADS)
  pthread_mutex_t player_lock;
#elif defined(_WIN32) 
  CRITICAL_SECTION player_lock;
#endif
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
