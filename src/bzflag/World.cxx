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
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "BzMaterial.h"
#include "PhysicsDriver.h"
#include "FlagSceneNode.h"


//
// World
//

World*			World::playingField = NULL;
BundleMgr*		World::bundleMgr;
std::string		World::locale("");
int			World::flagTexture(-1);

World::World() :
  gameStyle(PlainGameStyle),
  waterLevel(-1.0f),
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
  waterMaterial = NULL;
  linkMaterial = NULL;
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
  // delete the obstacles
  unsigned int u;
  for (u = 0; u < walls.size(); u++) {
    delete walls[u];
  }
  for (u = 0; u < meshes.size(); u++) {
    if (!meshes[u]->getIsLocal()) {
      delete meshes[u];
    }
  }
  for (u = 0; u < arcs.size(); u++) {
    delete arcs[u];
  }
  for (u = 0; u < cones.size(); u++) {
    delete cones[u];
  }
  for (u = 0; u < spheres.size(); u++) {
    delete spheres[u];
  }
  for (u = 0; u < tetras.size(); u++) {
    delete tetras[u];
  }
  for (u = 0; u < boxes.size(); u++) {
    delete boxes[u];
  }
  for (u = 0; u < basesR.size(); u++) {
    delete basesR[u];
  }
  for (u = 0; u < pyramids.size(); u++) {
    delete pyramids[u];
  }
  for (u = 0; u < teleporters.size(); u++) {
    delete teleporters[u];
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
  collisionManager.load(meshes, boxes, basesR, pyramids, teleporters);
  return;
}

