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
#include "RobotControl.h"


RobotCallbacks *RobotControl::CallbackSet(BZRobotPlayer *rrp)
{
  RobotCallbacks *cbset = new RobotCallbacks;
  cbset->data = rrp;
  cbset->Ahead = &RobotControl::Ahead;
  cbset->Back = &RobotControl::Back;
  cbset->ClearAllEvents = &RobotControl::ClearAllEvents;
  cbset->DoNothing = &RobotControl::DoNothing;
  cbset->Execute = &RobotControl::Execute;
  cbset->Fire = &RobotControl::Fire;
  cbset->FireBullet = &RobotControl::FireBullet;
  cbset->GetBattleFieldLength = &RobotControl::GetBattleFieldLength;
  cbset->GetBattleFieldWidth = &RobotControl::GetBattleFieldWidth;
  cbset->GetDistanceRemaining = &RobotControl::GetDistanceRemaining;
  cbset->GetEnergy = &RobotControl::GetEnergy;
  cbset->GetGunCoolingRate = &RobotControl::GetGunCoolingRate;
  cbset->GetGunHeat = &RobotControl::GetGunHeat;
  cbset->GetHeading = &RobotControl::GetHeading;
  cbset->GetHeight = &RobotControl::GetHeight;
  cbset->GetLength = &RobotControl::GetLength;
  cbset->GetName = &RobotControl::GetName;
  cbset->GetNumRounds = &RobotControl::GetNumRounds;
  cbset->GetOthers = &RobotControl::GetOthers;
  cbset->GetRadarHeading = &RobotControl::GetRadarHeading;
  cbset->GetRoundNum = &RobotControl::GetRoundNum;
  cbset->GetTime = &RobotControl::GetTime;
  cbset->GetTurnRemaining = &RobotControl::GetTurnRemaining;
  cbset->GetVelocity = &RobotControl::GetVelocity;
  cbset->GetWidth = &RobotControl::GetWidth;
  cbset->GetX = &RobotControl::GetX;
  cbset->GetY = &RobotControl::GetY;
  cbset->GetZ = &RobotControl::GetZ;
  cbset->Resume = &RobotControl::Resume;
  cbset->Scan = &RobotControl::Scan;
  cbset->SetAdjustGunForRobotTurn = &RobotControl::SetAdjustGunForRobotTurn;
  cbset->SetAdjustRadarForGunTurn = &RobotControl::SetAdjustRadarForGunTurn;
  cbset->SetAdjustRadarForRobotTurn = &RobotControl::SetAdjustRadarForRobotTurn;
  cbset->SetAhead = &RobotControl::SetAhead;
  cbset->SetFire = &RobotControl::SetFire;
  cbset->SetMaxVelocity = &RobotControl::SetMaxVelocity;
  cbset->SetResume = &RobotControl::SetResume;
  cbset->SetStop = &RobotControl::SetStop;
  cbset->SetTurnLeft = &RobotControl::SetTurnLeft;
  cbset->SetTurnRate = &RobotControl::SetTurnRate;
  cbset->Stop = &RobotControl::Stop;
  cbset->TurnGunLeft = &RobotControl::TurnGunLeft;
  cbset->TurnGunRight = &RobotControl::TurnGunRight;
  cbset->TurnLeft = &RobotControl::TurnLeft;
  cbset->TurnRadarLeft = &RobotControl::TurnRadarLeft;
  cbset->TurnRadarRight = &RobotControl::TurnRadarRight;
  cbset->TurnRight = &RobotControl::TurnRight;
  return cbset;
}

void RobotControl::Ahead(void *_rrp,double distance)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botAhead(distance);
}

void RobotControl::Back(void *_rrp,double distance)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botBack(distance);
}

void RobotControl::ClearAllEvents(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botClearAllEvents();
}

void RobotControl::DoNothing(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botDoNothing();
}

void RobotControl::Execute(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botExecute();
}

void RobotControl::Fire(void *_rrp, double power)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botFire(power);
}

BZRobots::Bullet* RobotControl::FireBullet(void *_rrp, double power)
{
  if(!_rrp) return NULL;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botFireBullet(power);
}

