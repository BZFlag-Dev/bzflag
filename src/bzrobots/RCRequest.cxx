#include "RCRequest.h"
#include "Roster.h"

RCRequest::RCRequest() :
			    fail(false),
			    failstr(NULL),
			    request_type(InvalidRequest),
			    next(NULL)
{
  request_type = InvalidRequest;
}

RCRequest::RCRequest(agent_req_t reqtype) :
			    fail(false),
			    failstr(NULL),
			    request_type(reqtype),
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
    request_type = HelloRequest;
  } else if (strcasecmp(argv[0], "execute") == 0 && argc == 2) {
    request_type = execute;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "setSpeed") == 0 && argc == 3) {
    request_type = setSpeed;
    set_robotindex(argv[1]);

    speed = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for desired speed.";
    }

    speed = clamp(speed, 0.0f, 1.0f);
  } else if (strcasecmp(argv[0], "setTurnRate") == 0 && argc == 3) {
    request_type = setTurnRate;
    set_robotindex(argv[1]);

    turnRate = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for angular velocity.";
    }

    turnRate = clamp(turnRate, 0.0f, 1.0f);
  } else if (strcasecmp(argv[0], "setAhead") == 0 && argc == 3) {
    request_type = setAhead;
    set_robotindex(argv[1]);

    distance = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for distance.";
    }
  } else if (strcasecmp(argv[0], "setTurnLeft") == 0 && argc == 3) {
    request_type = setTurnLeft;
    set_robotindex(argv[1]);

    turn = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for turn angle.";
    }
  } else if (strcasecmp(argv[0], "setFire") == 0 && argc == 2) {
    request_type = setFire;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "getGunHeat") == 0 && argc == 2) {
    request_type = getGunHeat;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "getDistanceRemaining") == 0 && argc == 2) {
    request_type = getDistanceRemaining;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "getTurnRemaining") == 0 && argc == 2) {
    request_type = getTurnRemaining;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "getTickDuration") == 0 && argc == 2) {
    request_type = getTickDuration;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "setTickDuration") == 0 && argc == 3) {
    request_type = setTickDuration;
    set_robotindex(argv[1]);

    duration = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for setTickDuration.";
    }
  } else if (strcasecmp(argv[0], "getTickRemaining") == 0 && argc == 2) {
    request_type = getTickRemaining;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "teams") == 0 && argc == 1) {
    request_type = TeamListRequest;
  } else if (strcasecmp(argv[0], "bases") == 0 && argc == 1) {
    request_type = BasesListRequest;
  } else if (strcasecmp(argv[0], "obstacles") == 0 && argc == 1) {
    request_type = ObstacleListRequest;
  } else if (strcasecmp(argv[0], "flags") == 0 && argc == 1) {
    request_type = FlagListRequest;
  } else if (strcasecmp(argv[0], "shots") == 0 && argc == 1) {
    request_type = ShotListRequest;
  } else if (strcasecmp(argv[0], "mytanks") == 0 && argc == 1) {
    request_type = MyTankListRequest;
  } else if (strcasecmp(argv[0], "othertanks") == 0 && argc == 1) {
    request_type = OtherTankListRequest;
  } else if (strcasecmp(argv[0], "constants") == 0 && argc == 1) {
    request_type = ConstListRequest;
  } else {
    request_type = InvalidRequest;
  }
}

void RCRequest::sendack(RCLink *link)
{
  float elapsed = TimeKeeper::getCurrent() - TimeKeeper::getStartTime();

  switch (request_type) {
    case execute:
      link->sendf("ack %f execute %d\n", elapsed, get_robotindex());
      break;
    case setAhead:
      link->sendf("ack %f setAhead %d %f\n", elapsed, get_robotindex(), distance);
      break;
    case setTurnLeft:
      link->sendf("ack %f setTurnLeft %d %f\n", elapsed, get_robotindex(), turn);
      break;
    case setSpeed:
      link->sendf("ack %f setSpeed %d %f\n", elapsed, get_robotindex(), speed);
      break;
    case setTurnRate:
      link->sendf("ack %f setTurnRate %d %f\n", elapsed, get_robotindex(), turnRate);
      break;
    case setFire:
      link->sendf("ack %f setFire %d\n", elapsed, get_robotindex());
      break;
    case getGunHeat:
      link->sendf("ack %f getGunHeat %d\n", elapsed, get_robotindex());
      break;
    case getDistanceRemaining:
      link->sendf("ack %f getDistanceRemaining %d\n", elapsed, get_robotindex());
      break;
    case getTurnRemaining:
      link->sendf("ack %f getTurnRemaining %d\n", elapsed, get_robotindex());
      break;
    case getTickRemaining:
      link->sendf("ack %f getTickRemaining %d\n", elapsed, get_robotindex());
      break;
    case getTickDuration:
      link->sendf("ack %f getTickDuration %d\n", elapsed, get_robotindex());
      break;
    case setTickDuration:
      link->sendf("ack %f setTickDuration %d %f\n", elapsed, get_robotindex(), duration);
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

void RCRequest::sendfail(RCLink *link)
{
  if (fail) {
    if (failstr) {
      link->sendf("fail %s\n", failstr);
    } else {
      link->send("fail\n");
    }
  }
}

int RCRequest::get_robotindex()
{
  return robotindex;
}

void RCRequest::set_robotindex(char *arg)
{
  char *endptr;
  robotindex = strtol(arg, &endptr, 0);
  if (endptr == arg) {
    robotindex = -1;
    fail = true;
    failstr = "Invalid parameter for tank.";
  }
  else if (robotindex >= numRobots) {
    robotindex = -1;
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

agent_req_t RCRequest::get_request_type()
{
  return request_type;
}

RCRequest *RCRequest::getnext()
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
