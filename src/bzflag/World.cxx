/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <string.h>
#include "World.h"
#include "global.h"
#include "Pack.h"
#include "Ray.h"
#include "Protocol.h"
#include "DeadPlayer.h"
#include "RemotePlayer.h"
#include "FlagSceneNode.h"
#include "FlagWarpSceneNode.h"
#include "SceneDatabase.h"
#include "EighthDBoxSceneNode.h"
#include "EighthDPyrSceneNode.h"
#include "EighthDBaseSceneNode.h"
#include "playing.h"
#include "texture.h"

static OpenGLTexture*	flagTexture = NULL;

//
// World
//

World*			World::playingField = NULL;
const int		World::maxDeadPlayers = 20;

World::World() : gameStyle(PlainGameStyle),
				linearAcceleration(0.0f),
				angularAcceleration(0.0f),
				maxPlayers(0),
				maxShots(1),
				maxFlags(0),
				teleportTargets(NULL),
				players(NULL),
				deadPlayers(NULL),
				flags(NULL),
				flagNodes(NULL),
				flagWarpNodes(NULL),
				boxInsideNodes(NULL),
				pyramidInsideNodes(NULL),
				baseInsideNodes(NULL)
{
  int i;
  for (i = 0; i < NumTeams; i++) {
    bases[i][0] = 0.0f;
    bases[i][1] = 0.0f;
    bases[i][2] = 0.0f;
    bases[i][3] = 0.0f;
    bases[i][4] = 0.0f;
    bases[i][5] = 0.0f;
    bases[i][6] = 0.0f;
    bases[i][7] = 0.0f;
    bases[i][8] = 0.0f;
  }
  deadPlayers = new DeadPlayer*[maxDeadPlayers];
  for (i = 0; i < maxDeadPlayers; i++)
    deadPlayers[i] = NULL;
}

World::~World()
{
  int i;
  freeFlags();
  freeInsideNodes();
  delete[] teleportTargets;
  for (i = 0; i < maxPlayers; i++)
    delete players[i];
  delete[] players;
  for (i = 0; i < maxDeadPlayers; i++)
    delete deadPlayers[i];
  delete[] deadPlayers;
}

void			World::init()
{
  flagTexture = new OpenGLTexture;
  *flagTexture = getTexture("flag", OpenGLTexture::Max, False);
}

void			World::done()
{
  delete flagTexture;
  flagTexture = NULL;
}

void			World::setFlagTexture(FlagSceneNode* flag)
{
  flag->setTexture(*flagTexture);
}

void			World::setWorld(World* _playingField)
{
  playingField = _playingField;
}

int			World::getTeleportTarget(int source) const
{
  assert(source >= 0 && source < 2 * teleporters.getLength());
  return teleportTargets[source];
}

int			World::getTeleporter(const Teleporter* teleporter,
							int face) const
{
  // search for teleporter
  const int count = teleporters.getLength();
  for (int i = 0; i < count; i++)
    if (teleporter == &teleporters[i])
      return 2 * i + face;
  return -1;
}

const Teleporter*	World::getTeleporter(int source, int& face) const
{
  assert(source >= 0 && source < 2 * teleporters.getLength());
  face = (source & 1);
  return &teleporters[source / 2];
}

EighthDimSceneNode*	World::getInsideSceneNode(const Obstacle* o) const
{
  if (!o) return NULL;

  int i;
  const int numBases = basesR.getLength();
  for (i = 0; i < numBases; i++)
    if (&(basesR[i]) == o)
      return baseInsideNodes[i];
  const int numBoxes = boxes.getLength();
  for (i = 0; i < numBoxes; i++)
    if (&(boxes[i]) == o)
      return boxInsideNodes[i];
  const int numPyramids = pyramids.getLength();
  for (i = 0; i < numPyramids; i++)
    if (&(pyramids[i]) == o)
      return pyramidInsideNodes[i];
  return NULL;
}

