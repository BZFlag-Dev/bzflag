#include "RCReplies.h"

#include "version.h"

RCReply::parseStatus GunHeatReply::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], heat))
    return InvalidArguments;

  return ParseOk;
}
void GunHeatReply::getParameters(std::ostream &stream)
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
void DistanceRemainingReply::getParameters(std::ostream &stream)
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
void TurnRemainingReply::getParameters(std::ostream &stream)
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
void TickDurationReply::getParameters(std::ostream &stream)
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
void TickRemainingReply::getParameters(std::ostream &stream)
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
