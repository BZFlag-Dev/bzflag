/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _WIN32
#pragma warning(4: 4786)
#endif

#include <string>
#include "BZDBCache.h"

bool  BZDBCache::displayMainFlags;
bool  BZDBCache::enhancedRadar;


float BZDBCache::maxLOD;
float BZDBCache::tankHeight;
float BZDBCache::flagRadius;

void BZDBCache::init()
{
  BZDB->addCallback("displayMainFlags", clientCallback, NULL);
  BZDB->addCallback("enhancedradar", clientCallback, NULL);

  BZDB->addCallback(StateDatabase::BZDB_MAXLOD, serverCallback, NULL);
  BZDB->addCallback(StateDatabase::BZDB_TANKHEIGHT, serverCallback, NULL);
  BZDB->addCallback(StateDatabase::BZDB_FLAGRADIUS, serverCallback, NULL);
}

void BZDBCache::clientCallback(const std::string& name, void *)
{
  if (name == "displayMainFlags")
    displayMainFlags = BZDB->isTrue("displayMainFlags");
  else if (name == "enhancedradar")
    enhancedRadar = BZDB->isTrue("enhancedradar");
}

void BZDBCache::serverCallback(const std::string& name, void *)
{
  if (name == StateDatabase::BZDB_MAXLOD)
    maxLOD = BZDB->eval(StateDatabase::BZDB_MAXLOD);
  else if (name == StateDatabase::BZDB_TANKHEIGHT)
    tankHeight = BZDB->eval(StateDatabase::BZDB_TANKHEIGHT);
  else if (name == StateDatabase::BZDB_FLAGRADIUS)
    flagRadius = BZDB->eval(StateDatabase::BZDB_FLAGRADIUS);
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
