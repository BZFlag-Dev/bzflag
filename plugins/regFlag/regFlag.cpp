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

#include "bzfsAPI.h"
#include <map>

class RegFlag : public bz_Plugin
{
public:
  virtual const char* Name() { return "RegFlag"; }
  virtual void Init(const char* config);

  virtual void Event(bz_EventData *eventData );

private:
  std::map<int, double> lastNotifiedTime;
};

BZ_PLUGIN(RegFlag)

void RegFlag::Init(const char* /*commandLine*/)
{
  Register(bz_ePlayerJoinEvent);
  Register(bz_ePlayerPartEvent);
  Register(bz_eAllowFlagGrab);
}


void RegFlag::Event(bz_EventData *eventData)
{
  switch (eventData->eventType) {
    // When a player joins, initialize the last notified time to -9999.0
    case bz_ePlayerJoinEvent:
      lastNotifiedTime[((bz_PlayerJoinPartEventData_V1*)eventData)->playerID] = -9999.0;
      break;

    // When a player leaves, remove the last notified time for that slot
    case bz_ePlayerPartEvent:
      lastNotifiedTime.erase(((bz_PlayerJoinPartEventData_V1*)eventData)->playerID);
      break;

    case bz_eAllowFlagGrab: {
      bz_AllowFlagGrabData_V1* data = (bz_AllowFlagGrabData_V1*)eventData;

      // Fetch the player record to check if they are a global user
      bz_BasePlayerRecord *player = bz_getPlayerByIndex(data->playerID);

      // Put the flag name into an easy-to-test variable type.
      bz_ApiString flagAbbrev = bz_getFlagName(data->flagID);

      // If the player isn't a global user, prevent them from grabbing a flag, and
      // notify them why if enough time has passed.  Always allow team flags, however.
      if (
	player && !player->globalUser &&
	!(flagAbbrev == "R*" || flagAbbrev == "G*" || flagAbbrev == "B*" || flagAbbrev == "P*")
      ) {
	data->allow = false;

	// Only notify once every 5 minutes
	if (lastNotifiedTime[data->playerID] + 300.0 < data->eventTime) {
	  bz_sendTextMessage(BZ_SERVER, data->playerID, "Sorry, flags are for registered players only.");
	  lastNotifiedTime[data->playerID] = data->eventTime;
	}
      }

      // Free the player record
      bz_freePlayerRecord(player);
    }

    default:
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
