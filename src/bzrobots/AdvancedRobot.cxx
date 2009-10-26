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
#include "AdvancedRobot.h"

/* implementation headers */
#include "RobotCallbacks.h"

#define BOT_CALLBACKS ((RobotCallbacks *)robotcb)
#define BOT_CLIENT_PLAYER ((RobotCallbacks *)robotcb)->data

BZRobots::AdvancedRobot::AdvancedRobot()
{
  robotType = "AdvancedRobot";
}

BZRobots::AdvancedRobot::~AdvancedRobot()
{

}

void BZRobots::AdvancedRobot::clearAllEvents()
{
  if(robotcb && BOT_CALLBACKS->ClearAllEvents)
    BOT_CALLBACKS->ClearAllEvents(BOT_CLIENT_PLAYER);
}

void BZRobots::AdvancedRobot::execute()
{
  if(robotcb && BOT_CALLBACKS->Execute)
    BOT_CALLBACKS->Execute(BOT_CLIENT_PLAYER);
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getAllEvents()
{
  if(robotcb && BOT_CALLBACKS->GetAllEvents) {
    return BOT_CALLBACKS->GetAllEvents(BOT_CLIENT_PLAYER);
  } else {
     std::list<BZRobots::Event> events;
	 return events; 
  }
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getBulletHitBulletEvents()
{
  if(robotcb && BOT_CALLBACKS->GetBulletHitBulletEvents) {
    return BOT_CALLBACKS->GetBulletHitBulletEvents(BOT_CLIENT_PLAYER);
  } else {
    std::list<BZRobots::Event> events;
	return events; 
  }
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getBulletHitEvents()
{
  if(robotcb && BOT_CALLBACKS->GetBulletHitEvents) {
    return BOT_CALLBACKS->GetBulletHitEvents(BOT_CLIENT_PLAYER);
  } else {
    std::list<BZRobots::Event> events;
	return events; 
  }
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getBulletMissedEvents()
{
  if(robotcb && BOT_CALLBACKS->GetBulletMissedEvents) {
    return BOT_CALLBACKS->GetBulletMissedEvents(BOT_CLIENT_PLAYER);
  } else {
    std::list<BZRobots::Event> events;
	return events; 
  }
}

double BZRobots::AdvancedRobot::getDistanceRemaining() const
{
  if(robotcb && BOT_CALLBACKS->GetDistanceRemaining)
    return BOT_CALLBACKS->GetDistanceRemaining(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobots::AdvancedRobot::getGunHeadingRadians() const
{
  if(robotcb && BOT_CALLBACKS->GetGunHeadingRadians)
    return BOT_CALLBACKS->GetGunHeadingRadians(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobots::AdvancedRobot::getGunTurnRemaining() const
{
  if(robotcb && BOT_CALLBACKS->GetGunTurnRemaining)
    return BOT_CALLBACKS->GetGunTurnRemaining(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobots::AdvancedRobot::getGunTurnRemainingRadians() const
{
  if(robotcb && BOT_CALLBACKS->GetGunTurnRemainingRadians)
    return BOT_CALLBACKS->GetGunTurnRemainingRadians(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobots::AdvancedRobot::getHeadingRadians() const
{
  if(robotcb && BOT_CALLBACKS->GetHeadingRadians)
    return BOT_CALLBACKS->GetHeadingRadians(BOT_CLIENT_PLAYER);
  return 0.0;
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getHitByBulletEvents()
{
  if(robotcb && BOT_CALLBACKS->GetHitByBulletEvents) {
    return BOT_CALLBACKS->GetHitByBulletEvents(BOT_CLIENT_PLAYER);
  } else {
    std::list<BZRobots::Event> events;
	return events; 
  }
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getHitRobotEvents()
{
  if(robotcb && BOT_CALLBACKS->GetHitRobotEvents) {
    return BOT_CALLBACKS->GetHitRobotEvents(BOT_CLIENT_PLAYER);
  } else {
    std::list<BZRobots::Event> events;
	return events; 
  }
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getHitWallEvents()
{
  if(robotcb && BOT_CALLBACKS->GetHitWallEvents) {
    return BOT_CALLBACKS->GetHitWallEvents(BOT_CLIENT_PLAYER);
  } else {
    std::list<BZRobots::Event> events;
	return events; 
  }
}

double BZRobots::AdvancedRobot::getRadarHeadingRadians() const
{
  if(robotcb && BOT_CALLBACKS->GetRadarHeadingRadians)
    return BOT_CALLBACKS->GetRadarHeadingRadians(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobots::AdvancedRobot::getRadarTurnRemaining() const
{
  if(robotcb && BOT_CALLBACKS->GetRadarTurnRemaining)
    return BOT_CALLBACKS->GetRadarTurnRemaining(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobots::AdvancedRobot::getRadarTurnRemainingRadians() const
{
  if(robotcb && BOT_CALLBACKS->GetRadarTurnRemainingRadians)
    return BOT_CALLBACKS->GetRadarTurnRemainingRadians(BOT_CLIENT_PLAYER);
  return 0.0;
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getRobotDeathEvents()
{
  if(robotcb && BOT_CALLBACKS->GetRobotDeathEvents) {
    return BOT_CALLBACKS->GetRobotDeathEvents(BOT_CLIENT_PLAYER);
  } else {
    std::list<BZRobots::Event> events;
	return events; 
  }
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getScannedRobotEvents()
{
  if(robotcb && BOT_CALLBACKS->GetScannedRobotEvents) {
    return BOT_CALLBACKS->GetScannedRobotEvents(BOT_CLIENT_PLAYER);
  } else {
    std::list<BZRobots::Event> events;
	return events; 
  }
}

std::list<BZRobots::Event> BZRobots::AdvancedRobot::getStatusEvents()
{
  if(robotcb && BOT_CALLBACKS->GetStatusEvents) {
    return BOT_CALLBACKS->GetStatusEvents(BOT_CLIENT_PLAYER);
  } else {
    std::list<BZRobots::Event> events;
	return events; 
  }
}

double BZRobots::AdvancedRobot::getTurnRemaining() const
{
  if(robotcb && BOT_CALLBACKS->GetTurnRemaining)
    return BOT_CALLBACKS->GetTurnRemaining(BOT_CLIENT_PLAYER);
  return 0.0;
}

double BZRobots::AdvancedRobot::getTurnRemainingRadians() const
{
  if(robotcb && BOT_CALLBACKS->GetTurnRemainingRadians)
    return BOT_CALLBACKS->GetTurnRemainingRadians(BOT_CLIENT_PLAYER);
  return 0.0;
}

bool BZRobots::AdvancedRobot::isAdjustGunForRobotTurn() const
{
  if(robotcb && BOT_CALLBACKS->IsAdjustGunForRobotTurn)
    return BOT_CALLBACKS->IsAdjustGunForRobotTurn(BOT_CLIENT_PLAYER);
  return 0.0;
}

bool BZRobots::AdvancedRobot::isAdjustRadarForGunTurn() const
{
  if(robotcb && BOT_CALLBACKS->IsAdjustRadarForGunTurn)
    return BOT_CALLBACKS->IsAdjustRadarForGunTurn(BOT_CLIENT_PLAYER);
  return 0.0;
}

bool BZRobots::AdvancedRobot::isAdjustRadarForRobotTurn() const
{
  if(robotcb && BOT_CALLBACKS->IsAdjustRadarForRobotTurn)
    return BOT_CALLBACKS->IsAdjustRadarForRobotTurn(BOT_CLIENT_PLAYER);
  return 0.0;
}

void BZRobots::AdvancedRobot::setAhead(double distance)
{
  if(robotcb && BOT_CALLBACKS->SetAhead)
    BOT_CALLBACKS->SetAhead(BOT_CLIENT_PLAYER,distance);
}

void BZRobots::AdvancedRobot::setBack(double distance)
{
  if(robotcb && BOT_CALLBACKS->SetBack)
    BOT_CALLBACKS->SetBack(BOT_CLIENT_PLAYER,distance);
}

void BZRobots::AdvancedRobot::setFire(double power)
{
  if(robotcb && BOT_CALLBACKS->SetFire)
    BOT_CALLBACKS->SetFire(BOT_CLIENT_PLAYER,power);
}

BZRobots::Bullet* BZRobots::AdvancedRobot::setFireBullet(double power) const
{
  if(robotcb && BOT_CALLBACKS->SetFireBullet)
    return BOT_CALLBACKS->SetFireBullet(BOT_CLIENT_PLAYER,power);
  return NULL;
}

void BZRobots::AdvancedRobot::setMaxTurnRate(double maxTurnRate)
{
  if(robotcb && BOT_CALLBACKS->SetMaxTurnRate)
    BOT_CALLBACKS->SetMaxTurnRate(BOT_CLIENT_PLAYER,maxTurnRate);
}

void BZRobots::AdvancedRobot::setMaxVelocity(double maxVelocity)
{
  if(robotcb && BOT_CALLBACKS->SetMaxVelocity)
    BOT_CALLBACKS->SetMaxVelocity(BOT_CLIENT_PLAYER,maxVelocity);
}

void BZRobots::AdvancedRobot::setResume()
{
  if(robotcb && BOT_CALLBACKS->SetResume)
    BOT_CALLBACKS->SetResume(BOT_CLIENT_PLAYER);
}

void BZRobots::AdvancedRobot::setStop(bool overwrite)
{
  if(robotcb && BOT_CALLBACKS->SetStop)
    BOT_CALLBACKS->SetStop(BOT_CLIENT_PLAYER,overwrite);
}

void BZRobots::AdvancedRobot::setTurnGunLeft(double degrees)
{
  if(robotcb && BOT_CALLBACKS->SetTurnGunLeft)
    BOT_CALLBACKS->SetTurnGunLeft(BOT_CLIENT_PLAYER,degrees);
}

void BZRobots::AdvancedRobot::setTurnGunLeftRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->SetTurnGunLeftRadians)
    BOT_CALLBACKS->SetTurnGunLeftRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::setTurnGunRight(double degrees)
{
  if(robotcb && BOT_CALLBACKS->SetTurnGunRight)
    BOT_CALLBACKS->SetTurnGunRight(BOT_CLIENT_PLAYER,degrees);
}

void BZRobots::AdvancedRobot::setTurnGunRightRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->SetTurnGunRightRadians)
    BOT_CALLBACKS->SetTurnGunRightRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::setTurnLeft(double degrees)
{
  if(robotcb && BOT_CALLBACKS->SetTurnLeft)
    BOT_CALLBACKS->SetTurnLeft(BOT_CLIENT_PLAYER,degrees);
}

void BZRobots::AdvancedRobot::setTurnLeftRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->SetTurnLeftRadians)
    BOT_CALLBACKS->SetTurnLeftRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::setTurnRadarLeft(double degrees)
{
  if(robotcb && BOT_CALLBACKS->SetTurnRadarLeft)
    BOT_CALLBACKS->SetTurnRadarLeft(BOT_CLIENT_PLAYER,degrees);
}

void BZRobots::AdvancedRobot::setTurnRadarLeftRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->SetTurnRadarLeftRadians)
    BOT_CALLBACKS->SetTurnRadarLeftRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::setTurnRadarRight(double degrees)
{
  if(robotcb && BOT_CALLBACKS->SetTurnRadarRight)
    BOT_CALLBACKS->SetTurnRadarRight(BOT_CLIENT_PLAYER,degrees);
}

void BZRobots::AdvancedRobot::setTurnRadarRightRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->SetTurnRadarRightRadians)
    BOT_CALLBACKS->SetTurnRadarRightRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::setTurnRight(double degrees)
{
  if(robotcb && BOT_CALLBACKS->SetTurnRight)
    BOT_CALLBACKS->SetTurnRight(BOT_CLIENT_PLAYER,degrees);
}

void BZRobots::AdvancedRobot::setTurnRightRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->SetTurnRightRadians)
    BOT_CALLBACKS->SetTurnRightRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::turnGunLeftRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->TurnGunLeftRadians)
    BOT_CALLBACKS->TurnGunLeftRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::turnGunRightRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->TurnGunRightRadians)
    BOT_CALLBACKS->TurnGunRightRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::turnLeftRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->TurnLeftRadians)
    BOT_CALLBACKS->TurnLeftRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::turnRadarLeftRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->TurnRadarLeftRadians)
    BOT_CALLBACKS->TurnRadarLeftRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::turnRadarRightRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->TurnRadarRightRadians)
    BOT_CALLBACKS->TurnRadarRightRadians(BOT_CLIENT_PLAYER,radians);
}

void BZRobots::AdvancedRobot::turnRightRadians(double radians)
{
  if(robotcb && BOT_CALLBACKS->TurnRightRadians)
    BOT_CALLBACKS->TurnRightRadians(BOT_CLIENT_PLAYER,radians);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
