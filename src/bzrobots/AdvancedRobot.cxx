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
#include "AdvancedRobot.h"

/* implementation headers */
#include "RobotCallbacks.h"

#define BOT_CALLBACKS ((RobotCallbacks *)robotcb)
#define BOT_CLIENT_PLAYER ((RobotCallbacks *)robotcb)->data

BZRobots::AdvancedRobot::AdvancedRobot()
{
  robotType = "AdvancedRobot";
}

BZRobots::AdvancedRobot::~AdvancedRobot()
{

}

void BZRobots::AdvancedRobot::clearAllEvents()
{
  if(robotcb && BOT_CALLBACKS->ClearAllEvents)
    BOT_CALLBACKS->ClearAllEvents(BOT_CLIENT_PLAYER);
}

void BZRobots::AdvancedRobot::execute()
{
  if(robotcb && BOT_CALLBACKS->Execute)
    BOT_CALLBACKS->Execute(BOT_CLIENT_PLAYER);
}

double BZRobots::AdvancedRobot::getDistanceRemaining() const
{
  if(robotcb && BOT_CALLBACKS->GetDistanceRemaining)
    return BOT_CALLBACKS->GetDistanceRemaining(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobots::AdvancedRobot::getTurnRemaining() const
{
  if(robotcb && BOT_CALLBACKS->GetTurnRemaining)
    return BOT_CALLBACKS->GetTurnRemaining(BOT_CLIENT_PLAYER);
  return 0.0;
}

void BZRobots::AdvancedRobot::setAhead(double distance)
{
  if(robotcb && BOT_CALLBACKS->SetAhead)
    BOT_CALLBACKS->SetAhead(BOT_CLIENT_PLAYER,distance);
}

void BZRobots::AdvancedRobot::setBack(double distance)
{
  if(robotcb && BOT_CALLBACKS->SetBack)
    BOT_CALLBACKS->SetBack(BOT_CLIENT_PLAYER,distance);
}

void BZRobots::AdvancedRobot::setFire(double power)
{
  if(robotcb && BOT_CALLBACKS->SetFire)
    BOT_CALLBACKS->SetFire(BOT_CLIENT_PLAYER,power);
}

BZRobots::Bullet* BZRobots::AdvancedRobot::setFireBullet(double power) const
{
  if(robotcb && BOT_CALLBACKS->SetFireBullet)
    return BOT_CALLBACKS->SetFireBullet(BOT_CLIENT_PLAYER,power);
  return NULL;
}

void BZRobots::AdvancedRobot::setMaxVelocity(double maxVelocity)
{
  if(robotcb && BOT_CALLBACKS->SetMaxVelocity)
    BOT_CALLBACKS->SetMaxVelocity(BOT_CLIENT_PLAYER,maxVelocity);
}

void BZRobots::AdvancedRobot::setMaxTurnRate(double maxTurnRate)
{
  if(robotcb && BOT_CALLBACKS->SetMaxTurnRate)
    BOT_CALLBACKS->SetMaxTurnRate(BOT_CLIENT_PLAYER,maxTurnRate);
}

void BZRobots::AdvancedRobot::setResume()
{
  if(robotcb && BOT_CALLBACKS->SetResume)
    BOT_CALLBACKS->SetResume(BOT_CLIENT_PLAYER);
}

void BZRobots::AdvancedRobot::setStop(bool overwrite)
{
  if(robotcb && BOT_CALLBACKS->SetStop)
    BOT_CALLBACKS->SetStop(BOT_CLIENT_PLAYER,overwrite);
}

void BZRobots::AdvancedRobot::setTurnLeft(double degrees)
{
  if(robotcb && BOT_CALLBACKS->SetTurnLeft)
    BOT_CALLBACKS->SetTurnLeft(BOT_CLIENT_PLAYER,degrees);
}

void BZRobots::AdvancedRobot::setTurnRate(double turnRate)
{
  if(robotcb && BOT_CALLBACKS->SetTurnRate)
    BOT_CALLBACKS->SetTurnRate(BOT_CLIENT_PLAYER,turnRate);
}

void BZRobots::AdvancedRobot::setTurnRight(double degrees)
{
  if(robotcb && BOT_CALLBACKS->SetTurnRight)
    BOT_CALLBACKS->SetTurnRight(BOT_CLIENT_PLAYER,degrees);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
