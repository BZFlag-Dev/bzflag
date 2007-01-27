// ShockwaveArena.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

class Handler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData )
	{
		if (eventData->eventType == bz_eFlagGrabbedEvent)
		{
			bz_FlagGrabbedEventData_V1 *data = (bz_FlagGrabbedEventData_V1*)eventData;
			data->shotType = eShockWaveShot;
		}

		if (eventData->eventType == bz_ePlayerSpawnEvent)
		{
			bz_PlayerSpawnEventData_V1 *data = (bz_PlayerSpawnEventData_V1*)eventData;
			bz_setPlayerShotType(data->playerID,eShockWaveShot);
		}

		if (eventData->eventType == bz_eFlagDroppedEvent)
		{
			bz_FlagDroppedEvenData_V1 *data = (bz_FlagDroppedEvenData_V1*)eventData;
			bz_setPlayerShotType(data->playerID,eShockWaveShot);
		}
		
	}
};

Handler handler;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
	bz_registerEvent(bz_eFlagGrabbedEvent,&handler);
	bz_registerEvent(bz_ePlayerSpawnEvent,&handler);
	bz_registerEvent(bz_eFlagDroppedEvent,&handler);
	bz_debugMessage(4,"ShockwaveArena plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeEvent(bz_eFlagGrabbedEvent,&handler);
	bz_removeEvent(bz_ePlayerSpawnEvent,&handler);
	bz_removeEvent(bz_eFlagDroppedEvent,&handler);
	bz_debugMessage(4,"ShockwaveArena plugin unloaded");
  return 0;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