void                    World::checkCollisionManager()
{
  if (collisionManager.needReload()) {
    // reload the collision grid
    collisionManager.load(meshes, boxes, basesR, pyramids, teleporters);
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
    if (teleporter == teleporters[i])
      return 2 * i + face;
  return -1;
}

const Teleporter*	World::getTeleporter(int source, int& face) const
{
  assert(source >= 0 && source < (int)(2 * teleporters.size()));
  face = (source & 1);
  return teleporters[source / 2];
}

EighthDimSceneNode*	World::getInsideSceneNode(const Obstacle* o) const
{
  if (!o) return NULL;

  int i;
  const int numBases = basesR.size();
  for (i = 0; i < numBases; i++)
    if (basesR[i] == o)
      return baseInsideNodes[i];
  const int numBoxes = boxes.size();
  for (i = 0; i < numBoxes; i++)
    if (boxes[i] == o)
      return boxInsideNodes[i];
  const int numPyramids = pyramids.size();
  for (i = 0; i < numPyramids; i++)
    if (pyramids[i] == o)
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
  std::vector<WallObstacle*>::const_iterator wallScan = walls.begin();
  while (wallScan != walls.end()) {
    const WallObstacle& wall = **wallScan;
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


static int sortHitNormal(const void* a, const void* b)
{
  const MeshFace* faceA = *((const MeshFace**) a);
  const MeshFace* faceB = *((const MeshFace**) b);

  // Up Planes come first
  if (faceA->isUpPlane() && !faceB->isUpPlane()) {
    return -1;
  }
  if (faceB->isUpPlane() && !faceA->isUpPlane()) {
    return +1;
  }

  // highest Up Plane comes first
  if (faceA->isUpPlane() && faceB->isUpPlane()) {
    if (faceA->getPosition()[2] > faceB->getPosition()[2]) {
      return -1;
    } else {
      return +1;
    }
  }

  // compare the dot products
  if (faceA->scratchPad < faceB->scratchPad) {
    return -1;
  } else {
    return +1;
  }
}

const Obstacle*		World::hitBuilding(const float* oldPos, float oldAngle,
					   const float* pos, float angle,
					   float dx, float dy, float dz) const
{
  // check walls
  std::vector<WallObstacle*>::const_iterator wallScan = walls.begin();
  while (wallScan != walls.end()) {
    const WallObstacle& wall = **wallScan;
    if (wall.inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz)) {
      return &wall;
    }
    wallScan++;
  }

  float vel[3];
  vel[0] = pos[0] - oldPos[0];  
  vel[1] = pos[1] - oldPos[1];  
  vel[2] = pos[2] - oldPos[2];
  
  bool goingDown = false;
  if (vel[2] <= 0.0f) {
    goingDown = true;
  }

  // get the list of potential hits from the collision manager
  const ObsList* olist =
    collisionManager.movingBoxTest (oldPos, oldAngle, pos, angle, dx, dy, dz);
    
  // make a list of the actual hits, or return
  // immediately if a non-mesh obstacle intersects
  int hitCount = 0;
  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (!obs->isDriveThrough()
        && obs->inMovingBox(oldPos, oldAngle, pos, angle, dx, dy, dz)) {
      if (obs->getType() != MeshFace::getClassName()) {
        return obs;
      } else {
        const MeshFace* face = (const MeshFace*) obs;
        const float facePos2 = face->getPosition()[2];
        if (face->isUpPlane() && 
            (!goingDown || (oldPos[2] < (facePos2 - 1.0e-3f)))) {
          continue;
        }
        else if (face->isDownPlane() && ((oldPos[2] >= facePos2) || goingDown)) {
          continue;
        }
        else {
          // add the face to the hitlist
          olist->list[hitCount] = (Obstacle*) obs;
          hitCount++;
          // compute its dot product and stick it in the scratchPad
          const float* p = face->getPlane();
          const float dot = (vel[0] * p[0]) + (vel[1] * p[1]) + (vel[2] * p[2]);
          face->scratchPad = dot;
        }
      }
    }
  }

  // sort the list by type and dot product  
  qsort (olist->list, hitCount, sizeof(Obstacle*), sortHitNormal);
  
  if (hitCount > 0) {
    const MeshFace* face = (const MeshFace*) olist->list[0];
    if (face->isUpPlane() || (face->scratchPad < 0.0f)) {
//      printf ("pos: <%10.10f> [%10.10f, %10.10f]  ", /*, %10.10f <%10.10f>   ", oldPos[2], pos[2],*/
//              face->getPosition()[2], face->getPlane()[2], face->getPlane()[3]);
      if (face->isUpPlane()) {
//        printf ("UpPlane\n");
      } else {
//        printf ("Not UpPlane\n");
      }
      return face;
    }
  }

  return NULL;
}

bool			World::crossingTeleporter(const float* pos,
					float angle, float dx, float dy, float dz,
					float* plane) const
{
  std::vector<Teleporter*>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter* teleporter = *teleporterScan;
    if (teleporter->isCrossing(pos, angle, dx, dy, dz, plane))
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
  std::vector<Teleporter*>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter* teleporter = *teleporterScan;
    if (teleporter->hasCrossed(oldPos, newPos, face))
      return teleporter;
    teleporterScan++;
  }

  // didn't cross
  return NULL;
}

const Teleporter*	World::crossesTeleporter(const Ray& r, int& face) const
{
  // check teleporters
  std::vector<Teleporter*>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter* teleporter = *teleporterScan;
    if (teleporter->isTeleported(r, face) > Epsilon)
      return teleporter;
    teleporterScan++;
  }

  // didn't cross
  return NULL;
}

