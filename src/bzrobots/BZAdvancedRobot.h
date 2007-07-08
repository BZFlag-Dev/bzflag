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
 * BZAdvancedRobot: A class for simulation and implementation of a Frontend advanced robot.
 */

#ifndef BZROBOTS_BZADVANCEDROBOT_H
#define BZROBOTS_BZADVANCEDROBOT_H

#include "RCLinkFrontend.h"

class RCLinkFrontend;

class BZAdvancedRobot
{
  RCLinkFrontend *link;
  protected:
    void execute();
    double getDistanceRemaining();
    double getTurnRemaining();
    double setAhead(double distance);
    void setFire();
    // TODO: Implement 'Bullet setFireBullet()'?
    void setTurnRate(double turnRate);
    void setSpeed(double speed);
    void setResume();
    void setStop();
    void setStop(bool overwrite);
    void setTurnLeft(double degrees);

    // These are normally in Robot and not AdvancedRobot, but due to
    // the upside-down hierarchy we have - they're here instead ;-)
    double getBattleFieldHeight();
    double getBattleFieldWidth();
    double getGunHeat();
    double getHeading();
    double getHeight();
    double getWidth();
    int getOthers();
    long getTime();
    double getVelocity();
    double getX();
    double getY();

  public:
    virtual void run() = 0;
    BZAdvancedRobot(RCLinkFrontend *_link) :link(_link) {}
    virtual ~BZAdvancedRobot() {}
    
    float gunHeat;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
