/* bzflag
* Copyright (c) 1993-2010 Tim Riker
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

#include "bzfsAPI.h"

// event handler callback
class bz_EventHandler
{
public:
  bz_Plugin *plugin;
  virtual ~bz_EventHandler(){plugin = NULL;}
  virtual void process ( bz_EventData *eventData )
  {
    if (plugin)
      plugin->Event(eventData);
  }
 // virtual bool autoDelete ( void ) { return false; }	// only set this to true if you are internal to the bzfs module ( on windows )
};

typedef std::vector<bz_EventHandler*> tvEventList;
typedef std::map<bz_eEventType, tvEventList> tmEventTypeList;

class WorldEventManager
{
public:
  WorldEventManager();
  ~WorldEventManager();

  void addEvent ( bz_eEventType eventType, bz_EventHandler* theEvent );
  void removeEvent ( bz_eEventType eventType, bz_EventHandler* theEvent );
  bool removeHandler ( bz_EventHandler* theEvent );

  tvEventList getEventList ( bz_eEventType eventType);
  void callEvents ( bz_eEventType eventType, bz_EventData	*eventData );
  void callEvents ( bz_EventData	*eventData );
  void callEvents ( const bz_EventData	&eventData ){callEvents((bz_EventData*)&eventData);}

  int getEventCount ( bz_eEventType eventType );

  tmEventTypeList eventList;
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
