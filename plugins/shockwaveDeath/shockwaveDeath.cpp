// shockwaveDeath.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>

BZ_GET_PLUGIN_VERSION

// event handler callback
class DeathHandler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData );

	bool	usePlayerForShot;
};

DeathHandler	deathHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"shockwaveDeath plugin loaded");

	bz_registerEvent(bz_ePlayerDieEvent,&deathHandler);

	std::string param = commandLine;

	deathHandler.usePlayerForShot = (param == "usevictim");
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeEvent(bz_ePlayerDieEvent,&deathHandler);
	bz_debugMessage(4,"shockwaveDeath plugin unloaded");
	return 0;
}

void DeathHandler::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerDieEvent)
		return;

	bz_PlayerDieEventData	*dieData = (bz_PlayerDieEventData*)eventData;

	int playerToUse = BZ_SERVER;
	if ( usePlayerForShot )
		playerToUse = dieData->playerID;

	float reloadTime = (float)bz_getBZDBDouble("_reloadTime");

	if (bz_BZDBItemExists("_swDeathReloadFactor") && bz_getBZDBDouble("_swDeathReloadFactor") > 0)
		reloadTime *= (float)bz_getBZDBDouble("_swDeathReloadFactor");
        
	bz_fireWorldWep("SW",reloadTime,playerToUse,dieData->pos,0,0,0,0.0f);
}

