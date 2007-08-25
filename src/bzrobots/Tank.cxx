#include "Tank.h"

#include "MessageUtilities.h"
#include "Team.h"
#include "Flag.h"

std::ostream& operator<<(std::ostream& os, const Tank& tank)
{
  return os << tank.callsign << " " << tank.team << " "
    << tank.paused << " " << tank.alive << " " << tank.frozen << " " << tank.super << " "
    << tank.team << " " << tank.position[0] << " " << tank.position[1] << " " << tank.position[2] << " "
    << tank.angle;
}

Tank::Tank() {}
Tank::Tank(RemotePlayer *tank)
  :callsign(tank->getCallSign()), team(Team::getShortName(tank->getTeam())),
    flag(tank->getFlag()->flagName), paused(tank->isPaused()),
    alive(tank->isAlive()), frozen(tank->canMove()),
    super(tank->isFlagActive()), angle(tank->getAngle())
{
  const float *pos = tank->getPosition();
  position[0] = pos[0];
  position[1] = pos[1];
  position[2] = pos[2];
}

messageParseStatus Tank::parse(char **arguments, int count)
{
  if (count != 11)
    return InvalidArgumentCount;

  if (!MessageUtilities::parse(arguments[0], callsign))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[1], team))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[2], paused))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[3], alive))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[4], frozen))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[5], super))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[6], flag))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[7], position[0]))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[8], position[1]))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[9], position[2]))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[10], angle))
      return InvalidArguments;

  return ParseOk;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
