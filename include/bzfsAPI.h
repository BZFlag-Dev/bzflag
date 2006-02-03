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

// all the exported functions for bzfs plugins

#ifndef _BZFS_API_H_
#define _BZFS_API_H_

/* system interface headers */
#include <string>
#include <vector>

#ifdef _WIN32
	#ifdef INSIDE_BZ
		#define BZF_API __declspec( dllexport )
	#else
		#define BZF_API __declspec( dllimport )
	#endif
	#define BZF_PLUGIN_CALL
	#ifndef strcasecmp
		#define strcasecmp stricmp
	#endif
#else
	#define BZF_API
	#define BZF_PLUGIN_CALL extern "C"
#endif

#define BZ_API_VERSION	1

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
	bz_eUnknownSlashCommand,
	bz_eGetPlayerSpawnPosEvent,
	bz_eGetAutoTeamEvent,
	bz_eAllowPlayer,
	bz_eTickEvent,
	bz_eGenerateWorldEvent,
	bz_eGetPlayerInfoEvent,
	bz_eAllowSpawn,
	bz_eListServerUpdateEvent,
	bz_eBanEvent,
	bz_eHostBanEvent,
	bz_eKickEvent,
	bz_eKillEvent,
	bz_ePlayerPausedEvent,
	bz_eMessagFilteredEvent,
	bz_eGameStartEvent,
	bz_eGameEndEvent,
	bz_eSlashCommandEvent,
	bz_ePlayerAuthEvent,
	bz_eServerMsgEvent,
	bz_eLastEvent    //this is never used as an event, just show it's the last one
}bz_eEventType;

// permision #defines
#define bz_perm_actionMessage  "actionMessage"
#define bz_perm_adminMessageReceive  "adminMessageReceive"
#define bz_perm_adminMessageSend  "adminMessageSend"
#define bz_perm_antiban   "antiban"
#define bz_perm_antideregister   "antideregister"
#define bz_perm_antikick   "antikick"
#define bz_perm_antikill   "antikill"
#define bz_perm_antipoll   "antipoll"
#define bz_perm_antipollban   "antipollban"
#define bz_perm_antipollkick   "antipollkick"
#define bz_perm_antipollkill   "antipollkill"
#define bz_perm_ban  "ban"
#define bz_perm_banlist  "banlist"
#define bz_perm_countdown  "countdown"
#define bz_perm_date  "date"
#define bz_perm_endGame  "endGame"
#define bz_perm_flagHistory  "flagHistory"
#define bz_perm_flagMod  "flagMod"
#define bz_perm_hideAdmin  "hideAdmin"
#define bz_perm_idleStats  "idleStats"
#define bz_perm_info  "info"
#define bz_perm_kick  "kick"
#define bz_perm_kill  "kill"
#define bz_perm_lagStats  "lagStats"
#define bz_perm_lagwarn  "lagwarn"
#define bz_perm_listPerms  "listPerms"
#define bz_perm_masterBan  "masterban"
#define bz_perm_mute  "mute"
#define bz_perm_playerList  "playerList"
#define bz_perm_poll  "poll"
#define bz_perm_pollBan  "pollBan"
#define bz_perm_pollKick  "pollKick"
#define bz_perm_pollKill  "pollKill"
#define bz_perm_pollSet  "pollSet"
#define bz_perm_pollFlagReset  "pollFlagReset"
#define bz_perm_privateMessage  "privateMessage"
#define bz_perm_record  "record"
#define bz_perm_rejoin  "rejoin"
#define bz_perm_removePerms  "removePerms"
#define bz_perm_replay  "replay"
#define bz_perm_requireIdentify  "requireIdentify"
#define bz_perm_say  "say"
#define bz_perm_sendHelp  "sendHelp"
#define bz_perm_setAll  "setAll"
#define bz_perm_setPassword  "setPassword"
#define bz_perm_setPerms  "setPerms"
#define bz_perm_setVar  "setVar"
#define bz_perm_showOthers  "showOthers"
#define bz_perm_shortBan  "shortBan"
#define bz_perm_shutdownServer  "shutdownServer"
#define bz_perm_spawn  "spawn"
#define bz_perm_superKill  "superKill"
#define bz_perm_talk  "talk"
#define bz_perm_unban  "unban"
#define bz_perm_unmute  "unmute"
#define bz_perm_veto  "veto"
#define bz_perm_viewReports  "viewReports"
#define bz_perm_vote  "vote"


