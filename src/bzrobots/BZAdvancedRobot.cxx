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

BZAdvancedRobot::BZAdvancedRobot()
{
  robotType = "BZAdvancedRobot";
}

BZAdvancedRobot::~BZAdvancedRobot()
{

}

void BZAdvancedRobot::execute()
{
  if(bzrobotcb && bzrobotcb->Execute)
    bzrobotcb->Execute(bzrobotcb->data);
}

double BZAdvancedRobot::getDistanceRemaining() const
{
  if(bzrobotcb && bzrobotcb->GetDistanceRemaining)
    return bzrobotcb->GetDistanceRemaining(bzrobotcb->data);
  return 0.0;
}

double BZAdvancedRobot::getTurnRemaining() const
{
  if(bzrobotcb && bzrobotcb->GetTurnRemaining)
    return bzrobotcb->GetTurnRemaining(bzrobotcb->data);
  return 0.0;
}

void BZAdvancedRobot::setAhead(double distance)
{
  if(bzrobotcb && bzrobotcb->SetAhead)
    bzrobotcb->SetAhead(bzrobotcb->data,distance);
}

void BZAdvancedRobot::setFire()
{
  if(bzrobotcb && bzrobotcb->SetFire)
    bzrobotcb->SetFire(bzrobotcb->data);
}

void BZAdvancedRobot::setTurnRate(double turnRate)
{
  if(bzrobotcb && bzrobotcb->SetTurnRate)
    bzrobotcb->SetTurnRate(bzrobotcb->data,turnRate);
}

void BZAdvancedRobot::setMaxVelocity(double maxVelocity)
{
  if(bzrobotcb && bzrobotcb->SetMaxVelocity)
    bzrobotcb->SetMaxVelocity(bzrobotcb->data,maxVelocity);
}

void BZAdvancedRobot::setResume()
{
  if(bzrobotcb && bzrobotcb->SetResume)
    bzrobotcb->SetResume(bzrobotcb->data);
}

void BZAdvancedRobot::setStop()
{
  setStop(false);
}

void BZAdvancedRobot::setStop(bool overwrite)
{
  if(bzrobotcb && bzrobotcb->SetStop)
    bzrobotcb->SetStop(bzrobotcb->data,overwrite);
}

void BZAdvancedRobot::setTurnLeft(double degrees)
{
  if(bzrobotcb && bzrobotcb->SetTurnLeft)
    bzrobotcb->SetTurnLeft(bzrobotcb->data,degrees);
}

void BZAdvancedRobot::setTickDuration(double duration)
{
  if(bzrobotcb && bzrobotcb->SetTickDuration)
    bzrobotcb->SetTickDuration(bzrobotcb->data,duration);
}

double BZAdvancedRobot::getBearing(const Tank &tank) const
{
  return getBearing(tank.position[0], tank.position[1]);
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

double BZAdvancedRobot::getDistance(const Tank &tank) const
{
  return getDistance(tank.position[0], tank.position[1]);
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
