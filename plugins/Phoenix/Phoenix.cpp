// Phoenix.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <map>

typedef struct
{
   float x,y,z,a;
} trDeathPos;



class PhoenixEvents : public bz_Plugin
{
public:

  std::map<int,trDeathPos> lastDeaded;
  virtual const char* Name () {return "Phoenix";}
  virtual void Init ( const char* /*config*/ )
  {
    lastDeaded.clear();

    Register(bz_ePlayerDieEvent);
    Register(bz_eGetPlayerSpawnPosEvent);
    Register(bz_ePlayerPartEvent);
    Register(bz_eCaptureEvent);
  }

  virtual void Event ( bz_EventData *eventData )
  {
    switch (eventData->eventType)
    {
      case bz_eCaptureEvent:
	lastDeaded.clear();
	break;

      case bz_ePlayerDieEvent:
	{
	  bz_PlayerDieEventData_V1* data = (bz_PlayerDieEventData_V1*)eventData;

	  trDeathPos pos;
	  pos.x = data->state.pos[0];
	  pos.y = data->state.pos[1];
	  pos.z = data->state.pos[2];
	  pos.a = data->state.rotation;

	  lastDeaded[data->playerID] = pos;
	}
      break;

      case bz_eGetPlayerSpawnPosEvent:
	{
	  bz_GetPlayerSpawnPosEventData_V1* data = (bz_GetPlayerSpawnPosEventData_V1*)eventData;

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
	  bz_PlayerJoinPartEventData_V1* data = (bz_PlayerJoinPartEventData_V1*)eventData;
	  if (lastDeaded.find(data->playerID) != lastDeaded.end())
	    lastDeaded.erase(lastDeaded.find(data->playerID));
	}
	break;
      default:
	break;
    }
  }
};

BZ_PLUGIN(PhoenixEvents)


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
