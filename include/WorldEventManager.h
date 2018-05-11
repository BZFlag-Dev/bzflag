/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
* world event manager definitions
************************************
* right now this just does team flag capture events for world weps
* but it should be able to be expandable to store any type of event
* for anything that can be triggerd.
*/

#ifndef WORLD_EVENT_MANAGER_H
#define	WORLD_EVENT_MANAGER_H

#include <map>
#include <vector>
#include <algorithm>

#include "bzfsAPI.h"

// event handler callback
class bz_EventHandler
{
public:
  bz_Plugin *plugin;
  virtual ~bz_EventHandler() { plugin = NULL; }
  virtual void process ( bz_EventData *eventData )
  {
    if (plugin)
      plugin->Event(eventData);
  }

  std::vector<bz_eEventType> HandledEvents;

  bool HasEvent( bz_eEventType evt)
  {
	  return std::find(HandledEvents.begin(),HandledEvents.end(),evt) != HandledEvents.end();
  }

  void AddEvent( bz_eEventType evt )
  {
	  if (std::find(HandledEvents.begin(),HandledEvents.end(),evt) == HandledEvents.end())
		  HandledEvents.push_back(evt);
  }

  void RemoveEvent( bz_eEventType evt )
  {
	  std::vector<bz_eEventType>::iterator itr = std::find(HandledEvents.begin(),HandledEvents.end(),evt);

	  if ( itr!= HandledEvents.end())
		  HandledEvents.erase(itr);
  }
};

typedef std::vector<bz_EventHandler*> tvEventList;

class WorldEventManager
{
public:
  WorldEventManager();
  ~WorldEventManager();

  void addEvent ( bz_eEventType eventType, bz_EventHandler* theEvent );
  void removeEvent ( bz_eEventType eventType, bz_EventHandler* theEvent );
  bool removeHandler ( bz_EventHandler* theEvent );

  void callEvents ( bz_eEventType eventType, bz_EventData	*eventData );
  void callEvents ( bz_EventData	*eventData );

private:
 tvEventList eventList;

protected:

  void processPending();

  bool callignEvents;

  tvEventList pendingAdds;
  tvEventList pendingRemovals;
};

extern WorldEventManager	worldEventManager;

bool RegisterEvent ( bz_eEventType eventType, bz_Plugin* plugin );
bool RemoveEvent ( bz_eEventType eventType, bz_Plugin* plugin );
bool FlushEvents(bz_Plugin* plugin);

#endif // WORLD_EVENT_MANAGER_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
