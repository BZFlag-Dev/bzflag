#include "RCReplies.h"

#include "version.h"

RCReply::parseStatus IdentifyBackend::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  /* Version-checking, to be sure we're speaking the same language! */
  if (strcasecmp(arguments[0], getRobotsProtocolVersion()) != 0)
    return InvalidArguments;
  version = strdup(arguments[0]);
  return ParseOk;
}
void IdentifyBackend::getParameters(std::ostream &stream) const
{
    stream << version;
} 


RCReply::parseStatus GunHeatReply::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], heat))
    return InvalidArguments;

  return ParseOk;
}
void GunHeatReply::getParameters(std::ostream &stream) const
{
    stream << heat;
}

RCReply::parseStatus DistanceRemainingReply::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], distance))
    return InvalidArguments;

  return ParseOk;
}
void DistanceRemainingReply::getParameters(std::ostream &stream) const
{
    stream << distance;
}

RCReply::parseStatus TurnRemainingReply::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], turn))
    return InvalidArguments;

  return ParseOk;
}
void TurnRemainingReply::getParameters(std::ostream &stream) const
{
    stream << turn;
}

RCReply::parseStatus TickDurationReply::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], duration))
    return InvalidArguments;

  return ParseOk;
}
void TickDurationReply::getParameters(std::ostream &stream) const
{
    stream << duration;
}

RCReply::parseStatus TickRemainingReply::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], remaining))
    return InvalidArguments;

  return ParseOk;
}
void TickRemainingReply::getParameters(std::ostream &stream) const
{
    stream << remaining;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
