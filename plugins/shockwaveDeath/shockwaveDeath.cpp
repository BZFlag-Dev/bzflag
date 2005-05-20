// schokwaveDeath.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>


// event handaler callback
class DeathHandaler : public bz_EventHandaler
{
public:
	virtual void process ( bz_EventData *eventData );
};

DeathHandaler	deathHandaler;

int bz_Load ( const char* commandLine )
{
	bz_registerEvent(bz_ePlayerDieEvent,BZ_ALL_USERS,&deathHandaler);
	return 0;
}

int bz_Unload ( void )
{
	return 0;
}

void DeathHandaler::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerDieEvent)
		return;

	bz_PlayerDieEventData	*dieData = (bz_PlayerDieEventData*)eventData;

	bz_fireWorldWep("SW",10,BZ_SERVER,dieData->pos,0,0,0,0);
}