TeamColor		World::whoseBase(const float* pos) const
{
  if (!(gameStyle & TeamFlagGameStyle))
    return NoTeam;

  for (int i = 1; i < NumTeams; i++) {
    float nx = pos[0] - bases[i][0];
    float ny = pos[1] - bases[i][1];
    float rx = (float) (cosf(atanf(ny/nx)-bases[i][3]) * sqrt((ny * ny) + (nx * nx)));
    float ry = (float) (sinf(atanf(ny/nx)-bases[i][3]) * sqrt((ny * ny) + (nx * nx)));
    if(fabsf(rx) < bases[i][4] &&
       fabsf(ry) < bases[i][5]) {
      float nz = (bases[i][2] > 0) ? (bases[i][2] + 1) : 0;
      float rz = pos[2] - nz;
      if(fabsf(rz) < 0.1) { // epsilon kludge
	return TeamColor(i);
      }
    }
  }
  return NoTeam;
}

const Obstacle*		World::inBuilding(const float* pos, float radius) const
{
  // check boxes
  BoxBuildingsCIterator boxScan(boxes);
  while (!boxScan.isDone()) {
    const BoxBuilding& box = boxScan.getItem();
    if (box.isInside(pos, radius))
      return &box;
    boxScan.next();
  }

  // check pyramids
  PyramidBuildingsCIterator pyramidScan(pyramids);
  while (!pyramidScan.isDone()) {
    const PyramidBuilding& pyramid = pyramidScan.getItem();
    if (pyramid.isInside(pos, radius))
      return &pyramid;
    pyramidScan.next();
  }

  // check bases
  BaseBuildingsCIterator baseScan(basesR);
  while(!baseScan.isDone()) {
    const BaseBuilding &base = baseScan.getItem();
    if(base.isInside(pos, radius))
      return &base;
    baseScan.next();
  }

  // check teleporters
  TeleportersCIterator teleporterScan(teleporters);
  while (!teleporterScan.isDone()) {
    const Teleporter& teleporter = teleporterScan.getItem();
    if (teleporter.isInside(pos, radius))
      return &teleporter;
    teleporterScan.next();
  }

  // nope
  return NULL;
}

const Obstacle*		World::hitBuilding(const float* pos, float angle,
						float dx, float dy) const
{
  // check walls
  WallObstaclesCIterator wallScan(walls);
  while (!wallScan.isDone()) {
    const WallObstacle& wall = wallScan.getItem();
    if (wall.isInside(pos, angle, dx, dy))
      return &wall;
    wallScan.next();
  }

  // check teleporters
  TeleportersCIterator teleporterScan(teleporters);
  while (!teleporterScan.isDone()) {
    const Teleporter& teleporter = teleporterScan.getItem();
    if (teleporter.isInside(pos, angle, dx, dy))
      return &teleporter;
    teleporterScan.next();
  }

  // strike one -- check boxes
  BoxBuildingsCIterator boxScan(boxes);
  while (!boxScan.isDone()) {
    const BoxBuilding& box = boxScan.getItem();
    if (box.isInside(pos, angle, dx, dy))
      return &box;
    boxScan.next();
  }

  // strike two -- check pyramids
  PyramidBuildingsCIterator pyramidScan(pyramids);
  while (!pyramidScan.isDone()) {
    const PyramidBuilding& pyramid = pyramidScan.getItem();
    if (pyramid.isInside(pos, angle, dx, dy))
      return &pyramid;
    pyramidScan.next();
  }

  // strike three -- check bases
  BaseBuildingsCIterator baseScan(basesR);
  while(!baseScan.isDone()) {
    const BaseBuilding &base = baseScan.getItem();
    if(base.isInside(pos, angle, dx, dy))
      return &base;
    baseScan.next();
  }
  // strike four -- you're out
  return NULL;
}

