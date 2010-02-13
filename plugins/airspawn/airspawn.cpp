/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// airspawn.cpp : Defines the entry point for the DLL application.
//


#include "bzfsAPI.h"
#include <string>
#include <map>

BZ_GET_PLUGIN_VERSION

// event handler callback

class airspawn : public bz_EventHandler
{
public:
  airspawn();
  virtual ~airspawn();

  virtual void process(bz_EventData *eventData);

  float spawnRange;
};

airspawn airspawnHandler;

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
  bz_debugMessage(4, "airspawn plugin loaded");

  float range = 0;
  if (commandLine)
    range = (float)atof(commandLine);
  if (range < 0.001f)
    range = 10.0f;

  airspawnHandler.spawnRange = range;
  bz_registerEvent(bz_eGetPlayerSpawnPosEvent, &airspawnHandler);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void)
{
  bz_removeEvent(bz_eGetPlayerSpawnPosEvent, &airspawnHandler);
  bz_debugMessage(4, "airspawn plugin unloaded");
  return 0;
}

airspawn::airspawn()
{
}

airspawn::~airspawn()
{
}

void airspawn::process (bz_EventData *eventData)
{
  switch (eventData->eventType) {
    default:
      // really WTF!!!!
      break;

    case bz_eGetPlayerSpawnPosEvent: {
      bz_GetPlayerSpawnPosEventData_V1 *spawn = (bz_GetPlayerSpawnPosEventData_V1*)eventData;

      float randPos = rand()/(float)RAND_MAX;
      spawn->pos[2] += randPos * spawnRange;
      spawn->handled = true;
    }
      break;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