typedef enum
{
	eNoTeam = -1,
	eRogueTeam = 0,
	eRedTeam,
	eGreenTeam,
	eBlueTeam,
	ePurpleTeam,
	eRabbitTeam,
	eHunterTeam,
	eObservers,
	eAdministrators
}bz_eTeamType;

#define BZ_SERVER		-2
#define BZ_ALLUSERS		-1
#define BZ_NULLUSER		-3

#define BZ_BZDBPERM_NA		0
#define BZ_BZDBPERM_USER	1
#define BZ_BZDBPERM_SERVER	2
#define BZ_BZDBPERM_CLIENT	3

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

	unsigned int size ( void ) const;

	const char* c_str(void) const;

	void format(const char* fmt, ...);

	void replaceAll ( const char* target, const char* with );

	void tolower ( void );
	void toupper ( void );

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

	 void tokenize ( const char* in, const char* delims, int maxTokens = 0, bool useQuotes = false);
 protected:
	 class dataBlob;

	 dataBlob *data;
 };

 BZF_API bzAPIStringList* bz_newStringList ( void );
 BZF_API void bz_deleteStringList( bzAPIStringList * l );

// event data types
class BZF_API bz_EventData
{
public:
	bz_EventData()
	{
		version = 1;
		eventType = bz_eNullEvent;
	}
	virtual ~bz_EventData(){};
	virtual void update (){};

	int				version;
	bz_eEventType	eventType;
};

class BZF_API bz_CTFCaptureEventData_V1 : public bz_EventData
{
public:
	bz_CTFCaptureEventData_V1() : bz_EventData()
	{
		eventType = bz_eCaptureEvent;
		teamCapped = eNoTeam;
		teamCapping = eNoTeam;
		playerCapping = -1;
	}
	virtual ~bz_CTFCaptureEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	bz_eTeamType teamCapped;
	bz_eTeamType teamCapping;
	int playerCapping;

	float pos[3];
	float rot;
	double time;
};

class BZF_API bz_PlayerDieEventData_V1 : public bz_EventData
{
public:
	bz_PlayerDieEventData_V1() : bz_EventData()
	{
		eventType = bz_ePlayerDieEvent;
		playerID = -1;
		team = eNoTeam;
		killerID = -1;
		killerTeam = eNoTeam;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}
	virtual ~bz_PlayerDieEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;
	int killerID;
	bz_eTeamType killerTeam;
	bzApiString flagKilledWith;

	float pos[3];
	float rot;
	double time;
};

class BZF_API bz_PlayerSpawnEventData_V1 : public bz_EventData
{
public:
	bz_PlayerSpawnEventData_V1() : bz_EventData()
	{
		eventType = bz_ePlayerSpawnEvent;
		playerID = -1;
		team = eNoTeam;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}

	virtual ~bz_PlayerSpawnEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;

	float pos[3];
	float rot;
	double time;
};

class BZF_API bz_ChatEventData_V1 : public bz_EventData
{
public:
	bz_ChatEventData_V1() : bz_EventData()
	{
		eventType = bz_eChatMessageEvent;

		from = -1;
		to = -1;
		time = 0.0;
		team = eNoTeam;
	}

	virtual ~bz_ChatEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int from;
	int to;
	bz_eTeamType	team;
	bzApiString message;

	double time;
};

class BZF_API bz_PlayerJoinPartEventData_V1 : public bz_EventData
{
public:
	bz_PlayerJoinPartEventData_V1() : bz_EventData()
	{
		eventType = bz_ePlayerJoinEvent;

		playerID = -1;
		team = eNoTeam;
		time = 0.0;
	}
	virtual ~bz_PlayerJoinPartEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;

	bzApiString callsign;
	bzApiString email;
	bool verified;
	bzApiString globalUser;
	bzApiString ipAddress;
	bzApiString reason;

	double time;
};

class BZF_API bz_UnknownSlashCommandEventData_V1 : public bz_EventData
{
public:
	bz_UnknownSlashCommandEventData_V1() : bz_EventData()
	{
		eventType = bz_eUnknownSlashCommand;
		from = -1;
		handled = false;
		time = 0;
	}

