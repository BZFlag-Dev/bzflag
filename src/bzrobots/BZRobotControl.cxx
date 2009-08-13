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
  cbset->Ahead = &BZRobotControl::Ahead;
  cbset->Back = &BZRobotControl::Back;
  cbset->Execute = &BZRobotControl::Execute;
  cbset->Fire = &BZRobotControl::Fire;
  cbset->GetBattleFieldSize = &BZRobotControl::GetBattleFieldSize;
  cbset->GetDistanceRemaining = &BZRobotControl::GetDistanceRemaining;
  cbset->GetGunCoolingRate = &BZRobotControl::GetGunCoolingRate;
  cbset->GetGunHeat = &BZRobotControl::GetGunHeat;
  cbset->GetHeading = &BZRobotControl::GetHeading;
  cbset->GetHeight = &BZRobotControl::GetHeight;
  cbset->GetName = &BZRobotControl::GetName;
  cbset->GetLength = &BZRobotControl::GetLength;
  cbset->GetTime = &BZRobotControl::GetTime;
  cbset->GetTurnRemaining = &BZRobotControl::GetTurnRemaining;
  cbset->GetVelocity = &BZRobotControl::GetVelocity;
  cbset->GetWidth = &BZRobotControl::GetWidth;
  cbset->GetX = &BZRobotControl::GetX;
  cbset->GetY = &BZRobotControl::GetY;
  cbset->GetZ = &BZRobotControl::GetZ;
  cbset->Resume = &BZRobotControl::Resume;
  cbset->Scan = &BZRobotControl::Scan;
  cbset->SetAhead = &BZRobotControl::SetAhead;
  cbset->SetFire = &BZRobotControl::SetFire;
  cbset->SetMaxVelocity = &BZRobotControl::SetMaxVelocity;
  cbset->SetResume = &BZRobotControl::SetResume;
  cbset->SetStop = &BZRobotControl::SetStop;
  cbset->SetTurnLeft = &BZRobotControl::SetTurnLeft;
  cbset->SetTurnRate = &BZRobotControl::SetTurnRate;
  cbset->Stop = &BZRobotControl::Stop;
  cbset->TurnLeft = &BZRobotControl::TurnLeft;
  cbset->TurnRight = &BZRobotControl::TurnRight;
  return cbset;
}

void BZRobotControl::Ahead(void *_rrp,double distance)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botAhead(distance);
}

void BZRobotControl::Back(void *_rrp,double distance)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botBack(distance);
}

void BZRobotControl::DoNothing(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botDoNothing();
}

void BZRobotControl::Execute(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botExecute();
}

void BZRobotControl::Fire(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botFire();
}

double BZRobotControl::GetBattleFieldSize(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetBattleFieldSize();
}

double BZRobotControl::GetDistanceRemaining(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetDistanceRemaining();
}

double BZRobotControl::GetGunCoolingRate(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetGunCoolingRate();
}

double BZRobotControl::GetGunHeat(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetGunHeat();
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

const char * BZRobotControl::GetName(void *_rrp)
{
  if(!_rrp) return "";
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetName();
}

double BZRobotControl::GetLength(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetLength();
}

double BZRobotControl::GetTime(void *_rrp)
{
  if(!_rrp) return 0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetTime();
}

double BZRobotControl::GetTurnRemaining(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetTurnRemaining();
}

double BZRobotControl::GetVelocity(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetVelocity();
}

double BZRobotControl::GetWidth(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetWidth();
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

void BZRobotControl::Resume(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botResume();
}

void BZRobotControl::Scan(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botScan();
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

void BZRobotControl::SetTurnRate(void *_rrp,double rate)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetTurnRate(rate);
}

void BZRobotControl::Stop(void *_rrp,bool overwrite)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botStop(overwrite);
}

void BZRobotControl::TurnLeft(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botTurnLeft(turn);
}

void BZRobotControl::TurnRight(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botTurnRight(turn);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
