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
  cbset->SetTickDuration = &BZRobotControl::SetTickDuration;
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
  
  if (rrp->pendingUpdates[BZRobotPlayer::speedUpdate])
    rrp->speed = rrp->nextSpeed;
  if (rrp->pendingUpdates[BZRobotPlayer::turnRateUpdate])
    rrp->turnRate = rrp->nextTurnRate;

  if (rrp->pendingUpdates[BZRobotPlayer::distanceUpdate]) {
    if (rrp->nextDistance < 0.0f)
      rrp->distanceForward = false;
    else
      rrp->distanceForward = true;
    rrp->distanceRemaining = (rrp->distanceForward ? 1 : -1) * rrp->nextDistance;
  }
  if (rrp->pendingUpdates[BZRobotPlayer::turnUpdate]) {
    if (rrp->nextTurn < 0.0f)
      rrp->turnLeft = false;
    else
      rrp->turnLeft = true;
    rrp->turnRemaining = (rrp->turnLeft ? 1 : -1) * rrp->nextTurn * M_PI/180.0f; /* We have to convert to radians! */
  }

  for (int i = 0; i < BZRobotPlayer::updateCount; ++i)
    rrp->pendingUpdates[i] = false;

  if (rrp->shoot) {
    rrp->shoot = false;
    rrp->fireShot();
  }
}

double BZRobotControl::GetDistanceRemaining(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->distanceRemaining;
}

double BZRobotControl::GetTurnRemaining(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->turnRemaining * 180.0f/M_PI;
}

void BZRobotControl::SetAhead(void *_rrp,double distance)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->pendingUpdates[BZRobotPlayer::distanceUpdate] = true;
  rrp->nextDistance = distance;
}

void BZRobotControl::SetFire(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->shoot = true;
}

void BZRobotControl::SetTurnRate(void *_rrp,double rate)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->nextTurnRate = rate;
  rrp->pendingUpdates[BZRobotPlayer::turnRateUpdate] = true;
}

void BZRobotControl::SetMaxVelocity(void *_rrp,double speed)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->nextSpeed = speed;
  rrp->pendingUpdates[BZRobotPlayer::speedUpdate] = true;
}

void BZRobotControl::SetResume(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;

  if (rrp->hasStopped) {
    rrp->hasStopped = false;
    rrp->distanceRemaining = rrp->stoppedDistance;
    rrp->turnRemaining = rrp->stoppedTurn;
    rrp->distanceForward = rrp->stoppedForward;
    rrp->turnLeft = rrp->stoppedLeft;
  }
}

void BZRobotControl::SetStop(void *_rrp,bool overwrite)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;

  if (!rrp->hasStopped || overwrite) {
    rrp->hasStopped = true;
    rrp->stoppedDistance = rrp->distanceRemaining;
    rrp->stoppedTurn = rrp->turnRemaining;
    rrp->stoppedForward = rrp->distanceForward;
    rrp->stoppedLeft = rrp->turnLeft;
  }
}

void BZRobotControl::SetTurnLeft(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->pendingUpdates[BZRobotPlayer::turnUpdate] = true;
  rrp->nextTurn = turn;
}

void BZRobotControl::SetTickDuration(void *_rrp,double duration)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->tickDuration = duration;
}

double BZRobotControl::GetBattleFieldSize(void *_rrp)
{
  if(!_rrp) return 0.0;
  //BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  // FIXME: Return proper value
  return 0.0;
}

double BZRobotControl::GetGunHeat(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->getReloadTime();
}

double BZRobotControl::GetVelocity(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->getSpeed();
}

double BZRobotControl::GetHeading(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->getAngle()*180.0f/M_PI;
}

double BZRobotControl::GetHeight(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->getDimensions()[2];
}

double BZRobotControl::GetWidth(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->getDimensions()[0];
}

double BZRobotControl::GetLength(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->getDimensions()[1];
}

long BZRobotControl::GetTime(void *_rrp)
{
  if(!_rrp) return 0;
  return TimeKeeper::getCurrent().getSeconds();
}

double BZRobotControl::GetX(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->getPosition()[0];
}

double BZRobotControl::GetY(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->getPosition()[1];
}

double BZRobotControl::GetZ(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->getPosition()[2];
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
