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

#include "common.h"

/* system interface headers */
#include <string>

/* local interface headers */
#include "BZRobot.h"

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
  StatusEventID,
  WinEventID
} EventID;

class BZRobotEvent
{
public:
  BZRobotEvent() { eventName = "Unknown"; eventID = -1; }
  virtual ~BZRobotEvent() {}

  void setTime(double _time) { time = _time; }
  void setPriority(int _priority) { priority = _priority; }

  inline double getTime()     const { return time;     }
  inline int  getEventID()  const { return eventID;  }
  inline int  getPriority() const { return priority; }
  inline const std::string& getEventName() const { return eventName; }

  void Execute(BZRobot *);

protected:
  std::string eventName;
  int eventID;
private:
  int priority;
  double time;
};

class BattleEndedEvent : public BZRobotEvent
{
public:
  BattleEndedEvent() { eventName = "BattleEnded"; eventID = BattleEndedEventID; }
};

class BulletHitEvent : public BZRobotEvent
{
public:
  BulletHitEvent() { eventName = "BulletHit"; eventID = BulletHitEventID; }
};

class BulletMissedEvent : public BZRobotEvent
{
public:
  BulletMissedEvent() { eventName = "BulletMissed"; eventID = BulletMissedEventID; }
};

class DeathEvent : public BZRobotEvent
{
public:
  DeathEvent() { eventName = "Death"; eventID = BulletMissedEventID; }
};

class HitByBulletEvent : public BZRobotEvent
{
public:
  HitByBulletEvent() { eventName = "HitByBullet"; eventID = HitByBulletEventID; }
  HitByBulletEvent(double _bearing) : bearing(_bearing) { eventName = "HitByBullet"; eventID = HitByBulletEventID; }

private:
  double bearing;
};

class HitWallEvent : public BZRobotEvent
{
public:
  HitWallEvent() { eventName = "HitWall"; eventID = HitWallEventID; }
  HitWallEvent(double _bearing) : bearing(_bearing) { eventName = "HitWall"; eventID = HitWallEventID; }

private:
  double bearing;
};

class RobotDeathEvent : public BZRobotEvent
{
public:
  RobotDeathEvent() { eventName = "RobotDeath"; eventID = RobotDeathEventID; }
};

class ScannedRobotEvent : public BZRobotEvent
{
public:
  ScannedRobotEvent() { eventName = "ScannedRobot"; eventID = ScannedRobotEventID; }
};

class StatusEvent : public BZRobotEvent
{
public:
  StatusEvent() { eventName = "Status"; eventID = StatusEventID; }
};

class WinEvent : public BZRobotEvent
{
public:
  WinEvent() { eventName = "Win"; eventID = WinEventID; }
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
