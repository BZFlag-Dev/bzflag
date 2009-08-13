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
#include "BZAdvancedRobot.h"

/* implementation headers */
#include "BZRobotCallbacks.h"

#define BOT_CALLBACKS ((BZRobotCallbacks *)bzrobotcb)
#define BOT_CLIENT_PLAYER ((BZRobotCallbacks *)bzrobotcb)->data

BZAdvancedRobot::BZAdvancedRobot()
{
  robotType = "BZAdvancedRobot";
}

BZAdvancedRobot::~BZAdvancedRobot()
{

}

void BZAdvancedRobot::execute()
{
  if(bzrobotcb && BOT_CALLBACKS->Execute)
    BOT_CALLBACKS->Execute(BOT_CLIENT_PLAYER);
}

double BZAdvancedRobot::getDistanceRemaining() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetDistanceRemaining)
    return BOT_CALLBACKS->GetDistanceRemaining(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZAdvancedRobot::getTurnRemaining() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetTurnRemaining)
    return BOT_CALLBACKS->GetTurnRemaining(BOT_CLIENT_PLAYER);
  return 0.0;
}

void BZAdvancedRobot::setAhead(double distance)
{
  if(bzrobotcb && BOT_CALLBACKS->SetAhead)
    BOT_CALLBACKS->SetAhead(BOT_CLIENT_PLAYER,distance);
}

void BZAdvancedRobot::setFire()
{
  if(bzrobotcb && BOT_CALLBACKS->SetFire)
    BOT_CALLBACKS->SetFire(BOT_CLIENT_PLAYER);
}

void BZAdvancedRobot::setTurnRate(double turnRate)
{
  if(bzrobotcb && BOT_CALLBACKS->SetTurnRate)
    BOT_CALLBACKS->SetTurnRate(BOT_CLIENT_PLAYER,turnRate);
}

void BZAdvancedRobot::setMaxVelocity(double maxVelocity)
{
  if(bzrobotcb && BOT_CALLBACKS->SetMaxVelocity)
    BOT_CALLBACKS->SetMaxVelocity(BOT_CLIENT_PLAYER,maxVelocity);
}

void BZAdvancedRobot::setResume()
{
  if(bzrobotcb && BOT_CALLBACKS->SetResume)
    BOT_CALLBACKS->SetResume(BOT_CLIENT_PLAYER);
}

void BZAdvancedRobot::setStop()
{
  setStop(false);
}

void BZAdvancedRobot::setStop(bool overwrite)
{
  if(bzrobotcb && BOT_CALLBACKS->SetStop)
    BOT_CALLBACKS->SetStop(BOT_CLIENT_PLAYER,overwrite);
}

void BZAdvancedRobot::setTurnLeft(double degrees)
{
  if(bzrobotcb && BOT_CALLBACKS->SetTurnLeft)
    BOT_CALLBACKS->SetTurnLeft(BOT_CLIENT_PLAYER,degrees);
}

double BZAdvancedRobot::getBearing(double x, double y) const
{
  double vec[2] = {x - getX(), y - getY()};
  double bearing = 0.0;

  if (vec[0] == 0 && vec[1] == 0)
    return 0.0;

  // Convert to a unit vector.
  double len = 1.0 / sqrt(vec[0]*vec[0] + vec[1]*vec[1]);
  vec[0] *= len;
  vec[1] *= len;
  
  bearing = atan2(vec[1], vec[0])*180.0/M_PI - getHeading();

  while(bearing > 180.0)
    bearing -= 360.0;
  while(bearing < -180)
    bearing += 360.0;
  return bearing;
}

double BZAdvancedRobot::getDistance(double x, double y) const
{
  double x0, y0;
  x0 = getX() - x;
  y0 = getY() - y;
  return sqrt(x0*x0 + y0*y0);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
