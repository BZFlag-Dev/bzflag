/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "WorldBuilder.h"

/* common implementation headers */
#include "StateDatabase.h"
#include "Pack.h"
#include "TextUtils.h"
#include "Protocol.h"
#include "EighthDBoxSceneNode.h"
#include "EighthDPyrSceneNode.h"
#include "EighthDBaseSceneNode.h"

WorldBuilder::WorldBuilder()
{
  world = new World;
  owned = true;
}

WorldBuilder::~WorldBuilder()
{
  if (owned) delete world;
}

void* WorldBuilder::unpack(void* buf)
{
  // unpack world database from network transfer
  // read style header
  uint16_t code, len;
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
  if (code != WorldCodeHeader) return NULL;

  // read style
  uint16_t gameStyle, maxPlayers, maxShots, maxFlags,serverMapVersion;
  buf = nboUnpackUShort(buf, serverMapVersion);
  if (serverMapVersion != mapVersion)
    return NULL;

  float worldSize;
  buf = nboUnpackFloat(buf, worldSize);
  BZDB.set(StateDatabase::BZDB_WORLDSIZE, string_util::format("%f", worldSize));
  buf = nboUnpackUShort(buf, gameStyle);
  setGameStyle(short(gameStyle));
  buf = nboUnpackUShort(buf, maxPlayers);
  setMaxPlayers(int(maxPlayers));
  buf = nboUnpackUShort(buf, maxShots);
  setMaxShots(int(maxShots));
  buf = nboUnpackUShort(buf, maxFlags);
  setMaxFlags(int(maxFlags));
  buf = nboUnpackFloat(buf, world->linearAcceleration);
  buf = nboUnpackFloat(buf, world->angularAcceleration);
  uint16_t shakeTimeout = 0, shakeWins;
  buf = nboUnpackUShort(buf, shakeTimeout);
  setShakeTimeout(0.1f * float(shakeTimeout));
  buf = nboUnpackUShort(buf, shakeWins);
  setShakeWins(shakeWins);
  uint32_t epochOffset;
  buf = nboUnpackUInt(buf, epochOffset);
  setEpochOffset(epochOffset);

  // read geometry
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);
  while (code != WorldCodeEnd) {
    switch (code) {
      case WorldCodeBox: {
	float data[7];
	unsigned char tempflags;

	if (len != WorldCodeBoxSize)
	  return NULL;

	memset(data, 0, sizeof(float) * 7);
	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	buf = nboUnpackUByte(buf, tempflags);
	BoxBuilding box(data, data[3], data[4], data[5], data[6],
			(tempflags & _DRIVE_THRU)!=0, (tempflags & _SHOOT_THRU)!=0);
	append(box);
	break;
      }
      case WorldCodePyramid: {
	float data[7];
	unsigned char tempflags;

	if (len != WorldCodePyramidSize)
	  return NULL;

	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	buf = nboUnpackUByte(buf, tempflags);

	PyramidBuilding pyr(data, data[3], data[4], data[5], data[6],
			    (tempflags & _DRIVE_THRU)!=0, (tempflags & _SHOOT_THRU)!=0);
	if (tempflags & _FLIP_Z)
	  pyr.setZFlip();

	append(pyr);
	break;
      }
      case WorldCodeTeleporter: {
	float data[8];
	unsigned char tempflags;

	if (len != WorldCodeTeleporterSize)
	  return NULL;

	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	buf = nboUnpackFloat(buf, data[7]);
	buf = nboUnpackUByte(buf, tempflags);
	Teleporter tele(data, data[3], data[4], data[5], data[6],data[7],
			(tempflags & _DRIVE_THRU)!=0, (tempflags & _SHOOT_THRU)!=0);
	append(tele);
	break;
      }
      case WorldCodeLink: {
	uint16_t data[2];

	if (len != WorldCodeLinkSize)
	  return NULL;

	buf = nboUnpackUShort(buf, data[0]);
	buf = nboUnpackUShort(buf, data[1]);
	setTeleporterTarget(int(data[0]), int(data[1]));
	break;
      }
      case WorldCodeWall: {
	float data[6];

	if (len != WorldCodeWallSize)
	  return NULL;

	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	WallObstacle wall(data, data[3], data[4], data[5]);
	append(wall);
	break;
      }
      case WorldCodeBase: {
	uint16_t team;
	float data[10];

	if (len != WorldCodeBaseSize)
	  return NULL;

	buf = nboUnpackUShort(buf, team);
	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	buf = nboUnpackFloat(buf, data[7]);
	buf = nboUnpackFloat(buf, data[8]);
	buf = nboUnpackFloat(buf, data[9]);
	BaseBuilding base(data, data[3], data +4, team);
	append(base);
	setBase(TeamColor(team), data, data[3], data[4], data[5], data[6]);
	break;
      }

      default:
	return NULL;
    }
    buf = nboUnpackUShort(buf, len);
    buf = nboUnpackUShort(buf, code);
  }

  return buf;
}

