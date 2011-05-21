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

#ifndef __BZROBOTS_ADVANCEDROBOT_H__
#define __BZROBOTS_ADVANCEDROBOT_H__

/* std interface headers */
#include <list>

/* local interface headers */
#include "Robot.h"
#include "Bullet.h"
#include "Events.h"

namespace BZRobots {

  class AdvancedRobot : public Robot {
    public:
      AdvancedRobot();
      virtual ~AdvancedRobot();

      void clearAllEvents();
      void execute();
      std::list<Event> getAllEvents();
      std::list<Event> getBulletHitBulletEvents();
      std::list<Event> getBulletHitEvents();
      std::list<Event> getBulletMissedEvents();
      double getDistanceRemaining() const;
      double getGunHeadingRadians() const;
      double getGunTurnRemaining() const;
      double getGunTurnRemainingRadians() const;
      double getHeadingRadians() const;
      std::list<Event> getHitByBulletEvents();
      std::list<Event> getHitRobotEvents();
      std::list<Event> getHitWallEvents();
      double getRadarHeadingRadians() const;
      double getRadarTurnRemaining() const;
      double getRadarTurnRemainingRadians() const;
      std::list<Event> getRobotDeathEvents();
      std::list<Event> getScannedRobotEvents();
      std::list<Event> getStatusEvents();
      double getTurnRemaining() const;
      double getTurnRemainingRadians() const;
      bool isAdjustGunForRobotTurn() const;
      bool isAdjustRadarForGunTurn() const;
      bool isAdjustRadarForRobotTurn() const;
      void setAhead(double distance);
      void setBack(double distance);
      void setFire(double power = 3.0f);
      Bullet* setFireBullet(double power = 3.0f) const;
      void setMaxTurnRate(double maxTurnRate);
      void setMaxVelocity(double maxVelocity);
      void setResume();
      void setStop(bool overwrite = false);
      void setTurnGunLeft(double degrees);
      void setTurnGunLeftRadians(double radians);
      void setTurnGunRight(double degrees);
      void setTurnGunRightRadians(double radians);
      void setTurnLeft(double degrees);
      void setTurnLeftRadians(double radians);
      void setTurnRadarLeft(double degrees);
      void setTurnRadarLeftRadians(double radians);
      void setTurnRadarRight(double degrees);
      void setTurnRadarRightRadians(double radians);
      void setTurnRight(double degrees);
      void setTurnRightRadians(double radians);
      void turnGunLeftRadians(double radians);
      void turnGunRightRadians(double radians);
      void turnLeftRadians(double radians);
      void turnRadarLeftRadians(double radians);
      void turnRadarRightRadians(double radians);
      void turnRightRadians(double radians);
  };


} // namespace BZRobots

#else
namespace BZRobots {
  class AdvancedRobot;
}
#endif  /* __BZROBOTS_ADVANCEDROBOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
