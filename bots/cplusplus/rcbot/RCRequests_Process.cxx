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

/* inteface header */
#include "RCRequests.h"

/* common implementation headers */
#include "BZDBCache.h"
#include "ObstacleMgr.h"

/* bzflag implementation headers */
#include "Roster.h"
#include "World.h"
#include "playing.h"

/* local implementation headers */
#include "RCReplies.h"
#include "RCRobotPlayer.h"
#include "MessageUtilities.h"
#include "BackendShot.h"


bool ExecuteReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  rrp->lastTickAt = BzTime::getCurrent().getSeconds();

  if (rrp->pendingUpdates[RCRobotPlayer::speedUpdate])
    rrp->speed = rrp->nextSpeed;
  if (rrp->pendingUpdates[RCRobotPlayer::turnRateUpdate])
    rrp->turnRate = rrp->nextTurnRate;

  if (rrp->pendingUpdates[RCRobotPlayer::distanceUpdate]) {
    if (rrp->nextDistance < 0.0f)
      rrp->distanceForward = false;
    else
      rrp->distanceForward = true;
    rrp->distanceRemaining = (rrp->distanceForward ? 1 : -1) * rrp->nextDistance;
  }
  if (rrp->pendingUpdates[RCRobotPlayer::turnUpdate]) {
    if (rrp->nextTurn < 0.0f)
      rrp->turnLeft = false;
    else
      rrp->turnLeft = true;
    rrp->turnRemaining = (rrp->turnLeft ? 1 : -1) * rrp->nextTurn * M_PI/180.0f; /* We have to convert to radians! */
  }

  for (int i = 0; i < RCRobotPlayer::updateCount; ++i)
    rrp->pendingUpdates[i] = false;

  if (rrp->shoot) {
    rrp->shoot = false;
    rrp->fireShot();
  }

  return true;
}

bool SetSpeedReq::process(RCRobotPlayer *rrp)
{
  rrp->nextSpeed = speed;
  rrp->pendingUpdates[RCRobotPlayer::speedUpdate] = true;
  return true;
}

bool SetTurnRateReq::process(RCRobotPlayer *rrp)
{
  rrp->nextTurnRate = rate;
  rrp->pendingUpdates[RCRobotPlayer::turnRateUpdate] = true;
  return true;
}

bool SetAheadReq::process(RCRobotPlayer *rrp)
{
  rrp->pendingUpdates[RCRobotPlayer::distanceUpdate] = true;
  rrp->nextDistance = distance;
  return true;
}

bool SetTurnLeftReq::process(RCRobotPlayer *rrp)
{
  rrp->pendingUpdates[RCRobotPlayer::turnUpdate] = true;
  rrp->nextTurn = turn;
  return true;
}

bool SetFireReq::process(RCRobotPlayer *rrp)
{
  rrp->shoot = true;
  return true;
}

bool SetResumeReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  if (rrp->hasStopped) {
    rrp->hasStopped = false;
    rrp->distanceRemaining = rrp->stoppedDistance;
    rrp->turnRemaining = rrp->stoppedTurn;
    rrp->distanceForward = rrp->stoppedForward;
    rrp->turnLeft = rrp->stoppedLeft;
  }
  return true;
}

bool GetGunHeatReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  link->sendm(GunHeatReply(rrp->getReloadTime()));
  return true;
}

bool GetDistanceRemainingReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  link->sendm(DistanceRemainingReply(rrp->distanceRemaining));
  return true;
}

bool GetTurnRemainingReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  link->sendm(TurnRemainingReply(rrp->turnRemaining * 180.0f/M_PI));
  return true;
}

bool GetTickDurationReq::process(RCRobotPlayer *rrp)
{
  link->sendf("GetTickDuration %f\n", rrp->tickDuration);
  return true;
}

bool SetTickDurationReq::process(RCRobotPlayer *rrp)
{
  rrp->tickDuration = duration;
  return true;
}

bool GetTickRemainingReq::process(RCRobotPlayer *rrp)
{
  if (rrp->isSteadyState())
    link->sendf("GetTickRemaining %f\n", (rrp->lastTickAt + rrp->tickDuration) - BzTime::getCurrent().getSeconds());
  else
    link->sendf("GetTickRemaining 0.0\n");

  return true;
}

bool GetBattleFieldSizeReq::process(RCRobotPlayer *)
{
  link->sendm(BattleFieldSizeReply(BZDBCache::worldSize));
  return true;
}

bool GetTeamsReq::process(RCRobotPlayer *)
{
  // TODO: Implement this. :p
  return true;
}
bool GetBasesReq::process(RCRobotPlayer *)
{
  // TODO: Implement this. :p
  return true;
}
bool GetFlagsReq::process(RCRobotPlayer *)
{
  // TODO: Implement this. :p
  return true;
}
bool GetShotsReq::process(RCRobotPlayer *)
{
  link->sendm(ShotsBeginReply());
  for (int i = 0; i < curMaxPlayers; i++) {
    if (!remotePlayers[i])
      continue;

    /*TeamColor team = remotePlayers[i]->getTeam();
    if (team == ObserverTeam)
      continue;
    if (team == startupInfo.team && startupInfo.team != AutomaticTeam)
      continue;*/

    for (int j = 0; j < remotePlayers[i]->getMaxShots(); j++) {
      ShotPath *path = remotePlayers[i]->getShot(j);
      if (!path || path->isExpired())
        continue;

      const FiringInfo &info = path->getFiringInfo();

      link->sendm(ShotReply(Shot(info.shot.player, info.shot.id)));
    }
  }

  return true;
}
bool GetMyTanksReq::process(RCRobotPlayer *)
{
  // TODO: Implement this. :p
  return true;
}
bool GetOtherTanksReq::process(RCRobotPlayer *)
{
  // TODO: Implement this. :p
  return true;
}
bool GetConstantsReq::process(RCRobotPlayer *)
{
  // TODO: Implement this. :p
  return true;
}

