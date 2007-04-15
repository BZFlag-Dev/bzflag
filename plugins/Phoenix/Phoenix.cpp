// Phoenix.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <map>

typedef struct 
{
   float x,y,z,a;
}trDeathPos;

std::map<int,trDeathPos>  lastDeaded;

class PhoenixEvents : public bz_EventHandler
{
  virtual void process ( bz_EventData *eventData )
  {
    switch(eventData->eventType)
    {
      case bz_eCaptureEvent:
	lastDeaded.clear();
	break;

      case bz_ePlayerDieEvent:
	{
	  bz_PlayerDieEventData* data = (bz_PlayerDieEventData*)eventData;

	  trDeathPos pos;
	  pos.x = data->pos[0];
	  pos.y = data->pos[1];
	  pos.z = data->pos[2];
	  pos.a = data->rot;

	  lastDeaded[data->playerID] = pos;
	}
      break;

      case bz_eGetPlayerSpawnPosEvent:
	{
	  bz_GetPlayerSpawnPosEventData* data = (bz_GetPlayerSpawnPosEventData*)eventData;

	  if (lastDeaded.find(data->playerID) == lastDeaded.end())
	    break;

	  trDeathPos &pos = lastDeaded[data->playerID];

	  data->handled = true;
	  data->pos[0] = pos.x;
	  data->pos[1] = pos.y;
	  data->pos[2] = pos.z;
	  data->rot = pos.a;
	}
	break;

      case bz_ePlayerPartEvent:
	{
	  bz_PlayerJoinPartEventData* data = (bz_PlayerJoinPartEventData*)eventData;
	  if (lastDeaded.find(data->playerID) != lastDeaded.end())
	    lastDeaded.erase(lastDeaded.find(data->playerID));
	}
	break;
    }
  }
};

PhoenixEvents phoenixEvents;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  lastDeaded.clear();

  bz_registerEvent(bz_ePlayerDieEvent,&phoenixEvents);
  bz_registerEvent(bz_eGetPlayerSpawnPosEvent,&phoenixEvents);
  bz_registerEvent(bz_ePlayerPartEvent,&phoenixEvents);
  bz_registerEvent(bz_eCaptureEvent,&phoenixEvents);

  bz_debugMessage(4,"Phoenix plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent(bz_ePlayerDieEvent,&phoenixEvents);
  bz_removeEvent(bz_eGetPlayerSpawnPosEvent,&phoenixEvents);
  bz_removeEvent(bz_ePlayerPartEvent,&phoenixEvents);
  bz_removeEvent(bz_eCaptureEvent,&phoenixEvents);

  lastDeaded.clear();
  bz_debugMessage(4,"Phoenix plugin unloaded");
  return 0;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