boolean			World::crossingTeleporter(const float* pos,
					float angle, float dx, float dy,
					float* plane) const
{
  TeleportersCIterator teleporterScan(teleporters);
  while (!teleporterScan.isDone()) {
    const Teleporter& teleporter = teleporterScan.getItem();
    if (teleporter.isCrossing(pos, angle, dx, dy, plane))
      return True;
    teleporterScan.next();
  }
  return False;
}

const Teleporter*	World::crossesTeleporter(const float* oldPos,
						const float* newPos,
						int& face) const
{
  // check teleporters
  TeleportersCIterator teleporterScan(teleporters);
  while (!teleporterScan.isDone()) {
    const Teleporter& teleporter = teleporterScan.getItem();
    if (teleporter.hasCrossed(oldPos, newPos, face))
      return &teleporter;
    teleporterScan.next();
  }

  // didn't cross
  return NULL;
}

const Teleporter*	World::crossesTeleporter(const Ray& r, int& face) const
{
  // check teleporters
  TeleportersCIterator teleporterScan(teleporters);
  while (!teleporterScan.isDone()) {
    const Teleporter& teleporter = teleporterScan.getItem();
    if (teleporter.isTeleported(r, face) > Epsilon)
      return &teleporter;
    teleporterScan.next();
  }

  // didn't cross
  return NULL;
}

float			World::getProximity(const float* p, float r) const
{
  // get maximum over all teleporters
  float bestProximity = 0.0;
  TeleportersCIterator teleporterScan(teleporters);
  while (!teleporterScan.isDone()) {
    const float proximity = teleporterScan.getItem().getProximity(p, r);
    if (proximity > bestProximity) bestProximity = proximity;
    teleporterScan.next();
  }
  return bestProximity;
}

void			World::freeFlags()
{
  int i;
  if (flagNodes)
    for (i = 0; i < maxFlags; i++)
      delete flagNodes[i];
  if (flagWarpNodes)
    for (i = 0; i < maxFlags; i++)
      delete flagWarpNodes[i];
  delete[] flags;
  delete[] flagNodes;
  delete[] flagWarpNodes;
  flags = NULL;
  flagNodes = NULL;
  flagWarpNodes = NULL;
}

void			World::freeInsideNodes()
{
  // free eighth dimension nodes
  if (boxInsideNodes) {
    const int numBoxes = boxes.getLength();
    for (int i = 0; i < numBoxes; i++)
      delete boxInsideNodes[i];
    delete[] boxInsideNodes;
    boxInsideNodes = NULL;
  }
  if (pyramidInsideNodes) {
    const int numPyramids = pyramids.getLength();
    for (int i = 0; i < numPyramids; i++)
      delete pyramidInsideNodes[i];
    delete[] pyramidInsideNodes;
    pyramidInsideNodes = NULL;
  }
  if (baseInsideNodes) {
    const int numBases = basesR.getLength();
    for(int i = 0; i < numBases; i++)
      delete baseInsideNodes[i];
    delete [] baseInsideNodes;
    baseInsideNodes = NULL;
  }
}

void			World::initFlag(int index)
{
  // set color of flag (opaque)
  const float* color = Flag::getColor(flags[index].id);
  flagNodes[index]->setColor(color[0], color[1], color[2]);

  // if coming or going then position warp
  Flag& flag = flags[index];
  if (flag.status == FlagComing || flag.status == FlagGoing) {
    GLfloat pos[3];
    pos[0] = flag.position[0];
    pos[1] = flag.position[1];
    pos[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	0.25f * Gravity * flag.flightEnd) + flag.position[2];
    flagWarpNodes[index]->move(pos);
    flagWarpNodes[index]->setSizeFraction(0.0f);
  }
}

