/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

class AdvancedRobot : public Robot
{
public:
  AdvancedRobot();
  virtual ~AdvancedRobot();

  void clearAllEvents();
  void execute();
  //std::list<Event> getAllEvents();
  //std::list<Event> getBulletHitBulletEvents();
  //std::list<Event> getBulletHitEvents();
  //std::list<Event> getBulletMissedEvents();
  double getDistanceRemaining() const;
  double getTurnRemaining() const;
  //double getGunTurnRemaining() const;
  //double getRadarTurnRemaining() const;
  //bool isAdjustGunForRobotTurn() const;
  //bool isAdjustRadarForRobotTurn() const;
  void setAhead(double distance);
  void setBack(double distance);
  void setFire(double power = 3.0f);
  Bullet *setFireBullet(double power = 3.0f) const;
  void setMaxTurnRate(double maxTurnRate);
  void setMaxVelocity(double maxVelocity);
  void setResume();
  void setStop(bool overwrite = false);
  //void setTurnGunLeft(double degrees);
  //void setTurnGunRight(double degrees);
  void setTurnLeft(double degrees);
  //void setTurnRadarLeft(double degrees);
  //void setTurnRadarRight(double degrees);
  void setTurnRate(double turnRate);
  void setTurnRight(double degrees);
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
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
