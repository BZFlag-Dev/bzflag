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

// todo: make this turn off for .net
#if defined(_MSC_VER)
	#pragma warning(disable: 4786)
#endif

#include <string.h>
#include <fstream>
#include "common.h"
#include "playing.h"
#include "World.h"
#include "global.h"
#include "Pack.h"
#include "Ray.h"
#include "Protocol.h"
#include "RemotePlayer.h"
#include "WorldPlayer.h"
#include "FlagSceneNode.h"
#include "FlagWarpSceneNode.h"
#include "SceneDatabase.h"
#include "EighthDBoxSceneNode.h"
#include "EighthDPyrSceneNode.h"
#include "EighthDBaseSceneNode.h"
#include "texture.h"
#include "StateDatabase.h"
#include "Flag.h"
#include "BZDBCache.h"
#include "TextUtils.h"
#include "TextureManager.h"

static OpenGLTexture*	flagTexture = NULL;

//
// World
//

World*			World::playingField = NULL;
BundleMgr*		World::bundleMgr;
std::string		World::locale("");

World::World() : gameStyle(PlainGameStyle),
				linearAcceleration(0.0f),
				angularAcceleration(0.0f),
				maxPlayers(0),
				curMaxPlayers(0),
				maxShots(1),
				maxFlags(0),
				players(NULL),
				flags(NULL),
				flagNodes(NULL),
				flagWarpNodes(NULL),
				boxInsideNodes(NULL),
				pyramidInsideNodes(NULL),
				baseInsideNodes(NULL)
{
  worldWeapons = new WorldPlayer();
}

World::~World()
{
  int i;
  freeFlags();
  freeInsideNodes();
  for (i = 0; i < curMaxPlayers; i++)
    delete players[i];
  delete[] players;
  delete worldWeapons;
  for (i = 0; i < NumTeams; i++) {
    bases[i].clear();
  }
}

void			World::init()
{
  TextureManager &tm = TextureManager::instance();
  flagTexture = tm.getTexture( "flag" );
}

void			World::done()
{
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
  assert(source >= 0 && source < (int)(2 * teleporters.size()));
  return teleportTargets[source];
}

int			World::getTeleporter(const Teleporter* teleporter,
							int face) const
{
  // search for teleporter
  const int count = teleporters.size();
  for (int i = 0; i < count; i++)
    if (teleporter == &teleporters[i])
      return 2 * i + face;
  return -1;
}

const Teleporter*	World::getTeleporter(int source, int& face) const
{
  assert(source >= 0 && source < (int)(2 * teleporters.size()));
  face = (source & 1);
  return &teleporters[source / 2];
}

EighthDimSceneNode*	World::getInsideSceneNode(const Obstacle* o) const
{
  if (!o) return NULL;

  int i;
  const int numBases = basesR.size();
  for (i = 0; i < numBases; i++)
    if (&(basesR[i]) == o)
      return baseInsideNodes[i];
  const int numBoxes = boxes.size();
  for (i = 0; i < numBoxes; i++)
    if (&(boxes[i]) == o)
      return boxInsideNodes[i];
  const int numPyramids = pyramids.size();
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
    for (TeamBases::const_iterator it = bases[i].begin(); it != bases[i].end(); ++it) {
      float nx = pos[0] - it->p[0];
      float ny = pos[1] - it->p[1];
      float rx = (float) (cosf(atanf(ny/nx)-it->p[3]) * sqrt((ny * ny) + (nx * nx)));
      float ry = (float) (sinf(atanf(ny/nx)-it->p[3]) * sqrt((ny * ny) + (nx * nx)));
      if(fabsf(rx) < it->p[4] &&
         fabsf(ry) < it->p[5]) {
        float nz = it->p[2] + it->p[6];
        float rz = pos[2] - nz;
        if(fabsf(rz) < 0.1) { // epsilon kludge
	  return TeamColor(i);
	}
      }
    }
  }

  return NoTeam;
}

