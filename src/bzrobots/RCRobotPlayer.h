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

/*
 * Remote Control Robot Player
 */

#ifndef	BZF_TCP_RC_ROBOT_PLAYER_H
#define	BZF_TCP_RC_ROBOT_PLAYER_H

#include "common.h"

/* system interface headers */
#include <vector>

/* interface header */
#include "RobotPlayer.h"

/* local interface headers */
#include "Region.h"
#include "RegionPriorityQueue.h"
#include "ServerLink.h"
#include "RCLinkBackend.h"


class RCRobotPlayer : public RobotPlayer {
  public:
    RCRobotPlayer(const PlayerId&,
        const char* name, ServerLink*,
        const char* _email);
    typedef enum {
      speedUpdate,
      turnRateUpdate,
      distanceUpdate,
      turnUpdate,
      updateCount
    } variableUpdates;

    void            restart(const float* pos, float azimuth);
    void            explodeTank();

    bool            isSteadyState();

    bool            pendingUpdates[updateCount];

    double          lastTickAt;
    double          tickDuration;
    float           speed, nextSpeed;
    float           turnRate, nextTurnRate;
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

#endif // BZF_TCP_RC_ROBOT_PLAYER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