double RobotControl::GetBattleFieldLength(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetBattleFieldLength();
}

double RobotControl::GetBattleFieldWidth(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetBattleFieldWidth();
}

double RobotControl::GetDistanceRemaining(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetDistanceRemaining();
}

double RobotControl::GetEnergy(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetEnergy();
}

double RobotControl::GetGunCoolingRate(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetGunCoolingRate();
}

double RobotControl::GetGunHeading(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetGunHeading();
}

double RobotControl::GetGunHeat(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetGunHeat();
}

double RobotControl::GetHeading(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetHeading();
}

double RobotControl::GetHeight(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetHeight();
}

double RobotControl::GetLength(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetLength();
}

std::string RobotControl::GetName(void *_rrp)
{
  if(!_rrp) return "";
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetName();
}

int RobotControl::GetNumRounds(void *_rrp)
{
  if(!_rrp) return 0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetNumRounds();
}

int RobotControl::GetOthers(void *_rrp)
{
  if(!_rrp) return 0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetOthers();
}

double RobotControl::GetRadarHeading(void *_rrp)
{
  if(!_rrp) return 0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetRadarHeading();
}

int RobotControl::GetRoundNum(void *_rrp)
{
  if(!_rrp) return 0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetRoundNum();
}

double RobotControl::GetTime(void *_rrp)
{
  if(!_rrp) return 0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetTime();
}

double RobotControl::GetTurnRemaining(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetTurnRemaining();
}

double RobotControl::GetVelocity(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetVelocity();
}

double RobotControl::GetWidth(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetWidth();
}

double RobotControl::GetX(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetX();
}

double RobotControl::GetY(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetY();
}

double RobotControl::GetZ(void *_rrp)
{
  if(!_rrp) return 0.0;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botGetZ();
}

void RobotControl::SetAdjustGunForRobotTurn(void *_rrp, bool independent)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetAdjustGunForRobotTurn(independent);
}

void RobotControl::SetAdjustRadarForGunTurn(void *_rrp, bool independent)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetAdjustRadarForGunTurn(independent);
}

void RobotControl::SetAdjustRadarForRobotTurn(void *_rrp, bool independent)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetAdjustRadarForRobotTurn(independent);
}

void RobotControl::Resume(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botResume();
}

void RobotControl::Scan(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botScan();
}

void RobotControl::SetAhead(void *_rrp,double distance)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetAhead(distance);
}

void RobotControl::SetBack(void *_rrp,double distance)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetBack(distance);
}

void RobotControl::SetFire(void *_rrp, double power)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetFire(power);
}

BZRobots::Bullet* RobotControl::SetFireBullet(void *_rrp, double power)
{
  if(!_rrp) return NULL;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  return rrp->botSetFireBullet(power);
}

void RobotControl::SetMaxTurnRate(void *_rrp,double turnRate)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetMaxTurnRate(turnRate);
}

void RobotControl::SetMaxVelocity(void *_rrp,double speed)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetMaxVelocity(speed);
}

void RobotControl::SetResume(void *_rrp)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetResume();
}

void RobotControl::SetStop(void *_rrp,bool overwrite)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetStop(overwrite);
}

void RobotControl::SetTurnLeft(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetTurnLeft(turn);
}

void RobotControl::SetTurnRate(void *_rrp,double rate)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetTurnRate(rate);
}

void RobotControl::SetTurnRight(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botSetTurnRight(turn);
}

void RobotControl::Stop(void *_rrp,bool overwrite)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botStop(overwrite);
}

void RobotControl::TurnGunLeft(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botTurnGunLeft(turn);
}

void RobotControl::TurnGunRight(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botTurnGunRight(turn);
}

void RobotControl::TurnLeft(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botTurnLeft(turn);
}

void RobotControl::TurnRadarLeft(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botTurnRadarLeft(turn);
}

void RobotControl::TurnRadarRight(void *_rrp,double turn)
{
  if(!_rrp) return;
  BZRobotPlayer *rrp = (BZRobotPlayer *)_rrp;
  rrp->botTurnRadarRight(turn);
}

void RobotControl::TurnRight(void *_rrp,double turn)
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