float			World::getProximity(const float* p, float r) const
{
  // get maximum over all teleporters
  float bestProximity = 0.0;
  std::vector<Teleporter*>::const_iterator teleporterScan = teleporters.begin();
  while (teleporterScan != teleporters.end()) {
    const Teleporter* teleporter = *teleporterScan;
    const float proximity = teleporter->getProximity(p, r);
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
  if (baseInsideNodes) {
    const int numBases = basesR.size();
    for(int i = 0; i < numBases; i++)
      delete baseInsideNodes[i];
    delete [] baseInsideNodes;
    baseInsideNodes = NULL;
  }
}


void		World::makeLinkMaterial()
{
  const std::string name = "LinkMaterial";
  
  linkMaterial = MATERIALMGR.findMaterial(name);
  if (linkMaterial != NULL) {
    return;
  }

  int dyncolId = DYNCOLORMGR.findColor(name);
  if (dyncolId < 0) {
    DynamicColor* dyncol = new DynamicColor;
    dyncol->setLimits(0, 0.0f, 0.1f); // red
    dyncol->setLimits(1, 0.0f, 0.1f); // green
    dyncol->setLimits(2, 0.0f, 0.1f); // blue
    dyncol->setLimits(3, 0.75f, 0.75f); // alpha
    // period, offset, weight
    float params[3] = {2.0f, 0.0f, 1.0f};
    params[1] = 0.0f * (params[0] / 3.0f); // red
    dyncol->addSinusoid(0, params);
    params[1] = 1.0f * (params[0] / 3.0f); // green
    dyncol->addSinusoid(1, params);
    params[1] = 2.0f * (params[0] / 3.0f); // blue
    dyncol->addSinusoid(2, params);
    dyncol->setName(name);
    dyncolId = DYNCOLORMGR.addColor (dyncol);  
  }
  
  int texmatId = TEXMATRIXMGR.findMatrix(name);
  if (texmatId < 0) {
    TextureMatrix* texmat = new TextureMatrix;
    texmat->setShiftParams(0.0f, -0.05f);
    texmat->setName(name);
    texmatId = TEXMATRIXMGR.addMatrix (texmat);
  }

  BzMaterial mat;
  const float color[4] = {0.0f, 0.0f, 0.0f, 0.5f};
  mat.setDiffuse(color);
  mat.setDynamicColor(dyncolId);
  mat.setTexture("telelink");
  mat.setTextureMatrix(texmatId);
  mat.setName(name);
  linkMaterial = MATERIALMGR.addMaterial(&mat);

  return;
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
	0.25f * BZDBCache::gravity * flag.flightEnd) + flag.position[2];
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
					0.5f * BZDBCache::gravity * flag.flightTime);
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
	    0.5f * BZDBCache::gravity * flag.flightTime) + flag.landingPosition[2];
	alpha = 1.0f;
      }
      else {
	// hovering
	flag.position[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	    0.25f * BZDBCache::gravity * flag.flightEnd) + flag.landingPosition[2];

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
	    0.5f * BZDBCache::gravity * flag.flightTime) + flag.landingPosition[2];
	alpha = 1.0f;
      }
      else {
	// hovering
	flag.position[2] = 0.5f * flag.flightEnd * (flag.initialVelocity +
	    0.25f * BZDBCache::gravity * flag.flightEnd) + flag.landingPosition[2];

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
    (*out) << "  -set " << name << " " << BZDB.get(name) << std::endl;
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
      out << "  -c" << std::endl;
      out << "  -mp 2,";
      for (int i = RedTeam; i <= PurpleTeam; i++) {
        if (getBase(i,0) != NULL)
          out << "2,";
        else
          out << "0,";
      }
      out << "2" << std::endl;
    }
    if (allowRabbit())
      out << "  -rabbit" << std::endl;
    if (allowJumping())
      out << "  -j" << std::endl;
    if (allShotsRicochet())
      out << "  +r" << std::endl;
    if (allowHandicap())
      out << "  -handicap" << std::endl;
    if (allowInertia()) {
      out << "  -a " << getLinearAcceleration() << " "
                     << getAngularAcceleration() << std::endl;
    }
    if (allowAntidote()) {
      out << "  -sa" << std::endl;
      out << "  -st " << getFlagShakeTimeout() << std::endl;
      out << "  -sw " << getFlagShakeWins() << std::endl;
    }

    out << "  -ms " << getMaxShots() << std::endl;

    // Write BZDB server variables that aren't defaults
    BZDB.iterate (writeBZDBvar, &out);

    out << "end" << std::endl << std::endl;
  }

  // Write World object
  {
    float worldSize = BZDBCache::worldSize;
    float flagHeight = BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT);
    if ((worldSize != atof(BZDB.getDefault(StateDatabase::BZDB_WORLDSIZE).c_str()))
    ||  (flagHeight != atof(BZDB.getDefault(StateDatabase::BZDB_FLAGHEIGHT).c_str())))
    {
      out << "world" << std::endl;
      if (worldSize != atof(BZDB.getDefault(StateDatabase::BZDB_WORLDSIZE).c_str())) {
	out << "  size " << worldSize / 2.0f << std::endl;
      }
      if (flagHeight != atof(BZDB.getDefault(StateDatabase::BZDB_FLAGHEIGHT).c_str())) {
	out << "  flagHeight " << flagHeight << std::endl;
      }
      out << "end" << std::endl << std::endl;
    }
  }

  // Write dynamic colors
  DYNCOLORMGR.print(out, 1);

  // Write texture matrices
  TEXMATRIXMGR.print(out, 1);

  // Write materials
  MATERIALMGR.print(out, 1);

  // Write materials
  PHYDRVMGR.print(out, 1);

  // Write water level
  {
    if (waterLevel >= 0.0f) {
      out << "waterLevel" << std::endl;
      out << "  height " << waterLevel << std::endl;
      out << "  refmat ";
      MATERIALMGR.printReference(out, waterMaterial);
      out << std::endl;
      out << "end" << std::endl << std::endl;
    }
  }

  // Write meshs
  {
    std::vector<MeshObstacle*>::iterator it;
    for (it = meshes.begin(); it != meshes.end(); ++it) {
      MeshObstacle* mesh = *it;
      mesh->print(out, 1);
    }
  }

  // Write arcs
  {
    std::vector<ArcObstacle*>::iterator it;
    for (it = arcs.begin(); it != arcs.end(); ++it) {
      ArcObstacle* arc = *it;
      arc->print(out, 1);
    }
  }

  // Write cones
  {
    std::vector<ConeObstacle*>::iterator it;
    for (it = cones.begin(); it != cones.end(); ++it) {
      ConeObstacle* cone = *it;
      cone->print(out, 1);
    }
  }

  // Write spheres
  {
    std::vector<SphereObstacle*>::iterator it;
    for (it = spheres.begin(); it != spheres.end(); ++it) {
      SphereObstacle* sphere = *it;
      sphere->print(out, 1);
    }
  }

  // Write tetras
  {
    std::vector<TetraBuilding*>::iterator it;
    for (it = tetras.begin(); it != tetras.end(); ++it) {
      TetraBuilding* tetra = *it;
      tetra->print(out, 1);
    }
  }

  // Write bases
  {
    for (std::vector<BaseBuilding*>::iterator it = basesR.begin(); it != basesR.end(); ++it) {
      BaseBuilding base = **it;
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
    for (std::vector<BoxBuilding*>::iterator it = boxes.begin(); it != boxes.end(); ++it) {
      BoxBuilding box = **it;
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
    for (std::vector<PyramidBuilding*>::iterator it = pyramids.begin();
	 it != pyramids.end(); ++it) {
      PyramidBuilding pyr = **it;
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

  // Write Teleporters
  {
    for (std::vector<Teleporter*>::iterator it = teleporters.begin(); it != teleporters.end(); ++it) {
      Teleporter tele = **it;
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
  GLboolean usingTextures;
  
  glGetBooleanv (GL_TEXTURE_2D, &usingTextures);
  glDisable (GL_TEXTURE_2D);
  
  glColor4fv (color);
  collisionManager.draw (drawLines);
  
  if (usingTextures) {
    glEnable (GL_TEXTURE_2D);
  }
  
  return;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
