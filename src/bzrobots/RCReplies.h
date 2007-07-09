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

#ifndef	BZF_RC_REPLIES_H
#define	BZF_RC_REPLIES_H

#include <string>

#include "RCReply.h"
#include "RCLink.h"

struct IdentifyBackend : public RCReply
{
  IdentifyBackend() :version("") {}
  IdentifyBackend(std::string _version) :version(_version) {}
  std::string getType() const { return "IdentifyBackend"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;

  private: std::string version;
};

struct CommandDoneReply : public RCReply
{
  CommandDoneReply(std::string _command) :command(_command) {}
  CommandDoneReply() :command("") {}
  std::string getType() const { return "CommandDone"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  std::string command;
};

struct GunHeatReply : public RCReply
{
  GunHeatReply() :heat(0.0f) {}
  GunHeatReply(float _heat) :heat(_heat) {}
  std::string getType() const { return "GunHeat"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(BZAdvancedRobot *robot) const;

  private: float heat;
};

struct DistanceRemainingReply : public RCReply
{
  DistanceRemainingReply() {}
  DistanceRemainingReply(float _distance) :distance(_distance) {}
  std::string getType() const { return "DistanceRemaining"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(BZAdvancedRobot *robot) const;
  
  private: float distance;
};

struct TurnRemainingReply : public RCReply
{
  TurnRemainingReply() {}
  TurnRemainingReply(float _turn) :turn(_turn) {}
  std::string getType() const { return "TurnRemaining"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(BZAdvancedRobot *robot) const;
  
  private: float turn;
};

struct TickDurationReply : public RCReply
{
  std::string getType() const { return "TickDuration"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  
  private: float duration;
};

struct TickRemainingReply : public RCReply
{
  std::string getType() const { return "TickRemaining"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;

  private: float remaining;
};

struct BattleFieldSizeReply : public RCReply
{
  BattleFieldSizeReply() {}
  BattleFieldSizeReply(float _size) :size(_size) {}
  std::string getType() const { return "BattleFieldSize"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(BZAdvancedRobot *robot) const;

  private: float size;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
