/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
				ServerLink* _server, RCLink* _agent,
				const char* _email = "anonymous") :
				RobotPlayer(_id, _name, _server, _email),
				agent(_agent),
				speed(0.0),
				angularvel(0.0),
				accelx(0.0),
				accely(0.0),
				shoot(false)
{
}


void			RCRobotPlayer::doUpdate(float dt)
{
  LocalPlayer::doUpdate(dt);
}

void			RCRobotPlayer::doUpdateMotion(float dt)
{
  if (isAlive()) {
    if (!BZDB.isTrue("hoverbot")) {
      setDesiredSpeed(speed);
    } else {
      setDesiredAccelX(accelx);
      setDesiredAccelY(accely);
    }
    setDesiredAngVel(angularvel);
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

void			RCRobotPlayer::processrequest(RCRequest* req,
							    RCLink* link)
{
  switch (req->get_request_type()) {
    case Speed:
      if (!BZDB.isTrue("hoverbot")) {
	speed = req->speed_level;
	link->respond("ok\n");
      } else {
	link->respond("fail speed not allowed (hoverbot-only)\n");
      }
      break;

    case AngularVel:
      angularvel = req->angularvel_level;
      link->respond("ok\n");
      break;

    case AccelX:
      if (BZDB.isTrue("hoverbot")) {
	accelx = req->accelx_level;
	link->respond("ok\n");
      } else {
	link->respond("fail accelx not allowed (not a hoverbot)\n");
      }
      break;

    case AccelY:
      if (BZDB.isTrue("hoverbot")) {
	accely = req->accely_level;
	link->respond("ok\n");
      } else {
	link->respond("fail accely not allowed (not a hoverbot)\n");
      }
      break;

    case Shoot:
      shoot = true;
      if (fireShot()) {
	link->respond("ok\n");
      } else {
	link->respond("fail\n");
      }
      break;

    default:
      break;
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

