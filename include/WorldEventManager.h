/* bzflag
* Copyright (c) 1993 - 2005 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

typedef enum
{
	eNullEvent = 0,
	eCaptureEvent
}teEventType;

class BaseEventData
{
public:
	BaseEventData();
	virtual ~BaseEventData(){};

	teEventType	eventType;
};

class CTFCaptureEventData : public BaseEventData
{
public:
	CTFCaptureEventData();
	virtual ~CTFCaptureEventData();

	int teamCAped;
	int	teamCaping;
	int playerCaping;
};

class BaseEventHandaler
{
public:
	virtual ~BaseEventHandaler(){};
	virtual void process ( BaseEventData *eventData ) = 0;
};

typedef std::vector<BaseEventHandaler*> tvEventList;

class WorldEventManager
{
public:
	WorldEventManager();
	~WorldEventManager();

	void addCapEvent ( int team, BaseEventHandaler* theEvetnt );
	void removeCapEvent ( int team, BaseEventHandaler* theEvetnt );
	tvEventList getCapEventList ( int team );
	void callAllCapEvents ( int team, BaseEventData	*eventData );

protected:
	typedef std::map<int, tvEventList> tmTeamCapEventMap;
	tmTeamCapEventMap	teamCapEventMap;
};

extern WorldEventManager	worldEventManager;

#endif // WORLD_EVENT_MANAGER_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
