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
#include "RobotStatus.h"

BZRobots::RobotStatus::RobotStatus() :
  botDistanceRemaining(0.0f),
  botEnergy(0.0f),
  botGunHeading(0.0f),
  botGunHeadingRadians(0.0f),
  botGunHeat(0.0f),
  botGunTurnRemaining(0.0f),
  botGunTurnRemainingRadians(0.0f),
  botHeading(0.0f),
  botHeadingRadians(0.0f),
  botNumRounds(0),
  botOthers(0),
  botRadarHeading(0.0f),
  botRadarHeadingRadians(0.0f),
  botRadarTurnRemaining(0.0f),
  botRadarTurnRemainingRadians(0.0f),
  botRoundNum(0),
  botTime(0.0f),
  botTurnRemaining(0.0f),
  botTurnRemainingRadians(0.0f),
  botVelocity(0.0f),
  botX(0.0f),
  botY(0.0f),
  botZ(0.0f) {

}

BZRobots::RobotStatus::RobotStatus(
  double _botDistanceRemaining,
  double _botEnergy,
  double _botGunHeading,
  double _botGunHeadingRadians,
  double _botGunHeat,
  double _botGunTurnRemaining,
  double _botGunTurnRemainingRadians,
  double _botHeading,
  double _botHeadingRadians,
  int    _botNumRounds,
  int    _botOthers,
  double _botRadarHeading,
  double _botRadarHeadingRadians,
  double _botRadarTurnRemaining,
  double _botRadarTurnRemainingRadians,
  int    _botRoundNum,
  double _botTime,
  double _botTurnRemaining,
  double _botTurnRemainingRadians,
  double _botVelocity,
  double _botX,
  double _botY,
  double _botZ
) :
  botDistanceRemaining(_botDistanceRemaining),
  botEnergy(_botEnergy),
  botGunHeading(_botGunHeading),
  botGunHeadingRadians(_botGunHeadingRadians),
  botGunHeat(_botGunHeat),
  botGunTurnRemaining(_botGunTurnRemaining),
  botGunTurnRemainingRadians(_botGunTurnRemainingRadians),
  botHeading(_botHeading),
  botHeadingRadians(_botHeadingRadians),
  botNumRounds(_botNumRounds),
  botOthers(_botOthers),
  botRadarHeading(_botRadarHeading),
  botRadarHeadingRadians(_botRadarHeadingRadians),
  botRadarTurnRemaining(_botRadarTurnRemaining),
  botRadarTurnRemainingRadians(_botRadarTurnRemainingRadians),
  botRoundNum(_botRoundNum),
  botTime(_botTime),
  botTurnRemaining(_botTurnRemaining),
  botTurnRemainingRadians(_botTurnRemainingRadians),
  botVelocity(_botVelocity),
  botX(_botX),
  botY(_botY),
  botZ(_botZ) {

}

BZRobots::RobotStatus::~RobotStatus() {

}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
