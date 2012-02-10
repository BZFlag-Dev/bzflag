// hiddenAdmin.cpp : Defines the entry point for the DLL application.
//


#include "bzfsAPI.h"
#include <map>

// event handler callback

class HiddenAdmin : public bz_Plugin
{
public:
  virtual const char* Name(){return "Hidden Admins";}
  virtual void Init(const char* config);

  virtual void Event ( bz_EventData *eventData );
protected:
};

BZ_PLUGIN(HiddenAdmin)

void HiddenAdmin::Init(const char* /*commandLine*/ )
{
	Register(bz_eGetPlayerInfoEvent);
}

void HiddenAdmin::Event ( bz_EventData *eventData )
{
  if (eventData->eventType != bz_eGetPlayerInfoEvent)
    return;

  bz_GetPlayerInfoEventData_V1	* infoData = (bz_GetPlayerInfoEventData_V1*)eventData;

  infoData->admin = false;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

