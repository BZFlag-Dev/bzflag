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

#ifndef __BZROBOT_H__
#define __BZROBOT_H__

#include "BZAdvancedRobot.h"


/**
 * BZRobot: A class for simulation and implementation of a Frontend
 * robot.
 */
class BZRobot :public BZAdvancedRobot
{
protected:
  void ahead(double distance);
  void doNothing();
  void fire();
  // TODO: Implement 'Bullet fireBullet();' ?
  void resume();
  void stop();
  void stop(bool overwrite);
  // TODO: void scan(); ?
  void turnLeft(double degrees);
};

#else
class BZRobot;
#endif /* __BZROBOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
