#include "RCRequests.h"
#include "RCRobotPlayer.h"

#include "version.h"

#define ZERO_ARGUMENT_REQUEST(COMMANDNAME) void COMMANDNAME ## Req::sendAck(bool) \
{ \
  RCRequest::sendAck(true); \
} \
RCRequest::parseStatus COMMANDNAME ## Req::parse(char ** /*arguments*/, int count) \
{ \
  if (count != 0) \
    return InvalidArgumentCount; \
  return ParseOk; \
}

#define BOTSPECIFIC_QUERY(COMMANDNAME) void COMMANDNAME ## Req::sendAck(bool) \
{ \
  RCRequest::sendAck(); \
  link->sendf(" %d\n", getRobotIndex()); \
} \
RCRequest::parseStatus COMMANDNAME ## Req::parse(char **arguments, int count) \
{ \
  if (count != 1) \
    return InvalidArgumentCount; \
  if (setRobotIndex(arguments[0]) == -1) \
    return InvalidArguments; \
  return ParseOk; \
}

void IdentifyFrontendReq::sendAck(bool)
{
  RCRequest::sendAck();
  link->sendf(" %s\n", version);
}
RCRequest::parseStatus IdentifyFrontendReq::parse(char **arguments, int count)
{
    if (count != 1)
        return InvalidArgumentCount;
    /* Version-checking, to be sure we're speaking the same language! */
    if (strcasecmp(arguments[0], getRobotsProtocolVersion()) != 0)
        return InvalidArguments;
    version = strdup(arguments[0]);
    return ParseOk;
}
bool IdentifyFrontendReq::process(RCRobotPlayer *)
{
    return true;
}

BOTSPECIFIC_QUERY(Execute);
bool ExecuteReq::process(RCRobotPlayer *)
{
  // TODO: Implement this. :p
  return true;
}

void SetSpeedReq::sendAck(bool)
{
  RCRequest::sendAck();
  link->sendf(" %d %f\n", getRobotIndex(), speed);
}
RCRequest::parseStatus SetSpeedReq::parse(char **arguments, int count)
{
  if (count != 2)
    return InvalidArgumentCount;
  if (setRobotIndex(arguments[0]) == -1)
    return InvalidArguments;
  if (!parseFloat(arguments[1], speed))
    return InvalidArguments;
  return ParseOk;
}
bool SetSpeedReq::process(RCRobotPlayer *rrp)
{
  rrp->nextSpeed = speed;
  return true;
}

void SetTurnRateReq::sendAck(bool)
{
  RCRequest::sendAck();
  link->sendf(" %d %f\n", getRobotIndex(), rate);
}
RCRequest::parseStatus SetTurnRateReq::parse(char **arguments, int count)
{
  if (count != 2)
    return InvalidArgumentCount;
  if (setRobotIndex(arguments[0]) == -1)
    return InvalidArguments;
  if (!parseFloat(arguments[1], rate))
    return InvalidArguments;
  return ParseOk;
}
bool SetTurnRateReq::process(RCRobotPlayer *rrp)
{
  rrp->nextTurnRate = rate;
  return true;
}

void SetAheadReq::sendAck(bool)
{
  RCRequest::sendAck();
  link->sendf(" %d %f\n", getRobotIndex(), distance);
}
RCRequest::parseStatus SetAheadReq::parse(char **arguments, int count)
{
  if (count != 2)
    return InvalidArgumentCount;
  if (setRobotIndex(arguments[0]) == -1)
    return InvalidArguments;
  if (!parseFloat(arguments[1], distance))
    return InvalidArguments;
  return ParseOk;
}
bool SetAheadReq::process(RCRobotPlayer *rrp)
{
  rrp->nextDistance = distance;
  return true;
}

