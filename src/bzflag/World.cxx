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
#include "World.h"

/* system implementation headers */
#include <fstream>
#include <time.h>
#include <vector>
#include <string>

/* common implementation headers */
#include "BZDBCache.h"
#include "TextureManager.h"
#include "FileManager.h"


//
// World
//

World*			World::playingField = NULL;
BundleMgr*		World::bundleMgr;
std::string		World::locale("");
int			World::flagTexture(-1);

World::World() : 
  gameStyle(PlainGameStyle),
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
  tetraInsideNodes(NULL),
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
  flagTexture = tm.getTextureID( "flag" );
}

void			World::done()
{
  flagTexture = 0;
}

void                    World::loadCollisionManager()
{
  collisionManager.load(boxes, basesR, pyramids, tetras, teleporters);
  return;
}

void                    World::checkCollisionManager()
{
  if (collisionManager.needReload()) {
    // reload the collision grid
    collisionManager.load(boxes, basesR, pyramids, tetras, teleporters);
  }
  return;
}

void			World::setFlagTexture(FlagSceneNode* flag)
{
  flag->setTexture(flagTexture);
}

void			World::setWorld(World* _playingField)
{
  playingField = _playingField;
}

int			World::getTeleportTarget(int source) const
{
  assert(source >= 0 && source < (int)(2 * teleporters.size()));
  int target = teleportTargets[source];
  if ((target != randomTeleporter) && // random tag
      (target >= (int)(2 * teleporters.size()))) {
    // the other side of the teleporter
    target = ((source / 2) * 2) + (1 - (source % 2));
  }
  return target;
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
  const int numTetras = tetras.size();
  for (i = 0; i < numTetras; i++)
    if (&(tetras[i]) == o)
      return tetraInsideNodes[i];
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
      float rx = (float) (cosf(atanf(ny / nx) - it->p[3]) * sqrt((ny * ny) + (nx * nx)));
      float ry = (float) (sinf(atanf(ny / nx) - it->p[3]) * sqrt((ny * ny) + (nx * nx)));
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

const Obstacle*		World::inBuilding(const float* pos, 
                                          float radius, float height) const
{
  // check everything but walls
  const ObsList* olist = collisionManager.cylinderTest (pos, radius, height);
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->inCylinder(pos, radius, height)) {
      return obs;
    }
  }

  return NULL;
}

const Obstacle*		World::hitBuilding(const float* pos, float angle,
                                           float dx, float dy, float dz) const
{
  // check walls
  std::vector<WallObstacle>::const_iterator wallScan = walls.begin();
  while (wallScan != walls.end()) {
    const WallObstacle& wall = *wallScan;
    if (wall.inBox(pos, angle, dx, dy, dz)) {
      return &wall;
    }
    wallScan++;
  }

  // check everything else
  const ObsList* olist = collisionManager.boxTest (pos, angle, dx, dy, dz);
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (!obs->isDriveThrough() && obs->inBox(pos, angle, dx, dy, dz)) {
      return obs;
    }
  }

  return NULL;
}

const Obstacle*		World::hitBuilding(const float* oldPos, float oldAngle,
					   const float* pos, float angle,
					   float dx, float dy, float dz) const
{
  // check walls
  std::vector<WallObstacle>::const_iterator wallScan = walls.begin();
  while (wallScan != walls.end()) {
    const WallObstacle& wall = *wallScan;
    if (wall.inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz)) {
      return &wall;
    }
    wallScan++;
  }

  // check everything else
  const ObsList* olist = 
    collisionManager.movingBoxTest (oldPos, oldAngle, pos, angle, dx, dy, dz);
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (!obs->isDriveThrough()
        && obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz)) {
      return obs;
    }
  }

  return NULL;
}

