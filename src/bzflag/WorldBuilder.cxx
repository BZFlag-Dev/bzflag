/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "WorldBuilder.h"

/* common implementation headers */
#include "TextUtils.h"
#include "Protocol.h"
#include "DynamicColor.h"
#include "TextureMatrix.h"
#include "PhysicsDriver.h"
#include "FlagSceneNode.h"
#include "ObstacleMgr.h"
#include "BaseBuilding.h"
#include "MeshObstacle.h"
#include "MeshFace.h"
#include "BzDocket.h"
#include "BzVFS.h"

/* compression library header */
#include "zlib.h"


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
  TimeKeeper start = TimeKeeper::getCurrent();

  // unpack world database from network transfer
  // read style header
  uint16_t code, len;
  buf = nboUnpackUInt16(buf, len);
  buf = nboUnpackUInt16(buf, code);
  if (code != WorldCodeHeader) return NULL;

  // read style
  uint16_t serverMapVersion;
  buf = nboUnpackUInt16(buf, serverMapVersion);
  if (serverMapVersion != mapVersion) {
    logDebugMessage(1,"WorldBuilder::unpack() bad map version\n");
    return NULL;
  }

  // decompress
  uint32_t compressedSize, uncompressedSize;
  buf = nboUnpackUInt32 (buf, uncompressedSize);
  buf = nboUnpackUInt32 (buf, compressedSize);
  uLongf destLen = uncompressedSize;
  char *uncompressedWorld = new char[destLen];
  char *compressedWorld = (char*) buf;

  if (uncompress ((Bytef*)uncompressedWorld, &destLen,
		  (Bytef*)compressedWorld, compressedSize) != Z_OK) {
    delete[] uncompressedWorld;
    logDebugMessage(1,"WorldBuilder::unpack() could not decompress\n");
    return NULL;
  }
  char* uncompressedEnd = uncompressedWorld + uncompressedSize;

  buf = uncompressedWorld;

  // setup the buffer overrun checking
  nboUseErrorChecking(true);
  nboSetBufferLength(uncompressedSize);
  nboClearBufferError();

  // unpack the map information
  world->mapInfo.clear();
  buf = world->mapInfo.unpack(buf);

  // unpack dynamic colors
  DYNCOLORMGR.clear();
  buf = DYNCOLORMGR.unpack(buf);

  // unpack texture matrices
  TEXMATRIXMGR.clear();
  buf = TEXMATRIXMGR.unpack(buf);

  // unpack materials
  MATERIALMGR.clear();
  buf = MATERIALMGR.unpack(buf);

  // unpack physics drivers
  PHYDRVMGR.clear();
  buf = PHYDRVMGR.unpack(buf);

  // unpack obstacle transforms
  TRANSFORMMGR.clear();
  buf = TRANSFORMMGR.unpack(buf);

  // unpack the obstacles
  OBSTACLEMGR.clear();
  buf = OBSTACLEMGR.unpack(buf);

  // unpack water level
  buf = nboUnpackFloat(buf, world->waterLevel);
  if (world->waterLevel >= 0.0f) {
    int32_t matindex;
    buf = nboUnpackInt32(buf, matindex);
    world->waterMaterial = MATERIALMGR.getMaterial(matindex);
  }

  uint32_t i, count;

  // unpack the weapons
  buf = nboUnpackUInt32(buf, count);
  for (i = 0; i < count; i++) {
    Weapon weapon;
    buf = weapon.unpack(buf);
    world->weapons.push_back(weapon);
  }

  // unpack the entry zones
  buf = nboUnpackUInt32(buf, count);
  for (i = 0; i < count; i++) {
    EntryZone zone;
    buf = zone.unpack(buf);
    world->entryZones.push_back(zone);
  }

  // unpack the LuaWorld docket
  BzDocket* docket = new BzDocket("LuaWorld");
  buf = docket->unpack(buf);
  bzVFS.removeFS(BZVFS_LUA_WORLD);
  bzVFS.addFS(BZVFS_LUA_WORLD, docket);

  // check if the unpacking was successful
  nboUseErrorChecking(false);
  if (nboGetBufferError()) {
    delete[] uncompressedWorld;
    logDebugMessage(1,"WorldBuilder::unpack() overrun\n");
    return NULL;
  }
  if ((char*)buf != uncompressedEnd) {
    delete[] uncompressedWorld;
    logDebugMessage(1,"WorldBuilder::unpack() ending mismatch (%i)\n",
	    (char*)buf - uncompressedEnd);
    return NULL;
  }

  // switch back to the original buffer
  buf = compressedWorld + compressedSize;
  buf = nboUnpackUInt16(buf, len);
  buf = nboUnpackUInt16(buf, code);
  if ((code != WorldCodeEnd) || (len != WorldCodeEndSize)) {
    delete[] uncompressedWorld;
    logDebugMessage(1,"WorldBuilder::unpack() bad ending\n");
    return NULL;
  }

  // delete the buffer
  delete[] uncompressedWorld;

  // build the world obstacles
  OBSTACLEMGR.makeWorld();

  // make the team bases
  if (world->gameType != ClassicCTF) {
    OBSTACLEMGR.replaceBasesWithBoxes();
  }
  else {
    const ObstacleList& bases = OBSTACLEMGR.getBases();
    for (i = 0; i < bases.size(); i++) {
      const BaseBuilding* base = (const BaseBuilding*) bases[i];
      setBase(base);
    }
    const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
    for (i = 0; i < meshes.size(); i++) {
      const MeshObstacle* mesh = (const MeshObstacle*) meshes[i];
      if (!mesh->getHasSpecialFaces()) {
        continue;
      }
      const int faceCount = mesh->getFaceCount();
      for (int f = 0; f < faceCount; f++) {
        const MeshFace* face = mesh->getFace(f);
        if (face->isBaseFace()) {
          setBase(face);
        }
      }
    }
  }

  world->makeMeshDrawMgrs();

  // NOTE: relying on checkCollisionManager() to do the first loading
  //       of ColiisionManager, because the BZDB variables come in later,
  //       and would cause a double loading if we did it now.

  if (debugLevel >= 3) {
    TimeKeeper end = TimeKeeper::getCurrent();
    const float elapsed = (float)(end - start);
    logDebugMessage(0,"WorldBuilder::unpack() processed in %f seconds.\n", elapsed);
  }

  return buf;
}


