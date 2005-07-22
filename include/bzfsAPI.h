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

#include <string>
#include <vector>

#ifdef _WIN32
	#ifdef INSIDE_BZ
		#define BZF_API __declspec( dllexport )
	#else
		#define BZF_API __declspec( dllimport )
	#endif
	#define BZF_PLUGIN_CALL	
#else
	#define BZF_API 
	#define BZF_PLUGIN_CALL extern "C"
#endif

#define BZ_API_VERSION	5

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
	bz_eAllowPlayer,				// will not take a team
	bz_eTickEvent,					// will not take a team
	bz_eGenerateWorldEvent,			// will not take a team
	bz_eGetPlayerInfoEvent,			// will not take a team
	bz_eAllowSpawn,
	bz_eListServerUpdateEvent		// will not take a team
}bz_eEventType;

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

//utility classes
 class BZF_API bzApiString
{
public:
	bzApiString();
	bzApiString(const char* c);
	bzApiString(const std::string &s);
	bzApiString(const bzApiString &r);

	~bzApiString();

	bzApiString& operator = ( const bzApiString& r );
	bzApiString& operator = ( const std::string& r );
	bzApiString& operator = ( const char* r );

	bool operator == ( const bzApiString&r );
	bool operator == ( const std::string& r );
	bool operator == ( const char* r );

	bool operator != ( const bzApiString&r );
	bool operator != ( const std::string& r );
	bool operator != ( const char* r );

	unsigned int size ( void );

	const char* c_str(void);

protected:
	class dataBlob;

	dataBlob	*data;
};

 class BZF_API bzAPIIntList
 {
 public:
	 bzAPIIntList();
	 bzAPIIntList(const bzAPIIntList	&r);
	 bzAPIIntList(const std::vector<int>	&r);

	 ~bzAPIIntList();

	 void push_back ( int value );
	 int get ( unsigned int i );

	 const int& operator[] (unsigned int i) const;
	 bzAPIIntList& operator = ( const bzAPIIntList& r );
	 bzAPIIntList& operator = ( const std::vector<int>& r );

	 unsigned int size ( void );
	 void clear ( void );

 protected:
	 class dataBlob;

	 dataBlob *data;
 };

 BZF_API bzAPIIntList* bz_newIntList ( void );
 BZF_API void bz_deleteIntList( bzAPIIntList * l );

 class BZF_API bzAPIFloatList
 {
 public:
	 bzAPIFloatList();
	 bzAPIFloatList(const bzAPIFloatList	&r);
	 bzAPIFloatList(const std::vector<float>	&r);

	 ~bzAPIFloatList();

	 void push_back ( float value );
	 float get ( unsigned int i );

	 const float& operator[] (unsigned int i) const;
	 bzAPIFloatList& operator = ( const bzAPIFloatList& r );
	 bzAPIFloatList& operator = ( const std::vector<float>& r );

	 unsigned int size ( void );
	 void clear ( void );

 protected:
	 class dataBlob;

	 dataBlob *data;
 };

 BZF_API bzAPIFloatList* bz_newFloatList ( void );
 BZF_API void bz_deleteFloatList( bzAPIFloatList * l );

 class BZF_API bzAPIStringList
 {
 public:
	 bzAPIStringList();
	 bzAPIStringList(const bzAPIStringList	&r);
	 bzAPIStringList(const std::vector<std::string>	&r);

	 ~bzAPIStringList();

	 void push_back ( const bzApiString &value );
	 void push_back ( const std::string &value );
	 bzApiString get ( unsigned int i );

	 const bzApiString& operator[] (unsigned int i) const;
	 bzAPIStringList& operator = ( const bzAPIStringList& r );
	 bzAPIStringList& operator = ( const std::vector<std::string>& r );

	 unsigned int size ( void );
	 void clear ( void );

 protected:
	 class dataBlob;

	 dataBlob *data;
 };

// event data types
class bz_EventData
{
public:
	bz_EventData(){eventType = bz_eNullEvent;}
	virtual ~bz_EventData(){};

	bz_eEventType	eventType;
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
	bzApiString flagKilledWith;

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

	bzApiString message;

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

	bzApiString callsign;
	bzApiString reason;

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
	bzApiString message;

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
	bzApiString callsign;
	bzApiString ipAddress;

	bzApiString reason;

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
	bzApiString callsign;
	bzApiString ipAddress;

	int team;