bool GetXReq::process(RCRobotPlayer *rrp)
{
  link->sendm(XReply(rrp->getPosition()[0]));
  return true;
}
bool GetYReq::process(RCRobotPlayer *rrp)
{
  link->sendm(YReply(rrp->getPosition()[1]));
  return true;
}
bool GetZReq::process(RCRobotPlayer *rrp)
{
  link->sendm(ZReply(rrp->getPosition()[2]));
  return true;
}

bool GetWidthReq::process(RCRobotPlayer *rrp)
{
  link->sendm(WidthReply(rrp->getDimensions()[0]));
  return true;
}
bool GetLengthReq::process(RCRobotPlayer *rrp)
{
  link->sendm(LengthReply(rrp->getDimensions()[1]));
  return true;
}
bool GetHeightReq::process(RCRobotPlayer *rrp)
{
  link->sendm(HeightReply(rrp->getDimensions()[2]));
  return true;
}
bool GetHeadingReq::process(RCRobotPlayer *rrp)
{
  link->sendm(HeadingReply(rrp->getAngle()*180.0f/M_PI));
  return true;
}

bool SetStopReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  if (!rrp->hasStopped || overwrite) {
    rrp->hasStopped = true;
    rrp->stoppedDistance = rrp->distanceRemaining;
    rrp->stoppedTurn = rrp->turnRemaining;
    rrp->stoppedForward = rrp->distanceForward;
    rrp->stoppedLeft = rrp->turnLeft;
  }
  return true;
}

bool GetPlayersReq::process(RCRobotPlayer *)
{
  link->sendm(PlayersBeginReply());
  for (int i = 0; i < curMaxPlayers; i++) {
    if (!remotePlayers[i])
      continue;
    if (robots[0]->getId() == remotePlayers[i]->getId())
      continue;

    TeamColor team = remotePlayers[i]->getTeam();
    if (team == ObserverTeam)
      continue;
    if (team == startupInfo.team && startupInfo.team != AutomaticTeam)
      continue;

		Tank tank(
			remotePlayers[i]->getCallSign(),
			Team::getShortName(remotePlayers[i]->getTeam()),
			remotePlayers[i]->getFlag()->flagName,
			remotePlayers[i]->isPaused(),
			remotePlayers[i]->isAlive(),
			remotePlayers[i]->canMove(),
			remotePlayers[i]->isFlagActive(),
			remotePlayers[i]->getPosition(),
			(remotePlayers[i]->getAngle()*180.0/M_PI),
			remotePlayers[i]->getVelocity(),
			remotePlayers[i]->getAngularVelocity()
			);

    link->sendm(PlayersReply(tank));
  }

  return true;
}

bool GetObstaclesReq::process(RCRobotPlayer *)
{
  unsigned int i;
  link->sendm(ObstaclesBeginReply());
  const ObstacleList &boxes = OBSTACLEMGR.getBoxes();
  for (i = 0; i < boxes.size(); i++) {
    Obstacle *obs = boxes[i];
    link->sendm(ObstacleReply(obs, boxType));
  }

  const ObstacleList &pyrs = OBSTACLEMGR.getPyrs();
  for (i = 0; i < pyrs.size(); i++) {
    Obstacle *obs = pyrs[i];
    link->sendm(ObstacleReply(obs, pyrType));
  }

  const ObstacleList &bases = OBSTACLEMGR.getBases();
  for (i = 0; i < bases.size(); i++) {
    Obstacle *obs = bases[i];
    link->sendm(ObstacleReply(obs, baseType));
  }

  const ObstacleList &meshes = OBSTACLEMGR.getMeshes();
  for (i = 0; i < meshes.size(); i++) {
    Obstacle *obs = meshes[i];
    link->sendm(ObstacleReply(obs, meshType));
  }

  const ObstacleList &walls = OBSTACLEMGR.getWalls();
  for (i = 0; i < walls.size(); i++) {
    Obstacle *obs = walls[i];
    link->sendm(ObstacleReply(obs, wallType));
  }
  return true;
}

bool GetShotPositionReq::process(RCRobotPlayer *)
{
	Shot shot(id);
  BackendShot bshot(shot);
  double x, y, z;

  bshot.getPosition(x, y, z, dt);

  link->sendm(ShotPositionReply(id, x, y, z));
  return true;
}

bool GetShotVelocityReq::process(RCRobotPlayer *)
{
	Shot shot(id);
  BackendShot bshot(shot);
  double x, y, z;

  bshot.getVelocity(x, y, z, dt);

  link->sendm(ShotVelocityReply(id, x, y, z));
  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
