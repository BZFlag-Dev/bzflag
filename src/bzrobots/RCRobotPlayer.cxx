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

// interface header
#include "RCRobotPlayer.h"

// common implementation headers
#include "BZDBCache.h"

// local implementation headers
#include "World.h"
#include "Intersect.h"
#include "TargetingUtils.h"

RCRobotPlayer::RCRobotPlayer(const PlayerId& _id, const char* _name,
				ServerLink* _server, RCLinkBackend* _agent,
				const char* _email = "anonymous") :
				RobotPlayer(_id, _name, _server, _email),
				agent(_agent),
                                lastTickAt(0.0), tickDuration(2.0),
				speed(1.0), nextSpeed(1.0),
				turnRate(1.0), nextTurnRate(1.0),
				shoot(false),
                                distanceRemaining(0.0), nextDistance(0.0),
                                turnRemaining(0.0), nextTurn(0.0)
{
}


void			RCRobotPlayer::doUpdate(float dt)
{
  LocalPlayer::doUpdate(dt);
}

void			RCRobotPlayer::doUpdateMotion(float dt)
{
  if (isAlive()) {
    double timeNow = TimeKeeper::getCurrent().getSeconds();
    /* Is the tick still running? */
    if (lastTickAt + tickDuration >= timeNow)
    {
      if (distanceRemaining > 0.0f)
      {
        if (distanceForward)
        {
          setDesiredSpeed(speed);
          distanceRemaining -= *getVelocity() * dt;
        }
        else
        {
          setDesiredSpeed(-speed);
          distanceRemaining += *getVelocity() * dt;
        }
      }
      else
      {
          setDesiredSpeed(0);
      }

      if (turnRemaining > 0.0f)
      {
        if (turnLeft)
        {
          setDesiredAngVel(turnRate);
          turnRemaining -= getAngularVelocity() * dt;
        }
        else
        {
          setDesiredAngVel(-turnRate);
          turnRemaining += getAngularVelocity() * dt;
        }
      }
      else
      {
          setDesiredAngVel(0);
      }
    }
    else
    {
      setDesiredAngVel(0);
      setDesiredSpeed(0);
    }

  }
  LocalPlayer::doUpdateMotion(dt);
}

void			RCRobotPlayer::explodeTank()
{
  LocalPlayer::explodeTank();
}

void			RCRobotPlayer::restart(const float* pos, float _azimuth)
{
  LocalPlayer::restart(pos, _azimuth);
}

bool                    RCRobotPlayer::isSteadyState()
{
    double timeNow = TimeKeeper::getCurrent().getSeconds();
    /* last tick done? */
    if (lastTickAt + tickDuration >= timeNow)
      return true;
    return false;
}

bool			RCRobotPlayer::processrequest(RCRequest* req,
							    RCLinkBackend* link)
{
  receivedUpdates[req->get_request_type()] = true;
  switch (req->get_request_type()) {
    case setSpeed:
      nextSpeed = req->speed;
      link->send("ok\n");
      break;

    case setTurnRate:
      nextTurnRate = req->turnRate;
      link->send("ok\n");
      break;

    case setFire:
      shoot = true;
      link->send("ok\n");
      break;

    case getGunHeat:
      if (isSteadyState())
        return false;
      link->sendf("getGunHeat %f\n", getReloadTime());
      break;
    
    case setAhead:
      nextDistance = req->distance;
      link->send("ok\n");
      break;

    case setTurnLeft:
      nextTurn = req->turn;
      link->send("ok\n");
      break;

    case getDistanceRemaining:
      if (isSteadyState())
        return false;
      link->sendf("getDistanceRemaining %f\n", distanceRemaining);
      break;

    case getTurnRemaining:
      if (isSteadyState())
        return false;
      link->sendf("getTurnRemaining %f\n", turnRemaining);
      break;

    case getTickDuration:
      link->sendf("getTickDuration %f\n", tickDuration);
      break;

    case setTickDuration:
      tickDuration = req->duration;
      break;

    case getTickRemaining:
      if (isSteadyState())
        link->sendf("getTickRemaining %f\n", (lastTickAt + tickDuration) - TimeKeeper::getCurrent().getSeconds());
      else
        link->send("getTickRemaining 0.0\n");
      break;

    case execute:
      if (isSteadyState())
        return false;

      lastTickAt = TimeKeeper::getCurrent().getSeconds();

      if (receivedUpdates[setTurnLeft])
      {
        turnRemaining = nextTurn;
        if (turnRemaining < 0.0f)
        {
          turnRemaining = -turnRemaining;
          turnLeft = false;
        }
        else
          turnLeft = true;
      }

      if (receivedUpdates[setAhead])
      {
        distanceRemaining = nextDistance;
        if (distanceRemaining < 0.0f)
        {
          distanceRemaining = -distanceRemaining;
          distanceForward = false;
        }
        else
          distanceForward = true;
      }

      if (receivedUpdates[setTurnRate])
        turnRate = nextTurnRate;
      if (receivedUpdates[setSpeed])
        speed = nextSpeed;

      for (int i = 0; i < RequestCount; ++i)
        receivedUpdates[i] = false;

      if (shoot)
      {
        shoot = false;
        fireShot();
      }

      break;

    default:
      break;
  }
  return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
