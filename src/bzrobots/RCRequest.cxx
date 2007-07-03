#include "RCRequest.h"
#include "Roster.h"

#include "RCLink.h"
#include "RCRequests.h"

#define ADD_LOOKUP(COMMAND) messageLookup[#COMMAND] = &instantiate<COMMAND ## Req>
/* These two are static functions that manipulate and access the lookup-
 * table for request commands -> request instances. :-) */
void RCRequest::initializeLookup(void)
{
    ADD_LOOKUP(IdentifyFrontend);
    ADD_LOOKUP(Execute);
    ADD_LOOKUP(SetSpeed);
    ADD_LOOKUP(SetTurnRate);
    ADD_LOOKUP(SetAhead);
    ADD_LOOKUP(SetTurnLeft);
    ADD_LOOKUP(SetFire);
    ADD_LOOKUP(GetGunHeat);
    ADD_LOOKUP(GetDistanceRemaining);
    ADD_LOOKUP(GetTurnRemaining);
    ADD_LOOKUP(GetTickDuration);
    ADD_LOOKUP(SetTickDuration);
    ADD_LOOKUP(GetTickRemaining);
    ADD_LOOKUP(GetTeams);
    ADD_LOOKUP(GetBases);
    ADD_LOOKUP(GetObstacles);
    ADD_LOOKUP(GetFlags);
    ADD_LOOKUP(GetShots);
    ADD_LOOKUP(GetMyTanks);
    ADD_LOOKUP(GetOtherTanks);
    ADD_LOOKUP(GetConstants);
}

RCRequest::~RCRequest() { }
RCRequest::RCRequest(RCLink *_link) :RCMessage<RCRequest>(_link)
{
}


void RCRequest::sendAck(bool newline)
{
  float elapsed = TimeKeeper::getCurrent() - TimeKeeper::getStartTime();
  link->sendf("ack %f %s%s", elapsed, getType().c_str(), (newline ? "\n" : ""));
}
bool RCRequest::process(RCRobotPlayer *rrp) { return true; }

int RCRequest::getRobotIndex()
{
  return robotIndex;
}

int RCRequest::setRobotIndex(char *arg)
{
  char *endptr;
  robotIndex = strtol(arg, &endptr, 0);
  if (endptr == arg)
    robotIndex = -1;
  else if (robotIndex >= numRobots)
    robotIndex = -1;

  return robotIndex;
}
