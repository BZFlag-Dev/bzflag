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

#ifndef __ROBOTCONTROL_H__
#define __ROBOTCONTROL_H__

#include <list>
#include <string>

#include "BZRobotPlayer.h"
#include "Bullet.h"
#include "Events.h"
#include "RobotCallbacks.h"

class RobotControl {
  public:
    static RobotCallbacks* CallbackSet(BZRobotPlayer* rrp);
    static void Ahead(void* _rrp, double);
    static void Back(void* _rrp, double);
    static void ClearAllEvents(void* _rrp);
    static void DoNothing(void* _rrp);
    static void Execute(void* _rrp);
    static void Fire(void* _rrp, double);
    static BZRobots::Bullet* FireBullet(void* _rrp, double);
    static std::list<BZRobots::Event> GetAllEvents(void* _rrp);
    static double GetBattleFieldLength(void* _rrp);
    static double GetBattleFieldWidth(void* _rrp);
    static std::list<BZRobots::Event> GetBulletHitBulletEvents(void* _rrp);
    static std::list<BZRobots::Event> GetBulletHitEvents(void* _rrp);
    static std::list<BZRobots::Event> GetBulletMissedEvents(void* _rrp);
    static double GetDistanceRemaining(void* _rrp);
    static double GetEnergy(void* _rrp);
    static double GetGunCoolingRate(void* _rrp);
    static double GetGunHeading(void* _rrp);
    static double GetGunHeadingRadians(void* _rrp);
    static double GetGunHeat(void* _rrp);
    static double GetGunTurnRemaining(void* _rrp);
    static double GetGunTurnRemainingRadians(void* _rrp);
    static double GetHeading(void* _rrp);
    static double GetHeadingRadians(void* _rrp);
    static double GetHeight(void* _rrp);
    static std::list<BZRobots::Event> GetHitByBulletEvents(void* _rrp);
    static std::list<BZRobots::Event> GetHitRobotEvents(void* _rrp);
    static std::list<BZRobots::Event> GetHitWallEvents(void* _rrp);
    static double GetLength(void* _rrp);
    static std::string GetName(void* _rrp);
    static int GetNumRounds(void* _rrp);
    static int GetOthers(void* _rrp);
    static double GetRadarHeading(void* _rrp);
    static double GetRadarHeadingRadians(void* _rrp);
    static double GetRadarTurnRemaining(void* _rrp);
    static double GetRadarTurnRemainingRadians(void* _rrp);
    static std::list<BZRobots::Event> GetRobotDeathEvents(void* _rrp);
    static int GetRoundNum(void* _rrp);
    static std::list<BZRobots::Event> GetScannedRobotEvents(void* _rrp);
    static std::list<BZRobots::Event> GetStatusEvents(void* _rrp);
    static double GetTime(void* _rrp);
    static double GetTurnRemaining(void* _rrp);
    static double GetTurnRemainingRadians(void* _rrp);
    static double GetVelocity(void* _rrp);
    static double GetWidth(void* _rrp);
    static double GetX(void* _rrp);
    static double GetY(void* _rrp);
    static double GetZ(void* _rrp);
    static bool IsAdjustGunForRobotTurn(void* _rrp);
    static bool IsAdjustRadarForGunTurn(void* _rrp);
    static bool IsAdjustRadarForRobotTurn(void* _rrp);
    static void Resume(void* _rrp);
    static void Scan(void* _rrp);
    static void SetAdjustGunForRobotTurn(void* _rrp, bool);
    static void SetAdjustRadarForGunTurn(void* _rrp, bool);
    static void SetAdjustRadarForRobotTurn(void* _rrp, bool);
    static void SetAhead(void* _rrp, double);
    static void SetBack(void* _rrp, double);
    static void SetFire(void* _rrp, double);
    static BZRobots::Bullet* SetFireBullet(void* _rrp, double);
    static void SetMaxTurnRate(void* _rrp, double);
    static void SetMaxVelocity(void* _rrp, double);
    static void SetResume(void* _rrp);
    static void SetStop(void* _rrp, bool);
    static void SetTurnGunLeft(void* _rrp, double);
    static void SetTurnGunLeftRadians(void* _rrp, double);
    static void SetTurnGunRight(void* _rrp, double);
    static void SetTurnGunRightRadians(void* _rrp, double);
    static void SetTurnLeft(void* _rrp, double);
    static void SetTurnLeftRadians(void* _rrp, double);
    static void SetTurnRadarLeft(void* _rrp, double);
    static void SetTurnRadarLeftRadians(void* _rrp, double);
    static void SetTurnRadarRight(void* _rrp, double);
    static void SetTurnRadarRightRadians(void* _rrp, double);
    static void SetTurnRight(void* _rrp, double);
    static void SetTurnRightRadians(void* _rrp, double);
    static void Stop(void* _rrp, bool);
    static void TurnGunLeft(void* _rrp, double);
    static void TurnGunLeftRadians(void* _rrp, double);
    static void TurnGunRight(void* _rrp, double);
    static void TurnGunRightRadians(void* _rrp, double);
    static void TurnLeft(void* _rrp, double);
    static void TurnLeftRadians(void* _rrp, double);
    static void TurnRadarLeft(void* _rrp, double);
    static void TurnRadarLeftRadians(void* _rrp, double);
    static void TurnRadarRight(void* _rrp, double);
    static void TurnRadarRightRadians(void* _rrp, double);
    static void TurnRight(void* _rrp, double);
    static void TurnRightRadians(void* _rrp, double);
};

#else

class RobotControl;

#endif /* __ROBOTCONTROL_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
