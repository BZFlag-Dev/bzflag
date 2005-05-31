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
		while (eventItr != teamItr->second.end())
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

void WorldEventManager::addEvent ( bz_eEventType eventType, int team, bz_EventHandler* theEvetnt )
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

void WorldEventManager::removeEvent ( bz_eEventType eventType, int team, bz_EventHandler* theEvetnt )
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

tvEventList WorldEventManager::getEventList ( bz_eEventType eventType, int team )

{
	tvEventList	eventList;

	tmEventTypeList*	teamEvents = getTeamEventList(team);

	tmEventTypeList::iterator teamItr = teamEvents->find(eventType);
	if ( teamItr == teamEvents->end() )
		return eventList;

	eventList = teamItr->second;
	return eventList;
}

void WorldEventManager::callEvents ( bz_eEventType eventType, int team, bz_EventData	*eventData )
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

int WorldEventManager::getEventCount ( bz_eEventType eventType, int team )
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
