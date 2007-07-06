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
 * Remote Control Replies: Encapsulates data and logic associated with
 * replies to the requests made by the frontend
 */

#ifndef	BZF_RC_REQUESTS_H
#define	BZF_RC_REQUESTS_H

#include <string>

#include "RCReply.h"
#include "RCLink.h"

struct GunHeatReply : public RCReply
{
  GunHeatReply(RCLink *l) :RCReply(l) {}
  std::string getType() { return "GunHeat"; }
  RCReply::parseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream);
  float heat;
};

struct DistanceRemainingReply : public RCReply
{
  DistanceRemainingReply(RCLink *l) :RCReply(l) {}
  std::string getType() { return "DistanceRemaining"; }
  RCReply::parseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream);
  float distance;
};

struct TurnRemainingReply : public RCReply
{
  TurnRemainingReply(RCLink *l) :RCReply(l) {}
  std::string getType() { return "TurnRemaining"; }
  RCReply::parseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream);
  float turn;
};

struct TickDurationReply : public RCReply
{
  TickDurationReply(RCLink *l) :RCReply(l) {}
  std::string getType() { return "TickDuration"; }
  RCReply::parseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream);
  float duration;
};

struct TickRemainingReply : public RCReply
{
  TickRemainingReply(RCLink *l) :RCReply(l) {}
  std::string getType() { return "TickRemaining"; }
  RCReply::parseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream);
  float remaining;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
