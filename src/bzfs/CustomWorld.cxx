/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "CustomWorld.h"

/* system implementation headers */
#include <string.h>

/* common implementation headers */
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "TextUtils.h"


CustomWorld::CustomWorld()
{
  createWalls = true;
  // initialize with database defaults
  _size = BZDBCache::worldSize;
  _fHeight = BZDB.eval(StateDatabase::BZDB_FLAGHEIGHT);
}


bool CustomWorld::read(const char *cmd, std::istream& input)
{
  BZDB.setSaveDefault(true);
  if (strcasecmp(cmd, "size") == 0) {
    input >> _size;
    _size *= 2.0;
    BZDB.set(StateDatabase::BZDB_WORLDSIZE, TextUtils::format("%f", _size));
  } else if (strcasecmp(cmd, "flagHeight") == 0) {
    input >> _fHeight;
    BZDB.set(StateDatabase::BZDB_FLAGHEIGHT, TextUtils::format("%f", _fHeight));
  } else if (strcasecmp(cmd, "noWalls") == 0) {
    createWalls = false;
  } else if (strcasecmp(cmd, "freeCtfSpawns") == 0) {
    BZDB.setBool("freeCtfSpawns", true);
  } else {
    return WorldFileObject::read(cmd, input);
  }
  BZDB.setSaveDefault(false);
  return true;
}


void CustomWorld::writeToWorld(WorldInfo* world) const
{
  if (createWalls) {
    float wallHeight = BZDB.eval(StateDatabase::BZDB_WALLHEIGHT);
    float worldSize = BZDBCache::worldSize * 0.5f;
    world->addWall(0.0f, worldSize, 0.0f, 1.5f*M_PI, worldSize, wallHeight);
    world->addWall(worldSize, 0.0f, 0.0f, M_PI, worldSize, wallHeight);
    world->addWall(0.0f, -worldSize, 0.0f, 0.5f*M_PI, worldSize, wallHeight);
    world->addWall(-worldSize, 0.0f, 0.0f, 0.0f, worldSize, wallHeight);
  }
}

std::map<std::string,bz_CustomMapObjectHandler*>	customObjectMap;

void registerCustomMapObject ( const char* object, bz_CustomMapObjectHandler *handler )
{
	std::string objectName = object;

	customObjectMap[TextUtils::toupper(objectName)] = handler;
}

void removeCustomMapObject ( const char* object )
{
	std::string objectName = object;

	if ( customObjectMap.find(TextUtils::toupper(objectName)) != customObjectMap.end() )
		customObjectMap.erase(customObjectMap.find(TextUtils::toupper(objectName)));
}

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
