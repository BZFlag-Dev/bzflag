/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "RCRequests.h"

#include "RCReplies.h"
#include "RCRobotPlayer.h"
#include "MessageUtilities.h"
#include "BZDBCache.h"

#include "version.h"

messageParseStatus RCRequestZeroArgument::parse(char **, int count)
{
  if (count != 0)
    return InvalidArgumentCount;
  return ParseOk;
}
void RCRequestZeroArgument::getParameters(std::ostream &) const
{
}

messageParseStatus IdentifyFrontend::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  version = arguments[0];
  /* Version-checking, to be sure we're speaking the same language! */
  if (version != getRobotsProtocolVersion())
    return InvalidArguments;
  return ParseOk;
}
void IdentifyFrontend::getParameters(std::ostream &stream) const
{
  stream << version;
}

messageParseStatus SetSpeedReq::parse(char **arguments, int count)
{
  messageParseStatus status = MessageUtilities::parseSingle(arguments, count, speed);
  if (status == ParseOk)
    speed = MessageUtilities::clamp(speed, 0.0, 1.0);
  return status;
}
void SetSpeedReq::getParameters(std::ostream &stream) const
{
  stream << speed;
}

messageParseStatus SetTurnRateReq::parse(char **arguments, int count)
{
  messageParseStatus status = MessageUtilities::parseSingle(arguments, count, rate);
  if (status == ParseOk)
    rate = MessageUtilities::clamp(rate, 0.0, 1.0);
  return status;
}
void SetTurnRateReq::getParameters(std::ostream &stream) const
{
  stream << rate;
}

messageParseStatus SetAheadReq::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, distance);
}
void SetAheadReq::getParameters(std::ostream &stream) const
{
  stream << distance;
}

messageParseStatus SetTurnLeftReq::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, turn);
}
void SetTurnLeftReq::getParameters(std::ostream &stream) const
{
  stream << turn;
}

messageParseStatus SetTickDurationReq::parse(char **arguments, int count)
{
  messageParseStatus status = MessageUtilities::parseSingle(arguments, count, duration);
  if (status == ParseOk)
    duration = std::max(duration, 0.0);
  return status;
}
void SetTickDurationReq::getParameters(std::ostream &stream) const
{
  stream << duration;
}

messageParseStatus SetStopReq::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, overwrite);
}
void SetStopReq::getParameters(std::ostream &stream) const
{
  stream << overwrite;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
