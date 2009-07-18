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
  _fHeight = BZDB.eval(BZDBNAMES.FLAGHEIGHT);
}


bool CustomWorld::read(const char *cmd, std::istream& input)
{
  BZDB.setSaveDefault(true);
  if (strcasecmp(cmd, "size") == 0) {
    input >> _size;
    _size *= 2.0;
    BZDB.set(BZDBNAMES.WORLDSIZE, TextUtils::format("%f", _size));
  } else if (strcasecmp(cmd, "flagHeight") == 0) {
    input >> _fHeight;
    BZDB.set(BZDBNAMES.FLAGHEIGHT, TextUtils::format("%f", _fHeight));
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
    float wallHeight = BZDB.eval(BZDBNAMES.WALLHEIGHT);
    float ws = BZDBCache::worldSize * 0.5f;
    world->addWall(0.0f, +ws,  0.0f, 1.5f * (float)M_PI, ws, wallHeight);
    world->addWall(+ws,  0.0f, 0.0f,        (float)M_PI, ws, wallHeight);
    world->addWall(0.0f, -ws,  0.0f, 0.5f * (float)M_PI, ws, wallHeight);
    world->addWall(-ws,  0.0f, 0.0f, 0.0f,               ws, wallHeight);
  }
}


//============================================================================//

std::map<std::string, CustomObjectMapData> customObjectMap;


bool registerCustomMapObject(const char* object, const char* end,
                             bz_CustomMapObjectHandler* handler)
{
  const std::string objectName = TextUtils::toupper(object);
  std::string endToken = "END";
  if (end != NULL) {
    endToken = TextUtils::toupper(end);
  }

  if (customObjectMap.find(objectName) != customObjectMap.end()) {
    return false;
  }

  customObjectMap[objectName] = CustomObjectMapData(handler, endToken);

  return true;
}


bool removeCustomMapObject(const char* object)
{
  std::string objectName = TextUtils::toupper(object);

  CustomObjectMap::iterator itr =  customObjectMap.find(objectName);

  if (itr != customObjectMap.end()) {
    customObjectMap.erase(itr);
  } else {
    return false;
  }

  return true;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
