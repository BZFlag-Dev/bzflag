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

#include "RCReplies.h"

#include "RCMessageFactory.h"
#include "BZAdvancedRobot.h"
#include "MessageUtilities.h"
#include "Tank.h"

#include "version.h"

messageParseStatus IdentifyBackend::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  version = arguments[0];
  /* Version-checking, to be sure we're speaking the same language! */
  if (version != getRobotsProtocolVersion())
    return InvalidArguments;
  return ParseOk;
}
void IdentifyBackend::getParameters(std::ostream &stream) const
{
  stream << version;
} 

messageParseStatus EventReply::parse(char **arguments, int count)
{
  if (count < 1)
    return InvalidArgumentCount;

  notification = RCEVENT.Message(arguments[0]);
  if (notification)
      return notification->parse(arguments + 1, count - 1);
  else
    return InvalidArguments;
}
void EventReply::getParameters(std::ostream &stream) const
{
  if (notification)
  {
    stream << notification->getType() << " ";
    notification->getParameters(stream);
  }
}
bool EventReply::updateBot(const BZAdvancedRobot *robot) const
{
  if (notification)
    notification->updateBot(robot);
  return true;
}

messageParseStatus CommandDoneReply::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;

  command = arguments[0];
  if (!RCREQUEST.IsRegistered(command))
    return InvalidArguments;

  return ParseOk;
}
void CommandDoneReply::getParameters(std::ostream &stream) const
{
  stream << command;
}

messageParseStatus GunHeatReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, heat);
}
void GunHeatReply::getParameters(std::ostream &stream) const
{
  stream << heat;
}
bool GunHeatReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->gunHeat = heat;
  return true;
}

messageParseStatus DistanceRemainingReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, distance);
}
void DistanceRemainingReply::getParameters(std::ostream &stream) const
{
  stream << distance;
}
bool DistanceRemainingReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->distanceRemaining = distance;
  return true;
}

messageParseStatus TurnRemainingReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, turn);
}
void TurnRemainingReply::getParameters(std::ostream &stream) const
{
  stream << turn;
}
bool TurnRemainingReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->turnRemaining = turn;
  return true;
}


messageParseStatus TickDurationReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, duration);
}
void TickDurationReply::getParameters(std::ostream &stream) const
{
  stream << duration;
}

messageParseStatus TickRemainingReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, remaining);
}
void TickRemainingReply::getParameters(std::ostream &stream) const
{
  stream << remaining;
}

messageParseStatus BattleFieldSizeReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, size);
}
void BattleFieldSizeReply::getParameters(std::ostream &stream) const
{
  stream << size;
}
bool BattleFieldSizeReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->battleFieldSize = size;
  return true;
}

messageParseStatus XReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, x);
}
void XReply::getParameters(std::ostream &stream) const
{
  stream << x;
}
bool XReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->xPosition = x;
  return true;
}

messageParseStatus YReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, y);
}
void YReply::getParameters(std::ostream &stream) const
{
  stream << y;
}
bool YReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->yPosition = y;
  return true;
}
messageParseStatus ZReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, z);
}
void ZReply::getParameters(std::ostream &stream) const
{
  stream << z;
}
bool ZReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->zPosition = z;
  return true;
}

messageParseStatus WidthReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, width);
}
void WidthReply::getParameters(std::ostream &stream) const
{
  stream << width;
}
bool WidthReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->tankWidth = width;
  return true;
}
messageParseStatus HeightReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, height);
}
void HeightReply::getParameters(std::ostream &stream) const
{
  stream << height;
}
bool HeightReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->tankHeight = height;
  return true;
}
messageParseStatus LengthReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, length);
}
void LengthReply::getParameters(std::ostream &stream) const
{
  stream << length;
}
bool LengthReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->tankLength = length;
  return true;
}

messageParseStatus HeadingReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingle(arguments, count, heading);
}
void HeadingReply::getParameters(std::ostream &stream) const
{
  stream << heading;
}
bool HeadingReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->heading = heading;
  return true;
}

messageParseStatus PlayersBeginReply::parse(char **, int count)
{
  return (count == 0 ? ParseOk : InvalidArgumentCount);
}
void PlayersBeginReply::getParameters(std::ostream &) const
{
}
bool PlayersBeginReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->players.clear();
  return true;
}

messageParseStatus PlayersReply::parse(char **arguments, int count)
{
  return tank.parse(arguments, count);
}
void PlayersReply::getParameters(std::ostream &stream) const
{
  stream << tank;
}
bool PlayersReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->players.push_back(tank);
  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
