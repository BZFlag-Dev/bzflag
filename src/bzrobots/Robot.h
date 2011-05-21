/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __BZROBOTS_ROBOT_H__
#define __BZROBOTS_ROBOT_H__

/* local interface headers */
#include "Bullet.h"
#include "Events.h"

namespace BZRobots {

  class Robot {
    protected:
      // robotcb is of type RobotCallbacks, but void*
      // is used here to keep the API headers clean
      mutable void* robotcb;
      std::string robotType;
    public:
      Robot();
      virtual ~Robot();
      void setCallbacks(void* _robotcb);
      inline std::string getRobotType() const { return robotType; }

      void ahead(double distance);
      void back(double distance);
      void doNothing();
      void fire(double power = 3.0f);
      Bullet* fireBullet(double power = 3.0f) const;
      double getBattleFieldLength() const;
      double getBattleFieldWidth() const;
      double getEnergy() const;
      double getGunCoolingRate() const;
      double getGunHeading() const;
      double getGunHeat() const;
      double getHeading() const;
      double getHeight() const;
      double getLength() const;
      std::string getName() const;
      int getNumRounds() const;
      int getOthers() const;
      double getRadarHeading() const;
      int getRoundNum() const;
      double getTime() const;
      double getVelocity() const;
      double getWidth() const;
      double getX() const;
      double getY() const;
      double getZ() const;
      void resume();
      void scan();
      void setAdjustGunForRobotTurn(bool independent);
      void setAdjustRadarForGunTurn(bool independent);
      void setAdjustRadarForRobotTurn(bool independent);
      void stop(bool overwrite = false);
      void turnGunRight(double degrees);
      void turnGunLeft(double degrees);
      void turnLeft(double degrees);
      void turnRadarRight(double degrees);
      void turnRadarLeft(double degrees);
      void turnRight(double degrees);

      virtual void run() {}

      /* Event Handlers. */

      virtual void onBattleEnded(const BattleEndedEvent& /*event*/) {}
      virtual void onBulletFired(const BulletFiredEvent& /*event*/) {}
      virtual void onBulletHit(const BulletHitEvent& /*event*/) {}
      virtual void onBulletHitBullet(const BulletHitBulletEvent& /*event*/) {}
      virtual void onBulletMissed(const BulletMissedEvent& /*event*/) {}
      virtual void onDeath(const DeathEvent& /*event*/) {}
      virtual void onHitByBullet(const HitByBulletEvent& /*event*/) {}
      virtual void onHitRobot(const HitRobotEvent& /*event*/) {}
      virtual void onHitWall(const HitWallEvent& /*event*/) {}
      virtual void onRobotDeath(const RobotDeathEvent& /*event*/) {}
      virtual void onScannedRobot(const ScannedRobotEvent& /*event*/) {}
      virtual void onSpawn(const SpawnEvent& /*event*/) {}
      virtual void onStatus(const StatusEvent& /*event*/) {}
      virtual void onWin(const WinEvent& /*event*/) {}
  };

} // namespace BZRobots

#else
namespace BZRobots {
  class Robot;
}
#endif /* __BZROBOTS_ROBOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
