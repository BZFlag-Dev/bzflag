#include "RCRequest.h"
#include "Roster.h"

#include "RCLink.h"
#include "RCRequests.h"

#define ADD_LOOKUP(COMMAND) requestLookup[#COMMAND] = &instantiate<COMMAND ## Req>
std::map<std::string, RCRequest *(*)(RCLink *)> RCRequest::requestLookup;
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
//    requestLookup["identifyFrontend"] = &instantiate<IdentifyFrontendReq>;
//    requestLookup["execute"] = &instantiate<ExecuteReq>;
}

RCRequest *RCRequest::getRequestInstance(std::string request, RCLink *_link)
{
    if (requestLookup.find(request) != requestLookup.end())
        return requestLookup[request](_link);
    return NULL;
}

RCRequest::~RCRequest() { }
RCRequest::RCRequest(RCLink *_link) :fail(false),
                        failstr(NULL),
                        next(NULL),
                        link(_link)
{
}

bool RCRequest::parseFloat(char *string, float &dest)
{
    char *endptr;
    dest = strtof(string, &endptr);
    if (endptr == string)
        return false;
    return true;
}

void RCRequest::sendAck(bool newline)
{
  float elapsed = TimeKeeper::getCurrent() - TimeKeeper::getStartTime();
  link->sendf("ack %f %s%s", elapsed, getType().c_str(), (newline ? "\n" : ""));
  /*switch (requestType) {
       SNIP 
    default:
      link->sendf("ack %f\n", elapsed);
  }*/
}
bool RCRequest::process(RCRobotPlayer *rrp) { return true; }

void RCRequest::sendFail()
{
  if (fail) {
    if (failstr) {
      link->sendf("fail %s\n", failstr);
    } else {
      link->send("fail\n");
    }
  }
}

int RCRequest::getRobotIndex()
{
  return robotIndex;
}

int RCRequest::setRobotIndex(char *arg)
{
  char *endptr;
  robotIndex = strtol(arg, &endptr, 0);
  if (endptr == arg) {
    robotIndex = -1;
    fail = true;
    failstr = "Invalid parameter for tank.";
  }
  else if (robotIndex >= numRobots) {
    robotIndex = -1;
  }
  return robotIndex;
}

// Mad cred to _neon_/#scene.no and runehol/#scene.no for these two sentences:
//  * If val is nan, the result is undefined
//  * If high < low, the result is undefined
template <class T>
T RCRequest::clamp(T val, T min, T max)
{
  if (val > max)
    return max;
  if (val < min)
    return min;
  return val;
}

RCRequest *RCRequest::getNext()
{
  return next;
}

void RCRequest::append(RCRequest *newreq)
{
  if (next == NULL) {
    next = newreq;
  } else {
    next->append(newreq);
  }
}
