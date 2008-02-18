/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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

#ifndef __RCREQUESTS_H__
#define	__RCREQUESTS_H__

#include "common.h"

/* system implementation headers */
#include <string>

/* local implementation headers */
#include "RCRequest.h"
#include "RCLink.h"


class RCRequestZeroArgument : public RCRequest
{
public:
  messageParseStatus parse(char **arguments, int count);
  virtual bool process(RCRobotPlayer *rrp) = 0;
  void getParameters(std::ostream &stream) const;
};

class ExecuteReq : public RCRequestZeroArgument
{
public:
  std::string getType() const { return "Execute"; }
  bool process(RCRobotPlayer *rrp);
};
class SetFireReq : public RCRequestZeroArgument
{
public:
  std::string getType() const { return "SetFire"; }
  bool process(RCRobotPlayer *rrp);
};
class SetResumeReq : public RCRequestZeroArgument
{
public:
  std::string getType() const { return "SetResume"; }
  bool process(RCRobotPlayer *rrp);
};
class GetGunHeatReq : public RCRequestZeroArgument
{
public:
  std::string getType() const { return "GetGunHeat"; }
  bool process(RCRobotPlayer *rrp);
};
class GetDistanceRemainingReq : public RCRequestZeroArgument
{
public:
  std::string getType() const { return "GetDistanceRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
class GetTurnRemainingReq : public RCRequestZeroArgument
{
public:
  std::string getType() const { return "GetTurnRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
class GetTickDurationReq : public RCRequestZeroArgument
{
public:
  std::string getType() const { return "GetTickDuration"; }
  bool process(RCRobotPlayer *rrp);
};
class GetTickRemainingReq : public RCRequestZeroArgument
{
public:
  std::string getType() const { return "GetTickRemaining"; }
  bool process(RCRobotPlayer *rrp);
};
class GetBattleFieldSizeReq : public RCRequestZeroArgument
{
public:
  std::string getType() const { return "GetBattleFieldSize"; }
  bool process(RCRobotPlayer *rrp);
};
class GetTeamsReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetTeams"; }
  bool process(RCRobotPlayer *rrp);
};
class GetBasesReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetBases"; }
  bool process(RCRobotPlayer *rrp);
};
class GetObstaclesReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetObstacles"; }
  bool process(RCRobotPlayer *rrp);
};
class GetFlagsReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetFlags"; }
  bool process(RCRobotPlayer *rrp);
};
class GetShotsReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetShots"; }
  bool process(RCRobotPlayer *rrp);
};
class GetMyTanksReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetMyTanks"; }
  bool process(RCRobotPlayer *rrp);
};
class GetOtherTanksReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetOtherTanks"; }
  bool process(RCRobotPlayer *rrp);
};
class GetConstantsReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetConstants"; }
  bool process(RCRobotPlayer *rrp);
};

class GetXReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetX"; }
  bool process(RCRobotPlayer *rrp);
};
class GetYReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetY"; }
  bool process(RCRobotPlayer *rrp);
};
class GetZReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetZ"; }
  bool process(RCRobotPlayer *rrp);
};

class GetWidthReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetWidth"; }
  bool process(RCRobotPlayer *rrp);
};
class GetHeightReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetHeight"; }
  bool process(RCRobotPlayer *rrp);
};
class GetLengthReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetLength"; }
  bool process(RCRobotPlayer *rrp);
};

class GetHeadingReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetHeading"; }
  bool process(RCRobotPlayer *rrp);
};

class IdentifyFrontend : public RCRequest {
public:
  IdentifyFrontend() : version("") {}
  IdentifyFrontend(std::string _version) : version(_version) {}
  std::string getType() const { return "IdentifyFrontend"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;

private:
  std::string version;
};

class GetPlayersReq : public RCRequestZeroArgument {
public:
  std::string getType() const { return "GetPlayers"; }
  bool process(RCRobotPlayer *rrp);
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
  SetSpeedReq(double _speed) : speed(_speed) {}
private:
  double speed;
};

DECLARE_REQUEST(SetTurnRate)
  SetTurnRateReq(double _rate) : rate(_rate) {}
private:
  double rate;
};

DECLARE_REQUEST(SetAhead)
  SetAheadReq(double _distance) : distance(_distance) {}
private:
  double distance;
};

DECLARE_REQUEST(SetTurnLeft)
  SetTurnLeftReq(double _turn) : turn(_turn) {}
private:
  double turn;
};

DECLARE_REQUEST(SetTickDuration)
  SetTickDurationReq(double _duration) : duration(_duration) {}
private:
  double duration;
};

DECLARE_REQUEST(SetStop)
  SetStopReq(bool _overwrite) : overwrite(_overwrite) {}
private:
  bool overwrite;
};

#undef DECLARE_REQUEST

#endif /* __RCREQUESTS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
