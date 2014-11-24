// customflagsample.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

// event handler callback
class CustomFlagSample : public bz_Plugin
{
public:
  virtual ~CustomFlagSample(){};
  virtual const char* Name () {return "Custom Flag Sample";}
  virtual void Init ( const char* c);
  virtual void Cleanup ( void );
  virtual void Event ( bz_EventData *eventData );
};

BZ_PLUGIN(CustomFlagSample)

void CustomFlagSample::Event(bz_EventData *eventData)
{
  switch (eventData->eventType) {

  default: {
    // no, sir, we didn't ask for THIS!!
    bz_debugMessage(1, "customflagsample: received event with unrequested eventType!");
    return;
  }

  case bz_eFlagTransferredEvent: {
    bz_FlagTransferredEventData_V1* fte = (bz_FlagTransferredEventData_V1*)eventData;
    if (strcmp(fte->flagType, "CF") == 0)
	bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Custom Flag transferred!");
    break;
  }

  case bz_eFlagGrabbedEvent: {
    bz_FlagGrabbedEventData_V1* fge = (bz_FlagGrabbedEventData_V1*)eventData;
    if (strcmp(fge->flagType, "CF") == 0)
      bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Custom Flag grabbed!");
    break;
  }

  case bz_eFlagDroppedEvent: {
    bz_FlagDroppedEventData_V1* fde = (bz_FlagDroppedEventData_V1*)eventData;
    if (strcmp(fde->flagType, "CF") == 0)
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
	bz_incrementPlayerWins(p, 10);
    }
    break;
  }

  case bz_ePlayerDieEvent: {
    bz_PlayerDieEventData_V1* deed = (bz_PlayerDieEventData_V1*)eventData;
    bz_ApiString flag = deed->flagKilledWith;
    int p = deed->playerID;
    bz_BasePlayerRecord *playerRecord = bz_getPlayerByIndex(p);
    if (flag == "CF")
      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Player %s killed by a player with Custom Flag!", playerRecord->callsign.c_str());
    break;
  }

  }
}

void CustomFlagSample::Init ( const char* /*commandLine*/ )
{
  bz_debugMessage(4, "customflagsample plugin loaded");

  // register our special custom flag
  bz_RegisterCustomFlag("CF", "Custom Flag", "A simple sample custom flag from the customflagsample plugin", /*eSuperShot*/0, eGoodFlag);

  // register events for pick up, drop, transfer, and fire
  Register(bz_eFlagTransferredEvent);
  Register(bz_eFlagGrabbedEvent);
  Register(bz_eFlagDroppedEvent);
  Register(bz_eShotFiredEvent);
  Register(bz_ePlayerDieEvent);
}

void CustomFlagSample::Cleanup ( void )
{
  // unregister our events
  Flush();

  bz_debugMessage(4, "customflagsample plugin unloaded");
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
