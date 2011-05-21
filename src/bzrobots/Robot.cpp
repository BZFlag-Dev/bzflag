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

/* interface header */
#include "Robot.h"

/* implementation headers */
#include "RobotCallbacks.h"

#define BOT_CALLBACKS ((RobotCallbacks *)robotcb)
#define BOT_CLIENT_PLAYER ((RobotCallbacks *)robotcb)->data


BZRobots::Robot::Robot() {
  robotcb = NULL;
  robotType = "Robot";
}

BZRobots::Robot::~Robot() {
}

void BZRobots::Robot::setCallbacks(void* _robotcb) {
  robotcb = _robotcb;
}


void BZRobots::Robot::ahead(double distance) {
  if (robotcb && BOT_CALLBACKS->Ahead) {
    BOT_CALLBACKS->Ahead(BOT_CLIENT_PLAYER, distance);
  }
}

void BZRobots::Robot::back(double distance) {
  if (robotcb && BOT_CALLBACKS->Back) {
    BOT_CALLBACKS->Back(BOT_CLIENT_PLAYER, distance);
  }
}

void BZRobots::Robot::doNothing() {
  if (robotcb && BOT_CALLBACKS->DoNothing) {
    BOT_CALLBACKS->DoNothing(BOT_CLIENT_PLAYER);
  }
}

void BZRobots::Robot::fire(double power) {
  if (robotcb && BOT_CALLBACKS->Fire) {
    BOT_CALLBACKS->Fire(BOT_CLIENT_PLAYER, power);
  }
}

BZRobots::Bullet* BZRobots::Robot::fireBullet(double power) const {
  if (robotcb && BOT_CALLBACKS->Fire) {
    BOT_CALLBACKS->FireBullet(BOT_CLIENT_PLAYER, power);
  }
  return NULL;
}

