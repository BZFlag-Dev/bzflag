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
#include "EighthDTetraSceneNode.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "MeshMaterial.h"
#include "FlagSceneNode.h"

/* compression library header */
#include "../zlib/zlib.h"


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
  if (serverMapVersion != mapVersion) {
    DEBUG1 ("WorldBuilder::unpack() bad map version\n");
    return NULL;
  }

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

  // decompress
  uint32_t compressedSize, uncompressedSize;
  buf = nboUnpackUInt (buf, uncompressedSize);
  buf = nboUnpackUInt (buf, compressedSize);
  uLongf destLen = uncompressedSize;
  char *uncompressedWorld = new char[destLen];
  char *compressedWorld = (char*) buf;

  if (uncompress ((Bytef*)uncompressedWorld, &destLen,
                  (Bytef*)compressedWorld, compressedSize) != Z_OK) {
    delete[] uncompressedWorld;
    DEBUG1 ("WorldBuilder::unpack() could not decompress\n");
    return NULL;
  }
  buf = uncompressedWorld;

  // unpack water level
  buf = nboUnpackFloat(buf, world->waterLevel);
  if (world->waterLevel >= 0.0f) {
    buf = world->waterMaterial.unpack(buf);
  }

  // unpack dynamic colors
  DYNCOLORMGR.clear();
  buf = DYNCOLORMGR.unpack(buf);

  // unpack texture matrices
  TEXMATRIXMGR.clear();
  buf = TEXMATRIXMGR.unpack(buf);

  // read geometry
  buf = nboUnpackUShort(buf, len);
  buf = nboUnpackUShort(buf, code);

  while (code != WorldCodeEnd) {
    switch (code) {

      case WorldCodeTetra: {

        // a good double check, but a bogus length
	if (len != WorldCodeTetraSize) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad tetra size\n");
	  return NULL;
        }

	TetraBuilding* tetra = new TetraBuilding;
	buf = tetra->unpack(buf);
        if (tetra->isValid()) {
	  world->tetras.push_back(tetra);
        } else {
          delete tetra;
        }
	break;
      }
      case WorldCodeMesh: {

        // a good double check, but a bogus length
	if (len != WorldCodeMeshSize) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad mesh size\n");
	  return NULL;
        }

	MeshObstacle* mesh = new MeshObstacle;
	buf = mesh->unpack(buf);
        if (mesh->isValid()) {
	  world->meshes.push_back(mesh);
        } else {
          delete mesh;
        }
	break;
      }
      case WorldCodeBox: {
	float data[7];
	unsigned char tempflags;

	if (len != WorldCodeBoxSize) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad box size\n");
	  return NULL;
        }

	memset(data, 0, sizeof(float) * 7);
	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	buf = nboUnpackUByte(buf, tempflags);
	BoxBuilding* box =
	  new BoxBuilding(data, data[3], data[4], data[5], data[6],
			  (tempflags & _DRIVE_THRU)!=0, (tempflags & _SHOOT_THRU)!=0);
        if (box->isValid()) {
	  world->boxes.push_back(box);
        } else {
          delete box;
        }
	break;
      }
      case WorldCodePyramid: {
	float data[7];
	unsigned char tempflags;

	if (len != WorldCodePyramidSize) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad pyramid size\n");
	  return NULL;
        }

	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	buf = nboUnpackUByte(buf, tempflags);

	PyramidBuilding* pyr =
	  new PyramidBuilding(data, data[3], data[4], data[5], data[6],
			      (tempflags & _DRIVE_THRU)!=0, (tempflags & _SHOOT_THRU)!=0);
	if (tempflags & _FLIP_Z) {
	  pyr->setZFlip();
        }
        if (pyr->isValid()) {
	  world->pyramids.push_back(pyr);
        } else {
          delete pyr;
        }
	break;
      }
      case WorldCodeTeleporter: {
	float data[8];
	unsigned char horizontal;
	unsigned char tempflags;

	if (len != WorldCodeTeleporterSize) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad teleporter size\n");
	  return NULL;
        }

	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	buf = nboUnpackFloat(buf, data[6]);
	buf = nboUnpackFloat(buf, data[7]);
	buf = nboUnpackUByte(buf, horizontal);
	buf = nboUnpackUByte(buf, tempflags);

	Teleporter* tele = 
	  new Teleporter(data, data[3], data[4], data[5], data[6],data[7],
	                 horizontal != 0, (tempflags & _DRIVE_THRU)!=0,
	                 (tempflags & _SHOOT_THRU)!=0);
        if (tele->isValid()) {
	  world->teleporters.push_back(tele);
        } else {
          delete tele;
        }
	break;
      }
      case WorldCodeLink: {
	uint16_t data[2];

	if (len != WorldCodeLinkSize) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad link size\n");
	  return NULL;
        }

	buf = nboUnpackUShort(buf, data[0]);
	buf = nboUnpackUShort(buf, data[1]);
	setTeleporterTarget(int(data[0]), int(data[1]));
	break;
      }
      case WorldCodeWall: {
	float data[6];

	if (len != WorldCodeWallSize) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad wall size\n");
	  return NULL;
        }

	buf = nboUnpackFloat(buf, data[0]);
	buf = nboUnpackFloat(buf, data[1]);
	buf = nboUnpackFloat(buf, data[2]);
	buf = nboUnpackFloat(buf, data[3]);
	buf = nboUnpackFloat(buf, data[4]);
	buf = nboUnpackFloat(buf, data[5]);
	WallObstacle* wall = new WallObstacle(data, data[3], data[4], data[5]);
        if (wall->isValid()) {
	  world->walls.push_back(wall);
        } else {
          delete wall;
        }
	break;
      }
      case WorldCodeBase: {
	uint16_t team;
	float data[10];

	if (len != WorldCodeBaseSize) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad base size\n");
	  return NULL;
        }

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
	BaseBuilding* base = new BaseBuilding(data, data[3], data +4, team);
        if (base->isValid()) {
	  world->basesR.push_back(base);
	  setBase(TeamColor(team), data, data[3], data[4], data[5], data[6]);
        } else {
          delete base;
        }
	break;
      }
      case WorldCodeWeapon: {
	Weapon weapon;
        uint16_t delays;

	buf = FlagType::unpack(buf, weapon.type);
	buf = nboUnpackFloat(buf, weapon.pos[0]);
	buf = nboUnpackFloat(buf, weapon.pos[1]);
	buf = nboUnpackFloat(buf, weapon.pos[2]);
	buf = nboUnpackFloat(buf, weapon.dir);
	buf = nboUnpackFloat(buf, weapon.initDelay);
	buf = nboUnpackUShort(buf, delays);

	uint16_t weapon_len = WorldCodeWeaponSize + (delays * sizeof(float));
	if (len != weapon_len) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad weapon size\n");
	  return NULL;
	}

	int i;
	for (i = 0; i < delays; i++) {
	  float delay;
	  buf = nboUnpackFloat(buf, delay);
	  weapon.delay.push_back(delay);
	}
        world->weapons.push_back(weapon);
	break;
      }
      case WorldCodeZone: {
        EntryZone zone;
        uint16_t flags, teams, safety;

	buf = nboUnpackFloat(buf, zone.pos[0]);
	buf = nboUnpackFloat(buf, zone.pos[1]);
	buf = nboUnpackFloat(buf, zone.pos[2]);
	buf = nboUnpackFloat(buf, zone.size[0]);
	buf = nboUnpackFloat(buf, zone.size[1]);
	buf = nboUnpackFloat(buf, zone.size[2]);
	buf = nboUnpackFloat(buf, zone.rot);
	buf = nboUnpackUShort(buf, flags);
	buf = nboUnpackUShort(buf, teams);
	buf = nboUnpackUShort(buf, safety);

        uint16_t zone_len = WorldCodeZoneSize;
        zone_len += FlagType::packSize * flags;
        zone_len += sizeof(uint16_t) * teams;
        zone_len += sizeof(uint16_t) * safety;
	if (len != zone_len) {
          delete[] uncompressedWorld;
          DEBUG1 ("WorldBuilder::unpack() bad zone size\n");
	  return NULL;
	}

        int i;
	for (i = 0; i < flags; i++) {
	  FlagType *type;
	  buf = FlagType::unpack (buf, type);
	  zone.flags.push_back(type);
	}
	for (i = 0; i < teams; i++) {
	  uint16_t team;
	  buf = nboUnpackUShort(buf, team);
	  zone.teams.push_back((TeamColor)team);
	}
	for (i = 0; i < safety; i++) {
	  uint16_t safety;
	  buf = nboUnpackUShort(buf, safety);
	  zone.safety.push_back((TeamColor)safety);
	}
	world->entryZones.push_back(zone);
	break;
      }
      default:
        delete[] uncompressedWorld;
        DEBUG1 ("WorldBuilder::unpack() unknown code\n");
	return NULL;
    }

    // switch back to the original buffer
    // for the WorldCodeEnd stuff
    if ((char*)buf >= (uncompressedWorld + uncompressedSize)) {
      buf = compressedWorld + compressedSize;
    }

    buf = nboUnpackUShort(buf, len);
    buf = nboUnpackUShort(buf, code);
  }

  delete[] uncompressedWorld;

  world->loadCollisionManager();

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
    const Obstacle& o = *world->boxes[i];
    obstacleSize[0] = o.getWidth();
    obstacleSize[1] = o.getBreadth();
    obstacleSize[2] = o.getHeight();
    world->boxInsideNodes[i] = new EighthDBoxSceneNode(o.getPosition(),
						       obstacleSize, o.getRotation());
  }
  const int numPyramids = world->pyramids.size();
  world->pyramidInsideNodes = new EighthDimSceneNode*[numPyramids];
  for (i = 0; i < numPyramids; i++) {
    const Obstacle& o = *world->pyramids[i];
    obstacleSize[0] = o.getWidth();
    obstacleSize[1] = o.getBreadth();
    obstacleSize[2] = o.getHeight();
    world->pyramidInsideNodes[i] = new EighthDPyrSceneNode(o.getPosition(),
							   obstacleSize, o.getRotation());
  }
  const int numTetras = world->tetras.size();
  world->tetraInsideNodes = new EighthDimSceneNode*[numTetras];
  for (i = 0; i < numTetras; i++) {
    const TetraBuilding& o = *world->tetras[i];
    world->tetraInsideNodes[i] = new EighthDTetraSceneNode(o.getVertices(),
                                                           o.getPlanes());
  }
  const int numBases = world->basesR.size();
  world->baseInsideNodes = new EighthDimSceneNode*[numBases];
  for (i = 0; i < numBases; i++) {
    const Obstacle& o = *world->basesR[i];
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

void WorldBuilder::setTeleporterTarget(int src, int tgt)
{
  if ((int)teleportTargets.size() < src+1)
    teleportTargets.resize(((src/2)+1)*2);

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

