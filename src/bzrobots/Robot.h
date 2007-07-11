/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Robot: A class for simulation and implementation of a simpler
 * RoboCode-supporting robot.
 */

#ifndef BZROBOTS_ROBOT_H
#define BZROBOTS_ROBOT_H

#include "BZRobot.h"

class Robot :public BZRobot
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
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