void			World::updateFlag(int index, float dt)
{
  if (!flagNodes) return;
  const GLfloat* color = flagNodes[index]->getColor();
  GLfloat alpha = color[3];
  Flag& flag = flags[index];

  float droop = 0.0f;
  switch (flag.status) {
    default:
      // do nothing (don't move cos either it's not moving or we
      // don't know the position to move it to)
      break;

    case FlagInAir:
      flag.flightTime += dt;
      if (flag.flightTime >= flag.flightEnd) {
	// touchdown
	flag.status = FlagOnGround;
	flag.position[0] = flag.landingPosition[0];
	flag.position[1] = flag.landingPosition[1];
	flag.position[2] = flag.landingPosition[2];
      }
      else {
	// still flying
	float t = flag.flightTime / flag.flightEnd;
	flag.position[0] = (1.0f - t) * flag.launchPosition[0] +
				t * flag.landingPosition[0];
	flag.position[1] = (1.0f - t) * flag.launchPosition[1] +
				t * flag.landingPosition[1];
	flag.position[2] = (1.0f - t) * flag.launchPosition[2] +
				t * flag.landingPosition[2] +
				flag.flightTime * (flag.initialVelocity +
					0.5f * Gravity * flag.flightTime);
      }
      break;

    case FlagComing:
      flag.flightTime += dt;
      if (flag.flightTime >= flag.flightEnd) {
	// touchdown
	flag.status = FlagOnGround;
	flag.position[2] = 0.0f;
	alpha = 1.0f;
      }
      else if (flag.flightTime >= 0.5f * flag.flightEnd) {
	// falling
	flag.position[2] = flag.flightTime * (flag.initialVelocity +
	    0.5f * Gravity * flag.flightTime) + flag.landingPosition[2];
	alpha = 1.0f;
      }
      else {
	// hovering
	flag.position[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	    0.25f * Gravity * flag.flightEnd) + flag.landingPosition[2];

	// flag is fades in during first half of hovering period
	// and is opaque during the second half.  flag warp grows
	// to full size during first half, and shrinks to nothing
	// during second half.
	if (flag.flightTime >= 0.25f * flag.flightEnd) {
	  // second half
	  float t = (flag.flightTime - 0.25f * flag.flightEnd) /
						(0.25f * flag.flightEnd);
	  alpha = 1.0f;
	  flagWarpNodes[index]->setSizeFraction(1.0f - t);
	}
	else {
	  // first half
	  float t = flag.flightTime / (0.25f * flag.flightEnd);
	  alpha = t;
	  flagWarpNodes[index]->setSizeFraction(t);
	}
      }
      break;

    case FlagGoing:
      flag.flightTime += dt;
      if (flag.flightTime >= flag.flightEnd) {
	// all gone
	flag.status = FlagNoExist;
      }
      else if (flag.flightTime < 0.5f * flag.flightEnd) {
	// rising
	flag.position[2] = flag.flightTime * (flag.initialVelocity +
	    0.5f * Gravity * flag.flightTime) + flag.landingPosition[2];
	alpha = 1.0f;
      }
      else {
	// hovering
	flag.position[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	    0.25f * Gravity * flag.flightEnd) + flag.landingPosition[2];

	// flag is opaque during first half of hovering period
	// and fades out during the second half.  flag warp grows
	// to full size during first half, and shrinks to nothing
	// during second half.
	if (flag.flightTime < 0.75f * flag.flightEnd) {
	  // first half
	  float t = (0.75f * flag.flightEnd - flag.flightTime) /
						(0.25f * flag.flightEnd);
	  alpha = 1.0f;
	  flagWarpNodes[index]->setSizeFraction(1.0f - t);
	}
	else {
	  // second half
	  float t = (flag.flightEnd - flag.flightTime) /
						(0.25f * flag.flightEnd);
	  alpha = t;
	  flagWarpNodes[index]->setSizeFraction(t);
	}
      }
      break;
  }
  flagNodes[index]->waveFlag(dt, droop);

  // update alpha if changed
  if (alpha != color[3])
    flagNodes[index]->setColor(color[0], color[1], color[2], alpha);

  // move flag scene node
  flagNodes[index]->move(flags[index].position);

  // narrow flag on tank turns with tank (so it's almost invisible head-on)
  if (flag.id == NarrowFlag && flag.status == FlagOnTank) {
    for (int i = 0; i < maxPlayers; i++)
      if (players[i] && players[i]->getId() == flag.owner) {
	const float* dir = players[i]->getForward();
	flagNodes[index]->setBillboard(False);
	flagNodes[index]->turn(atan2f(dir[1], dir[0]));
	break;
      }
  }
  else {
    flagNodes[index]->setBillboard(True);
  }
}