void* WorldBuilder::unpackGameSettings(void* buf)
{
  // read style
  uint16_t gameType, gameOptions, maxPlayers, maxShots, maxFlags;

  float worldSize;
  buf = nboUnpackFloat(buf, worldSize);
  BZDB.setFloat(StateDatabase::BZDB_WORLDSIZE, worldSize);
  buf = nboUnpackUInt16(buf, gameType);
  setGameType(short(gameType));
  buf = nboUnpackUInt16(buf, gameOptions);
  setGameOptions(short(gameOptions));
  buf = nboUnpackUInt16(buf, maxPlayers);
  setMaxPlayers(int(maxPlayers));
  buf = nboUnpackUInt16(buf, maxShots);
  setMaxShots(int(maxShots));
  buf = nboUnpackUInt16(buf, maxFlags);
  setMaxFlags(int(maxFlags));
  uint16_t shakeTimeout = 0, shakeWins;
  buf = nboUnpackUInt16(buf, shakeTimeout);
  setShakeTimeout(0.1f * float(shakeTimeout));
  buf = nboUnpackUInt16(buf, shakeWins);
  setShakeWins(shakeWins);

  return buf;
}

void WorldBuilder::preGetWorld()
{
  // prepare players array
  if (world->players) {
    delete[] world->players;
  }
  // FIXME
  // world->maxPlayers do not work as bzfs uses more player slot than
  // real players. Any tcp connection is assigned a slot.
  // So I put now 216. We should fix it though.
  const int maxPlayers = 216;
  int i;
  world->setPlayersSize(maxPlayers);

  // prepare flags array
  world->freeFlags();
  world->flags = new ClientFlag[world->maxFlags];
  world->flagNodes = new FlagSceneNode*[world->maxFlags];
  world->flagWarpNodes = new FlagWarpSceneNode*[world->maxFlags];
  for (i = 0; i < world->maxFlags; i++) {
    world->flags[i].id = i;
    world->flags[i].gfxBlock.init(GfxBlock::Flag, i, true);
    world->flags[i].type = Flags::Null;
    world->flags[i].status = FlagNoExist;
    world->flags[i].position.x = 0.0f;
    world->flags[i].position.y = 0.0f;
    world->flags[i].position.z = 0.0f;
    world->flagNodes[i] = new FlagSceneNode(world->flags[i].position);
    world->flagWarpNodes[i] = new FlagWarpSceneNode(world->flags[i].position);
    world->flagNodes[i]->setTexture(World::flagTexture);
  }

  return;
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

void WorldBuilder::setGameType(short gameType)
{
  world->gameType = (GameType)gameType;
}

void WorldBuilder::setGameOptions(short gameOptions)
{
  world->gameOptions = gameOptions;
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

void WorldBuilder::setBase(const Obstacle* base)
{
  world->bases[base->getBaseTeam()].push_back(base);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
