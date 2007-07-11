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
  messageParseStatus parse(char **arguments, int count);
  virtual bool process(RCRobotPlayer *rrp) = 0;
  void getParameters(std::ostream &stream) const;
};

struct ExecuteReq : public RCRequestZeroArgument
{
  std::string getType() const { return "Execute"; }
  bool process(RCRobotPlayer *rrp);
};
struct SetFireReq : public RCRequestZeroArgument
{
  std::string getType() const { return "SetFire"; }
  bool process(RCRobotPlayer *rrp);
};
struct SetResumeReq : public RCRequestZeroArgument
{
  std::string getType() const { return "SetResume"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetGunHeatReq : public RCRequestZeroArgument
{
  std::string getType() const { return "GetGunHeat"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetDistanceRemainingReq : public RCRequestZeroArgument
{
  std::string getType() const { return "GetDistanceRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetTurnRemainingReq : public RCRequestZeroArgument
{
  std::string getType() const { return "GetTurnRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetTickDurationReq : public RCRequestZeroArgument
{
  std::string getType() const { return "GetTickDuration"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetTickRemainingReq : public RCRequestZeroArgument
{
  std::string getType() const { return "GetTickRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetBattleFieldSizeReq : public RCRequestZeroArgument
{
  std::string getType() const { return "GetBattleFieldSize"; }
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

struct GetXReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetX"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetYReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetY"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetZReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetZ"; }
  bool process(RCRobotPlayer *rrp);
};

struct GetWidthReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetWidth"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetHeightReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetHeight"; }
  bool process(RCRobotPlayer *rrp);
};
struct GetLengthReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetLength"; }
  bool process(RCRobotPlayer *rrp);
};

struct GetHeadingReq : public RCRequestZeroArgument {
  std::string getType() const { return "GetHeading"; }
  bool process(RCRobotPlayer *rrp);
};

struct IdentifyFrontend :public RCRequest {
  IdentifyFrontend() :version("") {}
  IdentifyFrontend(std::string _version) :version(_version) {}
  std::string getType() const { return "IdentifyFrontend"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;

  private: std::string version;
};

/* This is just a shorthand to not repeat a bunch of typing. ;-) */
#define DECLARE_REQUEST(COMMANDNAME) class COMMANDNAME ## Req : public RCRequest \
{ \
  public: \
    COMMANDNAME ## Req() {} \
    messageParseStatus parse(char **arguments, int count); \
    std::string getType() const { return #COMMANDNAME; } \
    bool process(RCRobotPlayer *rrp); \
    void getParameters(std::ostream &stream) const;

DECLARE_REQUEST(SetSpeed)
  SetSpeedReq(float _speed) :speed(_speed) {}
private:
  float speed;
};

DECLARE_REQUEST(SetTurnRate)
  SetTurnRateReq(float _rate) :rate(_rate) {}
private:
  float rate;
};

DECLARE_REQUEST(SetAhead)
  SetAheadReq(float _distance) :distance(_distance) {}
private:
  float distance;
};

DECLARE_REQUEST(SetTurnLeft)
  SetTurnLeftReq(float _turn) :turn(_turn) {}
private:
  float turn;
};

DECLARE_REQUEST(SetTickDuration)
private:
  float duration;
};

DECLARE_REQUEST(SetStop)
  SetStopReq(bool _overwrite) :overwrite(_overwrite) {}
private:
  bool overwrite;
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
