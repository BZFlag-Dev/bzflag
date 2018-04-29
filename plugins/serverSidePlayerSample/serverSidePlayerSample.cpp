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

// serverSidePlayerSample.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "playerHandler.h"

class ServerSidePlayerSample : public bz_Plugin
{
public:
  virtual const char* Name () {return "Server Side Player Sample";}
  virtual void Init ( const char* config );

  virtual void Event ( bz_EventData * /* eventData */ );

  virtual void Cleanup();

protected:
	std::vector<PlayerHandler*> Bots;
};

BZ_PLUGIN(ServerSidePlayerSample)

void ServerSidePlayerSample::Init ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"serverSidePlayerSample plugin loaded");

  // bots need cycles
  MaxWaitTime = 0.01f;

  Register(bz_eTickEvent);
  Register(bz_eWorldFinalized);
}

void ServerSidePlayerSample::Cleanup()
{
	for (size_t i = 0; i < Bots.size(); i++)
	{
		bz_removeServerSidePlayer(Bots[i]->getPlayerID(), Bots[i]);
		delete(Bots[i]);
		Bots[i] = NULL;
	}
	Bots.clear();
}

void ServerSidePlayerSample::Event ( bz_EventData *eventData )
 {
	 if (eventData->eventType == bz_eWorldFinalized)
	 {
		 int botCount = 1;
		 for (int i = 0; i < botCount; i++)
		 {
			 PlayerHandler *bot = new PlayerHandler();
			 bz_addServerSidePlayer(bot);
			 Bots.push_back(bot);
		 }
	 }
	 else if (eventData->eventType == bz_eTickEvent)
	 {
		 for (size_t i = 0; i < Bots.size(); i++)
			 Bots[i]->update();
	 }
 }
// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
