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
#include "RCEvents.h"
#include "MessageUtilities.h"

#include "BZAdvancedRobot.h"

messageParseStatus HitWallEvent::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, bearing);
}
void HitWallEvent::getParameters(std::ostream &stream) const
{
  stream << bearing;
}
bool HitWallEvent::updateBot(BZAdvancedRobot *bot) const
{
  bot->onHitWall(*this);
  return true;
}

messageParseStatus DeathEvent::parse(char **, int count)
{
  if (count == 0)
    return ParseOk;
  return InvalidArgumentCount;
}
bool DeathEvent::updateBot(BZAdvancedRobot *bot) const
{
  bot->onDeath(*this);
  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
