/* bzflag
* Copyright (c) 1993 - 2005 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named LICENSE that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/* bzflag special common - 1st one */
#include "common.h"

#include <iostream>
#include <string>

#include "WorldEventManager.h"

//-------------------BaseEventData--------------------
BaseEventData::BaseEventData()
{
	eventType = eNullEvent;
}

//-------------------CTFCaptureEventData--------------------
CTFCaptureEventData::CTFCaptureEventData()
{
	eventType = eCaptureEvent;
	teamCaped = -1;
	teamCaping = -1;
	playerCaping = -1;
}

CTFCaptureEventData::~CTFCaptureEventData()
{
}

//-------------------PlayerDieEventData--------------------
PlayerDieEventData::PlayerDieEventData()
{
	eventType = ePlayerDieEvent;
	playerID = -1;
	teamID = -1;
	killerID = -1;
	killerTeamID = -1;

	pos[0] = pos[1] = pos[2] = 0.0f;
	rot = 0.0f;
	time = 0.0;
}

PlayerDieEventData::~PlayerDieEventData()
{
}

//-------------------PlayerSpawnEventData--------------------
PlayerSpawnEventData::PlayerSpawnEventData()
{
	eventType = ePlayerSpawnEvent;
	playerID = -1;
	teamID = -1;

	pos[0] = pos[1] = pos[2] = 0.0f;
	rot = 0.0f;
	time = 0.0;
}

PlayerSpawnEventData::~PlayerSpawnEventData()
{
}

//-------------------ZoneEntryExitEventData--------------------
ZoneEntryExitEventData::ZoneEntryExitEventData()
{
	eventType = eZoneEntryEvent;
	playerID = -1;
	teamID = -1;
	zoneID = -1;

	pos[0] = pos[1] = pos[2] = 0.0f;
	rot = 0.0f;
	time = 0.0;
}

ZoneEntryExitEventData::~ZoneEntryExitEventData()
{
}

//-------------------PlayerJoinPartEventData--------------------
PlayerJoinPartEventData::PlayerJoinPartEventData()
{
	eventType = ePlayerJoinEvent;

	playerID = -1;
	teamID = -1;
	time = 0.0;
}

PlayerJoinPartEventData::~PlayerJoinPartEventData()
{
}

//-------------------ChatEventData--------------------
ChatEventData::ChatEventData()
{
	eventType = eChatMessageEvent;
	from = -1;
	to = -1;
	time = 0.0;
}

ChatEventData::~ChatEventData()
{
}

//-------------------UnknownSlashCommandEventData--------------------
UnknownSlashCommandEventData::UnknownSlashCommandEventData()
{
	eventType = eUnknownSlashCommand;
	from = -1;
	handled = false;
	time = 0.0;
}

UnknownSlashCommandEventData::~UnknownSlashCommandEventData()
{
}

//-------------------GetPlayerSpawnPosEventData--------------------
GetPlayerSpawnPosEventData::GetPlayerSpawnPosEventData()
{
	eventType = eGetPlayerSpawnPosEvent;
	playeID = -1;
	teamID = -1;

	handled = false;

	pos[0] = pos[1] = pos[2] = 0.0f;
	rot = 0.0f;
	time = 0.0;
}

GetPlayerSpawnPosEventData::~GetPlayerSpawnPosEventData()
{
}

//-------------------WorldEventManager--------------------
WorldEventManager::WorldEventManager()
{
}

WorldEventManager::~WorldEventManager()
{
	tmEventMap::iterator teamItr = eventtMap.begin();
	while ( teamItr != eventtMap.end() )
	{
		tmEventTypeList::iterator eventItr = teamItr->second.begin();
		while (eventItr == teamItr->second.end())
		{
			tvEventList::iterator itr = eventItr->second.begin();
			while ( itr != eventItr->second.end() )
			{
				if ((*itr) && (*itr)->autoDelete())
					delete (*itr);
				*itr = NULL;

				itr++;
			}
			eventItr++;
		}
		teamItr++;
	}
}

void WorldEventManager::addEvent ( teEventType eventType, int team, BaseEventHandaler* theEvetnt )
{
	if (!theEvetnt)
		return;

	tmEventTypeList*	teamEvents = getTeamEventList(team);

	if (teamEvents->find(eventType) == teamEvents->end())
	{
		tvEventList newList;
		(*teamEvents)[eventType] = newList;
	}

	teamEvents->find(eventType)->second.push_back(theEvetnt);
}

void WorldEventManager::removeEvent ( teEventType eventType, int team, BaseEventHandaler* theEvetnt )
{
	if (!theEvetnt)
		return;

	tmEventTypeList*	teamEvents = getTeamEventList(team);

	if (teamEvents->find(eventType) == teamEvents->end())
		return;

	tmEventTypeList::iterator eventTypeItr = teamEvents->begin();
	if ( eventTypeItr != teamEvents->end() )
	{
		tvEventList::iterator itr = eventTypeItr->second.begin();
		while (itr == eventTypeItr->second.end())
		{
			if (*itr == theEvetnt)
				itr = eventTypeItr->second.erase(itr);
			else
				itr++;
		}
	}
}

tvEventList WorldEventManager::getEventList ( teEventType eventType, int team )

{
	tvEventList	eventList;

	tmEventTypeList*	teamEvents = getTeamEventList(team);

	tmEventTypeList::iterator teamItr = teamEvents->find(eventType);
	if ( teamItr == teamEvents->end() )
		return eventList;

	eventList = teamItr->second;
	return eventList;
}

void WorldEventManager::callEvents ( teEventType eventType, int team, BaseEventData	*eventData )
{
	if (!eventData)
		return;

	tvEventList	eventList = getEventList(eventType,team);

	for ( unsigned int i = 0; i < eventList.size(); i++)
		eventList[i]->process(eventData);
}

tmEventTypeList* WorldEventManager::getTeamEventList ( int team )
{
	if (eventtMap.find(team) == eventtMap.end())
	{
		tmEventTypeList newList;
		eventtMap[team] = newList;
	}
	return &(eventtMap.find(team)->second);
}

int WorldEventManager::getEventCount ( teEventType eventType, int team )
{
	return (int)getEventList(eventType,team).size();
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
