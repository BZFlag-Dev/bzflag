/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "WorldEventManager.h"

/* system implementation headers */
#include <string>
#include <set>

/* common implementation headers */
#include "global.h"
#include "bzfsAPI.h"


// initialize the singleton
template <>
WorldEventManager* Singleton<WorldEventManager>::_instance = (WorldEventManager*)NULL;


extern bz_eTeamType convertTeam(TeamColor team);
extern TeamColor convertTeam(bz_eTeamType team);

//-------------------WorldEventManager--------------------
WorldEventManager::WorldEventManager()
{
}

WorldEventManager::~WorldEventManager()
{
}

void WorldEventManager::addEvent(bz_eEventType eventType, bz_EventHandler* theEvent)
{
  if (!theEvent)
    return;

  if (eventList.find(eventType) == eventList.end()) {
    tvEventList newList;
    eventList[eventType] = newList;
  }

  eventList.find(eventType)->second.push_back(theEvent);
}

void WorldEventManager::removeEvent(bz_eEventType eventType, bz_EventHandler* theEvent)
{
  if (!theEvent)
    return;

  tmEventTypeList::iterator eventTypeItr = eventList.find(eventType);
  if (eventTypeItr == eventList.end())
    return;

  tvEventList::iterator itr = eventTypeItr->second.begin();
  while (itr != eventTypeItr->second.end()) {
    if (*itr == theEvent)
      itr = eventTypeItr->second.erase(itr);
    else
      itr++;
  }
}

bool WorldEventManager::removeHandler(bz_EventHandler* theEvent)
{
  bool foundOne = false;

  tmEventTypeList::iterator typeIt;
  for (typeIt = eventList.begin(); typeIt != eventList.end(); ++typeIt) {
    tvEventList& evList = typeIt->second;
    tvEventList::iterator listIt = evList.begin();
    while (listIt != evList.end()) {
      if (*listIt == theEvent) {
        listIt = evList.erase(listIt);
        foundOne = true;
      }
      listIt++;
    }
  }

  return foundOne;
}

tvEventList WorldEventManager::getEventList (bz_eEventType eventType)
{
  tvEventList eList;

  tmEventTypeList::iterator itr = eventList.find(eventType);
  if (itr == eventList.end())
    return eList;

  eList = itr->second;
  return eList;
}

void WorldEventManager::callEvents(bz_eEventType eventType, bz_EventData *eventData)
{
  if (!eventData)
    return;

  eventData->eventType = eventType;
  tvEventList	eList = getEventList(eventType);

  for (unsigned int i = 0; i < eList.size(); i++)
    eList[i]->process(eventData);
}

int WorldEventManager::getEventCount(bz_eEventType eventType)
{
  return (int)getEventList(eventType).size();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
