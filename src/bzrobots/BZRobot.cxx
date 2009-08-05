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
  memset(&bzrobotcb,0,sizeof(BZRobotCallbacks));
  robotType = "BZRobot";
}

BZRobot::~BZRobot()
{

}

void BZRobot::setCallbacks(BZRobotCallbacks _bzrobotcb)
{
  bzrobotcb = _bzrobotcb;
}

double BZRobot::getBattleFieldSize() const
{
  return bzrobotcb.GetBattleFieldSize(bzrobotcb.data);
}

double BZRobot::getGunHeat() const
{
  return bzrobotcb.GetGunHeat(bzrobotcb.data);
}

double BZRobot::getVelocity() const
{
  return bzrobotcb.GetVelocity(bzrobotcb.data);
}

double BZRobot::getHeading() const
{
  return bzrobotcb.GetHeading(bzrobotcb.data);
}

double BZRobot::getHeight() const
{
  return bzrobotcb.GetHeight(bzrobotcb.data);
}

double BZRobot::getWidth() const
{
  return bzrobotcb.GetWidth(bzrobotcb.data);
}

double BZRobot::getLength() const
{
  return bzrobotcb.GetLength(bzrobotcb.data);
}

long BZRobot::getTime() const
{
  return bzrobotcb.GetTime(bzrobotcb.data);
}

double BZRobot::getX() const
{
  return bzrobotcb.GetX(bzrobotcb.data);
}

double BZRobot::getY() const
{
  return bzrobotcb.GetY(bzrobotcb.data);
}

double BZRobot::getZ() const
{
  return bzrobotcb.GetZ(bzrobotcb.data);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
