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

/* interface header */
#include "FrontendShot.h"

/* bzflag implementation headers */
#include "Roster.h"

/* local implementation headers */
#include "MessageUtilities.h"
#include "RCRequest.h"
#include "RCRequests.h"

FrontendShot::FrontendShot() : Shot()
{
  robot = NULL;
}

void FrontendShot::setRobot(const BZAdvancedRobot *_robot)
{
  robot = _robot;
}

void FrontendShot::getPosition(double &tox, double &toy, double &toz, double dt) const
{
  RCLinkFrontend *link = robot->getLink();
  link->sendAndProcess(GetShotPositionReq(id,dt), robot);
  tox = x;
  toy = y;
  toz = z;
}

void FrontendShot::getVelocity(double &tox, double &toy, double &toz, double dt) const
{
  RCLinkFrontend *link = robot->getLink();
  link->sendAndProcess(GetShotVelocityReq(id,dt), robot);
  tox = vx;
  toy = vy;
  toz = vz;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
