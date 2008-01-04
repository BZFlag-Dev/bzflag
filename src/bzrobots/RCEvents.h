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

#ifndef __RCEVENTS_H__
#define __RCEVENTS_H__

#include "common.h"

/* system interface headers */
#include <string>
#include <ostream>

/* local interface headers */
#include "RCEvent.h"
#include "RCMessage.h"
#include "BZAdvancedRobot.h"


struct HitWallEvent : public RCEvent
{
  HitWallEvent() {}
  HitWallEvent(double _bearing) : bearing(_bearing) {}
  std::string getType() const { return "HitWall"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &stream) const;
  bool updateBot(BZAdvancedRobot *bot) const;

private:
  double bearing;
};

struct DeathEvent : public RCEvent
{
  DeathEvent() {}
  std::string getType() const { return "Death"; }
  messageParseStatus parse(char **arguments, int count);
  void getParameters(std::ostream &/*stream*/) const {}
  bool updateBot(BZAdvancedRobot *bot) const;
};

#else
struct HitWallEvent;
struct DeathEvent;
#endif /* __RCEVENTS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