const Obstacle*		World::inBuilding(const float* pos, float radius) const
{
  // check boxes
  std::vector<BoxBuilding>::const_iterator boxScan = boxes.begin();
  while (boxScan != boxes.end()) {
    const BoxBuilding& box = *boxScan;
    if (box.isInside(pos, radius))
      return &box;
    boxScan++;
  }

  // check pyramids
  std::vector<PyramidBuilding>::const_iterator pyramidScan = pyramids.begin();
  while (pyramidScan != pyramids.end()) {
    const PyramidBuilding& pyramid = *pyramidScan;
    if (pyramid.isInside(pos, radius))
      return &pyramid;
    pyramidScan++;
  }

  // check bases
  std::vector<BaseBuilding>::const_iterator baseScan = basesR.begin();
  while (baseScan != basesR.end()) {
    const BaseBuilding &base = *baseScan;
    if(base.isInside(pos, radius))
      return &base;
    baseScan++;
  }

  // check teleporters
  std::vector<Teleporter>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter& teleporter = *teleporterScan;
    if (teleporter.isInside(pos, radius))
      return &teleporter;
    teleporterScan++;
  }

  // nope
  return NULL;
}

const Obstacle*		World::hitBuilding(const float* pos, float angle,
						float dx, float dy) const
{
  // check walls
  std::vector<WallObstacle>::const_iterator wallScan = walls.begin();
  while (wallScan != walls.end()) {
    const WallObstacle& wall = *wallScan;
	if (!wall.isDriveThrough()){
    if (wall.isInside(pos, angle, dx, dy))
      return &wall;
	}
    wallScan++;
  }

  // check teleporters
  std::vector<Teleporter>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter& teleporter = *teleporterScan;
	if (!teleporter.isDriveThrough()){
    if (teleporter.isInside(pos, angle, dx, dy))
      return &teleporter;
	}
    teleporterScan++;
  }

  // strike one -- check boxes
  std::vector<BoxBuilding>::const_iterator boxScan = boxes.begin();
  while (boxScan != boxes.end()) {
    const BoxBuilding& box = *boxScan;
	if (!box.isDriveThrough()){
    if (box.isInside(pos, angle, dx, dy))
      return &box;
	}
    boxScan++;
  }

  // strike two -- check pyramids
  std::vector<PyramidBuilding>::const_iterator pyramidScan = pyramids.begin();
  while (pyramidScan != pyramids.end()) {
    const PyramidBuilding& pyramid = *pyramidScan;
	if (!pyramid.isDriveThrough()){
    if (pyramid.isInside(pos, angle, dx, dy))
      return &pyramid;
	}
    pyramidScan++;
  }

  // strike three -- check bases
  std::vector<BaseBuilding>::const_iterator baseScan = basesR.begin();
  while (baseScan != basesR.end()) {
    const BaseBuilding &base = *baseScan;
	if (!base.isDriveThrough()){
    if(base.isInside(pos, angle, dx, dy))
      return &base;
	}
    baseScan++;
  }
  // strike four -- you're out
  return NULL;
}

