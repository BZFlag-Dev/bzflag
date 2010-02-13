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

#ifndef __ROBOTCALLBACKS_H__
#define __ROBOTCALLBACKS_H__

#include <list>
#include <string>

#include "Bullet.h"
#include "Events.h"

typedef struct RobotCallbacks {
  void *data; // Data to be sent back to callbacks
  void (*Ahead)(void *data, double);
  void (*Back)(void *data, double);
  void (*ClearAllEvents)(void *data);
  void (*DoNothing)(void *data);
  void (*Execute)(void *data);
  void (*Fire)(void *data, double);
  BZRobots::Bullet* (*FireBullet)(void *data, double);
  std::list<BZRobots::Event> (*GetAllEvents)(void *data);
  double (*GetBattleFieldLength)(void *data);
  double (*GetBattleFieldWidth)(void *data);
  std::list<BZRobots::Event> (*GetBulletHitBulletEvents)(void *data);
  std::list<BZRobots::Event> (*GetBulletHitEvents)(void *data);
  std::list<BZRobots::Event> (*GetBulletMissedEvents)(void *data);
  double (*GetDistanceRemaining)(void *data);
  double (*GetEnergy)(void *data);
  double (*GetGunCoolingRate)(void *data);
  double (*GetGunHeading)(void *data);
  double (*GetGunHeadingRadians)(void *data);
  double (*GetGunHeat)(void *data);
  double (*GetGunTurnRemaining)(void *data);
  double (*GetGunTurnRemainingRadians)(void *data);
  double (*GetHeading)(void *data);
  double (*GetHeadingRadians)(void *data);
  double (*GetHeight)(void *data);
  std::list<BZRobots::Event> (*GetHitByBulletEvents)(void *data);
  std::list<BZRobots::Event> (*GetHitRobotEvents)(void *data);
  std::list<BZRobots::Event> (*GetHitWallEvents)(void *data);
  double (*GetLength)(void *data);
  std::string (*GetName)(void *data);
  int (*GetNumRounds)(void *data);
  int (*GetOthers)(void *data);
  double (*GetRadarHeading)(void *data);
  double (*GetRadarHeadingRadians)(void *data);
  double (*GetRadarTurnRemaining)(void *data);
  double (*GetRadarTurnRemainingRadians)(void *data);
  std::list<BZRobots::Event> (*GetRobotDeathEvents)(void *data);
  int (*GetRoundNum)(void *data);
  std::list<BZRobots::Event> (*GetScannedRobotEvents)(void *data);
  std::list<BZRobots::Event> (*GetStatusEvents)(void *data);
  double (*GetTime)(void *data);
  double (*GetTurnRemaining)(void *data);
  double (*GetTurnRemainingRadians)(void *data);
  double (*GetVelocity)(void *data);
  double (*GetWidth)(void *data);
  double (*GetX)(void *data);
  double (*GetY)(void *data);
  double (*GetZ)(void *data);
  bool (*IsAdjustGunForRobotTurn)(void *data);
  bool (*IsAdjustRadarForGunTurn)(void *data);
  bool (*IsAdjustRadarForRobotTurn)(void *data);
  void (*Resume)(void *data);
  void (*Scan)(void *data);
  void (*SetAdjustGunForRobotTurn)(void *data, bool);
  void (*SetAdjustRadarForGunTurn)(void *data, bool);
  void (*SetAdjustRadarForRobotTurn)(void *data, bool);
  void (*SetAhead)(void *data, double);
  void (*SetBack)(void *data, double);
  void (*SetFire)(void *data, double);
  BZRobots::Bullet* (*SetFireBullet)(void *data, double);
  void (*SetMaxTurnRate)(void *data, double);
  void (*SetMaxVelocity)(void *data, double);
  void (*SetResume)(void *data);
  void (*SetStop)(void *data, bool);
  void (*SetTurnGunLeft)(void *data, double);
  void (*SetTurnGunLeftRadians)(void *data, double);
  void (*SetTurnGunRight)(void *data, double);
  void (*SetTurnGunRightRadians)(void *data, double);
  void (*SetTurnLeft)(void *data, double);
  void (*SetTurnLeftRadians)(void *data, double);
  void (*SetTurnRadarLeft)(void *data, double);
  void (*SetTurnRadarLeftRadians)(void *data, double);
  void (*SetTurnRadarRight)(void *data, double);
  void (*SetTurnRadarRightRadians)(void *data, double);
  void (*SetTurnRight)(void *data, double);
  void (*SetTurnRightRadians)(void *data, double);
  void (*Stop)(void *data, bool);
  void (*TurnGunLeft)(void *data, double);
  void (*TurnGunLeftRadians)(void *data, double);
  void (*TurnGunRight)(void *data, double);
  void (*TurnGunRightRadians)(void *data, double);
  void (*TurnLeft)(void *data, double);
  void (*TurnLeftRadians)(void *data, double);
  void (*TurnRadarLeft)(void *data, double);
  void (*TurnRadarLeftRadians)(void *data, double);
  void (*TurnRadarRight)(void *data, double);
  void (*TurnRadarRightRadians)(void *data, double);
  void (*TurnRight)(void *data, double);
  void (*TurnRightRadians)(void *data, double);
} RobotCallbacks;

#else

struct RobotCallbacks;

#endif /* __ROBOTCALLBACKS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
