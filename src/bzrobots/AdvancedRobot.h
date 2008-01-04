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

#ifndef __ADVANCEDROBOT_H__
#define __ADVANCEDROBOT_H__

/* interface header */
#include "BZAdvancedRobot.h"


/**
 * AdvancedRobot: A class for the simulation and implementation of
 * mostly RoboCode-compliant AdvancedRobot (see BZRobot for the
 * "normal" one).  AdvancedRobots behave slightly differently.  The
 * implementor defines run(), including the while-loop.
 */
class AdvancedRobot : public BZAdvancedRobot
{
protected:
  double getGunTurnRemaining();
  double getRadarTurnRemaining();
  bool isAdjustGunForRobotTurn();
  bool isAdjustRadarForRobotTurn();
  void setBack(double distance);
  void setFire(double power);
  // TODO: Implement 'Bullet setFireBullet(double power)'?
  void setMaxTurnRate(double maxTurnRate); // This needs to divide by the max, so we get something between 0 and 1, then pass to setTurnRate
  void setMaxSpeed(double maxSpeed); // This needs to divide by the max, so we get something between 0 and 1, then pass to setSpeed
  void setTurnGunLeft(double degrees);
  void setTurnGunRight(double degrees);
  void setTurnRadarLeft(double degrees);
  void setTurnRadarRight(double degrees);
  void setTurnRight(double degrees);
public:

  void update() {}
  void initialize() {}
  virtual void run() = 0;
};

#endif  /* __ADVANCEDROBOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
