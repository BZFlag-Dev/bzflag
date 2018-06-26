/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// R2.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "playerHandler.h"
#include <memory>

class R2Plugin : public bz_Plugin
{
public:
  virtual const char* Name () {return "R2, a second generation robot";}
  virtual void Init ( const char* config );

  virtual void Event ( bz_EventData * /* eventData */ );

  virtual void Cleanup();

protected:
	std::vector<PlayerHandler::Ptr> Droids;

    int     realPlayers = 0;
};

BZ_PLUGIN(R2Plugin)

void R2Plugin::Init ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"R2 plugin loaded");

  // bots need cycles
  MaxWaitTime = 0.01f;

  Register(bz_eTickEvent);
  Register(bz_eWorldFinalized);
  Register(bz_ePlayerJoinEvent);
  Register(bz_ePlayerPartEvent);
}

void R2Plugin::Cleanup()
{
	for (auto droid : Droids)
	    bz_removeServerSidePlayer(droid->getPlayerID(), droid.get());
    Droids.clear();
}

void R2Plugin::Event ( bz_EventData *eventData )
 {
	 if (eventData->eventType == bz_eWorldFinalized)
	 {
		 int botCount = 1;

		 for (int i = 0; i < botCount; i++)
		 {
			 PlayerHandler::Ptr bot = std::make_shared<PlayerHandler>();
			 bz_addServerSidePlayer(bot.get());
             Droids.push_back(bot);
		 }
	 }
	 else if (eventData->eventType == bz_eTickEvent)
	 {
         for (auto droid : Droids)
             droid->update();
	 }
     else if (eventData->eventType == bz_ePlayerJoinEvent)
     {
         bz_PlayerJoinPartEventData_V1 *jpData = (bz_PlayerJoinPartEventData_V1*)eventData;

         bool active = realPlayers != 0;
         if (jpData->record->team != eObservers)
         {
             realPlayers++;
         }

         if (!active && realPlayers > 0)
         {
             // we got a live one, git em

             for (auto droid : Droids)
                 droid->startPlay();
         }
     }
 }
// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
