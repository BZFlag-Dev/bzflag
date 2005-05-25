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
	#define BZF_PLUGIN_CALL	
#else
	#define BZF_API extern "C"
	#define BZF_PLUGIN_CALL extern "C"
#endif

#define BZ_API_VERSION	2

#define BZ_GET_PLUGIN_VERSION BZF_PLUGIN_CALL int bz_GetVersion ( void ) { return BZ_API_VERSION;}

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
	bz_eChatMessageEvent,
	bz_eUnknownSlashCommand,		// will not take a team
	bz_eGetPlayerSpawnPosEvent,
	bz_eGetAutoTeamEvent,			// will not take a team
	bz_eAllowPlayer,			// will not take a team
	bz_eTickEvent,				// will not take a team
	bz_eGenerateWorldEvent,			// will not take a team
	bz_eGetPlayerInfoEvent			// will not take a team
}bz_teEventType;

#define BZ_ALL_USERS	-1
#define BZ_RED_TEAM		1
#define BZ_GREEN_TEAM	2
#define BZ_BLUE_TEAM	3
#define BZ_PURPLE_TEAM	4
#define BZ_ROGUE_TEAM	0
#define BZ_RABBIT_TEAM	5
#define BZ_HUNTER_TEAM	6
#define BZ_OBSERVERs	7

#define BZ_SERVER		-2

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
		teamCaped = -1;
		teamCaping = -1;
		playerCaping = -1;
	}
	virtual ~bz_CTFCaptureEventData(){};

	int teamCaped;
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

class bz_UnknownSlashCommandEventData : public bz_EventData
{
public:
	bz_UnknownSlashCommandEventData()
	{
		eventType = bz_eUnknownSlashCommand;
		from = -1;
		handled = false;
		time = 0;
	}

	virtual ~bz_UnknownSlashCommandEventData(){};

	int from;

	bool handled;
	std::string message;
	double time;
};

class  bz_GetPlayerSpawnPosEventData : public bz_EventData
{
public:
	bz_GetPlayerSpawnPosEventData()
	{
		eventType = bz_eGetPlayerSpawnPosEvent;
		playeID = -1;
		teamID = -1;

		handled = false;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}

	virtual ~bz_GetPlayerSpawnPosEventData(){};

	int playeID;
	int teamID;

	bool handled;

	float pos[3];
	float rot;
	double time;
};

class bz_AllowPlayerEventData : public bz_EventData
{
public:
	bz_AllowPlayerEventData()
	{
		eventType = bz_eAllowPlayer;
		playerID = -1;
		allow = true;
		time = 0.0;
	}
	
	virtual ~bz_AllowPlayerEventData(){};

	int playerID;
	std::string callsign;
	std::string ipAddress;

	std::string reason;
	bool allow;

	double time;
};

class bz_TickEventData : public bz_EventData
{
public:
	bz_TickEventData()
	{
		eventType = bz_eTickEvent;
		time = 0.0;
	}
	virtual ~bz_TickEventData(){};

	double time;
};

class bz_GenerateWorldEventData : public bz_EventData
{
public:
	bz_GenerateWorldEventData()
	{
		eventType = bz_eGenerateWorldEvent;
		handled = false;
		ctf = false;
		time = 0.0;
	}
	virtual ~bz_GenerateWorldEventData(){};

	bool handled;
	bool ctf;

	double time;
};

class bz_GetPlayerInfoEventData : public bz_EventData
{
public:
	bz_GetPlayerInfoEventData()
	{
		eventType = bz_eGetPlayerInfoEvent;
		playerID = -1;
		team = -1;
		admin = false;
		verified = false;
		registerd = false;
		time = 0.0;
	}
	virtual ~bz_GetPlayerInfoEventData(){};

	int playerID;
	std::string callsign;
	std::string ipAddress;
	int team;

	bool admin;
	bool verified;
	bool registerd;
	double time;
};

// event handler callback
class bz_EventHandler
{
public:
	virtual ~bz_EventHandler(){};
	virtual void process ( bz_EventData *eventData ) = 0;
	virtual bool autoDelete ( void ) { return false; }	// only set this to true if you are internal to the bzfs module ( on windows )
};

BZF_API bool bz_registerEvent ( bz_teEventType eventType, int team, bz_EventHandler* eventHandler );
BZF_API bool bz_registerGeneralEvent ( bz_teEventType eventType, bz_EventHandler* eventHandler );	// does "everyone" automaticly