void WorldBuilder::preGetWorld()
{
  // if no inertia gameStyle then make sure accelerations are zero (disabled)
  if (!(world->gameStyle & short(InertiaGameStyle)))
    setInertia(0.0, 0.0);

  // prepare players array
  if (world->players) delete[] world->players;
  world->players = new RemotePlayer*[world->maxPlayers];
  int i;
  for (i = 0; i < world->maxPlayers; i++)
    world->players[i] = NULL;

  // prepare flags array
  world->freeFlags();
  world->flags = new Flag[world->maxFlags];
  world->flagNodes = new FlagSceneNode*[world->maxFlags];
  world->flagWarpNodes = new FlagWarpSceneNode*[world->maxFlags];
  for (i = 0; i < world->maxFlags; i++) {
    world->flags[i].type = Flags::Null;
    world->flags[i].status = FlagNoExist;
    world->flags[i].position[0] = 0.0f;
    world->flags[i].position[1] = 0.0f;
    world->flags[i].position[2] = 0.0f;
    world->flagNodes[i] = new FlagSceneNode(world->flags[i].position);
    world->flagWarpNodes[i] = new FlagWarpSceneNode(world->flags[i].position);
    world->flagNodes[i]->setTexture(World::flagTexture);
  }

  // prepare inside nodes arrays
  world->freeInsideNodes();
  GLfloat obstacleSize[3];
  const int numBoxes = world->boxes.size();
  world->boxInsideNodes = new EighthDimSceneNode*[numBoxes];
  for (i = 0; i < numBoxes; i++) {
    const Obstacle& o = world->boxes[i];
    obstacleSize[0] = o.getWidth();
    obstacleSize[1] = o.getBreadth();
    obstacleSize[2] = o.getHeight();
    world->boxInsideNodes[i] = new EighthDBoxSceneNode(o.getPosition(),
						       obstacleSize, o.getRotation());
  }
  const int numPyramids = world->pyramids.size();
  world->pyramidInsideNodes = new EighthDimSceneNode*[numPyramids];
  for (i = 0; i < numPyramids; i++) {
    const Obstacle& o = world->pyramids[i];
    obstacleSize[0] = o.getWidth();
    obstacleSize[1] = o.getBreadth();
    obstacleSize[2] = o.getHeight();
    world->pyramidInsideNodes[i] = new EighthDPyrSceneNode(o.getPosition(),
							   obstacleSize, o.getRotation());
  }
  const int numBases = world->basesR.size();
  world->baseInsideNodes = new EighthDimSceneNode*[numBases];
  for (i = 0; i < numBases; i++) {
    const Obstacle& o = world->basesR[i];
    obstacleSize[0] = o.getWidth();
    obstacleSize[1] = o.getBreadth();
    obstacleSize[2] = o.getHeight();
    world->baseInsideNodes[i] = new EighthDBaseSceneNode(o.getPosition(),
							 obstacleSize, o.getRotation());
  }

  world->teleportTargets = teleportTargets;
}

World* WorldBuilder::getWorld()
{
  owned = false;
  preGetWorld();
  return world;
}

World* WorldBuilder::peekWorld()
{
  preGetWorld();
  return world;
}

void WorldBuilder::setGameStyle(short gameStyle)
{
  world->gameStyle = gameStyle;
}

void WorldBuilder::setInertia(float linearAccel,	float angularAccel)
{
  world->linearAcceleration = linearAccel;
  world->angularAcceleration = angularAccel;
}

void WorldBuilder::setMaxPlayers(int maxPlayers)
{
  world->maxPlayers = maxPlayers;
}

void WorldBuilder::setMaxShots(int maxShots)
{
  world->maxShots = maxShots;
}

void WorldBuilder::setMaxFlags(int maxFlags)
{
  world->maxFlags = maxFlags;
}

void WorldBuilder::setShakeTimeout(float timeout) const
{
  world->shakeTimeout = timeout;
}

void WorldBuilder::setShakeWins(int wins) const
{
  world->shakeWins = wins;
}

void WorldBuilder::setEpochOffset(uint32_t seconds) const
{
  world->epochOffset = seconds;
}

void WorldBuilder::append(const WallObstacle& wall)
{
  world->walls.push_back(wall);
}

void WorldBuilder::append(const BoxBuilding& box)
{
  world->boxes.push_back(box);
}

void WorldBuilder::append(const PyramidBuilding& pyramid)
{
  world->pyramids.push_back(pyramid);
}

void WorldBuilder::append(const BaseBuilding& base)
{
  world->basesR.push_back(base);
}

void WorldBuilder::append(const Teleporter& teleporter)
{
  // save teleporter
  world->teleporters.push_back(teleporter);
}

void WorldBuilder::setTeleporterTarget(int src, int tgt)
{
  if ((int)teleportTargets.size() < src+1)
    teleportTargets.resize(src+10);

  // record target in source entry
  teleportTargets[src] = tgt;
}

void WorldBuilder::setBase(TeamColor team,
			   const float* pos, float rotation,
			   float w, float b, float h)
{
  int teamIndex = int(team);

  World::BaseParms bp;
  bp.p[0] = pos[0];
  bp.p[1] = pos[1];
  bp.p[2] = pos[2];
  bp.p[3] = rotation;
  bp.p[4] = w;
  bp.p[5] = b;
  bp.p[6] = h;
  world->bases[teamIndex].push_back( bp );
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

