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

#ifndef __ROBOTCONTROL_H__
#define __ROBOTCONTROL_H__

#include <string>

#include "RobotCallbacks.h"
#include "BZRobotPlayer.h"
#include "Bullet.h"

class RobotControl {
public:
  static RobotCallbacks *CallbackSet(BZRobotPlayer *rrp);
  static void Ahead(void *_rrp, double);
  static void Back(void *_rrp, double);
  static void ClearAllEvents(void *_rrp);
  static void DoNothing(void *_rrp);
  static void Execute(void *_rrp);
  static void Fire(void *_rrp, double);
  static BZRobots::Bullet* FireBullet(void *_rrp, double);
  static double GetBattleFieldLength(void *_rrp);
  static double GetBattleFieldWidth(void *_rrp);
  static double GetDistanceRemaining(void *_rrp);
  static double GetEnergy(void *_rrp);
  static double GetGunCoolingRate(void *_rrp);
  static double GetGunHeading(void *_rrp);
  static double GetGunHeat(void *_rrp);
  static double GetHeading(void *_rrp);
  static double GetHeight(void *_rrp);
  static double GetLength(void *_rrp);
  static std::string GetName(void *_rrp);
  static int GetNumRounds(void *_rrp);
  static int GetOthers(void *_rrp);
  static double GetRadarHeading(void *_rrp);
  static int GetRoundNum(void *_rrp);
  static double GetTime(void *_rrp);
  static double GetTurnRemaining(void *_rrp);
  static double GetVelocity(void *_rrp);
  static double GetWidth(void *_rrp);
  static double GetX(void *_rrp);
  static double GetY(void *_rrp);
  static double GetZ(void *_rrp);
  static void SetAdjustGunForRobotTurn(void *_rrp, bool);
  static void SetAdjustRadarForGunTurn(void *_rrp, bool);
  static void SetAdjustRadarForRobotTurn(void *_rrp, bool);
  static void Resume(void *_rrp);
  static void Scan(void *_rrp);
  static void SetAhead(void *_rrp, double);
  static void SetBack(void *_rrp, double);
  static void SetFire(void *_rrp, double);
  static BZRobots::Bullet* SetFireBullet(void *_rrp, double);
  static void SetMaxTurnRate(void *_rrp, double);
  static void SetMaxVelocity(void *_rrp, double);
  static void SetResume(void *_rrp);
  static void SetStop(void *_rrp, bool);
  static void SetTurnLeft(void *_rrp, double);
  static void SetTurnRate(void *_rrp, double);
  static void SetTurnRight(void *_rrp, double);
  static void Stop(void *_rrp, bool);
  static void TurnGunLeft(void *_rrp, double);
  static void TurnGunRight(void *_rrp, double);
  static void TurnLeft(void *_rrp, double);
  static void TurnRadarLeft(void *_rrp, double);
  static void TurnRadarRight(void *_rrp, double);
  static void TurnRight(void *_rrp, double);
};

#else

struct RobotControl;

#endif /* __ROBOTCONTROL_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
