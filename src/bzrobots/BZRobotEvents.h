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

#ifndef __BZROBOTEVENTS_H__
#define __BZROBOTEVENTS_H__

/* system interface headers */
#include <string>

/* local interface headers */
#include "BZRobot.h"
#include "Bullet.h"

typedef enum EventID {
  UnknownEventID = -1,
  BattleEndedEventID = 0,
  BulletHitEventID,
  BulletMissedEventID,
  DeathEventID,
  HitByBulletEventID,
  HitWallEventID,
  RobotDeathEventID,
  ScannedRobotEventID,
  SpawnEventID,
  StatusEventID,
  WinEventID
} EventID;

class BZRobotEvent
{
public:
  BZRobotEvent() { eventID = UnknownEventID; priority = 80; time = 0.0; }
  virtual ~BZRobotEvent() {}

  void setTime(double _time) { time = _time; }
  void setPriority(int _priority) { priority = _priority; }

  inline double getTime() const { return time; }
  inline int getEventID() const { return eventID; }
  inline int getPriority() const { return priority; }

  void Execute(BZRobot *);

protected:
  int eventID;
  int priority;
  double time;
};

class BattleEndedEvent : public BZRobotEvent
{
public:
  BattleEndedEvent(bool _aborted) :
    aborted(_aborted) { eventID = BattleEndedEventID; priority = 80; }
  inline bool isAborted() const { return aborted; }

private:
  bool aborted;
};

class BulletHitEvent : public BZRobotEvent
{
public:
  BulletHitEvent(std::string _victimName, Bullet *_bullet) :
    victimName(_victimName), bullet(_bullet) { eventID = BulletHitEventID; priority = 50; }
  inline Bullet * getBullet() const { return bullet; }
  inline std::string getName() const { return victimName; }

private:
  std::string victimName;
  Bullet *bullet;
};

class BulletMissedEvent : public BZRobotEvent
{
public:
  BulletMissedEvent(Bullet *_bullet) :
    bullet(_bullet) { eventID = BulletMissedEventID; priority = 60; }
  inline Bullet * getBullet() const { return bullet; }

private:
  Bullet *bullet;
};

class DeathEvent : public BZRobotEvent
{
public:
  DeathEvent() { eventID = DeathEventID; priority = 0; }
};

class HitByBulletEvent : public BZRobotEvent
{
public:
  HitByBulletEvent(double _bearing, Bullet *_bullet) :
	bearing(_bearing), bullet(_bullet) { eventID = HitByBulletEventID; }
  inline Bullet * getBullet() const { return bullet; }
  inline double getBearing() const { return bearing; }

private:
  double bearing;
  Bullet *bullet;
};

class HitWallEvent : public BZRobotEvent
{
public:
  HitWallEvent(double _bearing) :
	bearing(_bearing) { eventID = HitWallEventID; priority = 30; }

  inline double getBearing() const { return bearing; }

private:
  double bearing;
};

class RobotDeathEvent : public BZRobotEvent
{
public:
  RobotDeathEvent() { eventID = RobotDeathEventID; priority = 70; }
};

class ScannedRobotEvent : public BZRobotEvent
{
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

class SpawnEvent : public BZRobotEvent
{
public:
  SpawnEvent() { eventID = SpawnEventID; priority = 0; }
};

class StatusEvent : public BZRobotEvent
{
public:
  StatusEvent() { eventID = StatusEventID; priority = 99; }
};

class WinEvent : public BZRobotEvent
{
public:
  WinEvent() { eventID = WinEventID; priority = 100; }
};

#else
class BattleEndedEvent;
class BulletHitEvent;
class BulletMissedEvent;
class DeathEvent;
class HitByBulletEvent;
class HitWallEvent;
class RobotDeathEvent;
class ScannedRobotEvent;
class SpawnEvent;
class StatusEvent;
class WinEvent;
#endif /* __BZROBOTEVENTS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
