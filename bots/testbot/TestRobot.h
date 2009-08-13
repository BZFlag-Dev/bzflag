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

#ifndef __TESTROBOT_H__
#define __TESTROBOT_H__

/* local interface headers */
#include "BZAdvancedRobot.h"


/**
 * TestRobot: Testing basic stuff.
 */
class TestRobot : public BZAdvancedRobot
{
public:
  TestRobot() {}

  void run();

  void onBattleEnded(const BattleEndedEvent &e);
  void onBulletHit(const BulletHitEvent &e);
  void onBulletMissed(const BulletMissedEvent &e);
  void onDeath(const DeathEvent &e);
  void onHitByBullet(const HitByBulletEvent &e);
  void onHitWall(const HitWallEvent &e);
  void onRobotDeath(const RobotDeathEvent &e);
  void onScannedRobot(const ScannedRobotEvent &e);
  void onSpawn(const SpawnEvent &e);
  void onStatus(const StatusEvent &e);
  void onWin(const WinEvent &e);
};

#endif /* __TESTROBOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
