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
 * Remote Control Replies: Encapsulates data and logic associated with
 * replies to the requests made by the frontend
 */

#ifndef	BZF_RC_REPLIES_H
#define	BZF_RC_REPLIES_H

#include <string>

#include "RCReply.h"
#include "RCEvent.h"
#include "Tank.h"
#include "RemotePlayer.h"

struct IdentifyBackend : public RCReply
{
  IdentifyBackend() :version("") {}
  IdentifyBackend(std::string _version) :version(_version) {}
  std::string getType() const { return "IdentifyBackend"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;

  private: std::string version;
};

class EventReply : public RCReply
{
  RCEvent *notification;
  public:
    EventReply(RCEvent *_notification) :notification(_notification) {}
    EventReply() :notification(NULL) {}

    std::string getType() const { return "Event"; }
    messageParseStatus parse(char **arguments, int count);
    void getParameters(std::ostream &stream) const;
    bool updateBot(const BZAdvancedRobot *robot) const;
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
  GunHeatReply(double _heat) :heat(_heat) {}
  std::string getType() const { return "GunHeat"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: double heat;
};

struct DistanceRemainingReply : public RCReply
{
  DistanceRemainingReply() {}
  DistanceRemainingReply(double _distance) :distance(_distance) {}
  std::string getType() const { return "DistanceRemaining"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;
  
  private: double distance;
};

struct TurnRemainingReply : public RCReply
{
  TurnRemainingReply() {}
  TurnRemainingReply(double _turn) :turn(_turn) {}
  std::string getType() const { return "TurnRemaining"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;
  
  private: double turn;
};

struct TickDurationReply : public RCReply
{
  std::string getType() const { return "TickDuration"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  
  private: double duration;
};

struct TickRemainingReply : public RCReply
{
  std::string getType() const { return "TickRemaining"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;

  private: double remaining;
};

struct BattleFieldSizeReply : public RCReply
{
  BattleFieldSizeReply() {}
  BattleFieldSizeReply(double _size) :size(_size) {}
  std::string getType() const { return "BattleFieldSize"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: double size;
};

struct XReply : public RCReply
{
  XReply() {}
  XReply(double _x) :x(_x) {}
  std::string getType() const { return "X"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: double x;
};
struct YReply : public RCReply
{
  YReply() {}
  YReply(double _y) :y(_y) {}
  std::string getType() const { return "Y"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: double y;
};
struct ZReply : public RCReply
{
  ZReply() {}
  ZReply(double _z) :z(_z) {}
  std::string getType() const { return "Z"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: double z;
};

struct WidthReply : public RCReply
{
  WidthReply() {}
  WidthReply(double _width) :width(_width) {}
  std::string getType() const { return "Width"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: double width;
};
struct HeightReply : public RCReply
{
  HeightReply() {}
  HeightReply(double _height) :height(_height) {}
  std::string getType() const { return "Height"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: double height;
};
struct LengthReply : public RCReply
{
  LengthReply() {}
  LengthReply(double _length) :length(_length) {}
  std::string getType() const { return "Length"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: double length;
};

struct HeadingReply : public RCReply
{
  HeadingReply() {}
  HeadingReply(double _heading) :heading(_heading) {}
  std::string getType() const { return "Heading"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: double heading;
};

struct PlayersBeginReply : public RCReply
{
  PlayersBeginReply() {}
  std::string getType() const { return "PlayersBegin"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;
};

struct PlayersReply : public RCReply
{
  PlayersReply() {}
  PlayersReply(RemotePlayer *_tank) :tank(_tank) {}
  std::string getType() const { return "Players"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(const BZAdvancedRobot *robot) const;

  private: Tank tank;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