const Obstacle*		World::hitBuilding(const float* oldPos, float oldAngle,
					   const float* pos, float angle,
					   float dx, float dy) const
{
  // check walls
  std::vector<WallObstacle>::const_iterator wallScan = walls.begin();
  while (wallScan != walls.end()) {
    const WallObstacle& wall = *wallScan;
	if (!wall.isDriveThrough()){
    if (wall.isInside(pos, angle, dx, dy))
      return &wall;
	}
    wallScan++;
  }

  // check teleporters
  std::vector<Teleporter>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter& teleporter = *teleporterScan;
	if (!teleporter.isDriveThrough()){
    if (teleporter.isInside(pos, angle, dx, dy))
      return &teleporter;
	}
    teleporterScan++;
  }

  // strike one -- check boxes
  std::vector<BoxBuilding>::const_iterator boxScan = boxes.begin();
  while (boxScan != boxes.end()) {
    const BoxBuilding& box = *boxScan;
	if (!box.isDriveThrough()){
    if (box.isInside(oldPos, oldAngle, pos, angle, dx, dy))
      return &box;
	}
    boxScan++;
  }

  // strike two -- check pyramids
  std::vector<PyramidBuilding>::const_iterator pyramidScan = pyramids.begin();
  while (pyramidScan != pyramids.end()) {
    const PyramidBuilding& pyramid = *pyramidScan;
	if (!pyramid.isDriveThrough()){
    if (pyramid.isInside(pos, angle, dx, dy))
      return &pyramid;
	}
    pyramidScan++;
  }

  // strike three -- check bases
  std::vector<BaseBuilding>::const_iterator baseScan = basesR.begin();
  while (baseScan != basesR.end()) {
    const BaseBuilding &base = *baseScan;
	if (!base.isDriveThrough()){
    if(base.isInside(oldPos, oldAngle, pos, angle, dx, dy))
      return &base;
	}
    baseScan++;
  }
  // strike four -- you're out
  return NULL;
}

bool			World::crossingTeleporter(const float* pos,
					float angle, float dx, float dy,
					float* plane) const
{
  std::vector<Teleporter>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter& teleporter = *teleporterScan;
    if (teleporter.isCrossing(pos, angle, dx, dy, plane))
      return true;
    teleporterScan++;
  }
  return false;
}

const Teleporter*	World::crossesTeleporter(const float* oldPos,
						const float* newPos,
						int& face) const
{
  // check teleporters
  std::vector<Teleporter>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter& teleporter = *teleporterScan;
    if (teleporter.hasCrossed(oldPos, newPos, face))
      return &teleporter;
    teleporterScan++;
  }

  // didn't cross
  return NULL;
}

const Teleporter*	World::crossesTeleporter(const Ray& r, int& face) const
{
  // check teleporters
  std::vector<Teleporter>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter& teleporter = *teleporterScan;
    if (teleporter.isTeleported(r, face) > Epsilon)
      return &teleporter;
    teleporterScan++;
  }

  // didn't cross
  return NULL;
}

float			World::getProximity(const float* p, float r) const
{
  // get maximum over all teleporters
  float bestProximity = 0.0;
  std::vector<Teleporter>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
	const float proximity = teleporterScan->getProximity(p, r);
	if (proximity > bestProximity) bestProximity = proximity;
		teleporterScan++;
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
    const int numBoxes = boxes.size();
    for (int i = 0; i < numBoxes; i++)
      delete boxInsideNodes[i];
    delete[] boxInsideNodes;
    boxInsideNodes = NULL;
  }
  if (pyramidInsideNodes) {
    const int numPyramids = pyramids.size();
    for (int i = 0; i < numPyramids; i++)
      delete pyramidInsideNodes[i];
    delete[] pyramidInsideNodes;
    pyramidInsideNodes = NULL;
  }
  if (baseInsideNodes) {
    const int numBases = basesR.size();
    for(int i = 0; i < numBases; i++)
      delete baseInsideNodes[i];
    delete [] baseInsideNodes;
    baseInsideNodes = NULL;
  }
}