void SetTurnLeftReq::sendAck(bool)
{
  RCRequest::sendAck();
  link->sendf(" %d %f\n", getRobotIndex(), turn);
}
RCRequest::parseStatus SetTurnLeftReq::parse(char **arguments, int count)
{
  if (count != 2)
    return InvalidArgumentCount;
  if (setRobotIndex(arguments[0]) == -1)
    return InvalidArguments;
  if (!parseFloat(arguments[1], turn))
    return InvalidArguments;
  return ParseOk;
}
bool SetTurnLeftReq::process(RCRobotPlayer *rrp)
{
  rrp->nextTurn = turn;
  return true;
}

BOTSPECIFIC_QUERY(SetFire);
bool SetFireReq::process(RCRobotPlayer *rrp)
{
  rrp->shoot = true;
  return true;
}

BOTSPECIFIC_QUERY(GetGunHeat);
bool GetGunHeatReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;
  link->sendf("GetGunHeat %f\n", rrp->getReloadTime());
  return true;
}

BOTSPECIFIC_QUERY(GetDistanceRemaining);
bool GetDistanceRemainingReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;
  link->sendf("GetDistanceRemaining %f\n", rrp->distanceRemaining);
  return true;
}

BOTSPECIFIC_QUERY(GetTurnRemaining);
bool GetTurnRemainingReq::process(RCRobotPlayer *rrp)
{
  if (!rrp->isSteadyState())
    return false;
  link->sendf("GetTurnRemaining %f\n", rrp->turnRemaining);
  return true;
}

BOTSPECIFIC_QUERY(GetTickDuration);
bool GetTickDurationReq::process(RCRobotPlayer *rrp)
{
  link->sendf("GetTickDuration %f\n", rrp->tickDuration);
  return true;
}

void SetTickDurationReq::sendAck(bool)
{
  RCRequest::sendAck();
  link->sendf(" %d %f\n", getRobotIndex(), duration);
}
RCRequest::parseStatus SetTickDurationReq::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (setRobotIndex(arguments[0]) == -1)
    return InvalidArguments;
  if (!parseFloat(arguments[1], duration))
    return InvalidArguments;
  return ParseOk;
}
bool SetTickDurationReq::process(RCRobotPlayer *rrp)
{
  rrp->tickDuration = duration;
  return true;
}

BOTSPECIFIC_QUERY(GetTickRemaining);
bool GetTickRemainingReq::process(RCRobotPlayer *rrp)
{
  if (rrp->isSteadyState())
    link->sendf("GetTickRemaining %f\n", (rrp->lastTickAt + rrp->tickDuration) - TimeKeeper::getCurrent().getSeconds());
  else
    link->send("GetTickRemaining 0.0\n");

  return true;
}

ZERO_ARGUMENT_REQUEST(GetTeams);
bool GetTeamsReq::process(RCRobotPlayer *rrp)
{
  // TODO: Implement this. :p
  return true;
}
ZERO_ARGUMENT_REQUEST(GetBases);
bool GetBasesReq::process(RCRobotPlayer *rrp)
{
  // TODO: Implement this. :p
  return true;
}
ZERO_ARGUMENT_REQUEST(GetObstacles);
bool GetObstaclesReq::process(RCRobotPlayer *rrp)
{
  // TODO: Implement this. :p
  return true;
}
ZERO_ARGUMENT_REQUEST(GetFlags);
bool GetFlagsReq::process(RCRobotPlayer *rrp)
{
  // TODO: Implement this. :p
  return true;
}
ZERO_ARGUMENT_REQUEST(GetShots);
bool GetShotsReq::process(RCRobotPlayer *rrp)
{
  // TODO: Implement this. :p
  return true;
}
ZERO_ARGUMENT_REQUEST(GetMyTanks);
bool GetMyTanksReq::process(RCRobotPlayer *rrp)
{
  // TODO: Implement this. :p
  return true;
}
ZERO_ARGUMENT_REQUEST(GetOtherTanks);
bool GetOtherTanksReq::process(RCRobotPlayer *rrp)
{
  // TODO: Implement this. :p
  return true;
}
ZERO_ARGUMENT_REQUEST(GetConstants);
bool GetConstantsReq::process(RCRobotPlayer *rrp)
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
