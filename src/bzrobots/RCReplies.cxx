/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "RCReplies.h"

/* common implementation headers */
#include "version.h"
#include "BoxBuilding.h"
#include "PyramidBuilding.h"
#include "WallObstacle.h"
#include "BaseBuilding.h"
#include "Teleporter.h"
#include "Roster.h"

/* local implementation headers */
#include "RCMessageFactory.h"
#include "BZAdvancedRobot.h"
#include "MessageUtilities.h"
#include "Tank.h"
#include "Shot.h"


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
  if (notification) {
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

messageParseStatus ObstaclesBeginReply::parse(char **, int count)
{
  return (count == 0 ? ParseOk : InvalidArgumentCount);
}

void ObstaclesBeginReply::getParameters(std::ostream &) const
{
}

bool ObstaclesBeginReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->obstacles.clear();
  return true;
}

void ObstacleReply::getParameters(std::ostream &stream) const
{
  const float *pos;
  Teleporter *tele;
  switch (type) {
    case boxType:
      pos = obs->getPosition();
      stream << type << " " << pos[0] << " " << pos[1] << " " << pos[2] << " ";
      stream << obs->getRotation() << " " << obs->getWidth() << " ";
      stream << obs->getBreadth() << " " << obs->getHeight() << " ";
      stream << (bool)obs->isDriveThrough() << " " << (bool)obs->isShootThrough() << " ";
      stream << dynamic_cast<BoxBuilding *>(obs)->isInvisible();
      break;
    case pyrType:
      pos = obs->getPosition();
      stream << type << " " << pos[0] << " " << pos[1] << " " << pos[2] << " ";
      stream << obs->getRotation() << " " << obs->getWidth() << " ";
      stream << obs->getBreadth() << " " << obs->getHeight() << " ";
      stream << (bool)obs->isDriveThrough() << " " << (bool)obs->isShootThrough();
      break;
    case wallType:
      pos = obs->getPosition();
      stream << type << " " << pos[0] << " " << pos[1] << " " << pos[2] << " ";
      stream << obs->getRotation() << " " << obs->getBreadth() << " ";
      stream << obs->getHeight();
      break;
    case baseType:
      pos = obs->getPosition();
      stream << type << " " << pos[0] << " " << pos[1] << " " << pos[2] << " ";
      stream << obs->getRotation() << " " << obs->getWidth() << " ";
      stream << obs->getBreadth() << " " << obs->getHeight() << " ";
      stream << ((BaseBuilding *)obs)->getBaseTeam();
    case teleType:
      pos = obs->getPosition();
      stream << type << " " << pos[0] << " " << pos[1] << " " << pos[2] << " ";
      stream << obs->getRotation() << " " << obs->getWidth() << " ";
      stream << obs->getBreadth() << " " << obs->getHeight() << " ";
      tele = (Teleporter *)obs;
      stream << tele->getBorder() << " ";
      stream << (bool)tele->isDriveThrough() << " ";
      stream << (bool)tele->isShootThrough();
      break;
    case meshType:
      /*
       * TODO: Implement this.
       */
      break;
    case arcType:
      /*
       * TODO: Implement this.
       */
      break;
    case coneType:
      /*
       * TODO: Implement this.
       */
      break;
    case sphereType:
      /*
       * TODO: Implement this.
       */
      break;
    default:
      /*
       * TODO: Implement this.
       */
      break;
  }
}

messageParseStatus ObstacleReply::parse(char **arguments, int count)
{

  if (count == 0)
    return InvalidArgumentCount;

  uint32_t t;

  if (!MessageUtilities::parse(arguments[0], t))
    return InvalidArguments;

  type = (ObstacleType)t;

  switch (type) {
    case boxType:
      return parseBox(arguments+1, count-1);
      break;
    case pyrType:
      return parsePyr(arguments+1, count-1);
      break;
    case wallType:
      return parseWall(arguments+1, count-1);
      break;
    case baseType:
      return parseBase(arguments+1, count-1);
      break;
    case teleType:
      return parseTele(arguments+1, count-1);
      break;
    case meshType:
      /*
       * TODO: Implement this.
       */
      break;
    case arcType:
      /*
       * TODO: Implement this.
       */
      break;
    case coneType:
      /*
       * TODO: Implement this.
       */
      break;
    case sphereType:
      /*
       * TODO: Implement this.
       */
      break;
    default:
      break;
  }
  return InvalidArguments;
}

messageParseStatus ObstacleReply::parseBox(char **arguments, int count)
{
  if (count != 10)
    return InvalidArgumentCount;

  fvec3 p;
  float rot, width, breadth, height;
  bool drive, shoot;
  bool invisible;

  if (!MessageUtilities::parse(arguments[0], p[0]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[1], p[1]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[2], p[2]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[3], rot))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[4], width))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[5], breadth))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[6], height))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[7], drive))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[8], shoot))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[9], invisible))
    return InvalidArguments;


  obs = new BoxBuilding(p, rot, width, breadth, height,
                        (unsigned char)drive, (unsigned char)shoot, false, invisible);
                        // FIXME false is for 'ricochet'
  return ParseOk;
}