void			World::initFlag(int index)
{
  // set color of flag (opaque)
  const GLfloat* color = flags[index].type->getColor();
  flagNodes[index]->setColor(color[0], color[1], color[2], color[3]);

  // if coming or going then position warp
  Flag& flag = flags[index];
  if (flag.status == FlagComing || flag.status == FlagGoing) {
    GLfloat pos[3];
    pos[0] = flag.position[0];
    pos[1] = flag.position[1];
    pos[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	0.25f * BZDB.eval(StateDatabase::BZDB_GRAVITY) * flag.flightEnd) + flag.position[2];
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
					0.5f * BZDB.eval(StateDatabase::BZDB_GRAVITY) * flag.flightTime);
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
	    0.5f * BZDB.eval(StateDatabase::BZDB_GRAVITY) * flag.flightTime) + flag.landingPosition[2];
	alpha = 1.0f;
      }
      else {
	// hovering
	flag.position[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	    0.25f * BZDB.eval(StateDatabase::BZDB_GRAVITY) * flag.flightEnd) + flag.landingPosition[2];

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
	    0.5f * BZDB.eval(StateDatabase::BZDB_GRAVITY) * flag.flightTime) + flag.landingPosition[2];
	alpha = 1.0f;
      }
      else {
	// hovering
	flag.position[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	    0.25f * BZDB.eval(StateDatabase::BZDB_GRAVITY) * flag.flightEnd) + flag.landingPosition[2];

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
  if (flag.type == Flags::Narrow && flag.status == FlagOnTank) {
    for (int i = 0; i < curMaxPlayers; i++)
      if (players[i] && players[i]->getId() == flag.owner) {
	const float* dir = players[i]->getForward();
	flagNodes[index]->setBillboard(false);
	flagNodes[index]->turn(atan2f(dir[1], dir[0]));
	break;
      }
  }
  else {
    flagNodes[index]->setBillboard(true);
  }
}

