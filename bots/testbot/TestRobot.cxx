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

/* interface header */
#include "TestRobot.h"

extern "C" {
  BZAdvancedRobot *create()
  {
    return new TestRobot();
  }
  void destroy(BZAdvancedRobot *robot)
  {
    delete robot;
  }
}

void TestRobot::run()
{
  while(true) {
    setFire();
    setAhead(200);
    do {
      double lastDistance = getDistanceRemaining();
      execute();
      sleep(1);
      if(lastDistance == getDistanceRemaining())
        break;
    } while (getDistanceRemaining() > 0);
  
    setAhead(-20);
    do {
      double lastDistance = getDistanceRemaining();
      execute();
      sleep(1);
      if(lastDistance == getDistanceRemaining())
        break;
    } while (getDistanceRemaining() > 0);
    
    setTurnLeft(90);
    do {
      double lastTurn = getTurnRemaining();
      execute();
      sleep(1);
      if(lastTurn == getTurnRemaining())
        break;
    } while (getTurnRemaining() > 0);
  }
}

void TestRobot::onHitWall(const HitWallEvent &/*hwe*/)
{
  setAhead(0);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