void			World::addFlags(SceneDatabase* scene)
{
  if (!flagNodes) return;
  for (int i = 0; i < maxFlags; i++) {
    if (flags[i].status == FlagNoExist) continue;
    // skip flag on a tank that isn't alive.  also skip Cloaking
    // flags on tanks.
    if (flags[i].status == FlagOnTank) {
      if (flags[i].id == CloakingFlag) continue;
      int j;
      for (j = 0; j < maxPlayers; j++)
	if (players[j] && players[j]->getId() == flags[i].owner)
	  break;

      if (j < maxPlayers && !(players[j]->getStatus() & Player::Alive))
	continue;
    }

    scene->addDynamicNode(flagNodes[i]);

    // add warp if coming/going and hovering
    if ((flags[i].status == FlagComing &&
	flags[i].flightTime < 0.5 * flags[i].flightEnd) ||
	(flags[i].status == FlagGoing &&
	flags[i].flightTime >= 0.5 * flags[i].flightEnd))
      scene->addDynamicNode(flagWarpNodes[i]);
  }
}

void			World::reviveDeadPlayer(Player* revivedPlayer)
{
  // see if player is in the list of the dead
  for (int i = 0; i < maxDeadPlayers; i++) {
    if (deadPlayers[i] == NULL)
      continue;
    if (deadPlayers[i]->getId().serverHost.s_addr !=
		revivedPlayer->getId().serverHost.s_addr)
      continue;
    if (strcmp(deadPlayers[i]->getCallSign(), revivedPlayer->getCallSign())!=0)
      continue;

    // that's the guy!  copy the local wins and losses (assuming the
    // revived player's local wins and losses are zero).
    revivedPlayer->changeLocalScore(deadPlayers[i]->getLocalWins(),
				    deadPlayers[i]->getLocalLosses());

    // remove dead player.  keep dead player list packed but don't
    // shuffle the order.  a linked list would be better.
    delete deadPlayers[i];
    for (int j = i + 1; j < maxDeadPlayers; j++)
      deadPlayers[j - 1] = deadPlayers[j];
    deadPlayers[maxDeadPlayers - 1] = NULL;
  }
}

void			World::addDeadPlayer(Player* dyingPlayer)
{
  // player is leaving so save player info.  if dead player list is
  // full then drop off oldest one.
  delete deadPlayers[maxDeadPlayers - 1];
  for (int i = maxDeadPlayers - 1; i > 0; i--)
    deadPlayers[i] = deadPlayers[i - 1];
  deadPlayers[0] = new DeadPlayer(*dyingPlayer);
}

//
// WorldBuilder
//

static const int	TeleportArrayGranularity = 16;

WorldBuilder::WorldBuilder() : targetArraySize(TeleportArrayGranularity)
{
  world = new World;
  owned = True;
  teleportTargets = new int[2 * targetArraySize];
}

WorldBuilder::~WorldBuilder()
{
  if (owned) delete world;
  delete[] teleportTargets;
}

