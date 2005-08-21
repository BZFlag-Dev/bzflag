// hiddenAdmin.cpp : Defines the entry point for the DLL application.
//


#include "bzfsAPI.h"
#include <string>
#include <map>

BZ_GET_PLUGIN_VERSION

// event handler callback

class HiddenAdmin : public bz_EventHandler
{
public:
	HiddenAdmin();
	virtual ~HiddenAdmin();

	virtual void process ( bz_EventData *eventData );

	virtual bool autoDelete ( void ) { return false;} // this will be used for more then one event
protected:
};

HiddenAdmin	hiddenAdmin;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"HiddenAdmin plugin loaded");

	bz_registerEvent(bz_eGetPlayerInfoEvent,&hiddenAdmin);

	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeEvent(bz_eGetPlayerInfoEvent,&hiddenAdmin);

	bz_debugMessage(4,"HiddenAdmin plugin unloaded");
	return 0;
}


HiddenAdmin::HiddenAdmin()
{
}

HiddenAdmin::~HiddenAdmin()
{
}

void HiddenAdmin::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_eGetPlayerInfoEvent)
		return;

	bz_GetPlayerInfoEventData	* infoData = (bz_GetPlayerInfoEventData*)eventData;

	infoData->admin = false;
}

