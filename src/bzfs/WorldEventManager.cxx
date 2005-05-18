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

BaseEventData::BaseEventData()
{
	eventType = eNullEvent;
}

CTFCaptureEventData::CTFCaptureEventData()
{
	eventType = eCaptureEvent;
	teamCAped = -1;
	teamCaping = -1;
	playerCaping = -1;
}

CTFCaptureEventData::~CTFCaptureEventData()
{
}


WorldEventManager::WorldEventManager()
{
}

WorldEventManager::~WorldEventManager()
{
	tmTeamCapEventMap::iterator teamItr = teamCapEventMap.begin();
	while ( teamItr != teamCapEventMap.end() )
	{
		tvEventList::iterator itr = teamItr->second.begin();
		while (itr == teamItr->second.end())
		{
			delete (*itr);
			itr++;
		}
		teamItr++;
	}
}

void WorldEventManager::addCapEvent ( int team, BaseEventHandaler* theEvetnt )
{
	if (!theEvetnt)
		return;

	if (teamCapEventMap.find(team) == teamCapEventMap.end())
	{
		tvEventList newList;
		teamCapEventMap[team] = newList;
	}

	teamCapEventMap.find(team)->second.push_back(theEvetnt);
}

void WorldEventManager::removeCapEvent ( int team, BaseEventHandaler* theEvetnt )
{
	if (!theEvetnt)
		return;

	if (teamCapEventMap.find(team) == teamCapEventMap.end())
		return;

	tmTeamCapEventMap::iterator teamItr = teamCapEventMap.begin();
	if ( teamItr != teamCapEventMap.end() )
	{
		tvEventList::iterator itr = teamItr->second.begin();
		while (itr == teamItr->second.end())
		{
			if (*itr == theEvetnt)
				itr = teamItr->second.erase(itr);
			else
				itr++;
		}
	}
}

tvEventList WorldEventManager::getCapEventList ( int team )
{
	tvEventList	eventList;

	tmTeamCapEventMap::iterator teamItr = teamCapEventMap.begin();
	if ( teamItr == teamCapEventMap.end() )
		return eventList;

	eventList = teamItr->second;
	return eventList;
}

void WorldEventManager::callAllCapEvents ( int team, BaseEventData	*eventData )
{
	if (!eventData)
		return;

	tvEventList	eventList = getCapEventList(team);

	for ( int i = 0; i < eventList.size(); i++)
		eventList[i]->process(eventData);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