bool			World::crossingTeleporter(const float* pos,
					float angle, float dx, float dy, float dz,
					float* plane) const
{
  std::vector<Teleporter>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter& teleporter = *teleporterScan;
    if (teleporter.isCrossing(pos, angle, dx, dy, dz, plane))
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
    if (proximity > bestProximity)
      bestProximity = proximity;
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
  if (tetraInsideNodes) {
    const int numTetras = tetras.size();
    for (int i = 0; i < numTetras; i++)
      delete tetraInsideNodes[i];
    delete[] tetraInsideNodes;
    tetraInsideNodes = NULL;
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
  const float* color = flags[index].type->getColor();
  flagNodes[index]->setColor(color[0], color[1], color[2]);

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


static void writeBZDBvar (const std::string& name, void *userData)
{
  std::ofstream *out = (std::ofstream *)userData;
  if ((BZDB.getPermission(name) == StateDatabase::Server)
      && (BZDB.get(name) != BZDB.getDefault(name))
      && (name != "poll")) {
    (*out) << "\t-set " << name << " " << BZDB.get(name) << std::endl;
  }
  return;
}


bool			World::writeWorld(std::string filename)
{
  std::ostream *stream = FILEMGR.createDataOutStream(filename.c_str());
  if (stream == NULL)
    return false;

  // for notational convenience    
  std::ostream& out = *stream;
  
  time_t nowTime = time (NULL);
  out << "# BZFlag client: saved world on " << ctime(&nowTime) << std::endl;

  // Write the Server Options    
  {
    out << "options" << std::endl;
    
    // FIXME - would be nice to get a few other thing
    //         -fb, -sb, rabbit style, a real -mp, etc... (also, flags?)

    if (allowTeamFlags()) {
      out << "\t-c" << std::endl;
      out << "\t-mp 2,";
      for (int i = RedTeam; i <= PurpleTeam; i++) {
        if (getBase(i,0) != NULL)
          out << "2,";
        else
          out << "0,";
      }
      out << "2" << std::endl;
    }
    if (allowRabbit())
      out << "\t-rabbit" << std::endl;
    if (allowJumping())
      out << "\t-j" << std::endl;
    if (allShotsRicochet())
      out << "\t+r" << std::endl;
    if (allowHandicap())
      out << "\t-handicap" << std::endl;
    if (allowInertia()) {
      out << "\t-a " << getLinearAcceleration() << " " 
                     << getAngularAcceleration() << std::endl;
    }
    if (allowAntidote()) {
      out << "\t-sa" << std::endl;
      out << "\t-st " << getFlagShakeTimeout() << std::endl;
      out << "\t-sw " << getFlagShakeWins() << std::endl;
    }

    out << "\t-ms " << getMaxShots() << std::endl;
    
    // Write BZDB server variables that aren't defaults
    BZDB.iterate (writeBZDBvar, &out);

    out << "end" << std::endl << std::endl;
  }

  // Write World object
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
      out << "end" << std::endl << std::endl;
    }
  }

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
      if (base.isDriveThrough() && base.isShootThrough())
        out << "\tpassable" << std::endl;
      else{
        if (base.isDriveThrough())
          out << "\tdrivethrough" << std::endl;
        if (base.isShootThrough())
          out << "\tshootthrough" << std::endl;
      }

      out << "end" << std::endl << std::endl;
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
      if (box.isDriveThrough() && box.isShootThrough())
        out << "\tpassable" << std::endl;
      else{
        if (box.isDriveThrough())
	  out << "\tdrivethrough" << std::endl;
        if (box.isShootThrough())
	  out << "\tshootthrough" << std::endl;
      }
      out << "end" << std::endl << std::endl;
    }
  }

  // Write pyramids
  {
    for (std::vector<PyramidBuilding>::iterator it = pyramids.begin();
	 it != pyramids.end(); ++it) {
      PyramidBuilding pyr = *it;
      out << "pyramid" << std::endl;
      const float *pos = pyr.getPosition();
      float height = pyr.getHeight();
      if (pyr.getZFlip())
        height = -height;
      out << "\tposition " << pos[0] << " " << pos[1] << " " << pos[2]
	  << std::endl;
      out << "\tsize " << pyr.getWidth() << " " << pyr.getBreadth() << " "
	  << height << std::endl;
      out << "\trotation " << ((pyr.getRotation() * 180.0) / M_PI)
	  << std::endl;
      if (pyr.isDriveThrough()&&pyr.isShootThrough())
        out << "\tpassable" << std::endl;
      else{
        if (pyr.isDriveThrough())
	  out << "\tdrivethrough" << std::endl;
        if (pyr.isShootThrough())
	  out << "\tshootthrough" << std::endl;
      }
      out << "end" << std::endl << std::endl;
    }
  }

  // Write tetrahedrons
  {
    for (std::vector<TetraBuilding>::iterator it = tetras.begin();
	 it != tetras.end(); ++it) {
      TetraBuilding tetra  = *it;
      out << "tetra" << std::endl;
      // write the vertices
      for (int v = 0; v < 4; v++) {
        const float* vertex = tetra.getVertex(v);
        out << "\tvertex " << vertex[0] << " " << vertex[1] << " " << vertex[2] << std::endl;
        if (tetra.isColoredPlane (v)) {
          const float* color = tetra.getPlaneColor(v);
          unsigned int bytecolor[4];
          for (int c = 0; c < 4; c++) {
            bytecolor[c] = (unsigned char)(color[c] * 255.5f);
          }
          out << "\tcolor " << bytecolor[0] << " " << bytecolor[1] << " "
                            << bytecolor[2] << " " << bytecolor[3] << std::endl;
        }
      }
      // write the plane visibility
      int p;
      bool allVisible = true;
      for (p = 0; p < 4; p++) {
        if (!tetra.isVisiblePlane(p)) {
          allVisible = false;
          break;
        }
      }
      if (!allVisible) {
        out << "\tvisible";
        for (p = 0; p < 4; p++) {
          if (tetra.isVisiblePlane(p)) {
            out << " 1";
          } else {
            out << " 0";
          }
        }
        out << std::endl;
      }
      // write the regular stuff
      if (tetra.isDriveThrough() && tetra.isShootThrough())
        out << "\tpassable" << std::endl;
      else{
        if (tetra.isDriveThrough())
	  out << "\tdrivethrough" << std::endl;
        if (tetra.isShootThrough())
	  out << "\tshootthrough" << std::endl;
      }
      out << "end" << std::endl << std::endl;
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
      out << "end" << std::endl << std::endl;
    }
  }

  // Write links
  {
    int from = 0;
    for (std::vector<int>::iterator it = teleportTargets.begin(); it != teleportTargets.end(); ++it, ++from) {
      int to = *it;
      out << "link" << std::endl;
      out << "\tfrom " << from << std::endl;
      if (to == randomTeleporter) {
        out << "\tto random" << std::endl;
      }
      else {
        out << "\tto " << to << std::endl;
      }
      out << "end" << std::endl << std::endl;
    }
  }

  // Write weapons
  {
    for (std::vector<Weapon>::iterator it = weapons.begin(); it != weapons.end(); ++it) {
      Weapon weapon = *it;
      out << "weapon" << std::endl;
      if (weapon.type != Flags::Null) {
        out << "\ttype " << weapon.type->flagAbbv << std::endl;
      }
      out << "\tposition " << weapon.pos[0] << " " << weapon.pos[1] << " " << weapon.pos[2] << std::endl;
      out << "\trotation " << ((weapon.dir * 180.0) / M_PI) << std::endl;
      out << "\tinitdelay " << weapon.initDelay << std::endl;
      if (weapon.delay.size() > 0) {
        out << "\tdelay";
        for (std::vector<float>::iterator dit = weapon.delay.begin(); dit != weapon.delay.end(); ++dit) {
          out << " " << (float)*dit;
        }
        out << std::endl;
      }
      out << "end" << std::endl << std::endl;
    }
  }

  // Write entry zones
  {
    for (std::vector<EntryZone>::iterator it = entryZones.begin(); it != entryZones.end(); ++it) {
      EntryZone zone = *it;
      out << "zone" << std::endl;
      out << "\tposition " << zone.pos[0] << " " << zone.pos[1] << " " << zone.pos[2] << std::endl;
      out << "\tsize " << zone.size[0] << " " << zone.size[1] << " " << zone.size[2] << std::endl;
      out << "\trotation " << ((zone.rot * 180.0) / M_PI) << std::endl;
      if (zone.flags.size() > 0) {
        out << "\tflag";
        std::vector<FlagType*>::iterator fit;
        for (fit = zone.flags.begin(); fit != zone.flags.end(); ++fit) {
          out << " " << (*fit)->flagAbbv;
        }
        out << std::endl;
      }
      if (zone.teams.size() > 0) {
        out << "\tteam";
        std::vector<TeamColor>::iterator tit;
        for (tit = zone.teams.begin(); tit != zone.teams.end(); ++tit) {
          out << " " << (*tit);
        }
        out << std::endl;
      }
      if (zone.safety.size() > 0) {
        out << "\tsafety";
        std::vector<TeamColor>::iterator sit;
        for (sit = zone.safety.begin(); sit != zone.safety.end(); ++sit) {
          out << " " << (*sit);
        }
        out << std::endl;
      }
      out << "end" << std::endl << std::endl;
    }
  }

  delete stream;

  return true;
}

static void drawLines (int count, const float (*vertices)[3])
{
  glBegin (GL_LINE_STRIP);
  for (int i = 0; i < count; i++) {
    glVertex3fv (vertices[i]);
  }
  glEnd ();
  return;
}

void			World::drawCollisionGrid()
{
  float color[4] = { 0.25f, 0.0f, 0.25f, 0.8f };
  glDisable (GL_TEXTURE_2D);
  glColor4fv (color);
  collisionManager.draw (drawLines);
  glEnable (GL_TEXTURE_2D);
  return;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
