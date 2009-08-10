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

/* inteface header */
#include "BZRobotControl.h"


BZRobotCallbacks *BZRobotControl::CallbackSet(BZRobotPlayer *rrp)
{
  BZRobotCallbacks *cbset = new BZRobotCallbacks;
  cbset->data = rrp;
  cbset->Execute = &BZRobotControl::Execute;
  cbset->GetDistanceRemaining = &BZRobotControl::GetDistanceRemaining;
  cbset->GetTurnRemaining = &BZRobotControl::GetTurnRemaining;
  cbset->SetAhead = &BZRobotControl::SetAhead;
  cbset->SetFire = &BZRobotControl::SetFire;
  cbset->SetTurnRate = &BZRobotControl::SetTurnRate;
  cbset->SetMaxVelocity = &BZRobotControl::SetMaxVelocity;
  cbset->SetResume = &BZRobotControl::SetResume;
  cbset->SetStop = &BZRobotControl::SetStop;
  cbset->SetTurnLeft = &BZRobotControl::SetTurnLeft;
  cbset->GetBattleFieldSize = &BZRobotControl::GetBattleFieldSize;
  cbset->GetGunHeat = &BZRobotControl::GetGunHeat;
  cbset->GetVelocity = &BZRobotControl::GetVelocity;
  cbset->GetHeading = &BZRobotControl::GetHeading;
  cbset->GetHeight = &BZRobotControl::GetHeight;
  cbset->GetWidth = &BZRobotControl::GetWidth;
  cbset->GetLength = &BZRobotControl::GetLength;
  cbset->GetTime = &BZRobotControl::GetTime;
  cbset->GetX = &BZRobotControl::GetX;
  cbset->GetY = &BZRobotControl::GetY;
  cbset->GetZ = &BZRobotControl::GetZ;
  return cbset;
}

void BZRobotControl::Execute(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botExecute();
}

double BZRobotControl::GetDistanceRemaining(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetDistanceRemaining();
}

double BZRobotControl::GetTurnRemaining(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetTurnRemaining();
}

void BZRobotControl::SetAhead(void *_rrp,double distance)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetAhead(distance);
}

void BZRobotControl::SetFire(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetFire();
}

void BZRobotControl::SetTurnRate(void *_rrp,double rate)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetTurnRate(rate);
}

void BZRobotControl::SetMaxVelocity(void *_rrp,double speed)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetMaxVelocity(speed);
}

void BZRobotControl::SetResume(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetResume();
}

void BZRobotControl::SetStop(void *_rrp,bool overwrite)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetStop(overwrite);
}

void BZRobotControl::SetTurnLeft(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetTurnLeft(turn);
}

double BZRobotControl::GetBattleFieldSize(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetBattleFieldSize();
}

double BZRobotControl::GetGunHeat(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetGunHeat();
}

double BZRobotControl::GetVelocity(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetVelocity();
}

double BZRobotControl::GetHeading(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetHeading();
}

double BZRobotControl::GetHeight(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetHeight();
}

double BZRobotControl::GetWidth(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetWidth();
}

double BZRobotControl::GetLength(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetLength();
}

long BZRobotControl::GetTime(void *_rrp)
{
  if(!_rrp) return 0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetTime();
}

double BZRobotControl::GetX(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetX();
}

double BZRobotControl::GetY(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetY();
}

double BZRobotControl::GetZ(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetZ();
}


/*


double BZRobotControl::FlushEvents()
{

  switch(event->getEventID())
  {
    case BattleEndedEventID:
	  robot->onBattleEnded((BattleEndedEvent *)event);
	  break;
    case BulletHitEventID:
	  robot->onBulletHit((BulletHitEvent *)event);
	  break;
    case BulletMissedEventID:
	  robot->onBulletMissed((BulletMissedEvent *)event);
	  break;
    case DeathEventID:
	  robot->onDeath((DeathEvent *)event);
	  break;
    case HitByBulletEventID:
	  robot->onHitByBullet((HitByBulletEvent *)event);
	  break;
    case HitWallEventID:
	  robot->onHitWall((HitWallEvent *)event);
	  break;
    case RobotDeathEventID:
	  robot->onRobotDeath((RobotDeathEvent *)event);
	  break;
    case ScannedRobotEventID:
	  robot->onScannedRobot((ScannedRobotEvent *)event);
	  break;
    case StatusEventID:
	  robot->onStatus((StatusEvent *)event);
	  break;
    case WinEventID:
	  robot->onWin((WinEvent *)event);
	  break;
  }

}

*/


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
