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

/* This is just a shorthand to not repeat a bunch of typing. ;-) */
#define DECLARE_REQUEST_BEGIN(COMMANDNAME) class COMMANDNAME ## Req : public RCRequest \
{ \
  public: \
    COMMANDNAME ## Req(RCLink *_link) :RCRequest(_link) { } \
    RCRequest::parseStatus parse(char **arguments, int count); \
    std::string getType() { return #COMMANDNAME; } \
    void sendAck(bool newline = false); \
    bool process(RCRobotPlayer *rrp);
#define DECLARE_REQUEST_END() }
#define DECLARE_REQUEST(classname) DECLARE_REQUEST_BEGIN(classname); \
DECLARE_REQUEST_END()

DECLARE_REQUEST_BEGIN(IdentifyFrontend);
private:
  char *version;
DECLARE_REQUEST_END();

DECLARE_REQUEST(Execute);

DECLARE_REQUEST_BEGIN(SetSpeed);
private:
  float speed;
DECLARE_REQUEST_END();

DECLARE_REQUEST_BEGIN(SetTurnRate);
private:
  float rate;
DECLARE_REQUEST_END();

DECLARE_REQUEST_BEGIN(SetAhead);
private:
  float distance;
DECLARE_REQUEST_END();

DECLARE_REQUEST_BEGIN(SetTurnLeft);
private:
  float turn;
DECLARE_REQUEST_END();

DECLARE_REQUEST(SetFire);
DECLARE_REQUEST(GetGunHeat);
DECLARE_REQUEST(GetDistanceRemaining);
DECLARE_REQUEST(GetTurnRemaining);
DECLARE_REQUEST(GetTickDuration);

DECLARE_REQUEST_BEGIN(SetTickDuration);
private:
  float duration;
DECLARE_REQUEST_END();

DECLARE_REQUEST(GetTickRemaining);
DECLARE_REQUEST(GetTeams);
DECLARE_REQUEST(GetBases);
DECLARE_REQUEST(GetObstacles);
DECLARE_REQUEST(GetFlags);
DECLARE_REQUEST(GetShots);
DECLARE_REQUEST(GetMyTanks);
DECLARE_REQUEST(GetOtherTanks);
DECLARE_REQUEST(GetConstants);

#undef REQUEST_DEFAULT_BODY

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
