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

#define BZ_API_SHORT_STR 32
#define BZ_API_MAX_STR 512
#define BZ_API_MAX_LIST	256

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
	char flagKilledWith[BZ_API_SHORT_STR];

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

	char message[BZ_API_MAX_STR];
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

	char callsign[BZ_API_SHORT_STR];
	char reason[BZ_API_MAX_STR];
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
	char message[BZ_API_MAX_STR];
	double time;
};

class  bz_GetPlayerSpawnPosEventData : public bz_EventData
{
public:
	bz_GetPlayerSpawnPosEventData()
	{
		eventType = bz_eGetPlayerSpawnPosEvent;
		playerID = -1;
		teamID = -1;

		handled = false;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}

	virtual ~bz_GetPlayerSpawnPosEventData(){};

	int playerID;
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
	char callsign[BZ_API_SHORT_STR];
	char ipAddress[BZ_API_SHORT_STR];

	char reason[BZ_API_MAX_STR];
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
		registered = false;
		time = 0.0;
	}
	virtual ~bz_GetPlayerInfoEventData(){};

	int playerID;
	char callsign[BZ_API_SHORT_STR];
	char ipAddress[BZ_API_SHORT_STR];
	int team;

	bool admin;
	bool verified;
	bool registered;
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

BZF_API bool bz_getPlayerIndexCount ( int *count );
BZF_API bool bz_getPlayerIndexList ( int *playerList );
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
	char callsign[BZ_API_SHORT_STR];
	int team;

	float pos[3];
	float rot;

	char ipAddress[BZ_API_SHORT_STR];

	char currentFlag[BZ_API_SHORT_STR];
	char flagHistory[BZ_API_SHORT_STR][BZ_API_MAX_LIST];

	bool spawned;
	bool verified;
	bool globalUser;
	bool admin;
	char groups[BZ_API_SHORT_STR][BZ_API_MAX_LIST];

	int wins;
	int losses;
};

// message API
BZF_API bool bz_sendTextMessage (int from, int to, const char* message);

// world weapons
BZF_API bool bz_fireWorldWep ( const char* flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, int shotID , float dt );

// time API
BZF_API double bz_getCurrentTime ( void );
BZF_API float bz_getMaxWaitTime ( void );
BZF_API void bz_setMaxWaitTime ( float time );

// info
BZF_API bool bz_getBZDBDouble ( const char* variable, double *value );
BZF_API bool bz_getBZDString( const char* variable, char *value );
BZF_API bool bz_getBZDBool( const char* variable, bool *value );
BZF_API bool bz_getBZDInt( const char* variable, bool *value );

// loging
BZF_API bool bz_debugMessage ( int debugLevel, const char* message );

// admin
BZF_API bool bz_kickUser ( int playerIndex, const char* reason, bool notify );
BZF_API bool bz_IPBanUser ( int playerIndex, const char* ip, int time, const char* reason );

// custom commands

class bz_CustomSlashCommandHandler
{
public:
	virtual ~bz_CustomSlashCommandHandler(){};
	virtual bool handle ( int playerID, const char* command, const char* message ) = 0;
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
	char name[BZ_API_MAX_STR];

	typedef struct 
	{
		char		texture[BZ_API_MAX_STR];
		bool		useAlpha;
		bool		useColorOnTexture;
		bool		useSphereMap;
		int			combineMode;
	}bz_MaterialTexture;

	int			numTextures;
	bz_MaterialInfo::bz_MaterialTexture textures[BZ_API_MAX_LIST];

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
BZF_API bool bz_addWorldWeapon( const char* flagType, float *pos, float rot, float tilt, float initDelay, float *delays, int delaySize );

BZF_API bool bz_setWorldSize( float size, float wallHeight = -1.0 );

// public server info
BZF_API bool bz_getPublic( bool *value );
BZF_API bool bz_getPublicAddr( char *value );
BZF_API bool bz_getPublicDescription( char *value );

// custom client sounds
BZF_API bool bz_sendPlayCustomLocalSound ( int playerID, const char* soundName );


#endif //_BZFS_API_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
