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
	eCaptureEvent,
	ePlayerDieEvent,
	ePlayerSpawnEvent,
	eZoneEntryEvent,
	eZoneExitEvent,
	ePlayerJoinEvent,
	ePlayerPartEvent,
	eChatMessageEvent
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

	float pos[3];
	float rot;
	double time;
};

class PlayerDieEventData : public BaseEventData
{
public:
	PlayerDieEventData();
	virtual ~PlayerDieEventData();

	int playerID;
	int teamID;
	int killerID;
	int killerTeamID;
	std::string flagKilledWith;

	float pos[3];
	float rot;
	double time;
};

class PlayerSpawnEventData : public BaseEventData
{
public:
	PlayerSpawnEventData();
	virtual ~PlayerSpawnEventData();

	int playerID;
	int teamID;

	float pos[3];
	float rot;
	double time;
};

class ZoneEntryExitEventData : public BaseEventData
{
public:
	ZoneEntryExitEventData();
	virtual ~ZoneEntryExitEventData();

	int playerID;
	int teamID;

	int zoneID;

	float pos[3];
	float rot;
	double time;
};

class PlayerJoinPartEventData : public BaseEventData
{
public:
	PlayerJoinPartEventData();
	virtual ~PlayerJoinPartEventData();

	int playerID;
	int teamID;

	std::string callsign;
	std::string reason;
	double time;
};

class ChatEventData : public BaseEventData
{
public:
	ChatEventData();
	virtual ~ChatEventData();

	int from;
	int to;

	std::string message;
	double time;
};

class BaseEventHandaler
{
public:
	virtual ~BaseEventHandaler(){};
	virtual void process ( BaseEventData *eventData ) = 0;
	virtual bool autoDelete ( void ) { return true;}
};

typedef std::vector<BaseEventHandaler*> tvEventList;
typedef std::map<teEventType, tvEventList> tmEventTypeList;
typedef std::map<int, tmEventTypeList> tmEventMap;

class WorldEventManager
{
public:
	WorldEventManager();
	~WorldEventManager();

	void addEvent ( teEventType eventType, int team, BaseEventHandaler* theEvetnt );
	void removeEvent ( teEventType eventType, int team, BaseEventHandaler* theEvetnt );
	tvEventList getEventList ( teEventType eventType, int team );
	void callEvents ( teEventType eventType, int team, BaseEventData	*eventData );

	int getEventCount ( teEventType eventType, int team );
protected:
	tmEventMap eventtMap;

	tmEventTypeList* getTeamEventList ( int team );
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
