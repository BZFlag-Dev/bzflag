// weaponArena.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

bool randomShots = false;
bz_eShotType shotType = eNoShot;

class Handler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData )
	{
		if (randomShots)
			shotType = (bz_eShotType)((rand()%eLastShotType-1)+1);

		if (shotType == eNoShot)
			return;

		if (eventData->eventType == bz_eFlagGrabbedEvent)
		{
			bz_FlagGrabbedEventData_V1 *data = (bz_FlagGrabbedEventData_V1*)eventData;
			data->shotType = shotType;
		}

		if (eventData->eventType == bz_ePlayerSpawnEvent)
		{
			bz_PlayerSpawnEventData_V1 *data = (bz_PlayerSpawnEventData_V1*)eventData;
			bz_setPlayerShotType(data->playerID,shotType);
		}

		if (eventData->eventType == bz_eFlagDroppedEvent)
		{
			bz_FlagDroppedEventData_V1 *data = (bz_FlagDroppedEventData_V1*)eventData;
			bz_setPlayerShotType(data->playerID,shotType);
		}
	}
};

Handler handler;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_registerEvent(bz_eFlagGrabbedEvent,&handler);
	bz_registerEvent(bz_ePlayerSpawnEvent,&handler);
	bz_registerEvent(bz_eFlagDroppedEvent,&handler);
	bz_debugMessage(4,"weaponArena plugin loaded");

	randomShots = true;
	if (commandLine && strlen(commandLine))
	{
		shotType = eNoShot;
		std::string flag = commandLine;
		if ( flag == "SW" )
			shotType = eShockWaveShot;
		else if ( flag == "LZ")
			shotType = eLaserShot;
		else if ( flag == "GM")
			shotType = eGMShot;
		else if ( flag == "SB")
			shotType = eSuperShot;
		else if ( flag == "IB")
			shotType = eInvisibleShot;
		else if ( flag == "SS")
			shotType = eStandardShot;

		randomShots = false;
	}

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeEvent(bz_eFlagGrabbedEvent,&handler);
	bz_removeEvent(bz_ePlayerSpawnEvent,&handler);
	bz_removeEvent(bz_eFlagDroppedEvent,&handler);
	bz_debugMessage(4,"weaponArena plugin unloaded");
  return 0;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

