#include "RCRequests.h"

#include "RCReplies.h"
#include "RCRobotPlayer.h"
#include "MessageUtilities.h"
#include "BZDBCache.h"

#include "version.h"

messageParseStatus RCRequestZeroArgument::parse(char **, int count)
{
  if (count != 0)
    return InvalidArgumentCount;
  return ParseOk;
}
void RCRequestZeroArgument::getParameters(std::ostream &) const
{
}


messageParseStatus IdentifyFrontend::parse(char **arguments, int count)
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

messageParseStatus SetSpeedReq::parse(char **arguments, int count)
{
  messageParseStatus status = MessageUtilities::parseSingle(arguments, count, speed);
  if (status == ParseOk)
    speed = MessageUtilities::clamp(speed, 0.0f, 1.0f);
  return status;
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

messageParseStatus SetTurnRateReq::parse(char **arguments, int count)
{
  messageParseStatus status = MessageUtilities::parseSingle(arguments, count, rate);
  if (status == ParseOk)
    rate = MessageUtilities::clamp(rate, 0.0f, 1.0f);
  return status;
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

messageParseStatus SetAheadReq::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, distance);
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

messageParseStatus SetTurnLeftReq::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, turn);
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

messageParseStatus SetTickDurationReq::parse(char **arguments, int count)
{
  messageParseStatus status = MessageUtilities::parseSingle(arguments, count, duration);
  if (status == ParseOk)
    duration = std::max(duration, 0.0f);
  return status;
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

messageParseStatus SetStopReq::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, overwrite);
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
void SetStopReq::getParameters(std::ostream &stream) const
{
  stream << overwrite;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