	bool admin;
	bool verified;
	bool registered;
	double time;
};

class bz_GetAutoTeamEventData : public bz_EventData
{
public:
	bz_GetAutoTeamEventData()
	{
		playeID = -1;
		teamID = -1;
		handled = false;
	}

	virtual ~bz_GetAutoTeamEventData(){};

	int playeID;
	bzApiString callsign;
	int teamID;

	bool handled;
};

class  bz_AllowSpawnData : public bz_EventData
{
public:
	bz_AllowSpawnData()
	{
		eventType = bz_eAllowSpawn;
		playerID = -1;
		teamID = -1;

		handled = false;
		allow = true;

		time = 0.0;
	}

	virtual ~bz_AllowSpawnData(){};

	int playerID;
	int teamID;

	bool handled;
	bool allow;
	double time;
};

class  bz_ListServerUpdateEvent : public bz_EventData
{
public:
	bz_ListServerUpdateEvent()
	{
		eventType = bz_eListServerUpdateEvent;
		handled = false;
		time = 0.0;
	}

	virtual ~bz_ListServerUpdateEvent(){};

	bzApiString		address;
	bzApiString		description;
	bzApiString		groups;

	bool handled;
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

BZF_API bool bz_registerEvent ( bz_eEventType eventType, int team, bz_EventHandler* eventHandler );
BZF_API bool bz_registerGeneralEvent ( bz_eEventType eventType, bz_EventHandler* eventHandler );	// does "everyone" automaticly

BZF_API bool bz_removeEvent ( bz_eEventType eventType, int team, bz_EventHandler* eventHandler );

// player info

class bz_PlayerRecord;

BZF_API bool bz_getPlayerIndexList ( bzAPIIntList *playerList );
BZF_API bz_PlayerRecord *bz_getPlayerByIndex ( int index );
BZF_API bool bz_updatePlayerData ( bz_PlayerRecord *playerRecord );
BZF_API bool bz_freePlayerRecord ( bz_PlayerRecord *playerRecord );

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
	bzApiString callsign;

	int team;

	float pos[3];
	float rot;

	bzApiString ipAddress;

	bzApiString currentFlag;
	bzAPIStringList flagHistory;

	bool spawned;
	bool verified;
	bool globalUser;
	bool admin;
	bzAPIStringList groups;

	int wins;
	int losses;
	int teamKills;
};

BZF_API bool bz_setPlayerWins (int playerId, int wins);
BZF_API bool bz_setPlayerLosses (int playerId, int losses);
BZF_API bool bz_setPlayerTKs(int playerId, int tks);
BZF_API bool bz_resetPlayerScore(int playerId);

// message API
BZF_API bool bz_sendTextMessage (int from, int to, const char* message);
BZF_API bool bz_sentFetchResMessage ( int playerID,  const char* URL );

// world weapons
BZF_API bool bz_fireWorldWep ( const char* flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, int shotID , float dt );

// time API
BZF_API double bz_getCurrentTime ( void );
BZF_API float bz_getMaxWaitTime ( void );
BZF_API void bz_setMaxWaitTime ( float time );

typedef struct 
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	bool daylightSavings;
}bz_localTime;

BZF_API void bz_getLocaltime ( bz_localTime	*ts );

// BZDB API
BZF_API double bz_getBZDBDouble ( const char* variable );
BZF_API bzApiString bz_getBZDBString( const char* variable );
BZF_API bool bz_getBZDBBool( const char* variable );
BZF_API int bz_getBZDBInt( const char* variable );

BZF_API bool bz_setBZDBDouble ( const char* variable, double val );
BZF_API bool bz_setBZDBString( const char* variable, const char *val );
BZF_API bool bz_setBZDBBool( const char* variable, bool val );
BZF_API bool bz_setBZDBInt( const char* variable, int val );

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
	virtual bool handle ( int playerID, bzApiString command, bzApiString message ) = 0;

};

BZF_API bool bz_registerCustomSlashCommand ( const char* command, bz_CustomSlashCommandHandler *handler );
BZF_API bool bz_removeCustomSlashCommand ( const char* command );

// spawning
BZF_API bool bz_getStandardSpawn ( int playeID, float pos[3], float *rot );

// dying
BZF_API bool bz_killPlayer ( int playeID, bool spawnOnBasem, int killerID = -1, const char* flagID = NULL );

