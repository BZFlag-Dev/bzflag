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

// shockwaveDeath.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>

BZ_GET_PLUGIN_VERSION

// event handler callback
class SWDeathHandler : public bz_EventHandler
{
public:
  virtual void	process ( bz_EventData *eventData );
};

SWDeathHandler	swDeathHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* /* commandLine */ )
{
  bz_debugMessage(4,"shockwaveDeath plugin loaded");

  bz_registerEvent(bz_ePlayerDieEvent,&swDeathHandler);

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent(bz_ePlayerDieEvent,&swDeathHandler);
  bz_debugMessage(4,"shockwaveDeath plugin unloaded");
  return 0;
}

void SWDeathHandler::process ( bz_EventData *eventData )
{
  if (eventData->eventType != bz_ePlayerDieEvent)
    return;

  bz_PlayerDieEventData_V1 *dieData = (bz_PlayerDieEventData_V1*)eventData;

  float reloadTime = (float)bz_getBZDBDouble("_reloadTime");

  if (bz_BZDBItemExists("_swDeathReloadFactor") && bz_getBZDBDouble("_swDeathReloadFactor") > 0)
    reloadTime *= (float)bz_getBZDBDouble("_swDeathReloadFactor");

  bz_fireWorldWep("SW",reloadTime,dieData->state.pos,0,0,0,0.0f);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
