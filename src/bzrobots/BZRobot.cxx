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
  printf("setting callbacks!\n");
  bzrobotcb = _bzrobotcb;
}

double BZRobot::getBattleFieldSize() const
{
  if(bzrobotcb && bzrobotcb->GetBattleFieldSize)
    return bzrobotcb->GetBattleFieldSize(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getGunHeat() const
{
  if(bzrobotcb && bzrobotcb->GetGunHeat)
    return bzrobotcb->GetGunHeat(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getVelocity() const
{
  if(bzrobotcb && bzrobotcb->GetVelocity)
    return bzrobotcb->GetVelocity(bzrobotcb->data);
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

double BZRobot::getWidth() const
{
  if(bzrobotcb && bzrobotcb->GetWidth)
    return bzrobotcb->GetWidth(bzrobotcb->data);
  return 0.0;
}

double BZRobot::getLength() const
{
  if(bzrobotcb && bzrobotcb->GetLength)
    return bzrobotcb->GetLength(bzrobotcb->data);
  return 0.0;
}

long BZRobot::getTime() const
{
  if(bzrobotcb && bzrobotcb->GetTime)
    return bzrobotcb->GetTime(bzrobotcb->data);
  return 0;
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


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
