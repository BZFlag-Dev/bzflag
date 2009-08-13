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
#include "BZRobot.h"

/* implementation headers */
#include "BZRobotCallbacks.h"

#define BOT_CALLBACKS ((BZRobotCallbacks *)bzrobotcb)
#define BOT_CLIENT_PLAYER ((BZRobotCallbacks *)bzrobotcb)->data


BZRobot::BZRobot()
{
  bzrobotcb = NULL;
  robotType = "BZRobot";
}

BZRobot::~BZRobot()
{
}

void BZRobot::setCallbacks(void *_bzrobotcb)
{
  bzrobotcb = _bzrobotcb;
}


void BZRobot::ahead(double distance)
{
  if(bzrobotcb && BOT_CALLBACKS->Ahead)
    BOT_CALLBACKS->Ahead(BOT_CLIENT_PLAYER,distance);
}

void BZRobot::back(double distance)
{
  if(bzrobotcb && BOT_CALLBACKS->Back)
    BOT_CALLBACKS->Back(BOT_CLIENT_PLAYER,distance);
}

void BZRobot::doNothing()
{
  if(bzrobotcb && BOT_CALLBACKS->DoNothing)
    BOT_CALLBACKS->DoNothing(BOT_CLIENT_PLAYER);
}

void BZRobot::fire()
{
  if(bzrobotcb && BOT_CALLBACKS->Fire)
    BOT_CALLBACKS->Fire(BOT_CLIENT_PLAYER);
}

double BZRobot::getBattleFieldSize() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetBattleFieldSize)
    return BOT_CALLBACKS->GetBattleFieldSize(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobot::getGunCoolingRate() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetGunCoolingRate)
    return BOT_CALLBACKS->GetGunCoolingRate(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobot::getGunHeat() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetGunHeat)
    return BOT_CALLBACKS->GetGunHeat(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobot::getHeading() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetHeading)
    return BOT_CALLBACKS->GetHeading(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobot::getHeight() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetHeight)
    return BOT_CALLBACKS->GetHeight(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobot::getLength() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetLength)
    return BOT_CALLBACKS->GetLength(BOT_CLIENT_PLAYER);
  return 0.0;
}

const char * BZRobot::getName() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetName)
    return BOT_CALLBACKS->GetName(BOT_CLIENT_PLAYER);
  return "";
}

double BZRobot::getTime() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetTime)
    return BOT_CALLBACKS->GetTime(BOT_CLIENT_PLAYER);
  return 0;
}

double BZRobot::getWidth() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetWidth)
    return BOT_CALLBACKS->GetWidth(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobot::getVelocity() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetVelocity)
    return BOT_CALLBACKS->GetVelocity(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobot::getX() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetX)
    return BOT_CALLBACKS->GetX(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobot::getY() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetY)
    return BOT_CALLBACKS->GetY(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobot::getZ() const
{
  if(bzrobotcb && BOT_CALLBACKS->GetZ)
    return BOT_CALLBACKS->GetZ(BOT_CLIENT_PLAYER);
  return 0.0;
}

void BZRobot::resume()
{
  if(bzrobotcb && BOT_CALLBACKS->Resume)
    BOT_CALLBACKS->Resume(BOT_CLIENT_PLAYER);
}

void BZRobot::scan()
{
  if(bzrobotcb && BOT_CALLBACKS->Scan)
    BOT_CALLBACKS->Scan(BOT_CLIENT_PLAYER);
}

void BZRobot::stop(bool overwrite)
{
  if(bzrobotcb && BOT_CALLBACKS->Stop)
    BOT_CALLBACKS->Stop(BOT_CLIENT_PLAYER,overwrite);
}

void BZRobot::turnLeft(double degrees)
{
  if(bzrobotcb && BOT_CALLBACKS->TurnLeft)
    BOT_CALLBACKS->TurnLeft(BOT_CLIENT_PLAYER,degrees);
}

void BZRobot::turnRight(double degrees)
{
  if(bzrobotcb && BOT_CALLBACKS->TurnRight)
    BOT_CALLBACKS->TurnRight(BOT_CLIENT_PLAYER,degrees);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