double BZRobots::Robot::getBattleFieldLength() const {
  if (robotcb && BOT_CALLBACKS->GetBattleFieldLength) {
    return BOT_CALLBACKS->GetBattleFieldLength(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getBattleFieldWidth() const {
  if (robotcb && BOT_CALLBACKS->GetBattleFieldWidth) {
    return BOT_CALLBACKS->GetBattleFieldWidth(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}


double BZRobots::Robot::getEnergy() const {
  if (robotcb && BOT_CALLBACKS->GetEnergy) {
    return BOT_CALLBACKS->GetEnergy(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getGunCoolingRate() const {
  if (robotcb && BOT_CALLBACKS->GetGunCoolingRate) {
    return BOT_CALLBACKS->GetGunCoolingRate(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getGunHeading() const {
  if (robotcb && BOT_CALLBACKS->GetGunHeading) {
    return BOT_CALLBACKS->GetGunHeading(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getGunHeat() const {
  if (robotcb && BOT_CALLBACKS->GetGunHeat) {
    return BOT_CALLBACKS->GetGunHeat(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getHeading() const {
  if (robotcb && BOT_CALLBACKS->GetHeading) {
    return BOT_CALLBACKS->GetHeading(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getHeight() const {
  if (robotcb && BOT_CALLBACKS->GetHeight) {
    return BOT_CALLBACKS->GetHeight(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getLength() const {
  if (robotcb && BOT_CALLBACKS->GetLength) {
    return BOT_CALLBACKS->GetLength(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

std::string BZRobots::Robot::getName() const {
  if (robotcb && BOT_CALLBACKS->GetName) {
    return BOT_CALLBACKS->GetName(BOT_CLIENT_PLAYER);
  }
  return "";
}

int BZRobots::Robot::getNumRounds() const {
  if (robotcb && BOT_CALLBACKS->GetNumRounds) {
    return BOT_CALLBACKS->GetNumRounds(BOT_CLIENT_PLAYER);
  }
  return 0;
}

int BZRobots::Robot::getOthers() const {
  if (robotcb && BOT_CALLBACKS->GetOthers) {
    return BOT_CALLBACKS->GetOthers(BOT_CLIENT_PLAYER);
  }
  return 0;
}

double BZRobots::Robot::getRadarHeading() const {
  if (robotcb && BOT_CALLBACKS->GetRadarHeading) {
    return BOT_CALLBACKS->GetRadarHeading(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

int BZRobots::Robot::getRoundNum() const {
  if (robotcb && BOT_CALLBACKS->GetRoundNum) {
    return BOT_CALLBACKS->GetRoundNum(BOT_CLIENT_PLAYER);
  }
  return 0;
}

double BZRobots::Robot::getTime() const {
  if (robotcb && BOT_CALLBACKS->GetTime) {
    return BOT_CALLBACKS->GetTime(BOT_CLIENT_PLAYER);
  }
  return 0;
}

double BZRobots::Robot::getVelocity() const {
  if (robotcb && BOT_CALLBACKS->GetVelocity) {
    return BOT_CALLBACKS->GetVelocity(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getWidth() const {
  if (robotcb && BOT_CALLBACKS->GetWidth) {
    return BOT_CALLBACKS->GetWidth(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getX() const {
  if (robotcb && BOT_CALLBACKS->GetX) {
    return BOT_CALLBACKS->GetX(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getY() const {
  if (robotcb && BOT_CALLBACKS->GetY) {
    return BOT_CALLBACKS->GetY(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

double BZRobots::Robot::getZ() const {
  if (robotcb && BOT_CALLBACKS->GetZ) {
    return BOT_CALLBACKS->GetZ(BOT_CLIENT_PLAYER);
  }
  return 0.0f;
}

void BZRobots::Robot::resume() {
  if (robotcb && BOT_CALLBACKS->Resume) {
    BOT_CALLBACKS->Resume(BOT_CLIENT_PLAYER);
  }
}

void BZRobots::Robot::scan() {
  if (robotcb && BOT_CALLBACKS->Scan) {
    BOT_CALLBACKS->Scan(BOT_CLIENT_PLAYER);
  }
}

void BZRobots::Robot::setAdjustGunForRobotTurn(bool independent) {
  if (robotcb && BOT_CALLBACKS->SetAdjustGunForRobotTurn) {
    BOT_CALLBACKS->SetAdjustGunForRobotTurn(BOT_CLIENT_PLAYER, independent);
  }
}

void BZRobots::Robot::setAdjustRadarForGunTurn(bool independent) {
  if (robotcb && BOT_CALLBACKS->SetAdjustRadarForGunTurn) {
    BOT_CALLBACKS->SetAdjustRadarForGunTurn(BOT_CLIENT_PLAYER, independent);
  }
}

void BZRobots::Robot::setAdjustRadarForRobotTurn(bool independent) {
  if (robotcb && BOT_CALLBACKS->SetAdjustRadarForRobotTurn) {
    BOT_CALLBACKS->SetAdjustRadarForRobotTurn(BOT_CLIENT_PLAYER, independent);
  }
}

void BZRobots::Robot::stop(bool overwrite) {
  if (robotcb && BOT_CALLBACKS->Stop) {
    BOT_CALLBACKS->Stop(BOT_CLIENT_PLAYER, overwrite);
  }
}

void BZRobots::Robot::turnGunLeft(double degrees) {
  if (robotcb && BOT_CALLBACKS->TurnGunLeft) {
    BOT_CALLBACKS->TurnGunLeft(BOT_CLIENT_PLAYER, degrees);
  }
}

void BZRobots::Robot::turnGunRight(double degrees) {
  if (robotcb && BOT_CALLBACKS->TurnGunRight) {
    BOT_CALLBACKS->TurnGunRight(BOT_CLIENT_PLAYER, degrees);
  }
}

void BZRobots::Robot::turnLeft(double degrees) {
  if (robotcb && BOT_CALLBACKS->TurnLeft) {
    BOT_CALLBACKS->TurnLeft(BOT_CLIENT_PLAYER, degrees);
  }
}

void BZRobots::Robot::turnRadarLeft(double degrees) {
  if (robotcb && BOT_CALLBACKS->TurnRadarLeft) {
    BOT_CALLBACKS->TurnRadarLeft(BOT_CLIENT_PLAYER, degrees);
  }
}

void BZRobots::Robot::turnRadarRight(double degrees) {
  if (robotcb && BOT_CALLBACKS->TurnRadarRight) {
    BOT_CALLBACKS->TurnRadarRight(BOT_CLIENT_PLAYER, degrees);
  }
}

void BZRobots::Robot::turnRight(double degrees) {
  if (robotcb && BOT_CALLBACKS->TurnRight) {
    BOT_CALLBACKS->TurnRight(BOT_CLIENT_PLAYER, degrees);
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
