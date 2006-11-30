// flagStay.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>
#include <vector>
#include <math.h>

BZ_GET_PLUGIN_VERSION

class FlagStayZoneHandler : public bz_CustomMapObjectHandler
{
public:
	virtual bool handle ( bzApiString object, bz_CustomMapObjectInfo *data );
};

FlagStayZoneHandler	flagStayZoneHandler;

class EventHandler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData );
};

EventHandler eventHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"flagStay plugin loaded");
  bz_registerCustomMapObject("FLAGSTAYZONE",&flagStayZoneHandler);

  bz_registerEvent(bz_ePlayerUpdateEvent,&eventHandler);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent(bz_ePlayerUpdateEvent,&eventHandler);
  bz_removeCustomMapObject("FLAGSTAYZONE");
  bz_debugMessage(4,"flagStay plugin unloaded");
  return 0;
}


class FlagStayZone
{
public:
	FlagStayZone()
	{
		box = false;
		xMax = xMin = yMax = yMin = zMax = zMin = rad = 0;
	}

	bool box;
	float xMax,xMin,yMax,yMin,zMax,zMin;
	float rad;

	std::string message;
	bool pointIn ( float pos[3] )
	{
		if ( box )
		{
			if ( pos[0] > xMax || pos[0] < xMin )
				return false;

			if ( pos[1] > yMax || pos[1] < yMin )
				return false;

			if ( pos[2] > zMax || pos[2] < zMin )
				return false;
		}
		else
		{
			float vec[3];
			vec[0] = pos[0]-xMax;
			vec[1] = pos[1]-yMax;
			vec[2] = pos[2]-zMax;

			float dist = sqrt(vec[0]*vec[0]+vec[1]*vec[1]);
			if ( dist > rad)
				return false;

			if ( pos[2] > zMax || pos[2] < zMin )
				return false;

		}
		return true;
	}

	bool checkFlag ( const char* flag )
	{
		for ( unsigned int i = 0; i < flagList.size(); i++ )
		{
			if ( flagList[i] == flag )
				return true;
		}
		return false;
	}

	std::vector<std::string> flagList;
};

std::vector <FlagStayZone> zoneList;

bool FlagStayZoneHandler::handle ( bzApiString object, bz_CustomMapObjectInfo *data )
{
	if (object != "FLAGSTAYZONE" || !data)
		return false;

	FlagStayZone newZone;

	// parse all the chunks
	for ( unsigned int i = 0; i < data->data.size(); i++ )
	{
		std::string line = data->data.get(i).c_str();

		bzAPIStringList *nubs = bz_newStringList();
		nubs->tokenize(line.c_str()," ",0,true);
		
		if ( nubs->size() > 0)
		{
			std::string key = bz_toupper(nubs->get(0).c_str());

			if ( key == "BBOX" && nubs->size() > 6)
			{
				newZone.box = true;
				newZone.xMin = (float)atof(nubs->get(1).c_str());
				newZone.xMax = (float)atof(nubs->get(2).c_str());
				newZone.yMin = (float)atof(nubs->get(3).c_str());
				newZone.yMax = (float)atof(nubs->get(4).c_str());
				newZone.zMin = (float)atof(nubs->get(5).c_str());
				newZone.zMax = (float)atof(nubs->get(6).c_str());
			}
			else if ( key == "CYLINDER" && nubs->size() > 5)
			{
				newZone.box = false;
				newZone.rad = (float)atof(nubs->get(5).c_str());
				newZone.xMax =(float)atof(nubs->get(1).c_str());
				newZone.yMax =(float)atof(nubs->get(2).c_str());
				newZone.zMin =(float)atof(nubs->get(3).c_str());
				newZone.zMax =(float)atof(nubs->get(4).c_str());
			}
			else if ( key == "FLAG" && nubs->size() > 1)
			{
				std::string flag = nubs->get(1).c_str();
				newZone.flagList.push_back(flag);
			}
			else if ( key == "MESSAGE" && nubs->size() > 1 )
			{
				newZone.message = nubs->get(1).c_str();
			}
		}
		bz_deleteStringList(nubs);
	}
	zoneList.push_back(newZone);
	return true;
}

void EventHandler::process ( bz_EventData *eventData )
{
	float pos[3] = {0};

	int playerID = -1;

	switch (eventData->eventType)
	{
	case bz_ePlayerUpdateEvent:
		pos[0] = ((bz_PlayerUpdateEventData*)eventData)->pos[0];
		pos[1] = ((bz_PlayerUpdateEventData*)eventData)->pos[1];
		pos[2] = ((bz_PlayerUpdateEventData*)eventData)->pos[2];
		playerID = ((bz_PlayerUpdateEventData*)eventData)->playerID;
		break;

	case bz_eShotFiredEvent:
		pos[0] = ((bz_ShotFiredEventData*)eventData)->pos[0];
		pos[1] = ((bz_ShotFiredEventData*)eventData)->pos[1];
		pos[2] = ((bz_ShotFiredEventData*)eventData)->pos[2];
		playerID = ((bz_ShotFiredEventData*)eventData)->playerID;
		break;
	}

	const char* flagAbrev = bz_getPlayerFlag(playerID);
	if (!flagAbrev)
		return;

	// check and see if a zone cares about the current flag
	for ( unsigned int i = 0; i < zoneList.size(); i++ )
	{
		if ( zoneList[i].checkFlag(flagAbrev) )
		{
			if ( !zoneList[i].pointIn(pos) )	// they have taken the flag out of a zone, pop it.
			{
				bz_removePlayerFlag(playerID);
				if (zoneList[i].message.size())
					bz_sendTextMessage(BZ_SERVER,playerID,zoneList[i].message.c_str());
				i = (unsigned int)zoneList.size();	// finish out
			}
		}
	}
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

