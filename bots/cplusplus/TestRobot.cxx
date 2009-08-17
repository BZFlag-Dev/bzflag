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

/* BZRobots API Header */
#include "AdvancedRobot.h"

using namespace BZRobots;

/**
 * TestRobot: Testing basic stuff.
 */
class TestRobot : public AdvancedRobot
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

extern "C" {
  AdvancedRobot *create()
  {
    return new TestRobot();
  }
  void destroy(AdvancedRobot *robot)
  {
    delete robot;
  }
}

void TestRobot::run()
{
  while(true) {
	printf("TestRobot: others: %d\n",getOthers());
	printf("TestRobot: run\n");
	ahead(10);
    fire();
	/*
    setFire();
    setAhead(200);
    do {
      double lastDistance = getDistanceRemaining();
      execute();
      if(lastDistance == getDistanceRemaining())
        break;
    } while (getDistanceRemaining() > 0);
  
    setAhead(-20);
    do {
      double lastDistance = getDistanceRemaining();
      execute();
      if(lastDistance == getDistanceRemaining())
        break;
    } while (getDistanceRemaining() > 0);
    
    setTurnLeft(90);
    do {
      double lastTurn = getTurnRemaining();
      execute();
      if(lastTurn == getTurnRemaining())
        break;
    } while (getTurnRemaining() > 0);
	*/
  }
}

void TestRobot::onBattleEnded(const BattleEndedEvent &/*e*/)
{
  //printf("TestRobot: BattleEndedEvent\n");
}

void TestRobot::onBulletHit(const BulletHitEvent &e)
{
  printf("TestRobot: BulletHitEvent (%s)\n",e.getName().c_str());
}

void TestRobot::onBulletMissed(const BulletMissedEvent &/*e*/)
{
  //printf("TestRobot: BulletMissedEvent\n");
}

void TestRobot::onDeath(const DeathEvent &/*e*/)
{
  printf("TestRobot: DeathEvent\n");
}

void TestRobot::onHitByBullet(const HitByBulletEvent &/*e*/)
{
  //printf("TestRobot: HitByBulletEvent\n");
}

void TestRobot::onHitWall(const HitWallEvent &/*e*/)
{
  printf("TestRobot: HitWallEvent\n");
  back(100);
  turnLeft(90);
}

void TestRobot::onRobotDeath(const RobotDeathEvent &e)
{
  printf("TestRobot: RobotDeathEvent (%s)\n",e.getName().c_str());
}

void TestRobot::onScannedRobot(const ScannedRobotEvent &/*e*/)
{
  printf("TestRobot: ScannedRobotEvent\n");
}

void TestRobot::onSpawn(const SpawnEvent &/*e*/)
{
  printf("TestRobot: SpawnEvent\n");
}

void TestRobot::onStatus(const StatusEvent &/*e*/)
{
  //printf("TestRobot: StatusEvent\n");
}

void TestRobot::onWin(const WinEvent &/*e*/)
{
  //printf("TestRobot: WinEvent\n");
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
