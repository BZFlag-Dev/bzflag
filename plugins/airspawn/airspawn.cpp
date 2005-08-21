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

	virtual void process ( bz_EventData *eventData );

	virtual bool autoDelete ( void ) { return false;} // this will be used for more then one event
	
	float spawnRange;
};

airspawn	airspawnHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"airspawn plugin loaded");

	float range = 0;
	if ( commandLine )
		range = (float)atof(commandLine);
	if ( range < 0.001f )
		range = 10.0f;
	
	airspawnHandler.spawnRange = range;
	bz_registerEvent(bz_eGetPlayerSpawnPosEvent,&airspawnHandler);
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeEvent(bz_eGetPlayerSpawnPosEvent,&airspawnHandler);
	bz_debugMessage(4,"airspawn plugin unloaded");
	return 0;
}

airspawn::airspawn()
{
}

airspawn::~airspawn()
{
}

void airspawn::process ( bz_EventData *eventData )
{
	switch (eventData->eventType)
	{
	default:
		// really WTF!!!!
		break;

	case bz_eGetPlayerSpawnPosEvent:
		{
			bz_GetPlayerSpawnPosEventData *spawn = (bz_GetPlayerSpawnPosEventData*)eventData;

			float randPos = rand()/(float)RAND_MAX;
			spawn->pos[2] += randPos * spawnRange;
			spawn->handled = true;
		}
		break;
	}
}

