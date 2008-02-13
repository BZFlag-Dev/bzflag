/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "RCLinkBackend.h"


/**
 * Remote Control Robot Player
 */
class RCRobotPlayer : public RobotPlayer
{
public:
  RCRobotPlayer(const PlayerId&,
		const char* name, ServerLink*,
		const char* _email = "anonymous");

  typedef enum {
    speedUpdate,
    turnRateUpdate,
    distanceUpdate,
    turnUpdate,
    updateCount
  } variableUpdates;
  
  void            restart(const double* pos, double azimuth);
  void            explodeTank();
  
  bool            isSteadyState();
  
  bool            pendingUpdates[updateCount];
  
  double          lastTickAt;
  double          tickDuration;
  double           speed, nextSpeed;
  double           turnRate, nextTurnRate;
  bool            shoot;
  
  double          distanceRemaining, nextDistance;
  bool            distanceForward, turnLeft;
  double          turnRemaining, nextTurn;
  
  bool            hasStopped;
  double          stoppedDistance, stoppedTurn;
  bool            stoppedForward, stoppedLeft;
  
private:
  void            doUpdate(float dt);
  void            doUpdateMotion(float dt);
};

#else
class RCRobotPlayer;
#endif // __RCROBOTPLAYER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
