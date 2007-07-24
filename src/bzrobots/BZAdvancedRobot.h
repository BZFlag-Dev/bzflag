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
  bool compatability;
  protected:
    /* These are related to controlling the bot. */
    void execute();
    double getDistanceRemaining();
    double getTurnRemaining();
    void setAhead(double distance);
    void setFire();
    // TODO: Implement 'Bullet setFireBullet()'?
    void setTurnRate(double turnRate);
    void setSpeed(double speed);
    void setResume();
    void setStop();
    void setStop(bool overwrite);
    void setTurnLeft(double degrees);
    double getBattleFieldSize();

    // These are normally in Robot and not AdvancedRobot, but due to
    // the upside-down hierarchy we have - they're here instead ;-)
    double getGunHeat();
    double getHeading();
    double getHeight();
    double getWidth();
    double getLength();
    int getOthers();
    long getTime();
    double getVelocity();
    double getX();
    double getY();
    double getZ();

  public:
    virtual ~BZAdvancedRobot() {}
    BZAdvancedRobot();

    virtual void initialize() {}
    virtual void update() = 0;
    virtual void run();

    /* These are related to the internal state/setup of the bot. */
    bool getCompatability();
    void setCompatability(bool newState);
    
    void setLink(RCLinkFrontend *_link);
    
    float gunHeat, distanceRemaining, turnRemaining;
    float battleFieldSize;
    float yPosition, xPosition, zPosition;
    float tankWidth, tankLength, tankHeight;
    float heading;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
