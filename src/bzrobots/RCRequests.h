/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Remote Control Requests: Encapsulates data and logic associated with
 * requests from the frontend to the backend.
 */

#ifndef	BZF_RC_REQUESTS_H
#define	BZF_RC_REQUESTS_H

#include <string>

#include "RCRequest.h"
#include "RCLink.h"

struct RCRequestZeroArgument : public RCRequest
{
  RCRequestZeroArgument(RCLink *_link) :RCRequest(_link) { }
  RCRequest::parseStatus parse(char **arguments, int count);
  void sendAck(bool newline = false);
  virtual bool process(RCRobotPlayer *rrp) = 0;
  void getParameters(std::ostream &stream);
};
struct RCRequestBotSpecific : public RCRequest
{
  RCRequestBotSpecific(RCLink *_link) :RCRequest(_link) { }
  RCRequest::parseStatus parse(char **arguments, int count);
  void sendAck(bool newline = false);
  virtual bool process(RCRobotPlayer *rrp) = 0;
  void getParameters(std::ostream &stream);
};

struct ExecuteReq : public RCRequestBotSpecific
{
  ExecuteReq(RCLink *l) :RCRequestBotSpecific(l) {}
  std::string getType() { return "Execute"; }
  bool process(RCRobotPlayer *rrp);
};
struct SetFireReq : public RCRequestBotSpecific
{
  SetFireReq(RCLink *l) :RCRequestBotSpecific(l) {}
  std::string getType() { return "SetFire"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetGunHeatReq : public RCRequestBotSpecific
{
  GetGunHeatReq(RCLink *l) :RCRequestBotSpecific(l) {}
  std::string getType() { return "GetGunHeat"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetDistanceRemainingReq : public RCRequestBotSpecific
{
  GetDistanceRemainingReq(RCLink *l) :RCRequestBotSpecific(l) {}
  std::string getType() { return "GetDistanceRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetTurnRemainingReq : public RCRequestBotSpecific
{
  GetTurnRemainingReq(RCLink *l) :RCRequestBotSpecific(l) {}
  std::string getType() { return "GetTurnRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetTickDurationReq : public RCRequestBotSpecific
{
  GetTickDurationReq(RCLink *l) :RCRequestBotSpecific(l) {}
  std::string getType() { return "GetTickDuration"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetTickRemainingReq : public RCRequestBotSpecific
{
  GetTickRemainingReq(RCLink *l) :RCRequestBotSpecific(l) {}
  std::string getType() { return "GetTickRemaining"; }
  bool process(RCRobotPlayer *rrp);
};

struct GetTeamsReq : public RCRequestZeroArgument {
  GetTeamsReq(RCLink *l) :RCRequestZeroArgument(l) {}
  std::string getType() { return "GetTeams"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetBasesReq : public RCRequestZeroArgument {
  GetBasesReq(RCLink *l) :RCRequestZeroArgument(l) {}
  std::string getType() { return "GetBases"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetObstaclesReq : public RCRequestZeroArgument {
  GetObstaclesReq(RCLink *l) :RCRequestZeroArgument(l) {}
  std::string getType() { return "GetObstacles"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetFlagsReq : public RCRequestZeroArgument {
  GetFlagsReq(RCLink *l) :RCRequestZeroArgument(l) {}
  std::string getType() { return "GetFlags"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetShotsReq : public RCRequestZeroArgument {
  GetShotsReq(RCLink *l) :RCRequestZeroArgument(l) {}
  std::string getType() { return "GetShots"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetMyTanksReq : public RCRequestZeroArgument {
  GetMyTanksReq(RCLink *l) :RCRequestZeroArgument(l) {}
  std::string getType() { return "GetMyTanks"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetOtherTanksReq : public RCRequestZeroArgument {
  GetOtherTanksReq(RCLink *l) :RCRequestZeroArgument(l) {}
  std::string getType() { return "GetOtherTanks"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetConstantsReq : public RCRequestZeroArgument {
  GetConstantsReq(RCLink *l) :RCRequestZeroArgument(l) {}
  std::string getType() { return "GetConstants"; }
  bool process(RCRobotPlayer *rrp);
};

/* This is just a shorthand to not repeat a bunch of typing. ;-) */
#define DECLARE_REQUEST(COMMANDNAME) class COMMANDNAME ## Req : public RCRequest \
{ \
  public: \
    COMMANDNAME ## Req(RCLink *_link) :RCRequest(_link) { } \
    RCRequest::parseStatus parse(char **arguments, int count); \
    std::string getType() { return #COMMANDNAME; } \
    void sendAck(bool newline = false); \
    bool process(RCRobotPlayer *rrp); \
    void getParameters(std::ostream &stream);

DECLARE_REQUEST(IdentifyFrontend)
private:
  char *version;
};

DECLARE_REQUEST(SetSpeed)
private:
  float speed;
};

DECLARE_REQUEST(SetTurnRate)
private:
  float rate;
};

DECLARE_REQUEST(SetAhead)
private:
  float distance;
};

DECLARE_REQUEST(SetTurnLeft)
private:
  float turn;
};

DECLARE_REQUEST(SetTickDuration)
private:
  float duration;
};

#undef DECLARE_REQUEST

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
