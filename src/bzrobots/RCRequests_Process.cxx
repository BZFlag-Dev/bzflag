#include "RCRequests.h"

#include "RCReplies.h"
#include "RCRobotPlayer.h"
#include "MessageUtilities.h"
#include "BZDBCache.h"
#include "Roster.h"
#include "World.h"
#include "playing.h"

#include "version.h"


bool ExecuteReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  rrp->lastTickAt = TimeKeeper::getCurrent().getSeconds();

  if (rrp->pendingUpdates[RCRobotPlayer::speedUpdate])
    rrp->speed = rrp->nextSpeed;
  if (rrp->pendingUpdates[RCRobotPlayer::turnRateUpdate])
    rrp->turnRate = rrp->nextTurnRate;

  if (rrp->pendingUpdates[RCRobotPlayer::distanceUpdate])
  {
    if (rrp->nextDistance < 0.0f)
      rrp->distanceForward = false;
    else 
      rrp->distanceForward = true;
    rrp->distanceRemaining = (rrp->distanceForward ? 1 : -1) * rrp->nextDistance;
  }
  if (rrp->pendingUpdates[RCRobotPlayer::turnUpdate])
  {
    if (rrp->nextTurn < 0.0f)
      rrp->turnLeft = false;
    else
      rrp->turnLeft = true;
    rrp->turnRemaining = (rrp->turnLeft ? 1 : -1) * rrp->nextTurn * M_PI/180.0f; /* We have to convert to radians! */
  }

  for (int i = 0; i < RCRobotPlayer::updateCount; ++i)
    rrp->pendingUpdates[i] = false;

  if (rrp->shoot)
  {
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

  if (rrp->hasStopped)
  {
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

  link->send(GunHeatReply(rrp->getReloadTime()));
  return true;
}

bool GetDistanceRemainingReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  link->send(DistanceRemainingReply(rrp->distanceRemaining));
  return true;
}

bool GetTurnRemainingReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  link->send(TurnRemainingReply(rrp->turnRemaining * 180.0f/M_PI));
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
    link->sendf("GetTickRemaining %f\n", (rrp->lastTickAt + rrp->tickDuration) - TimeKeeper::getCurrent().getSeconds());
  else
    link->send("GetTickRemaining 0.0\n");

  return true;
}

bool GetBattleFieldSizeReq::process(RCRobotPlayer *)
{
  link->send(BattleFieldSizeReply(BZDBCache::worldSize));
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
bool GetObstaclesReq::process(RCRobotPlayer *)
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
  // TODO: Implement this. :p
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
  link->send(XReply(rrp->getPosition()[0]));
  return true;
}
bool GetYReq::process(RCRobotPlayer *rrp)
{
  link->send(YReply(rrp->getPosition()[1]));
  return true;
}
bool GetZReq::process(RCRobotPlayer *rrp)
{
  link->send(ZReply(rrp->getPosition()[2]));
  return true;
}

bool GetWidthReq::process(RCRobotPlayer *rrp)
{
  link->send(WidthReply(rrp->getDimensions()[0]));
  return true;
}
bool GetLengthReq::process(RCRobotPlayer *rrp)
{
  link->send(LengthReply(rrp->getDimensions()[1]));
  return true;
}
bool GetHeightReq::process(RCRobotPlayer *rrp)
{
  link->send(HeightReply(rrp->getDimensions()[2]));
  return true;
}
bool GetHeadingReq::process(RCRobotPlayer *rrp)
{
  link->send(HeadingReply(rrp->getAngle()*180.0f/M_PI));
  return true;
}

bool SetStopReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;

  if (!rrp->hasStopped || overwrite)
  {
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
  link->send(PlayersBeginReply());
  for (int i = 0; i < curMaxPlayers; i++) {
    if (!player[i])
      continue;

    TeamColor team = player[i]->getTeam();
    if (team == ObserverTeam)
      continue;
    if (team == startupInfo.team && startupInfo.team != AutomaticTeam)
      continue;

    link->send(PlayersReply(player[i]));
  }

  return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
