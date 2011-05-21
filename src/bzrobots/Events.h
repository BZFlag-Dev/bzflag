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

#ifndef __BZROBOTS_EVENTS_H__
#define __BZROBOTS_EVENTS_H__

/* system interface headers */
#include <string>

/* local interface headers */
#include "Robot.h"
#include "Bullet.h"
#include "RobotStatus.h"

namespace BZRobots {

  typedef enum EventID {
    UnknownEventID = -1,
    BattleEndedEventID = 0,
    BulletFiredEventID,
    BulletHitEventID,
    BulletHitBulletEventID,
    BulletMissedEventID,
    DeathEventID,
    HitByBulletEventID,
    HitRobotEventID,
    HitWallEventID,
    RobotDeathEventID,
    ScannedRobotEventID,
    SpawnEventID,
    StatusEventID,
    WinEventID
  } EventID;

  /*
     BattleEndedEvent:     100 (reserved)
     WinEvent:             100 (reserved)
     SkippedTurnEvent:     100 (reserved)
     StatusEvent:           99
     Key and mouse events:  98
     CustomEvent:           80 (default value)
     MessageEvent:          75
     RobotDeathEvent:       70
     BulletMissedEvent:     60
     BulletHitBulletEvent:  55
     BulletHitEvent:        50
     HitByBulletEvent:      40
     HitWallEvent:          30
     HitEvent:              20
     ScannedRobotEvent:     10
     PaintEvent:             5
     DeathEvent:            -1 (reserved)
  */

  class Event {
    public:
      Event() { eventID = UnknownEventID; priority = 80; time = 0.0; }
      virtual ~Event() {}

      void setTime(double _time) { time = _time; }
      void setPriority(int _priority) { priority = _priority; }

      inline double getTime() const { return time; }
      inline int getEventID() const { return eventID; }
      inline int getPriority() const { return priority; }

    protected:
      int eventID;
      int priority;
      double time;
  };

  class BattleEndedEvent : public Event {
    public:
      BattleEndedEvent(bool _aborted) :
        aborted(_aborted) { eventID = BattleEndedEventID; priority = 80; }
      inline bool isAborted() const { return aborted; }

    private:
      bool aborted;
  };

  class BulletFiredEvent : public Event {
    public:
      BulletFiredEvent(Bullet* _bullet) :
        bullet(_bullet) { eventID = BulletFiredEventID; priority = 50; }
      inline Bullet* getBullet() const { return bullet; }

    private:
      Bullet* bullet;
  };

  class BulletHitEvent : public Event {
    public:
      BulletHitEvent(std::string _victimName, Bullet* _bullet) :
        victimName(_victimName), bullet(_bullet) { eventID = BulletHitEventID; priority = 50; }
      inline Bullet* getBullet() const { return bullet; }
      inline std::string getName() const { return victimName; }

    private:
      std::string victimName;
      Bullet* bullet;
  };

  class BulletHitBulletEvent : public Event {
    public:
      BulletHitBulletEvent(Bullet* _bullet, Bullet* _hitBullet) :
        bullet(_bullet), hitBullet(_hitBullet) { eventID = BulletHitBulletEventID; priority = 50; }
      inline Bullet* getBullet() const { return bullet; }
      inline Bullet* getHitBullet() const { return hitBullet; }

    private:
      Bullet* bullet;
      Bullet* hitBullet;
  };

  class BulletMissedEvent : public Event {
    public:
      BulletMissedEvent(Bullet* _bullet) :
        bullet(_bullet) { eventID = BulletMissedEventID; priority = 60; }
      inline Bullet* getBullet() const { return bullet; }

    private:
      Bullet* bullet;
  };

  class DeathEvent : public Event {
    public:
      DeathEvent() { eventID = DeathEventID; priority = 0; }
  };

  class HitByBulletEvent : public Event {
    public:
      HitByBulletEvent(double _bearing, Bullet* _bullet) :
        bearing(_bearing), bullet(_bullet) { eventID = HitByBulletEventID; }
      inline Bullet* getBullet() const { return bullet; }
      inline double getBearing() const { return bearing; }

    private:
      double bearing;
      Bullet* bullet;
  };

  class HitRobotEvent : public Event {
    public:
      HitRobotEvent(std::string _name, double _bearing, double _energy, bool _atFault) :
        name(_name), bearing(_bearing), energy(_energy), atFault(_atFault)
      { eventID = HitWallEventID; priority = 30; }

      inline std::string getName() const { return name; }
      inline double getEnergy() const { return energy; }
      inline double getBearing() const { return bearing; }
      inline bool isMyFault() const { return atFault; }

    private:
      std::string name;
      double bearing;
      double energy;
      bool atFault;
  };

  class HitWallEvent : public Event {
    public:
      HitWallEvent(double _bearing) :
        bearing(_bearing) { eventID = HitWallEventID; priority = 30; }

      inline double getBearing() const { return bearing; }

    private:
      double bearing;
  };

  class RobotDeathEvent : public Event {
    public:
      RobotDeathEvent(std::string _name) :
        name(_name) { eventID = RobotDeathEventID; priority = 70; }

      inline std::string getName() const { return name; }

    private:
      std::string name;
  };

  class ScannedRobotEvent : public Event {
    public:
      ScannedRobotEvent(std::string _name,
                        double _bearing,
                        double _distance,
                        double _x,
                        double _y,
                        double _z,
                        double _heading,
                        double _velocity) :
        name(_name),
        bearing(_bearing),
        distance(_distance),
        x(_x), y(_y), z(_z),
        heading(_heading),
        velocity(_velocity)
      { eventID = ScannedRobotEventID; priority = 10; }

      inline std::string getName() const { return name; }
      inline double getBearing() const { return bearing; }
      inline double getDistance() const { return distance; }
      inline double getX() const { return x; }
      inline double getY() const { return y; }
      inline double getZ() const { return z; }
      inline double getHeading() const { return heading; }
      inline double getVelocity() const { return velocity; }

    private:
      std::string name;
      double bearing;
      double distance;
      double x;
      double y;
      double z;
      double heading;
      double velocity;
  };

  class SpawnEvent : public Event {
    public:
      SpawnEvent() { eventID = SpawnEventID; priority = 0; }
  };

  class StatusEvent : public Event {
    public:
      StatusEvent(RobotStatus _status) : status(_status) { eventID = StatusEventID; priority = 99; }

      inline RobotStatus getStatus() const { return status; }

    private:
      RobotStatus status;
  };

  class WinEvent : public Event {
    public:
      WinEvent() { eventID = WinEventID; priority = 100; }
  };


} // namespace BZRobots

#else
namespace BZRobots {
  class BattleEndedEvent;
  class BulletFiredEvent;
  class BulletHitEvent;
  class BulletHitBulletEvent;
  class BulletMissedEvent;
  class DeathEvent;
  class HitByBulletEvent;
  class HitRobotEvent;
  class HitWallEvent;
  class RobotDeathEvent;
  class ScannedRobotEvent;
  class SpawnEvent;
  class StatusEvent;
  class WinEvent;
}
#endif /* __BZROBOTS_EVENTS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
