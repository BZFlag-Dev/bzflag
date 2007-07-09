#include "RCRequests.h"
#include "RCReplies.h"
#include "RCRobotPlayer.h"

#include "version.h"

RCRequest::parseStatus RCRequestZeroArgument::parse(char **, int count)
{
  if (count != 0)
    return InvalidArgumentCount;
  return ParseOk;
}
void RCRequestZeroArgument::getParameters(std::ostream &) const
{
}


RCRequest::parseStatus IdentifyFrontend::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  version = arguments[0];
  /* Version-checking, to be sure we're speaking the same language! */
  if (version != getRobotsProtocolVersion())
    return InvalidArguments;
  return ParseOk;
}
void IdentifyFrontend::getParameters(std::ostream &stream) const
{
  stream << version;
}

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
    rrp->turnRemaining = (rrp->turnLeft ? 1 : -1) * rrp->nextTurn;
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

RCRequest::parseStatus SetSpeedReq::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], speed))
    return InvalidArguments;

  speed = clamp(speed, 0.0f, 1.0f);
  return ParseOk;
}
bool SetSpeedReq::process(RCRobotPlayer *rrp)
{
  rrp->nextSpeed = speed;
  rrp->pendingUpdates[RCRobotPlayer::speedUpdate] = true;
  return true;
}
void SetSpeedReq::getParameters(std::ostream &stream) const
{
  stream << speed;
}

RCRequest::parseStatus SetTurnRateReq::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], rate))
    return InvalidArguments;
  rate = clamp(rate, 0.0f, 1.0f);
  return ParseOk;
}
bool SetTurnRateReq::process(RCRobotPlayer *rrp)
{
  rrp->nextTurnRate = rate;
  rrp->pendingUpdates[RCRobotPlayer::turnRateUpdate] = true;
  return true;
}
void SetTurnRateReq::getParameters(std::ostream &stream) const
{
  stream << rate;
}

RCRequest::parseStatus SetAheadReq::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], distance))
    return InvalidArguments;
  return ParseOk;
}
bool SetAheadReq::process(RCRobotPlayer *rrp)
{
  rrp->pendingUpdates[RCRobotPlayer::distanceUpdate] = true;
  rrp->nextDistance = distance;
  return true;
}
void SetAheadReq::getParameters(std::ostream &stream) const
{
  stream << distance;
}

RCRequest::parseStatus SetTurnLeftReq::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], turn))
    return InvalidArguments;
  return ParseOk;
}
bool SetTurnLeftReq::process(RCRobotPlayer *rrp)
{
  rrp->pendingUpdates[RCRobotPlayer::turnUpdate] = true;
  rrp->nextTurn = turn;
  return true;
}
void SetTurnLeftReq::getParameters(std::ostream &stream) const
{
  stream << turn;
}

bool SetFireReq::process(RCRobotPlayer *rrp)
{
  rrp->shoot = true;
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
  link->sendf("GetDistanceRemaining %f\n", rrp->distanceRemaining);
  return true;
}

bool GetTurnRemainingReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;
  link->sendf("GetTurnRemaining %f\n", rrp->turnRemaining);
  return true;
}

bool GetTickDurationReq::process(RCRobotPlayer *rrp)
{
  link->sendf("GetTickDuration %f\n", rrp->tickDuration);
  return true;
}

RCRequest::parseStatus SetTickDurationReq::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], duration))
    return InvalidArguments;

  duration = std::max(duration, 0.0f);
  return ParseOk;
}
bool SetTickDurationReq::process(RCRobotPlayer *rrp)
{
  rrp->tickDuration = duration;
  return true;
}
void SetTickDurationReq::getParameters(std::ostream &stream) const
{
  stream << duration;
}

bool GetTickRemainingReq::process(RCRobotPlayer *rrp)
{
  if (rrp->isSteadyState())
    link->sendf("GetTickRemaining %f\n", (rrp->lastTickAt + rrp->tickDuration) - TimeKeeper::getCurrent().getSeconds());
  else
    link->send("GetTickRemaining 0.0\n");

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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
