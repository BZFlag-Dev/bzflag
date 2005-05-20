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

// all the exported functions for bzfs plugins

#ifndef _BZFS_API_H_
#define _BZFS_API_H_

#include <vector>
#include <string>

#ifdef _WIN32
	#ifdef INSIDE_BZ
		#define BZF_API __declspec( dllexport )
	#else
		#define BZF_API __declspec( dllimport )
	#endif
#else
	#define BZF_API
#endif

// versioning
BZF_API int bz_APIVersion ( void );

// event stuff

typedef enum
{
	bz_eNullEvent = 0,
	bz_eCaptureEvent,
	bz_ePlayerDieEvent,
	bz_ePlayerSpawnEvent,
	bz_eZoneEntryEvent,
	bz_eZoneExitEvent,
	bz_ePlayerJoinEvent,
	bz_ePlayerPartEvent,
	bz_eChatMessageEvent
}bz_teEventType;

#define BZ_ALL_USERS	-1
#define BZ_RED_TEAM		1
#define BZ_GREEN_TEAM	2
#define BZ_BLUE_TEAM	3
#define BZ_PURPLE_TEAM	4
#define BZ_ROUGE_TEAM	0
#define BZ_RABBIT_TEAM	5
#define BZ_HUNTER_TEAM	6
#define BZ_OBSERVERs	7

#define BZ_SERVER		-2

#define std_bzStr(v) std::sring(v.c_str())

// because you can't export templates to a new module ( DLL )
class BZF_API bz_String
{
public:
	bz_String();
	bz_String(const char* c);
	~bz_String();

	const char* c_str();
	void set ( const char* c);
protected:
	void	*data;
};

class BZF_API bz_StringList
{
public:
	bz_StringList();
	~bz_StringList();

	unsigned int size ( void );
	const char* get ( int i );
	void push ( const char* c );
	void clear ( void );
protected:
	void	*data;
};

// event data types
class bz_EventData
{
public:
	bz_EventData(){eventType = bz_eNullEvent;}
	virtual ~bz_EventData(){};

	bz_teEventType	eventType;
};

class bz_CTFCaptureEventData : public bz_EventData
{
public:
	bz_CTFCaptureEventData()
	{
		eventType = bz_eCaptureEvent;
		teamCAped = -1;
		teamCaping = -1;
		playerCaping = -1;
	}
	virtual ~bz_CTFCaptureEventData(){};

	int teamCAped;
	int	teamCaping;
	int playerCaping;

	float pos[3];
	float rot;
	double time;
};

class bz_PlayerDieEventData : public bz_EventData
{
public:
	bz_PlayerDieEventData()
	{
		eventType = bz_ePlayerDieEvent;
		playerID = -1;
		teamID = -1;
		killerID = -1;
		killerTeamID = -1;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}
	virtual ~bz_PlayerDieEventData(){};

	int playerID;
	int teamID;
	int killerID;
	int killerTeamID;
	std::string flagKilledWith;

	float pos[3];
	float rot;
	double time;
};

class bz_PlayerSpawnEventData : public bz_EventData
{
public:
	bz_PlayerSpawnEventData()
	{
		eventType = bz_ePlayerSpawnEvent;
		playerID = -1;
		teamID = -1;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}

	virtual ~bz_PlayerSpawnEventData(){};

	int playerID;
	int teamID;

	float pos[3];
	float rot;
	double time;
};

class bz_ChatEventData : public bz_EventData
{
public:
	bz_ChatEventData()
	{
		eventType = bz_eChatMessageEvent;

		from = -1;
		to = -1;
		time = 0.0;
	}

	virtual ~bz_ChatEventData(){};

	int from;
	int to;

	std::string message;
	double time;
};

class bz_PlayerJoinPartEventData : public bz_EventData
{
public:
	bz_PlayerJoinPartEventData()
	{
		eventType = bz_ePlayerJoinEvent;

		playerID = -1;
		teamID = -1;
		time = 0.0;
	}
	virtual ~bz_PlayerJoinPartEventData(){};

	int playerID;
	int teamID;

	std::string callsign;
	std::string reason;
	double time;
};

// event handaler callback
class bz_EventHandaler
{
public:
	virtual ~bz_EventHandaler(){};
	virtual void process ( bz_EventData *eventData ) = 0;
	virtual bool autoDelete ( void ) { return false; }	// only set this to true if you are internal to the bzfs module ( on windows )
};

BZF_API bool bz_registerEvent ( bz_teEventType eventType, int team, bz_EventHandaler* eventHandaler );

// user info API
class BZF_API bz_PlayerRecord
{
public:
	bz_PlayerRecord();
	~bz_PlayerRecord();

	void update ( void );	// call to update with current data

	int playerID;
	bz_String callsign;
	int team;

	float pos[3];
	float rot;

	bz_String ipAddress;

	bz_String currentFlag;
	bz_StringList flagHistory;

	bool spawned;
	bool verified;
	bool globalUser;
	bool admin;
	bz_StringList groups;
};

BZF_API bool bz_getPlayerByIndex ( int index, bz_PlayerRecord *playerRecord );

// message API
BZF_API bool bz_sendTextMessage (int from, int to, const char* message);

// world weapons
BZF_API bool bz_fireWorldWep ( std::string flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, int shotID , float dt );

// time API
BZF_API double bz_getCurrentTime ( void );

// info
BZF_API double bz_getBZDBDouble ( const char* variable );

// loging
BZF_API void bz_debugMessage ( int debugLevel, const char* message );

// admin
BZF_API bool bz_kickUser ( int playerIndex, const char* reason, bool notify );
BZF_API bool bz_IPBanUser ( int playerIndex, const char* ip, int time, const char* reason );


#endif //_BZFS_API_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
