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

#include <string.h>

#include "BZRobot.h"

BZRobot::BZRobot()
{
  bzrobotcb = NULL;
  robotType = "BZRobot";
}

BZRobot::~BZRobot()
{
  delete bzrobotcb;
}

void BZRobot::setCallbacks(BZRobotCallbacks *_bzrobotcb)
{
  bzrobotcb = _bzrobotcb;
}


void BZRobot::ahead(double distance)
{
  if(bzrobotcb && bzrobotcb->Ahead)
    bzrobotcb->Ahead(bzrobotcb->data,distance);
}

void BZRobot::back(double distance)
{
  if(bzrobotcb && bzrobotcb->Back)
    bzrobotcb->Back(bzrobotcb->data,distance);
}

void BZRobot::fire()
{
  if(bzrobotcb && bzrobotcb->Fire)
    bzrobotcb->Fire(bzrobotcb->data);
}

double BZRobot::getBattleFieldSize() const
{
  if(bzrobotcb && bzrobotcb->GetBattleFieldSize)
    return bzrobotcb->GetBattleFieldSize(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getGunCoolingRate() const
{
  if(bzrobotcb && bzrobotcb->GetGunCoolingRate)
    return bzrobotcb->GetGunCoolingRate(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getGunHeat() const
{
  if(bzrobotcb && bzrobotcb->GetGunHeat)
    return bzrobotcb->GetGunHeat(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getHeading() const
{
  if(bzrobotcb && bzrobotcb->GetHeading)
    return bzrobotcb->GetHeading(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getHeight() const
{
  if(bzrobotcb && bzrobotcb->GetHeight)
    return bzrobotcb->GetHeight(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getLength() const
{
  if(bzrobotcb && bzrobotcb->GetLength)
    return bzrobotcb->GetLength(bzrobotcb->data);
  return 0.0;
}

const char * BZRobot::getName() const
{
  if(bzrobotcb && bzrobotcb->GetName)
    return bzrobotcb->GetName(bzrobotcb->data);
  return "";
}

double BZRobot::getTime() const
{
  if(bzrobotcb && bzrobotcb->GetTime)
    return bzrobotcb->GetTime(bzrobotcb->data);
  return 0;
}

double BZRobot::getWidth() const
{
  if(bzrobotcb && bzrobotcb->GetWidth)
    return bzrobotcb->GetWidth(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getVelocity() const
{
  if(bzrobotcb && bzrobotcb->GetVelocity)
    return bzrobotcb->GetVelocity(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getX() const
{
  if(bzrobotcb && bzrobotcb->GetX)
    return bzrobotcb->GetX(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getY() const
{
  if(bzrobotcb && bzrobotcb->GetY)
    return bzrobotcb->GetY(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getZ() const
{
  if(bzrobotcb && bzrobotcb->GetZ)
    return bzrobotcb->GetZ(bzrobotcb->data);
  return 0.0;
}

void BZRobot::resume()
{
  if(bzrobotcb && bzrobotcb->Resume)
    bzrobotcb->Resume(bzrobotcb->data);
}

void BZRobot::scan()
{
  if(bzrobotcb && bzrobotcb->Scan)
    bzrobotcb->Scan(bzrobotcb->data);
}

void BZRobot::stop(bool overwrite)
{
  if(bzrobotcb && bzrobotcb->Stop)
    bzrobotcb->Stop(bzrobotcb->data,overwrite);
}

void BZRobot::turnLeft(double degrees)
{
  if(bzrobotcb && bzrobotcb->TurnLeft)
    bzrobotcb->TurnLeft(bzrobotcb->data,degrees);
}

void BZRobot::turnRight(double degrees)
{
  if(bzrobotcb && bzrobotcb->TurnRight)
    bzrobotcb->TurnRight(bzrobotcb->data,degrees);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