void*			WorldBuilder::unpack(void* buf)
{
  // unpack world database from network transfer
  // read style header
  uint16_t code, len;
  void* tmpBuf = buf;
  buf = nboUnpackUShort(buf, code);
  if (code != WorldCodeStyle) return tmpBuf;
  buf = nboUnpackUShort(buf, len);
  tmpBuf = (void*)((char*)buf + len);

  // read style
  uint16_t gameStyle, maxPlayers, maxShots, maxFlags;
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
  uint16_t shakeTimeout, shakeWins;
  buf = nboUnpackUShort(buf, shakeTimeout);
  setShakeTimeout(0.1f * float(shakeTimeout));
  buf = nboUnpackUShort(buf, shakeWins);
  setShakeWins(shakeWins);
  uint32_t epochOffset;
  buf = nboUnpackUInt(buf, epochOffset);
  setEpochOffset(epochOffset);

  // read geometry
  buf = tmpBuf;
  buf = nboUnpackUShort(buf, code);
  while (code != WorldCodeEnd) {
    switch (code) {
      case WorldCodeBox: {
	float data[7];
	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	BoxBuilding box(data, data[3], data[4], data[5], data[6]);
	append(box);
	break;
      }
      case WorldCodePyramid: {
	float data[7];
	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	PyramidBuilding pyr(data, data[3], data[4], data[5], data[6]);
	append(pyr);
	break;
      }
      case WorldCodeTeleporter: {
	float data[8];
	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	buf = nboUnpackFloat(buf, data[7]);
	Teleporter tele(data, data[3], data[4], data[5], data[6], data[7]);
	append(tele);
	break;
      }
      case WorldCodeLink: {
	uint16_t data[2];
	buf = nboUnpackUShort(buf, data[0]);
	buf = nboUnpackUShort(buf, data[1]);
	setTeleporterTarget(int(data[0]), int(data[1]));
	break;
      }
      case WorldCodeWall: {
	float data[6];
	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	WallObstacle pyr(data, data[3], data[4], data[5]);
	append(pyr);
	break;
      }
      case WorldCodeBase: {
	uint16_t team;
	float data[9];
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
	BaseBuilding base(data, data[3], data +4, team);
	append(base);
	setBase(TeamColor(team), data, data[3], data[4], data[5], data + 6);
	break;
      }
    }
    buf = nboUnpackUShort(buf, code);
  }

  return buf;
}

void			WorldBuilder::preGetWorld()
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
    world->flags[i].id = NoFlag;
    world->flags[i].status = FlagNoExist;
    world->flags[i].position[0] = 0.0f;
    world->flags[i].position[1] = 0.0f;
    world->flags[i].position[2] = 0.0f;
    world->flagNodes[i] = new FlagSceneNode(world->flags[i].position);
    world->flagWarpNodes[i] = new FlagWarpSceneNode(world->flags[i].position);
    world->flagNodes[i]->setTexture(*flagTexture);
  }

  // prepare inside nodes arrays
  world->freeInsideNodes();
  GLfloat obstacleSize[3];
  const int numBoxes = world->boxes.getLength();
  world->boxInsideNodes = new EighthDimSceneNode*[numBoxes];
  for (i = 0; i < numBoxes; i++) {
    const Obstacle& o = world->boxes[i];
    obstacleSize[0] = o.getWidth();
    obstacleSize[1] = o.getBreadth();
    obstacleSize[2] = o.getHeight();
    world->boxInsideNodes[i] = new EighthDBoxSceneNode(o.getPosition(),
						obstacleSize, o.getRotation());
  }
  const int numPyramids = world->pyramids.getLength();
  world->pyramidInsideNodes = new EighthDimSceneNode*[numPyramids];
  for (i = 0; i < numPyramids; i++) {
    const Obstacle& o = world->pyramids[i];
    obstacleSize[0] = o.getWidth();
    obstacleSize[1] = o.getBreadth();
    obstacleSize[2] = o.getHeight();
    world->pyramidInsideNodes[i] = new EighthDPyrSceneNode(o.getPosition(),
						obstacleSize, o.getRotation());
  }
  const int numBases = world->basesR.getLength();
  world->baseInsideNodes = new EighthDimSceneNode*[numBases];
  for (i = 0; i < numBases; i++) {
    const Obstacle& o = world->basesR[i];
    obstacleSize[0] = o.getWidth();
    obstacleSize[1] = o.getBreadth();
    obstacleSize[2] = o.getHeight();
    world->baseInsideNodes[i] = new EighthDBaseSceneNode(o.getPosition(),
						obstacleSize, o.getRotation());
  }

  // copy teleporter target list
  if (world->teleportTargets)
    delete[] world->teleportTargets;
  const int size = 2 * world->teleporters.getLength();
  world->teleportTargets = new int[size];
  ::memcpy(world->teleportTargets, teleportTargets, size * sizeof(int));
}

