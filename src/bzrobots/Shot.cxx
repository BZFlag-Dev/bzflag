/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "Shot.h"

/* bzflag implementation headers */
#include "Roster.h"

/* local implementation headers */
#include "MessageUtilities.h"
#include "Team.h"
#include "Flag.h"
#include "RCRequest.h"
#include "RCRequests.h"

Shot::Shot() {}

Shot::Shot(uint64_t _id) : id(_id)
{
}

Shot::Shot(PlayerId _plr, uint16_t _id)
{
  id = ((uint64_t)_plr << 16) + _id;
}

PlayerId Shot::getPlayerId(void) const
{
  return id >> 16;
}

uint16_t Shot::getShotId(void) const
{
  return id & 0xffff;
}

uint64_t Shot::getId(void) const
{
  return id;
}

void Shot::setId(uint64_t _id)
{
  id = _id;
}

std::ostream& operator<<(std::ostream& os, const Shot& shot)
{
  return os << shot.getId();
}

messageParseStatus Shot::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;

  if (!MessageUtilities::parse(arguments[0], id))
    return InvalidArguments;

  return ParseOk;
}

ShotPath *searchShot(PlayerId plr, uint16_t shotid)
{
  if (plr >= curMaxPlayers) {
    return NULL;
  }

  for (int i = 0; i < player[plr]->getMaxShots(); i++) {
    ShotPath *path = player[plr]->getShot(i);
    if (!path || path->getFiringInfo().shot.id != shotid)
      continue;

    return path;
  }
  
  return NULL;
}

void Shot::getPosition(double &x, double &y, double &z, double dt) const
{
  ShotPath *path = searchShot(getPlayerId(), getShotId());

  if(path) {
    /*
     * We found the shot.
     */

    if (dt == 0) {
      const float *pos = path->getPosition();
      x = pos[0];
      y = pos[1];
      z = pos[2];
    } else {
      //Make a copy of the ShotPath/ShotStrategy, run update(dt) and check the new position
      //TODO: Find a way to do this, we can easily copy ShotPath but to get ShotStrategy
      //we'd have to access a protected member function
      //
      //Now just return the current position...
      //

      const float *pos = path->getPosition();
      x = pos[0];
      y = pos[1];
      z = pos[2];
    }
  }
}

void Shot::getVelocity(double &x, double &y, double &z, double dt) const
{
  ShotPath *path = searchShot(getPlayerId(), getShotId());
  
  if(path) {
    /*
     * We found the shot.
     */

    if (dt == 0) {
      const float *pos = path->getVelocity();
      x = pos[0];
      y = pos[1];
      z = pos[2];
    } else {
      //Make a copy of the ShotPath/ShotStrategy, run update(dt) and check the new velocity
      //TODO: Find a way to do this, we can easily copy ShotPath but to get ShotStrategy
      //we'd have to access a protected member function
      //
      //Now just return the current velocity...
      //

      const float *pos = path->getVelocity();
      x = pos[0];
      y = pos[1];
      z = pos[2];
    }
  }
}

FrontendShot::FrontendShot() : Shot()
{
  robot = NULL;
}

void FrontendShot::setRobot(const BZAdvancedRobot *_robot)
{
  robot = _robot;
}

void FrontendShot::getPosition(double &tox, double &toy, double &toz, double dt) const
{
  RCLinkFrontend *link = robot->getLink();
  link->sendAndProcess(GetShotPositionReq(id,dt), robot);
  tox = x;
  toy = y;
  toz = z;
}

void FrontendShot::getVelocity(double &tox, double &toy, double &toz, double dt) const
{
  RCLinkFrontend *link = robot->getLink();
  link->sendAndProcess(GetShotVelocityReq(id,dt), robot);
  tox = vx;
  toy = vy;
  toz = vz;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