	virtual ~bz_UnknownSlashCommandEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int from;

	bool handled;
	bzApiString message;

	double time;
};

class BZF_API bz_GetPlayerSpawnPosEventData_V1 : public bz_EventData
{
public:
	bz_GetPlayerSpawnPosEventData_V1() : bz_EventData()
	{
		eventType = bz_eGetPlayerSpawnPosEvent;
		playerID = -1;
		team = eNoTeam;

		handled = false;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}

	virtual ~bz_GetPlayerSpawnPosEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;

	bool handled;

	float pos[3];
	float rot;
	double time;
};

class BZF_API bz_AllowPlayerEventData_V1 : public bz_EventData
{
public:
	bz_AllowPlayerEventData_V1() : bz_EventData()
	{
		eventType = bz_eAllowPlayer;
		playerID = -1;
		allow = true;
		time = 0.0;
	}

	virtual ~bz_AllowPlayerEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bzApiString callsign;
	bzApiString ipAddress;

	bzApiString reason;

	bool allow;

	double time;
};

class BZF_API bz_TickEventData_V1 : public bz_EventData
{
public:
	bz_TickEventData_V1() : bz_EventData()
	{
		eventType = bz_eTickEvent;
		time = 0.0;
	}
	virtual ~bz_TickEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	double time;
};

class BZF_API bz_GenerateWorldEventData_V1 : public bz_EventData
{
public:
	bz_GenerateWorldEventData_V1() : bz_EventData()
	{
		eventType = bz_eGenerateWorldEvent;
		handled = false;
		ctf = false;
		time = 0.0;
	}
	virtual ~bz_GenerateWorldEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	bool handled;
	bool ctf;

	double time;
};

class BZF_API bz_GetPlayerInfoEventData_V1 : public bz_EventData
{
public:
	bz_GetPlayerInfoEventData_V1() : bz_EventData()
	{
		eventType = bz_eGetPlayerInfoEvent;
		playerID = -1;
		team = eNoTeam;
		admin = false;
		verified = false;
		registered = false;
		time = 0.0;
	}
	virtual ~bz_GetPlayerInfoEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bzApiString callsign;
	bzApiString ipAddress;

	bz_eTeamType team;

	bool admin;
	bool verified;
	bool registered;
	double time;
};

class BZF_API bz_GetAutoTeamEventData_V1 : public bz_EventData
{
public:
	bz_GetAutoTeamEventData_V1() : bz_EventData()
	{
		playeID = -1;
		team = eNoTeam;
		handled = false;
	}
	virtual ~bz_GetAutoTeamEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playeID;
	bzApiString callsign;
	bz_eTeamType team;

	bool handled;
};

class BZF_API bz_AllowSpawnData_V1 : public bz_EventData
{
public:
	bz_AllowSpawnData_V1() : bz_EventData()
	{
		eventType = bz_eAllowSpawn;
		playerID = -1;
		team = eNoTeam;

		handled = false;
		allow = true;

		time = 0.0;
	}

	virtual ~bz_AllowSpawnData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;

	bool handled;
	bool allow;
	double time;
};

class BZF_API bz_ListServerUpdateEvent_V1 : public bz_EventData
{
public:
	bz_ListServerUpdateEvent_V1() : bz_EventData()
	{
		eventType = bz_eListServerUpdateEvent;
		handled = false;
		time = 0.0;
	}

	virtual ~bz_ListServerUpdateEvent_V1(){};
	virtual void update (){bz_EventData::update();}

	bzApiString		address;
	bzApiString		description;
	bzApiString		groups;

	bool handled;
	double time;
};

class BZF_API bz_BanEventData_V1 : public bz_EventData
{
public:
	bz_BanEventData_V1() : bz_EventData()
	{
		eventType = bz_eBanEvent;
		bannerID = -1;
		banneeID = -1;
		duration = -1;
	}
	virtual ~bz_BanEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int bannerID;
	int banneeID;
	int duration;
	bzApiString ipAddress;
	bzApiString reason;
};