// flags
BZF_API bool bz_removePlayerFlag ( int playeID );
BZF_API void bz_resetFlags ( bool onlyUnused );

// world
typedef struct 
{
	bool	driveThru;
	bool	shootThru;
}bz_WorldObjectOptions;

typedef struct 
{
	bzApiString		texture;
	bool		useAlpha;
	bool		useColorOnTexture;
	bool		useSphereMap;
	int			combineMode;
}bz_MaterialTexture;

class BZF_API bzAPITextureList
{
public:
	bzAPITextureList();
	bzAPITextureList(const bzAPITextureList	&r);

	~bzAPITextureList();

	void push_back ( bz_MaterialTexture &value );
	bz_MaterialTexture get ( unsigned int i );

	const bz_MaterialTexture& operator[] (unsigned int i) const;
	bzAPITextureList& operator = ( const bzAPITextureList& r );

	unsigned int size ( void );
	void clear ( void );

protected:
	class dataBlob;

	dataBlob *data;
};

typedef struct bz_MaterialInfo
{
	bzApiString name;
	bzAPITextureList textures;

	float		ambient[4];
	float		diffuse[4];
	float		specular[4];
	float		emisive[4];
	float		shine;

	float		alphaThresh;
	bool		culling;
	bool		sorting;
}bz_MaterialInfo;

// have bz make you a new material
bz_MaterialInfo* bz_anewMaterial ( void );
// tell bz you are done with a material
void bz_deleteMaterial ( bz_MaterialInfo *material );

BZF_API bool bz_addWorldBox ( float *pos, float rot, float* scale, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldPyramid ( float *pos, float rot, float* scale, bool fliped, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldBase( float *pos, float rot, float* scale, int team, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldTeleporter ( float *pos, float rot, float* scale, float border, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldWaterLevel( float level, bz_MaterialInfo *material );
BZF_API bool bz_addWorldWeapon( const char* flagType, float *pos, float rot, float tilt, float initDelay, bzAPIFloatList &delays );

BZF_API bool bz_setWorldSize( float size, float wallHeight = -1.0 );

// public server info
BZF_API bool bz_getPublic( void );
BZF_API bzApiString bz_getPublicAddr( void );
BZF_API bzApiString bz_getPublicDescription( void );

// custom client sounds
BZF_API bool bz_sendPlayCustomLocalSound ( int playerID, const char* soundName );

class bz_APIPluginHandler
{
public:
	virtual ~bz_APIPluginHandler(){};
	virtual bool handle ( bzApiString plugin, bzApiString param ) = 0;
};
// custom pluginHandler
BZF_API bool bz_registerCustomPluginHandler ( const char* extension, bz_APIPluginHandler * handler );
BZF_API bool bz_removeCustomPluginHandler ( const char* extension, bz_APIPluginHandler * handler );

// team info
BZF_API int bz_getTeamCount (int teamIndex );
BZF_API int bz_getTeamScore (int teamIndex );
BZF_API int bz_getTeamWins (int teamIndex );
BZF_API int bz_getTeamLosses (int teamIndex );

BZF_API void bz_setTeamWins (int teamIndex, int wins );
BZF_API void bz_setTeamLosses (int teamIndex, int losses );

BZF_API void bz_resetTeamScore (int teamIndex );

// list server 
BZF_API void bz_updateListServer ( void );

// url API
class bz_URLHandler
{
public:
	virtual ~bz_URLHandler(){};
	virtual void done ( const char* URL, void * data, unsigned int size, bool complete ) = 0;
	virtual void timeout ( const char* URL, int errorCode ){};
	virtual void error ( const char* URL, int errorCode, const char *errorString ){};
};

BZF_API bool bz_addURLJob ( const char* URL, bz_URLHandler* handler );
BZF_API bool bz_removeURLJob ( const char* URL );
BZF_API bool bz_stopAllURLJobs ( void );

// inter plugin comunication
BZF_API bool bz_clipFieldExists ( const char *name );
BZF_API const char* bz_getclipFieldString ( const char *name );
BZF_API float bz_getclipFieldFloat ( const char *name );
BZF_API int bz_getclipFieldint( const char *name );

BZF_API bool bz_setclipFieldString ( const char *name, const char* data );
BZF_API bool bz_setclipFieldFloat ( const char *name, float data );
BZF_API bool bz_setclipFieldInt( const char *name, int data );


#endif //_BZFS_API_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
