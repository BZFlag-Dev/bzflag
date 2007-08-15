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
#include "RCEvents.h"

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

    /** These are event handlers, optional to implement them. **/
    virtual void onHitWall(const HitWallEvent &/*event*/) {}
    virtual void onDeath(const DeathEvent &/*event*/) {}
    //virtual void onSkippedTurn(const SkippedTurnEvent &/*event*/) {}
    //virtual void onBulletHit(const BulletHitEvent &/*event*/) {}
    //This method is called when one of your bullets hits another robot.
    //virtual void onBulletHitBullet(const BulletHitBulletEvent &/*event*/) {}
    //This method is called when one of your bullets hits another bullet.
    //virtual void onBulletMissed(const BulletMissedEvent &/*event*/) {}
    //This method is called when one of your bullets misses, i.e. hits a wall.
    //This method is called if your robot dies.
    //virtual void onHitByBullet(const HitByBulletEvent &/*event*/) {}
    //This method is called when your robot is hit by a bullet.
    //virtual void onHitRobot(const HitRobotEvent &/*event*/) {}
    //This method is called when your robot collides with another robot.
    //virtual void onRobotDeath(const RobotDeathEvent &/*event*/) {}
    //This method is called when another robot dies.
    //virtual void onScannedRobot(const ScannedRobotEvent &/*event*/) {}
    //This method is called when your robot sees another robot, i.e. when the robot's radar scan "hits" another robot.
    //virtual void onWin(const WinEvent &/*event*/) {}
    //This method is called if your robot wins a battle.

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