messageParseStatus ObstacleReply::parsePyr(char **arguments, int count)
{
  if (count != 9)
    return InvalidArgumentCount;
  fvec3 p;
  float rot, width, breadth, height;
  bool drive, shoot;

  if (!MessageUtilities::parse(arguments[0], p[0]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[1], p[1]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[2], p[2]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[3], rot))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[4], width))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[5], breadth))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[6], height))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[7], drive))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[8], shoot))
    return InvalidArguments;

  obs = new PyramidBuilding(p, rot, width, breadth, height,
                            (unsigned char)drive, (unsigned char)shoot, false);
                            // FIXME false is for ricochet
  return ParseOk;
}

messageParseStatus ObstacleReply::parseWall(char **arguments, int count)
{
  if (count != 6)
    return InvalidArgumentCount;

  fvec3 p;
  float rot, breadth, height;

  if (!MessageUtilities::parse(arguments[0], p[0]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[1], p[1]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[2], p[2]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[3], rot))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[4], breadth))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[5], height))
    return InvalidArguments;

  obs = new WallObstacle(p, rot, breadth, height, false);
                         // FIXME false is for ricochet
  return ParseOk;
}

messageParseStatus ObstacleReply::parseBase(char **arguments, int count)
{
  if (count != 8)
    return InvalidArgumentCount;

  fvec3 p;
  fvec3 s;
  float rot;
  uint32_t team;

  if (!MessageUtilities::parse(arguments[0], p[0]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[1], p[1]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[2], p[2]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[3], rot))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[4], s[0]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[5], s[1]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[6], s[2]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[7], team))
    return InvalidArguments;

  obs = new BaseBuilding(p, rot, s, team, false);
                         // FIXME false is for ricochet
  return ParseOk;
}

messageParseStatus ObstacleReply::parseTele(char **arguments, int count)
{
  if (count != 11)
    return InvalidArgumentCount;

  fvec3 p;
  fvec3 s;
  float rot, border;
  bool horiz, drive, shoot;

  if (!MessageUtilities::parse(arguments[0], p[0]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[1], p[1]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[2], p[2]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[3], rot))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[4], s[0]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[5], s[1]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[6], s[2]))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[7], border))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[8], horiz))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[9], drive))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[10], shoot))
    return InvalidArguments;

  const MeshTransform transform;
  obs = new Teleporter(transform, p, rot, s[0], s[1], s[2], border, 0.0f,
                       (unsigned char)drive, (unsigned char)shoot, false);
                         // FIXME false is for ricochet
  return ParseOk;
}

bool ObstacleReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->obstacles.push_back(obs);
  return true;
}

messageParseStatus ShotsBeginReply::parse(char **, int count)
{
  return (count == 0 ? ParseOk : InvalidArgumentCount);
}

void ShotsBeginReply::getParameters(std::ostream &) const
{
}

bool ShotsBeginReply::updateBot(const BZAdvancedRobot *robot) const
{
  robot->shots.clear();
  return true;
}

void ShotReply::getParameters(std::ostream &stream) const
{
  stream << shot;
}

messageParseStatus ShotReply::parse(char **arguments, int count)
{
  return shot.parse(arguments, count);
}

bool ShotReply::updateBot(const BZAdvancedRobot *robot) const
{
  FrontendShot s(shot);
  s.setRobot(robot);
  robot->shots.push_back(s);
  return true;
}

messageParseStatus ShotPositionReply::parse(char **arguments, int count)
{
  if (count != 4)
    return InvalidArgumentCount;

  if (!MessageUtilities::parse(arguments[0], id))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[1], x))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[2], y))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[3], z))
    return InvalidArguments;

  return ParseOk;
}

void ShotPositionReply::getParameters(std::ostream &stream) const
{
  stream << id << " " << x << " " << y << " " << z;
}

bool ShotPositionReply::updateBot(const BZAdvancedRobot *robot) const
{
  const FrontendShot *shot = robot->getShot(id);

  if(!shot)
    return false;

  shot->x = x;
  shot->y = y;
  shot->z = z;

  return true;
}

messageParseStatus ShotVelocityReply::parse(char **arguments, int count)
{
  if (count != 4)
    return InvalidArgumentCount;

  if (!MessageUtilities::parse(arguments[0], id))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[1], x))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[2], y))
    return InvalidArguments;
  if (!MessageUtilities::parse(arguments[3], z))
    return InvalidArguments;

  return ParseOk;
}

void ShotVelocityReply::getParameters(std::ostream &stream) const
{
  stream << id << " " << x << " " << y << " " << z;
}

bool ShotVelocityReply::updateBot(const BZAdvancedRobot *robot) const
{
  const FrontendShot *shot = robot->getShot(id);

  if(!shot)
    return false;

  shot->vx = x;
  shot->vy = y;
  shot->vz = z;

  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
