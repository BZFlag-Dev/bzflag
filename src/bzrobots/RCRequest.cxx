#include "RCRequest.h"
#include "Roster.h"

std::map<std::string, RCRequest *(*)()> RCRequest::requestLookup;
/* These two are static functions that manipulate and access the lookup-
 * table for request commands -> request instances. :-) */
void RCRequest::initializeLookup(void)
{
    requestLookup["agent"] = &instantiate<RCRequest>;
}
RCRequest *RCRequest::getRequestInstance(std::string request)
{
    if (requestLookup.find(request) != requestLookup.end())
        return requestLookup[request]();
    return NULL;
}

RCRequest::RCRequest() :
			    fail(false),
			    failstr(NULL),
			    requestType(InvalidRequest),
			    next(NULL)
{
  requestType = InvalidRequest;
}

RCRequest::RCRequest(AgentReqType reqtype) :
			    fail(false),
			    failstr(NULL),
			    requestType(reqtype),
			    next(NULL)
{
}

RCRequest::RCRequest(int argc, char **argv) :
			    fail(false),
			    failstr(NULL),
			    next(NULL)
{
  char *endptr;
  // TODO: give a better error message if argc is wrong.
  if (strcasecmp(argv[0], "agent") == 0 && argc == 2) {
    requestType = HelloRequest;
  } else if (strcasecmp(argv[0], "execute") == 0 && argc == 2) {
    requestType = execute;
    setRobotIndex(argv[1]);
  } else if (strcasecmp(argv[0], "setSpeed") == 0 && argc == 3) {
    requestType = setSpeed;
    setRobotIndex(argv[1]);

    speed = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for desired speed.";
    }

    speed = clamp(speed, 0.0f, 1.0f);
  } else if (strcasecmp(argv[0], "setTurnRate") == 0 && argc == 3) {
    requestType = setTurnRate;
    setRobotIndex(argv[1]);

    turnRate = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for angular velocity.";
    }

    turnRate = clamp(turnRate, 0.0f, 1.0f);
  } else if (strcasecmp(argv[0], "setAhead") == 0 && argc == 3) {
    requestType = setAhead;
    setRobotIndex(argv[1]);

    distance = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for distance.";
    }
  } else if (strcasecmp(argv[0], "setTurnLeft") == 0 && argc == 3) {
    requestType = setTurnLeft;
    setRobotIndex(argv[1]);

    turn = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for turn angle.";
    }
  } else if (strcasecmp(argv[0], "setFire") == 0 && argc == 2) {
    requestType = setFire;
    setRobotIndex(argv[1]);
  } else if (strcasecmp(argv[0], "getGunHeat") == 0 && argc == 2) {
    requestType = getGunHeat;
    setRobotIndex(argv[1]);
  } else if (strcasecmp(argv[0], "getDistanceRemaining") == 0 && argc == 2) {
    requestType = getDistanceRemaining;
    setRobotIndex(argv[1]);
  } else if (strcasecmp(argv[0], "getTurnRemaining") == 0 && argc == 2) {
    requestType = getTurnRemaining;
    setRobotIndex(argv[1]);
  } else if (strcasecmp(argv[0], "getTickDuration") == 0 && argc == 2) {
    requestType = getTickDuration;
    setRobotIndex(argv[1]);
  } else if (strcasecmp(argv[0], "setTickDuration") == 0 && argc == 3) {
    requestType = setTickDuration;
    setRobotIndex(argv[1]);

    duration = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for setTickDuration.";
    }
  } else if (strcasecmp(argv[0], "getTickRemaining") == 0 && argc == 2) {
    requestType = getTickRemaining;
    setRobotIndex(argv[1]);
  } else if (strcasecmp(argv[0], "teams") == 0 && argc == 1) {
    requestType = TeamListRequest;
  } else if (strcasecmp(argv[0], "bases") == 0 && argc == 1) {
    requestType = BasesListRequest;
  } else if (strcasecmp(argv[0], "obstacles") == 0 && argc == 1) {
    requestType = ObstacleListRequest;
  } else if (strcasecmp(argv[0], "flags") == 0 && argc == 1) {
    requestType = FlagListRequest;
  } else if (strcasecmp(argv[0], "shots") == 0 && argc == 1) {
    requestType = ShotListRequest;
  } else if (strcasecmp(argv[0], "mytanks") == 0 && argc == 1) {
    requestType = MyTankListRequest;
  } else if (strcasecmp(argv[0], "othertanks") == 0 && argc == 1) {
    requestType = OtherTankListRequest;
  } else if (strcasecmp(argv[0], "constants") == 0 && argc == 1) {
    requestType = ConstListRequest;
  } else {
    requestType = InvalidRequest;
  }
}

void RCRequest::sendAck(RCLink *link)
{
  float elapsed = TimeKeeper::getCurrent() - TimeKeeper::getStartTime();

  switch (requestType) {
    case execute:
      link->sendf("ack %f execute %d\n", elapsed, getRobotIndex());
      break;
    case setAhead:
      link->sendf("ack %f setAhead %d %f\n", elapsed, getRobotIndex(), distance);
      break;
    case setTurnLeft:
      link->sendf("ack %f setTurnLeft %d %f\n", elapsed, getRobotIndex(), turn);
      break;
    case setSpeed:
      link->sendf("ack %f setSpeed %d %f\n", elapsed, getRobotIndex(), speed);
      break;
    case setTurnRate:
      link->sendf("ack %f setTurnRate %d %f\n", elapsed, getRobotIndex(), turnRate);
      break;
    case setFire:
      link->sendf("ack %f setFire %d\n", elapsed, getRobotIndex());
      break;
    case getGunHeat:
      link->sendf("ack %f getGunHeat %d\n", elapsed, getRobotIndex());
      break;
    case getDistanceRemaining:
      link->sendf("ack %f getDistanceRemaining %d\n", elapsed, getRobotIndex());
      break;
    case getTurnRemaining:
      link->sendf("ack %f getTurnRemaining %d\n", elapsed, getRobotIndex());
      break;
    case getTickRemaining:
      link->sendf("ack %f getTickRemaining %d\n", elapsed, getRobotIndex());
      break;
    case getTickDuration:
      link->sendf("ack %f getTickDuration %d\n", elapsed, getRobotIndex());
      break;
    case setTickDuration:
      link->sendf("ack %f setTickDuration %d %f\n", elapsed, getRobotIndex(), duration);
      break;
    case TeamListRequest:
      link->sendf("ack %f teams\n", elapsed);
      break;
    case BasesListRequest:
      link->sendf("ack %f bases\n", elapsed);
      break;
    case ObstacleListRequest:
      link->sendf("ack %f obstacles\n", elapsed);
      break;
    case FlagListRequest:
      link->sendf("ack %f flags\n", elapsed);
      break;
    case ShotListRequest:
      link->sendf("ack %f shots\n", elapsed);
      break;
    case MyTankListRequest:
      link->sendf("ack %f mytanks\n", elapsed);
      break;
    case OtherTankListRequest:
      link->sendf("ack %f othertanks\n", elapsed);
      break;
    case ConstListRequest:
      link->sendf("ack %f constants\n", elapsed);
      break;
    default:
      link->sendf("ack %f\n", elapsed);
  }
}

void RCRequest::sendFail(RCLink *link)
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

void RCRequest::setRobotIndex(char *arg)
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

AgentReqType RCRequest::getRequestType()
{
  return requestType;
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