class BZF_API bz_HostBanEventData_V1 : public bz_EventData
{
public:
	bz_HostBanEventData_V1() : bz_EventData()
	{
		eventType = bz_eHostBanEvent;
		bannerID = -1;
		duration = -1;
	}
	virtual ~bz_HostBanEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int bannerID;
	int duration;
	bzApiString hostPattern;
	bzApiString reason;
};

class BZF_API bz_KickEventData_V1 : public bz_EventData
{
public:
	bz_KickEventData_V1() : bz_EventData()
	{
		eventType = bz_eKickEvent;
		kickerID = -1;
		kickedID = -1;
	}
	virtual ~bz_KickEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int kickerID;
	int kickedID;
	bzApiString reason;
};

class BZF_API bz_KillEventData_V1 : public bz_EventData
{
public:
	bz_KillEventData_V1()
	{
		eventType = bz_eKillEvent;
		killerID = -1;
		killedID = -1;
	}
	virtual ~bz_KillEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int killerID;
	int killedID;
	bzApiString reason;
};

class BZF_API bz_PlayerPausedEventData_V1 : public bz_EventData
{
public:
	bz_PlayerPausedEventData_V1() : bz_EventData()
	{
		eventType = bz_ePlayerPausedEvent;
		player = -1;
		time = 0.0;
	}
	virtual ~bz_PlayerPausedEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int player;
	double time;
};

class BZF_API bz_MessageFilteredEventData_V1 : public bz_EventData
{
public:
	bz_MessageFilteredEventData_V1() : bz_EventData()
	{
		eventType = bz_eMessagFilteredEvent;
		player = -1;
		time = 0.0;
	}
	virtual ~bz_MessageFilteredEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int player;
	double time;

	bzApiString		rawMessage;
	bzApiString		filteredMessage;
};

class BZF_API bz_GameStartEndEventData_V1 : public bz_EventData
{
public:
	bz_GameStartEndEventData_V1() : bz_EventData()
	{
		eventType = bz_eGameStartEvent;
		time = 0.0;
		duration = 0.0;
	}
	virtual ~bz_GameStartEndEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	double time;
	double duration;
};

class BZF_API bz_SlashCommandEventData_V1 : public bz_EventData
{
public:
	bz_SlashCommandEventData_V1() : bz_EventData()
	{
		eventType = bz_eSlashCommandEvent;
		from = -1;
		time = 0;
	}

	virtual ~bz_SlashCommandEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int from;

	bzApiString message;

	double time;
};


class BZF_API bz_PlayerAuthEventData_V1 : public bz_EventData
{
public:
	bz_PlayerAuthEventData_V1() : bz_EventData()
	{
		eventType = bz_ePlayerAuthEvent;
		playerID = -1;
	}

	virtual ~bz_PlayerAuthEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
};

class BZF_API bz_ServerMsgEventData_V1 : public bz_EventData
{
public:
	bz_ServerMsgEventData_V1() : bz_EventData()
	{
		eventType = bz_eServerMsgEvent;

		to = -1;
		time = 0.0;
		team = eNoTeam;
	}

	virtual ~bz_ServerMsgEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int to;
	bz_eTeamType team;
	bzApiString message;

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

BZF_API bool bz_registerEvent ( bz_eEventType eventType, bz_EventHandler* eventHandler );
BZF_API bool bz_removeEvent ( bz_eEventType eventType, bz_EventHandler* eventHandler );

// player info

class bz_BasePlayerRecord;

BZF_API bool bz_getPlayerIndexList ( bzAPIIntList *playerList );
BZF_API bz_BasePlayerRecord *bz_getPlayerByIndex ( int index );
BZF_API bool bz_updatePlayerData ( bz_BasePlayerRecord *playerRecord );
BZF_API bool bz_hasPerm ( int playerID, const char* perm );
BZF_API bool bz_grantPerm ( int playerID, const char* perm );
BZF_API bool bz_revokePerm ( int playerID, const char* perm );
BZF_API bool bz_freePlayerRecord ( bz_BasePlayerRecord *playerRecord );

class BZF_API bz_BasePlayerRecord
{
public:
	bz_BasePlayerRecord()
	{
		playerID = -1;
		team = eNoTeam;

		pos[0] = pos[1] = pos[2] = 0;
		rot = 0;

		spawned = false;
		verified = false;
		globalUser = false;
		admin = false;
		op=false;

		lag = 0;

		wins = 0;
		losses = 0;
		version = 1;
	};

