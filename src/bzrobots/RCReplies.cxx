#include "RCReplies.h"

#include "RCMessageFactory.h"
#include "MessageUtilities.h"

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
  return MessageUtilities::parseSingleFloat(arguments, count, heat);
}
void GunHeatReply::getParameters(std::ostream &stream) const
{
  stream << heat;
}
bool GunHeatReply::updateBot(BZAdvancedRobot *robot) const
{
  robot->gunHeat = heat;
  return true;
}

messageParseStatus DistanceRemainingReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingleFloat(arguments, count, distance);
}
void DistanceRemainingReply::getParameters(std::ostream &stream) const
{
  stream << distance;
}
bool DistanceRemainingReply::updateBot(BZAdvancedRobot *robot) const
{
  robot->distanceRemaining = distance;
  return true;
}

messageParseStatus TurnRemainingReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingleFloat(arguments, count, turn);
}
void TurnRemainingReply::getParameters(std::ostream &stream) const
{
  stream << turn;
}
bool TurnRemainingReply::updateBot(BZAdvancedRobot *robot) const
{
  robot->turnRemaining = turn;
  return true;
}


messageParseStatus TickDurationReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingleFloat(arguments, count, duration);
}
void TickDurationReply::getParameters(std::ostream &stream) const
{
  stream << duration;
}

messageParseStatus TickRemainingReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingleFloat(arguments, count, remaining);
}
void TickRemainingReply::getParameters(std::ostream &stream) const
{
  stream << remaining;
}

messageParseStatus BattleFieldSizeReply::parse(char **arguments, int count)
{
  return MessageUtilities::parseSingleFloat(arguments, count, size);
}
void BattleFieldSizeReply::getParameters(std::ostream &stream) const
{
  stream << size;
}
bool BattleFieldSizeReply::updateBot(BZAdvancedRobot *robot) const
{
  robot->battleFieldSize = size;
  return true;
}

//BZDB.set(StateDatabase::BZDB_WORLDSIZE,

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