World*			WorldBuilder::getWorld()
{
  owned = False;
  preGetWorld();
  return world;
}

World*			WorldBuilder::peekWorld()
{
  preGetWorld();
  return world;
}

void			WorldBuilder::setGameStyle(short gameStyle)
{
  world->gameStyle = gameStyle;
}

void			WorldBuilder::setInertia(float linearAccel,
						float angularAccel)
{
  world->linearAcceleration = linearAccel;
  world->angularAcceleration = angularAccel;
}

void			WorldBuilder::setMaxPlayers(int maxPlayers)
{
  world->maxPlayers = maxPlayers;
}

void			WorldBuilder::setMaxShots(int maxShots)
{
  world->maxShots = maxShots;
}

void			WorldBuilder::setMaxFlags(int maxFlags)
{
  world->maxFlags = maxFlags;
}

void			WorldBuilder::setShakeTimeout(float timeout) const
{
  world->shakeTimeout = timeout;
}

void			WorldBuilder::setShakeWins(int wins) const
{
  world->shakeWins = wins;
}

void			WorldBuilder::setEpochOffset(uint32_t seconds) const
{
  world->epochOffset = seconds;
}

void			WorldBuilder::append(const WallObstacle& wall)
{
  world->walls.append(wall);
}

void			WorldBuilder::append(const BoxBuilding& box)
{
  world->boxes.append(box);
}

void			WorldBuilder::append(const PyramidBuilding& pyramid)
{
  world->pyramids.append(pyramid);
}

void			WorldBuilder::append(const BaseBuilding& base)
{
  world->basesR.append(base);
}

void			WorldBuilder::append(const Teleporter& teleporter)
{
  // save telelporter
  world->teleporters.append(teleporter);
}

void			WorldBuilder::setTeleporterTarget(int src, int tgt)
{
  // make sure list is big enough
  growTargetList(src / 2 + 1);

  // record target in source entry
  teleportTargets[src] = tgt;
}

void			WorldBuilder::setBase(TeamColor team,
					const float* pos, float rotation,
					float w, float b, const float* safety)
{
  int teamIndex = int(team);
  world->bases[teamIndex][0] = pos[0];
  world->bases[teamIndex][1] = pos[1];
  world->bases[teamIndex][2] = pos[2];
  world->bases[teamIndex][3] = rotation;
  world->bases[teamIndex][4] = w;
  world->bases[teamIndex][5] = b;
  world->bases[teamIndex][6] = safety[0];
  world->bases[teamIndex][7] = safety[1];
  world->bases[teamIndex][8] = safety[2];
}

void			WorldBuilder::growTargetList(int newMinSize)
{
  if (newMinSize < targetArraySize) return;

  // get new size at lease as big as newMinSize
  int newSize = targetArraySize;
  while (newSize <= newMinSize) newSize += TeleportArrayGranularity;

  // copy targets into a larger buffer
  int* newTargets = new int[2 * newSize];
  ::memcpy(newTargets, teleportTargets, 2 * targetArraySize * sizeof(int));
  delete[] teleportTargets;
  targetArraySize = newSize;
  teleportTargets = newTargets;
}
// ex: shiftwidth=2 tabstop=8
