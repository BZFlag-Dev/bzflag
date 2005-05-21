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
	eChatMessageEvent,
	eUnknownSlashCommand,	// will not take a team
	eGetPlayerSpawnPosEvent,
	eGetAutoTeamEvent,		// will not take a team
	eAllowPlayer,					// will not take a team
	eTickEvent,						// will not take a team
	eGenerateWorldEvent	// will not take a team
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

	int teamCaped;
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

class UnknownSlashCommandEventData : public BaseEventData
{
public:
	UnknownSlashCommandEventData();
	virtual ~UnknownSlashCommandEventData();

	int from;

	bool handled;
	std::string message;
	double time;
};

class GetPlayerSpawnPosEventData : public BaseEventData
{
public:
	GetPlayerSpawnPosEventData();
	virtual ~GetPlayerSpawnPosEventData();

	int playeID;
	int teamID;

	bool handled;

	float pos[3];
	float rot;
	double time;
};

class GetAutoTeamEventData : public BaseEventData
{
public:
	GetAutoTeamEventData();
	virtual ~GetAutoTeamEventData();

	int playeID;
	std::string callsign;
	int teamID;

	bool handled;
};

class AllowPlayerEventData : public BaseEventData
{
public:
	AllowPlayerEventData();
	virtual ~AllowPlayerEventData();

	int playerID;
	std::string callsign;
	std::string ipAddress;

	std::string reason;
	bool allow;

	double time;
};

class TickEventData : public BaseEventData
{
public:
	TickEventData()
	{
		eventType = eTickEvent;
		time = 0.0;
	}
	virtual ~TickEventData(){};

	double time;
};

class GenerateWorldEventData : public BaseEventData
{
public:
	GenerateWorldEventData()
	{
		eventType = eGenerateWorldEvent;
		handled = false;
		ctf = false;
		time = 0.0;
	}
	virtual ~GenerateWorldEventData(){};

	bool handled;
	bool ctf;

	double time;
};

class BaseEventHandler
{
public:
	virtual ~BaseEventHandler(){};
	virtual void process ( BaseEventData *eventData ) = 0;
	virtual bool autoDelete ( void ) { return true;}
};

typedef std::vector<BaseEventHandler*> tvEventList;
typedef std::map<teEventType, tvEventList> tmEventTypeList;
typedef std::map<int, tmEventTypeList> tmEventMap;

class WorldEventManager
{
public:
	WorldEventManager();
	~WorldEventManager();

	void addEvent ( teEventType eventType, int team, BaseEventHandler* theEvetnt );
	void removeEvent ( teEventType eventType, int team, BaseEventHandler* theEvetnt );
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