void			World::addFlags(SceneDatabase* scene)
{
  if (!flagNodes) return;
  for (int i = 0; i < maxFlags; i++) {
    // if not showing flags, only allow FlagOnTank through
    if (flags[i].status != FlagOnTank && !BZDBCache::displayMainFlags) {
      continue;
    }

    if (flags[i].status == FlagNoExist) continue;
    // skip flag on a tank that isn't alive.  also skip Cloaking
    // flags on tanks.
    if (flags[i].status == FlagOnTank) {
      if (flags[i].type == Flags::Cloaking) continue;
      int j;
      for (j = 0; j < curMaxPlayers; j++)
	if (players[j] && players[j]->getId() == flags[i].owner)
	  break;

      if (j < curMaxPlayers && !(players[j]->getStatus() & PlayerState::Alive))
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

bool			World::writeWorld(std::string filename)
{
  std::ofstream out(filename.c_str());
  if (!out)
    return false;

  // Write bases
  {
    for (std::vector<BaseBuilding>::iterator it = basesR.begin(); it != basesR.end(); ++it) {
      BaseBuilding base = *it;
      out << "base" << std::endl;
      const float *pos = base.getPosition();
      out << "\tposition " << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;
      out << "\tsize " << base.getWidth() << " " << base.getBreadth() << " " << base.getHeight() << std::endl;
      out << "\trotation " << ((base.getRotation() * 180.0) / M_PI) << std::endl;
      out << "\tcolor " << base.getTeam() << std::endl;
      if (base.isDriveThrough()&&base.isShootThrough())
        out << "passable" << std::endl;
      else{
        if (base.isDriveThrough())
          out << "drivethrough" << std::endl;
        if (base.isShootThrough())
          out << "shootthrough" << std::endl;
      }

      out << "end" << std::endl;
      out << std::endl;
    }
  }

  // Write boxes

  {
    for (std::vector<BoxBuilding>::iterator it = boxes.begin(); it != boxes.end(); ++it) {
      BoxBuilding box = *it;
      out << "box" << std::endl;
      const float *pos = box.getPosition();
      out << "\tposition " << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;
      out << "\tsize " << box.getWidth() << " " << box.getBreadth() << " " << box.getHeight() << std::endl;
      out << "\trotation " << ((box.getRotation() * 180.0) / M_PI) << std::endl;
      if (box.isDriveThrough()&&box.isShootThrough())
        out << "passable" << std::endl;
      else{
        if (box.isDriveThrough())
	  out << "drivethrough" << std::endl;
        if (box.isShootThrough())
	  out << "shootthrough" << std::endl;
      }
      out << "end" << std::endl;
      out << std::endl;
    }
  }

  // Write pyramids

  {
    for (std::vector<PyramidBuilding>::iterator it = pyramids.begin(); it != pyramids.end(); ++it) {
      PyramidBuilding pyr = *it;
      out << "pyramid" << std::endl;
      const float *pos = pyr.getPosition();
      float z = pos[2];
      if (pyr.getZFlip())
        z = -z;
      out << "\tposition " << pos[0] << " " << pos[1] << " " << z << std::endl;
      out << "\tsize " << pyr.getWidth() << " " << pyr.getBreadth() << " " << pyr.getHeight() << std::endl;
      out << "\trotation " << ((pyr.getRotation() * 180.0) / M_PI) << std::endl;
      if (pyr.isDriveThrough()&&pyr.isShootThrough())
        out << "passable" << std::endl;
      else{
        if (pyr.isDriveThrough())
	  out << "drivethrough" << std::endl;
        if (pyr.isShootThrough())
	  out << "shootthrough" << std::endl;
      }
      out << "end" << std::endl;
      out << std::endl;
    }
  }

  // Write Teleporters

  {
    for (std::vector<Teleporter>::iterator it = teleporters.begin(); it != teleporters.end(); ++it) {
      Teleporter tele = *it;
      out << "teleporter" << std::endl;
      const float *pos = tele.getPosition();
      out << "\tposition " << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;
      out << "\tsize " << tele.getWidth() << " " << tele.getBreadth() << " " << tele.getHeight() << std::endl;
      out << "\trotation " << ((tele.getRotation() * 180.0) / M_PI) << std::endl;
      out << "\tborder " << tele.getBorder() << std::endl;
      out << "end" << std::endl;
      out << std::endl;
    }
  }

  // Write links

  {
    int from = 0;
    for (std::vector<int>::iterator it = teleportTargets.begin(); it != teleportTargets.end(); ++it, ++from) {
      int to = *it;
      out << "link" << std::endl;
      out << "\tfrom " << from << std::endl;
      out << "\tto " << to << std::endl;
      out << "end" << std::endl;
      out << std::endl;
    }
  }

  // Write World
  {
    float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    float flagHeight = BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT);
    if ((worldSize != atof(BZDB.getDefault(StateDatabase::BZDB_WORLDSIZE).c_str()))
    ||  (flagHeight != atof(BZDB.getDefault(StateDatabase::BZDB_FLAGHEIGHT).c_str())))
    {
      out << "world" << std::endl;
      if (worldSize != atof(BZDB.getDefault(StateDatabase::BZDB_WORLDSIZE).c_str())) {
	out << "\tsize " << worldSize / 2.0f << std::endl;
      }
      if (flagHeight != atof(BZDB.getDefault(StateDatabase::BZDB_FLAGHEIGHT).c_str())) {
	out << "\tflagHeight " << flagHeight << std::endl;
      }
      out << "end" << std::endl;
      out << std::endl;
    }
  }

  out.close();

  return true;
}


//
// WorldBuilder
//

WorldBuilder::WorldBuilder()
{
  world = new World;
  owned = true;
}

WorldBuilder::~WorldBuilder()
{
  if (owned) delete world;
}

void*			WorldBuilder::unpack(void* buf)
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
    world->flags[i].type = Flags::Null;
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

World*			WorldBuilder::getWorld()
{
  owned = false;
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
  world->walls.push_back(wall);
}

void			WorldBuilder::append(const BoxBuilding& box)
{
  world->boxes.push_back(box);
}

void			WorldBuilder::append(const PyramidBuilding& pyramid)
{
  world->pyramids.push_back(pyramid);
}

void			WorldBuilder::append(const BaseBuilding& base)
{
  world->basesR.push_back(base);
}

void			WorldBuilder::append(const Teleporter& teleporter)
{
  // save teleporter
  world->teleporters.push_back(teleporter);
}

void			WorldBuilder::setTeleporterTarget(int src, int tgt)
{
  if ((int)teleportTargets.size() < src+1)
    teleportTargets.resize(src+10);

  // record target in source entry
  teleportTargets[src] = tgt;
}

void			WorldBuilder::setBase(TeamColor team,
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

