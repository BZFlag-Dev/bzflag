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

#ifndef BZROBOTS_RCEVENTS_H
#define BZROBOTS_RCEVENTS_H

#include "RCEvent.h"

class BZAdvancedRobot;

struct HitWallEvent : public RCEvent
{
  HitWallEvent() {}
  HitWallEvent(float _bearing) :bearing(_bearing) {}
  std::string getType() const { return "HitWall"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(BZAdvancedRobot *bot) const;

  private: float bearing;
};

struct DeathEvent : public RCEvent
{
  DeathEvent() {}
  std::string getType() const { return "Death"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &/*stream*/) const {}
  bool updateBot(BZAdvancedRobot *bot) const;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
