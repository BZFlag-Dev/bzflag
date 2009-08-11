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
#include "BZRobotEvents.h"

void BZRobotEvent::Execute(BZRobot *robot)
{
  switch(eventID)
  {
    case BattleEndedEventID:
      robot->onBattleEnded(*((BattleEndedEvent *)this));
      break;
    case BulletHitEventID:
      robot->onBulletHit(*((BulletHitEvent *)this));
      break;
    case BulletMissedEventID:
      robot->onBulletMissed(*((BulletMissedEvent *)this));
      break;
    case DeathEventID:
      robot->onDeath(*((DeathEvent *)this));
      break;
    case HitByBulletEventID:
      robot->onHitByBullet(*((HitByBulletEvent *)this));
      break;
    case HitWallEventID:
      robot->onHitWall(*((HitWallEvent *)this));
      break;
    case RobotDeathEventID:
      robot->onRobotDeath(*((RobotDeathEvent *)this));
      break;
    case ScannedRobotEventID:
      robot->onScannedRobot(*((ScannedRobotEvent *)this));
      break;
    case StatusEventID:
      robot->onStatus(*((StatusEvent *)this));
      break;
    case WinEventID:
      robot->onWin(*((WinEvent *)this));
      break;
    default:
      break;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
