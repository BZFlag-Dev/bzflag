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

// customflagsample.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION


class CustomFlagSampleHandler : public bz_EventHandler {
public:
  void process(bz_EventData *eventData)
  {
    switch (eventData->eventType) {

      default: {
	// no, sir, we didn't ask for THIS!!
	bz_debugMessage(1, "customflagsample: received event with unrequested eventType!");
	return;
      }

      case bz_eFlagTransferredEvent: {
	bz_FlagTransferredEventData_V1* fte = (bz_FlagTransferredEventData_V1*)eventData;
	if (fte->flagType == "CF")
	  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Custom Flag transferred!");
	break;
      }

      case bz_eFlagGrabbedEvent: {
	bz_FlagGrabbedEventData_V1* fge = (bz_FlagGrabbedEventData_V1*)eventData;
	if (fge->flagType == "CF")
	  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Custom Flag grabbed!");
	break;
      }

      case bz_eFlagDroppedEvent: {
	bz_FlagDroppedEventData_V1* fde = (bz_FlagDroppedEventData_V1*)eventData;
	if (fde->flagType == "CF")
	  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Custom Flag dropped!");
	break;
      }

      case bz_eShotFiredEvent: {
	bz_ShotFiredEventData_V1* sfed = (bz_ShotFiredEventData_V1*)eventData;
	int p = sfed->playerID;
	bz_BasePlayerRecord *playerRecord = bz_getPlayerByIndex(p);
	if (!playerRecord) break;
	if (playerRecord->currentFlag == "Custom Flag (+CF)")
	  {
	    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Shot fired by %s with Custom Flag!", playerRecord->callsign.c_str());
	    // this user must be cool, add 10 to their score
	    bz_setPlayerWins(p, bz_getPlayerWins(p)+10);
	  }
	break;
      }
    }
  }

};

CustomFlagSampleHandler cfs;

BZF_PLUGIN_CALL int bz_Load(const char* /*commandLine*/)
{
  bz_debugMessage(4, "customflagsample plugin loaded");

  // register our special custom flag
  bz_RegisterCustomFlag("CF", "Custom Flag", "A simple sample custom flag from the customflagsample plugin", eSuperShot, eGoodFlag);

  // register events for pick up, drop, transfer, and fire
  bz_registerEvent(bz_eFlagTransferredEvent, &cfs);
  bz_registerEvent(bz_eFlagGrabbedEvent, &cfs);
  bz_registerEvent(bz_eFlagDroppedEvent, &cfs);
  bz_registerEvent(bz_eShotFiredEvent, &cfs);

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void)
{
  // unregister our events
  bz_removeEvent(bz_eFlagTransferredEvent, &cfs);
  bz_removeEvent(bz_eFlagGrabbedEvent, &cfs);
  bz_removeEvent(bz_eFlagDroppedEvent, &cfs);
  bz_removeEvent(bz_eShotFiredEvent, &cfs);

  bz_debugMessage(4, "customflagsample plugin unloaded");
  return 0;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
