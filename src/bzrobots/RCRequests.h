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
  RCRequest::parseStatus parse(char **arguments, int count);
  virtual bool process(RCRobotPlayer *rrp) = 0;
  void getParameters(std::ostream &stream) const;
};
struct RCRequestBotSpecific : public RCRequest
{
  RCRequest::parseStatus parse(char **arguments, int count);
  virtual bool process(RCRobotPlayer *rrp) = 0;
  void getParameters(std::ostream &stream) const;
};

struct ExecuteReq : public RCRequestBotSpecific
{
  std::string getType() const { return "Execute"; }
  bool process(RCRobotPlayer *rrp);
};
struct SetFireReq : public RCRequestBotSpecific
{
  std::string getType() const { return "SetFire"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetGunHeatReq : public RCRequestBotSpecific
{
  std::string getType() const { return "GetGunHeat"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetDistanceRemainingReq : public RCRequestBotSpecific
{
  std::string getType() const { return "GetDistanceRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetTurnRemainingReq : public RCRequestBotSpecific
{
  std::string getType() const { return "GetTurnRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetTickDurationReq : public RCRequestBotSpecific
{
  std::string getType() const { return "GetTickDuration"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetTickRemainingReq : public RCRequestBotSpecific
{
  std::string getType() const { return "GetTickRemaining"; }
  bool process(RCRobotPlayer *rrp);
};

struct GetTeamsReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetTeams"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetBasesReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetBases"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetObstaclesReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetObstacles"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetFlagsReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetFlags"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetShotsReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetShots"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetMyTanksReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetMyTanks"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetOtherTanksReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetOtherTanks"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetConstantsReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetConstants"; }
  bool process(RCRobotPlayer *rrp);
};

struct IdentifyFrontend :public RCRequest {
  std::string getType() const { return "IdentifyFrontend"; }
  RCRequest::parseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;

  private: char *version;
};

/* This is just a shorthand to not repeat a bunch of typing. ;-) */
#define DECLARE_REQUEST(COMMANDNAME) class COMMANDNAME ## Req : public RCRequest \
{ \
  public: \
    RCRequest::parseStatus parse(char **arguments, int count); \
    std::string getType() const { return #COMMANDNAME; } \
    bool process(RCRobotPlayer *rrp); \
    void getParameters(std::ostream &stream) const;

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
