// flagStay.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <map>
#include <cmath>

class FlagStayZoneHandler : public bz_CustomMapObjectHandler
{
public:
	virtual bool MapObject ( bz_ApiString object, bz_CustomMapObjectInfo *data );
};

FlagStayZoneHandler	flagStayZoneHandler;

class EventHandler : public bz_Plugin
{
public:
	virtual const char* Name (){return "Flag Stay Zones";}
	virtual void Init ( const char* cl );
	virtual void Cleanup ();
	virtual void Event ( bz_EventData *eventData );
};

BZ_PLUGIN(EventHandler)

void EventHandler::Init ( const char* /*commandLine*/ )
{
  bz_registerCustomMapObject("FLAGSTAYZONE",&flagStayZoneHandler);

  Register(bz_ePlayerUpdateEvent);
}

void EventHandler::Cleanup ( void )
{
  Flush();
  bz_removeCustomMapObject("FLAGSTAYZONE");
}

class FlagStayZone : public bz_CustomZoneObject
{
public:
  FlagStayZone() : bz_CustomZoneObject() {}

	std::string message;

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

bool FlagStayZoneHandler::MapObject ( bz_ApiString object, bz_CustomMapObjectInfo *data )
{
	if (object != "FLAGSTAYZONE" || !data)
		return false;

	FlagStayZone newZone;
	newZone.handleDefaultOptions(data);

	// parse all the chunks
	for ( unsigned int i = 0; i < data->data.size(); i++ )
	{
		std::string line = data->data.get(i).c_str();

		bz_APIStringList *nubs = bz_newStringList();
		nubs->tokenize(line.c_str()," ",0,true);

		if ( nubs->size() > 0)
		{
			std::string key = bz_toupper(nubs->get(0).c_str());

			if ( key == "FLAG" && nubs->size() > 1)
			{
				std::string flag = bz_toupper(nubs->get(1).c_str());
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

std::map<int,int>	playerIDToZoneMap;

void EventHandler::Event ( bz_EventData *eventData )
{
	float pos[3] = {0};

	int playerID = -1;

	switch (eventData->eventType)
	{
	case bz_ePlayerUpdateEvent:
		pos[0] = ((bz_PlayerUpdateEventData_V1*)eventData)->state.pos[0];
		pos[1] = ((bz_PlayerUpdateEventData_V1*)eventData)->state.pos[1];
		pos[2] = ((bz_PlayerUpdateEventData_V1*)eventData)->state.pos[2];
		playerID = ((bz_PlayerUpdateEventData_V1*)eventData)->playerID;
		break;

	case bz_eShotFiredEvent:
		pos[0] = ((bz_ShotFiredEventData_V1*)eventData)->pos[0];
		pos[1] = ((bz_ShotFiredEventData_V1*)eventData)->pos[1];
		pos[2] = ((bz_ShotFiredEventData_V1*)eventData)->pos[2];
		playerID = ((bz_ShotFiredEventData_V1*)eventData)->playerID;
		break;

	default:
		return;
	}

	const char* flagAbrev = bz_getPlayerFlag(playerID);
	if (!flagAbrev)
		return;

	std::vector<FlagStayZone*> validZones;

	// check and see if a zone cares about the current flag
	for ( unsigned int i = 0; i < zoneList.size(); i++ )
	{
		if ( zoneList[i].checkFlag(flagAbrev) )
			validZones.push_back(&zoneList[i]);
	}

	// Check each zone for this flag to see if we are in one
	bool insideOne = false;
	for ( unsigned int i = 0; i < validZones.size(); i++ )
	{
		if ( validZones[i]->pointInZone(pos) )
		{
			insideOne = true;
			playerIDToZoneMap[playerID] = i;
		}
	}

	// if they have taken the flag out of a zone, pop it.
	if (!insideOne && validZones.size() > 0)
	{
		int lastZone = -1;
		if ( playerIDToZoneMap.find(playerID) != playerIDToZoneMap.end() )
			lastZone = playerIDToZoneMap[playerID];
		bz_removePlayerFlag(playerID);
		if (lastZone != -1 && zoneList[lastZone].message.size())
			bz_sendTextMessage(BZ_SERVER,playerID,zoneList[lastZone].message.c_str());
	}
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
