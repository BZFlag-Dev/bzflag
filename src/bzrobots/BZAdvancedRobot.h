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

#ifndef __BZADVANCEDROBOT_H__
#define __BZADVANCEDROBOT_H__

#include "common.h"

/* system interface headers */
#include <vector>

/* local interface headers */
#include "RCLinkFrontend.h"
#include "RCEvents.h"
#include "Tank.h"
#include "Shot.h"
#include "Obstacle.h"


/**
 * BZAdvancedRobot: A class for simulation and implementation of a
 * Frontend advanced robot.
 */
class BZAdvancedRobot
{
private:
  mutable RCLinkFrontend *link;
  bool compatability;

public:
  /* These are related to controlling the bot. */
  void execute();
  double getDistanceRemaining() const;
  double getTurnRemaining() const;
  void setAhead(double distance);
  void setFire();
  // TODO: Implement 'Bullet setFireBullet()'?
  void setTurnRate(double turnRate);
  void setSpeed(double speed);
  void setResume();
  void setStop();
  void setStop(bool overwrite);
  void setTurnLeft(double degrees);
  void setTickDuration(double duration);
  double getBattleFieldSize() const;

  // These are normally in Robot and not AdvancedRobot, but due to
  // the upside-down hierarchy we have - they're here instead ;-)
  double getGunHeat() const;
  double getHeading() const;
  double getHeight() const;
  double getWidth() const;
  double getLength() const;
  void getPlayers() const;
  void getObstacles() const;
  void getShots() const;
  long getTime() const;
  double getVelocity() const;
  double getX() const;
  double getY() const;
  double getZ() const;

  virtual ~BZAdvancedRobot();
  BZAdvancedRobot();

  virtual void initialize() {}
  virtual void update() = 0;
  virtual void run();

  /* These are event handlers, optional to implement them. */
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
  bool getCompatability() const;
  void setCompatability(bool newState);
    
  void setLink(RCLinkFrontend *_link);
  RCLinkFrontend *getLink(void) const;

  /* These are helper functions. */
  /* This returns the bearing between our current angle and the location of the tank. */
  double getBearing(const Tank &tank) const;
  double getBearing(double x, double y) const;

  /* This returns the distance between us and the location of the tank. */
  double getDistance(const Tank &tank) const;
  double getDistance(double x, double y) const;

  /* This returns the shot by given id */
  const FrontendShot *getShot(uint64_t id) const;
 
  mutable double gunHeat, distanceRemaining, turnRemaining;
  mutable double battleFieldSize;
  mutable double yPosition, xPosition, zPosition;
  mutable double tankWidth, tankLength, tankHeight;
  mutable double heading;
  mutable std::vector<Tank> players;
  mutable std::vector<Obstacle *> obstacles;
  mutable std::vector<FrontendShot> shots;
};

#else
class BZAdvancedRobot;
#endif  /* __BZADVANCEDROBOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
