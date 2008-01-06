/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __ROBOT_H__
#define __ROBOT_H__

/* local interface headers */
#include "BZRobot.h"


/**
 * Robot: A class for simulation and implementation of a simpler
 * RoboCode-supporting robot.
 */
class Robot : public BZRobot
{
protected:
  void back(double distance);
  void fire(double power);
  // TODO: Implement 'Bullet fireBullet(double power);' ?

  double getEnergy();
  double getGunCoolingRate(); // This should return the same as getTickDuration. :-)
  double getGunHeading();
  double getRadarHeading();

  int getNumRounds();
  int getRoundNum();

  void setAdjustGunForRobotTurn(bool independent);
  void setAdjustRadarForGunTurn(bool independent);
  void setAdjustRadarForRobotTurn(bool independent);

  double getBattleFieldHeight();
  double getBattleFieldWidth();

  void turnGunRight(double degrees);
  void turnGunLeft(double degrees);
  void turnRadarRight(double degrees);
  void turnRadarLeft(double degrees);
  void turnRight(double degrees);

public:
  /* Robots behave slightly differently. The implementor defines run, including the while-loop. */
  void update() {}
  void initialize() {}
  virtual void run() = 0;
};

#endif /* __ROBOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