	~bz_BasePlayerRecord(){};

	void update ( void ){bz_updatePlayerData(this);}	// call to update with current data

	bool hasPerm ( const char* perm ){return bz_hasPerm(playerID,perm);}
	bool grantPerm ( const char* perm ){return bz_grantPerm(playerID,perm);}
	bool revokePerm ( const char* perm ){return bz_revokePerm(playerID,perm);}

	int version;
	int playerID;
	bzApiString callsign;
	bzApiString email;

	bz_eTeamType team;

	float pos[3];
	float rot;

	bzApiString ipAddress;

	bzApiString currentFlag;
	bzAPIStringList flagHistory;

	bool spawned;
	bool verified;
	bool globalUser;
	bool admin;
	bool op;
	bzAPIStringList groups;

	int lag;

	int wins;
	int losses;
	int teamKills;
};

BZF_API bool bz_setPlayerOperator (int playerId);

// player score
BZF_API bool bz_setPlayerWins (int playerId, int wins);
BZF_API bool bz_setPlayerLosses (int playerId, int losses);
BZF_API bool bz_setPlayerTKs (int playerId, int tks);

BZF_API bool bz_resetPlayerScore(int playerId);

// groups API
BZF_API bzAPIStringList* bz_getGroupList ( void );
BZF_API bzAPIStringList* bz_getGroupPerms ( const char* group );
BZF_API bool bz_groupAllowPerm ( const char* group, const char* perm );

// message API
BZF_API bool bz_sendTextMessage (int from, int to, const char* message);
BZF_API bool bz_sendTextMessage (int from, bz_eTeamType to, const char* message);
BZF_API bool bz_sendTextMessagef(int from, int to, const char* fmt, ...);
BZF_API bool bz_sendTextMessagef(int from, bz_eTeamType to, const char* fmt, ...);
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

BZF_API int bz_getBZDBItemPerms( const char* variable );
BZF_API bool bz_getBZDBItemPesistent( const char* variable );
BZF_API bool bz_BZDBItemExists( const char* variable );

BZF_API bool bz_setBZDBDouble ( const char* variable, double val, int perms = 0, bool persistent = false );
BZF_API bool bz_setBZDBString( const char* variable, const char *val, int perms = 0, bool persistent = false  );
BZF_API bool bz_setBZDBBool( const char* variable, bool val, int perms = 0, bool persistent = false  );
BZF_API bool bz_setBZDBInt( const char* variable, int val, int perms = 0, bool persistent = false  );

BZF_API int bz_getBZDBVarList( bzAPIStringList	*varList );

// logging
BZF_API void bz_debugMessage ( int debugLevel, const char* message );
BZF_API void bz_debugMessagef( int debugLevel, const char* fmt, ... );
BZF_API int bz_getDebugLevel ( void );

// admin
BZF_API bool bz_kickUser ( int playerIndex, const char* reason, bool notify );
BZF_API bool bz_IPBanUser ( int playerIndex, const char* ip, int time, const char* reason );
BZF_API bool bz_IPUnbanUser ( const char* ip );
BZF_API std::vector<std::string> bz_getReports( void );

// lagwarn
BZF_API int bz_getLagWarn( void );
BZF_API bool bz_setLagWarn( int lagwarn );

// polls
BZF_API bool bz_pollVeto( void );

// help
BZF_API const std::vector<std::string> &bz_getHelpTopics( void );
BZF_API const std::vector<std::string> *bz_getHelpTopic( std::string name );

// custom commands

class bz_CustomSlashCommandHandler
{
public:
	virtual ~bz_CustomSlashCommandHandler(){};
	virtual bool handle ( int playerID, bzApiString command, bzApiString message, bzAPIStringList *params ) = 0;

};

BZF_API bool bz_registerCustomSlashCommand ( const char* command, bz_CustomSlashCommandHandler *handler );
BZF_API bool bz_removeCustomSlashCommand ( const char* command );

// spawning
BZF_API bool bz_getStandardSpawn ( int playeID, float pos[3], float *rot );

// dying
BZF_API bool bz_killPlayer ( int playeID, bool spawnOnBase, int killerID = -1, const char* flagID = NULL );

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
BZF_API bool bz_addWorldBase( float *pos, float rot, float* scale, bz_eTeamType team, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldTeleporter ( float *pos, float rot, float* scale, float border, bz_WorldObjectOptions options );
BZF_API bool bz_addWorldWaterLevel( float level, bz_MaterialInfo *material );
BZF_API bool bz_addWorldWeapon( const char* flagType, float *pos, float rot, float tilt, float initDelay, bzAPIFloatList &delays );

BZF_API bool bz_setWorldSize( float size, float wallHeight = -1.0 );

// custom map objects

typedef struct bz_CustomMapObjectInfo
{
	bzApiString name;
	bzAPIStringList data;
}bz_CustomMapObjectInfo;

class bz_CustomMapObjectHandler
{
public:
	virtual ~bz_CustomMapObjectHandler(){};
	virtual bool handle ( bzApiString object, bz_CustomMapObjectInfo *data ) = 0;

};

BZF_API bool bz_registerCustomMapObject ( const char* object, bz_CustomMapObjectHandler *handler );
BZF_API bool bz_removeCustomMapObject ( const char* object );


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
BZF_API int bz_getTeamCount (bz_eTeamType team );
BZF_API int bz_getTeamScore (bz_eTeamType team );
BZF_API int bz_getTeamWins (bz_eTeamType team );
BZF_API int bz_getTeamLosses (bz_eTeamType team );

BZF_API void bz_setTeamWins (bz_eTeamType team, int wins );
BZF_API void bz_setTeamLosses (bz_eTeamType team, int losses );

BZF_API void bz_resetTeamScore (bz_eTeamType team );

// list server
BZF_API void bz_updateListServer ( void );

// url API
class bz_BaseURLHandler
{
public:
	bz_BaseURLHandler(){version = 1;}
	virtual ~bz_BaseURLHandler(){};
	virtual void done ( const char* URL, void * data, unsigned int size, bool complete ) = 0;
	virtual void timeout ( const char* /*URL*/, int /*errorCode*/ ){};
	virtual void error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ ){};

protected:
	int version;
};

BZF_API bool bz_addURLJob ( const char* URL, bz_BaseURLHandler* handler = NULL, const char* postData = NULL );
BZF_API bool bz_removeURLJob ( const char* URL );
BZF_API bool bz_stopAllURLJobs ( void );

// inter plugin communication
BZF_API bool bz_clipFieldExists ( const char *name );
BZF_API const char* bz_getclipFieldString ( const char *name );
BZF_API float bz_getclipFieldFloat ( const char *name );
BZF_API int bz_getclipFieldInt( const char *name );

BZF_API bool bz_setclipFieldString ( const char *name, const char* data );
BZF_API bool bz_setclipFieldFloat ( const char *name, float data );
BZF_API bool bz_setclipFieldInt( const char *name, int data );

// path checks
BZF_API bzApiString bz_filterPath ( const char* path );

// Record-Replay
BZF_API bool bz_saveRecBuf( const char * _filename, int seconds);

// cheap Text Utils
BZF_API const char *bz_format(const char* fmt, ...);
BZF_API const char *bz_toupper(const char* val );
BZF_API const char *bz_tolower(const char* val );

// server control
BZF_API void bz_shutdown();
BZF_API void bz_superkill();
BZF_API void bz_gameOver(int,int = -1);

// info about the world
BZF_API bz_eTeamType bz_checkBaseAtPoint ( float pos[3] );

// server side player API

typedef struct 
{
	int index;
	char type[2];
	int status;
	int endurance;
	int owner;
	float position[3];
	float launchPosition[3];
	float landingPosition[3];
	float flightTime;
	float flightEnd;
	float initialVelocity;
}bz_FlagUpdateRecord;

class bz_ServerSidePlayerHandler
{
public:
	virtual ~bz_ServerSidePlayerHandler(){};

	virtual void removed ( void ) = 0;
	virtual void playerRemoved ( int playerID ) = 0;

	virtual void flagUpdate ( int count, bz_FlagUpdateRecord** flagList );
};

int bz_addServerSidePlayer ( bz_ServerSidePlayerHandler *handler );
bool bz_removeServerSidePlayer ( int player, bz_ServerSidePlayerHandler *handler ); // you have to pass int he handler to ensure you "own" the player

#endif //_BZFS_API_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
