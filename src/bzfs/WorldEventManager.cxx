/* bzflag
 * Copyright (c) 1993-2011 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
  tmEventTypeList::iterator eventItr = eventList.begin();
  while (eventItr != eventList.end())
  {
    tvEventList::iterator itr = eventItr->second.begin();
    while ( itr != eventItr->second.end() )
    {
      delete (*itr);
      *itr = NULL;

      itr++;
    }
    eventItr++;
  }
}

void WorldEventManager::addEvent ( bz_eEventType eventType, bz_EventHandler* theEvent )
{
  if (!theEvent)
    return;

  if (eventList.find(eventType) == eventList.end())
  {
    tvEventList newList;
    eventList[eventType] = newList;
  }

  eventList.find(eventType)->second.push_back(theEvent);
}

void WorldEventManager::removeEvent ( bz_eEventType eventType, bz_EventHandler* theEvent )
{
  if (!theEvent)
    return;

  tmEventTypeList::iterator eventTypeItr = eventList.find(eventType);
  if (eventTypeItr == eventList.end())
    return;

  tvEventList::iterator itr = eventTypeItr->second.begin();
  while (itr != eventTypeItr->second.end())
  {
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
      } else {
	listIt++;
      }
    }
  }

  return foundOne;
}

tvEventList WorldEventManager::getEventList ( bz_eEventType eventType)
{
  tvEventList	eList;

  tmEventTypeList::iterator itr = eventList.find(eventType);
  if ( itr == eventList.end() )
    return eList;

  eList = itr->second;
  return eList;
}

void WorldEventManager::callEvents ( bz_eEventType eventType, bz_EventData  *eventData )
{
  if (!eventData || getEventCount(eventType)==0 )
    return;

  eventData->eventType = eventType;

  tvEventList	eList = getEventList(eventType);

  for ( unsigned int i = 0; i < eList.size(); i++)
    eList[i]->process(eventData);
}

void WorldEventManager::callEvents (  bz_EventData  *eventData )
{
  if (!eventData || getEventCount(eventData->eventType)==0 )
    return;

  tvEventList	eList = getEventList(eventData->eventType);

  for ( unsigned int i = 0; i < eList.size(); i++)
    eList[i]->process(eventData);
}

int WorldEventManager::getEventCount ( bz_eEventType eventType )
{
  return (int)getEventList(eventType).size();
}

bool RegisterEvent ( bz_eEventType eventType, bz_Plugin* plugin )
{
  if (!plugin)
    return false;

  bz_EventHandler *handler = new bz_EventHandler();
  handler->plugin = plugin;

  if (worldEventManager.getEventCount(eventType) == 0)
    worldEventManager.addEvent(eventType,handler);
  else
  {
    tvEventList& list = worldEventManager.eventList[eventType];
    tvEventList::iterator itr = list.begin();
    while (itr != list.end())
    {
      if ((*itr)->plugin == plugin)
	return false;
      itr++;
    }
    list.push_back(handler);
  }
  return true;
}

bool RemoveEvent ( bz_eEventType eventType, bz_Plugin* plugin )
{
  if (!plugin || worldEventManager.getEventCount(eventType) == 0)
    return false;

  tvEventList& list = worldEventManager.eventList[eventType];

  tvEventList::iterator itr = list.begin();
  while (itr != list.end())
  {
    bz_EventHandler* handler = *itr;

    if (handler->plugin == plugin)
    {
      itr = list.erase(itr);
      delete(handler);
      return true;
    }
    else
      itr++;
  }

  return false;
}

bool FlushEvents(bz_Plugin* plugin)
{
  if (!plugin)
    return false;

  bool foundOne = false;

  tmEventTypeList::iterator typeIt;
  for (typeIt = worldEventManager.eventList.begin(); typeIt != worldEventManager.eventList.end(); ++typeIt) 
  {
    tvEventList& evList = typeIt->second;
    tvEventList::iterator listIt = evList.begin();
    while (listIt != evList.end())
    {
      bz_EventHandler* handler = *listIt;

      if (handler->plugin == plugin)
      {
	listIt = evList.erase(listIt);
	delete(handler);
	foundOne = true;
      }
      else
	listIt++;
    }
  }

  return foundOne;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