BZF_API bool bz_removeEvent ( bz_teEventType eventType, int team, bz_EventHandler* eventHandler );

// player info

class bz_PlayerRecord;

BZF_API bool bz_getPlayerIndexList ( std::vector<int> *playerList );
BZF_API bool bz_getPlayerByIndex ( int index, bz_PlayerRecord *playerRecord );
BZF_API bool bz_updatePlayerData ( bz_PlayerRecord *playerRecord );

class bz_PlayerRecord
{
public:
	bz_PlayerRecord()
	{
		playerID = -1;
		team = -1;

		pos[0] = pos[1] = pos[2] = 0;
		rot = 0;

		spawned = false;
		verified = false;
		globalUser = false;
		admin = false;

		wins = 0;
		losses = 0;
	}

	~bz_PlayerRecord(){};

	void update ( void ){bz_updatePlayerData(this);}	// call to update with current data

	int playerID;
	std::string callsign;
	int team;

	float pos[3];
	float rot;

	std::string ipAddress;

	std::string currentFlag;
	std::vector<std::string> flagHistory;

	bool spawned;
	bool verified;
	bool globalUser;
	bool admin;
	std::vector<std::string> groups;

	int wins;
	int losses;
};

// message API
BZF_API bool bz_sendTextMessage (int from, int to, const char* message);

// world weapons
BZF_API bool bz_fireWorldWep ( std::string flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, int shotID , float dt );

// time API
BZF_API double bz_getCurrentTime ( void );
BZF_API float bz_getMaxWaitTime ( void );
BZF_API void bz_setMaxWaitTime ( float time );

// info
BZF_API double bz_getBZDBDouble ( const char* variable );
BZF_API std::string bz_getBZDString( const char* variable );
BZF_API bool bz_getBZDBool( const char* variable );
BZF_API int bz_getBZDInt( const char* variable );

// loging
BZF_API void bz_debugMessage ( int debugLevel, const char* message );

// admin
BZF_API bool bz_kickUser ( int playerIndex, const char* reason, bool notify );
BZF_API bool bz_IPBanUser ( int playerIndex, const char* ip, int time, const char* reason );

// custom commands

class bz_CustomSlashCommandHandler
{
public:
	virtual ~bz_CustomSlashCommandHandler(){};
	virtual bool handle ( int playerID, std::string command, std::string message ) = 0;
};

BZF_API bool bz_registerCustomSlashCommand ( const char* command, bz_CustomSlashCommandHandler *handler );
BZF_API bool bz_removeCustomSlashCommand ( const char* command );

// spawning
BZF_API bool bz_getStandardSpawn ( int playeID, float pos[3], float *rot );

// dying
BZF_API bool bz_killPlayer ( int playeID, bool spawnOnBase );

// flags
BZF_API bool bz_removePlayerFlag ( int playeID );

// world
typedef struct 
{
	bool	driveThru;
	bool	shootThru;
}bz_WorldObjectOptions;

typedef struct bz_MaterialInfo
{
	std::string name;

	typedef struct 
	{
		std::string		texture;
		bool		useAlpha;
		bool		useColorOnTexture;
		bool		useSphereMap;
		int			combineMode;
	}bz_MaterialTexture;

	std::vector<bz_MaterialInfo::bz_MaterialTexture> textures;

	float		ambient[4];
	float		diffuse[4];
	float		specular[4];
	float		emisive[4];
	float		shine;

	float		alphaThresh;
	bool		culling;
	bool		sorting;
}bz_MaterialInfo;

BZF_API bool bz_addWorldBox ( float *pos, float rot, float* scale, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldPyramid ( float *pos, float rot, float* scale, bool fliped, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldBase( float *pos, float rot, float* scale, int team, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldTeleporter ( float *pos, float rot, float* scale, float border, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldWaterLevel( float level, bz_MaterialInfo *material );
BZF_API bool bz_addWorldWeapon( std::string flagType, float *pos, float rot, float tilt, float initDelay, std::vector<float> delays );

BZF_API bool bz_setWorldSize( float size, float wallHeight = -1.0 );

// public server info
BZF_API bool bz_getPublic( void );
BZF_API std::string bz_getPublicAddr( void );
BZF_API std::string bz_getPublicDescription( void );

#endif //_BZFS_API_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
