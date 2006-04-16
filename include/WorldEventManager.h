/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
	#define WORLD_EVENT_MANAGER_H

	#include <map>
	#include <vector>

	#include "bzfsAPI.h"

typedef std::vector < bz_EventHandler * > tvEventList;
typedef std::map < bz_eEventType, tvEventList > tmEventTypeList;

class WorldEventManager
{
public:
	WorldEventManager();
	~WorldEventManager();

	void addEvent( bz_eEventType eventType, bz_EventHandler *theEvent );
	void removeEvent( bz_eEventType eventType, bz_EventHandler *theEvent );
	tvEventList getEventList( bz_eEventType eventType );
	void callEvents( bz_eEventType eventType, bz_EventData *eventData );

	int getEventCount( bz_eEventType eventType );
protected:
	tmEventTypeList eventList;

};

extern WorldEventManager worldEventManager;

#endif // WORLD_EVENT_MANAGER_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
