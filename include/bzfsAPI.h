/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
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

/* DO NOT INCLUDE ANY OTHER HEADERS IN THIS FILE */
/* PLUGINS NEED TO BE BUILT WITHOUT THE BZ SOURCE TREE */
/* JUST THIS ONE FILE */

#ifdef _WIN32
	#ifdef INSIDE_BZ
		#define BZF_API __declspec( dllexport )
	#else
		#define BZF_API __declspec( dllimport )
	#endif
	#define BZF_PLUGIN_CALL
	#ifndef strcasecmp
	    #if defined(_MSC_VER) && (_MSC_VER >= 1400)
		#define strcasecmp _stricmp
	    #else
		#define strcasecmp stricmp
	    #endif
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
	bz_eRawChatMessageEvent,		// before filter
	bz_eFilteredChatMessageEvent,	// after filter
	bz_eUnknownSlashCommand,
	bz_eGetPlayerSpawnPosEvent,
	bz_eGetAutoTeamEvent,
	bz_eAllowPlayer,
	bz_eTickEvent,
	bz_eGetWorldEvent,
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
	bz_eShotFiredEvent,
	bz_eAnointRabbitEvent,
	bz_eNewRabbitEvent,
	bz_eReloadEvent,
	bz_ePlayerUpdateEvent,
	bz_eNetDataSendEvent,
	bz_eNetDataReceveEvent,
	bz_eLogingEvent,
    bz_eFlagTransferredEvent,
	bz_eFlagGrabbedEvent,
	bz_eFlagDroppedEvent,
	bz_eShotEndedEvent,
	bz_eLastEvent    //this is never used as an event, just show it's the last one
}bz_eEventType;

// permision #defines
#define bz_perm_actionMessage  "actionMessage"
#define bz_perm_adminMessageReceive  "adminMessageReceive"
#define bz_perm_adminMessageSend  "adminMessageSend"
#define bz_perm_antiban   "antiban"
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
	eAutomaticTeam = -2,
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

typedef enum
{
	eTeamFFAGame = 0,
	eClassicCTFGame,
	eRabbitGame,
	eOpenFFAGame
}bz_eGameType;

typedef enum
{
	eNoShot = 0,
	eStandardShot,
	eGMShot,
	eLaserShot,
	eThiefShot,
	eSuperShot,
	ePhantomShot,
	eShockWaveShot,
	eRicoShot,
	eMachineGunShot,
	eInvisibleShot,
	eRapidFireShot,
	eLastShotType
}bz_eShotType;

//utility classes
class BZF_API bz_ApiString
{
public:
	bz_ApiString();
	bz_ApiString(const char* c);
	bz_ApiString(const std::string &s);
	bz_ApiString(const bz_ApiString &r);

	~bz_ApiString();

	bz_ApiString& operator = ( const bz_ApiString& r );
	bz_ApiString& operator = ( const std::string& r );
	bz_ApiString& operator = ( const char* r );

	bool operator == ( const bz_ApiString&r );
	bool operator == ( const std::string& r );
	bool operator == ( const char* r );

	bool operator != ( const bz_ApiString&r );
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

 class BZF_API bz_APIIntList
 {
 public:
	 bz_APIIntList();
	 bz_APIIntList(const bz_APIIntList	&r);
	 bz_APIIntList(const std::vector<int>	&r);

	 ~bz_APIIntList();

	 void push_back ( int value );
	 int get ( unsigned int i );

	 const int& operator[] (unsigned int i) const;
	 bz_APIIntList& operator = ( const bz_APIIntList& r );
	 bz_APIIntList& operator = ( const std::vector<int>& r );

	 unsigned int size ( void );
	 void clear ( void );

 protected:
	 class dataBlob;

	 dataBlob *data;
 };

 BZF_API bz_APIIntList* bz_newIntList ( void );
 BZF_API void bz_deleteIntList( bz_APIIntList * l );

 class BZF_API bz_APIFloatList
 {
 public:
	 bz_APIFloatList();
	 bz_APIFloatList(const bz_APIFloatList	&r);
	 bz_APIFloatList(const std::vector<float>	&r);

	 ~bz_APIFloatList();

	 void push_back ( float value );
	 float get ( unsigned int i );

	 const float& operator[] (unsigned int i) const;
	 bz_APIFloatList& operator = ( const bz_APIFloatList& r );
	 bz_APIFloatList& operator = ( const std::vector<float>& r );

	 unsigned int size ( void );
	 void clear ( void );

 protected:
	 class dataBlob;

	 dataBlob *data;
 };

 BZF_API bz_APIFloatList* bz_newFloatList ( void );
 BZF_API void bz_deleteFloatList( bz_APIFloatList * l );

 class BZF_API bz_APIStringList
 {
 public:
	 bz_APIStringList();
	 bz_APIStringList(const bz_APIStringList	&r);
	 bz_APIStringList(const std::vector<std::string>	&r);

	 ~bz_APIStringList();

	 void push_back ( const bz_ApiString &value );
	 void push_back ( const std::string &value );
	 bz_ApiString get ( unsigned int i );

	 const bz_ApiString& operator[] (unsigned int i) const;
	 bz_APIStringList& operator = ( const bz_APIStringList& r );
	 bz_APIStringList& operator = ( const std::vector<std::string>& r );

	 unsigned int size ( void );
	 void clear ( void );

	 void tokenize ( const char* in, const char* delims, int maxTokens = 0, bool useQuotes = false);
 protected:
	 class dataBlob;

	 dataBlob *data;
 };

 BZF_API bz_APIStringList* bz_newStringList ( void );
 BZF_API void bz_deleteStringList( bz_APIStringList * l );

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
	double eventTime;
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
		eventTime = 0.0;
	}
	virtual ~bz_PlayerDieEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;
	int killerID;
	bz_eTeamType killerTeam;
	bz_ApiString flagKilledWith;
	int shotID;

	float pos[3];
	float rot;
	double eventTime;
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
		eventTime = 0.0;
	}

	virtual ~bz_PlayerSpawnEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;

	float pos[3];
	float rot;
	double eventTime;
};

class BZF_API bz_ChatEventData_V1 : public bz_EventData
{
public:
	bz_ChatEventData_V1() : bz_EventData()
	{
		eventType = bz_eRawChatMessageEvent;

		from = -1;
		to = -1;
		eventTime = 0.0;
		team = eNoTeam;
	}

	virtual ~bz_ChatEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int from;
	int to;
	bz_eTeamType	team;
	bz_ApiString message;

	double eventTime;
};

class BZF_API bz_PlayerJoinPartEventData_V1 : public bz_EventData
{
public:
	bz_PlayerJoinPartEventData_V1() : bz_EventData()
	{
		eventType = bz_ePlayerJoinEvent;

		playerID = -1;
		team = eNoTeam;
		eventTime = 0.0;
	}
	virtual ~bz_PlayerJoinPartEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;

	bz_ApiString callsign;
	bz_ApiString email;
	bool verified;
	bz_ApiString globalUser;
	bz_ApiString ipAddress;
	bz_ApiString reason;

	double eventTime;
};

class BZF_API bz_UnknownSlashCommandEventData_V1 : public bz_EventData
{
public:
	bz_UnknownSlashCommandEventData_V1() : bz_EventData()
	{
		eventType = bz_eUnknownSlashCommand;
		from = -1;
		handled = false;
		eventTime = 0;
	}

	virtual ~bz_UnknownSlashCommandEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int from;

	bool handled;
	bz_ApiString message;

	double eventTime;
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
		eventTime = 0.0;
	}

	virtual ~bz_GetPlayerSpawnPosEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;

	bool handled;

	float pos[3];
	float rot;
	double eventTime;
};

class BZF_API bz_AllowPlayerEventData_V1 : public bz_EventData
{
public:
	bz_AllowPlayerEventData_V1() : bz_EventData()
	{
		eventType = bz_eAllowPlayer;
		playerID = -1;
		allow = true;
		eventTime = 0.0;
	}

	virtual ~bz_AllowPlayerEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_ApiString callsign;
	bz_ApiString ipAddress;

	bz_ApiString reason;

	bool allow;

	double eventTime;
};

class BZF_API bz_TickEventData_V1 : public bz_EventData
{
public:
	bz_TickEventData_V1() : bz_EventData()
	{
		eventType = bz_eTickEvent;
		eventTime = 0.0;
	}
	virtual ~bz_TickEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	double eventTime;
};

class BZF_API bz_GetWorldEventData_V1 : public bz_EventData
{
public:
	bz_GetWorldEventData_V1() : bz_EventData()
	{
		eventType = bz_eGetWorldEvent;
		generated = false;
		openFFA = rabbit = ctf = false;
		eventTime = 0.0;
	}
	virtual ~bz_GetWorldEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	bool generated;
	bool ctf;
	bool rabbit;
	bool openFFA;

	bz_ApiString	worldFile;

	double eventTime;
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
		eventTime = 0.0;
	}
	virtual ~bz_GetPlayerInfoEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_ApiString callsign;
	bz_ApiString ipAddress;

	bz_eTeamType team;

	bool admin;
	bool verified;
	bool registered;
	double eventTime;
};

class BZF_API bz_GetAutoTeamEventData_V1 : public bz_EventData
{
public:
	bz_GetAutoTeamEventData_V1() : bz_EventData()
	{
		eventType = bz_eGetAutoTeamEvent;
		playeID = -1;
		team = eNoTeam;
		handled = false;
	}
	virtual ~bz_GetAutoTeamEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playeID;
	bz_ApiString callsign;
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

		eventTime = 0.0;
	}

	virtual ~bz_AllowSpawnData_V1(){};
	virtual void update (){bz_EventData::update();}

	int playerID;
	bz_eTeamType team;

	bool handled;
	bool allow;
	double eventTime;
};

class BZF_API bz_ListServerUpdateEvent_V1 : public bz_EventData
{
public:
	bz_ListServerUpdateEvent_V1() : bz_EventData()
	{
		eventType = bz_eListServerUpdateEvent;
		handled = false;
		eventTime = 0.0;
	}

	virtual ~bz_ListServerUpdateEvent_V1(){};
	virtual void update (){bz_EventData::update();}

	bz_ApiString		address;
	bz_ApiString		description;
	bz_ApiString		groups;

	bool handled;
	double eventTime;
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
	bz_ApiString ipAddress;
	bz_ApiString reason;
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
	bz_ApiString hostPattern;
	bz_ApiString reason;
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
	bz_ApiString reason;
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
	bz_ApiString reason;
};

class BZF_API bz_PlayerPausedEventData_V1 : public bz_EventData
{
public:
	bz_PlayerPausedEventData_V1() : bz_EventData()
	{
		eventType = bz_ePlayerPausedEvent;
		player = -1;
		eventTime = 0.0;
		pause = false;
	}
	virtual ~bz_PlayerPausedEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int player;
	double eventTime;
	bool pause;
};

class BZF_API bz_MessageFilteredEventData_V1 : public bz_EventData
{
public:
	bz_MessageFilteredEventData_V1() : bz_EventData()
	{
		eventType = bz_eMessagFilteredEvent;
		player = -1;
		eventTime = 0.0;
	}
	virtual ~bz_MessageFilteredEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int player;
	double eventTime;

	bz_ApiString		rawMessage;
	bz_ApiString		filteredMessage;
};

class BZF_API bz_GameStartEndEventData_V1 : public bz_EventData
{
public:
	bz_GameStartEndEventData_V1() : bz_EventData()
	{
		eventType = bz_eGameStartEvent;
		eventTime = 0.0;
		duration = 0.0;
	}
	virtual ~bz_GameStartEndEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	double eventTime;
	double duration;
};

class BZF_API bz_SlashCommandEventData_V1 : public bz_EventData
{
public:
	bz_SlashCommandEventData_V1() : bz_EventData()
	{
		eventType = bz_eSlashCommandEvent;
		from = -1;
		eventTime = 0;
	}

	virtual ~bz_SlashCommandEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int from;

	bz_ApiString message;

	double eventTime;
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
		eventTime = 0.0;
		team = eNoTeam;
	}

	virtual ~bz_ServerMsgEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int to;
	bz_eTeamType team;
	bz_ApiString message;

	double eventTime;
};

class bz_ShotFiredEventData_V1 : public bz_EventData
{
public:
	bz_ShotFiredEventData_V1() : bz_EventData()
	{
		eventType = bz_eShotFiredEvent;
		pos[0] = pos[1] = pos[2] = 0;
		changed = false;
	}

	virtual ~bz_ShotFiredEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	bool		changed;
	float		pos[3];
	bz_ApiString	type;
	int				player;
};

class bz_AnointRabbitEventData_V1 : public bz_EventData
{
public:
	bz_AnointRabbitEventData_V1() : bz_EventData()
	{
		eventType = bz_eAnointRabbitEvent;
		newRabbit = -1;
		swap = true;
	}

	virtual ~bz_AnointRabbitEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int newRabbit;
	bool	swap;
};

class bz_NewRabbitEventData_V1 : public bz_EventData
{
public:
	bz_NewRabbitEventData_V1() : bz_EventData()
	{
		eventType = bz_eNewRabbitEvent;
		newRabbit = -1;
	}

	virtual ~bz_NewRabbitEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int newRabbit;
};

class bz_ReloadEventData_V1 : public bz_EventData
{
public:
	bz_ReloadEventData_V1() : bz_EventData()
	{
		eventType = bz_eReloadEvent;
		player = -1;
	}

	virtual ~bz_ReloadEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	int player;
};

class bz_PlayerUpdateEventData_V1 : public bz_EventData
{
public:
	bz_PlayerUpdateEventData_V1()
	{
		eventType = bz_ePlayerUpdateEvent;
		pos[0] = pos[1] = pos[2] = 0;
		velocity[0] = velocity[1] = velocity[2] = 0;
		azimuth = angVel = 0.0f;
		phydrv = 0;
		eventTime = 0;
		player = -1;
	}

	virtual ~bz_PlayerUpdateEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	float	pos[3];
	float	velocity[3];
	float	azimuth;
	float	angVel;
	int		phydrv;
	int		player;

	double eventTime;
};

class bz_NetTransferEventData_V1 : public bz_EventData
{
public:
	bz_NetTransferEventData_V1()
	{
		eventType = bz_eNetDataReceveEvent;
		send = false;
		udp = false;
		iSize = 0;
		data = NULL;

		eventTime = 0;
	}

	virtual ~bz_NetTransferEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	bool send;
	bool udp;
	unsigned int iSize;

	double eventTime;

	// DON'T CHANGE THIS!!!
	unsigned char* data;
};

class bz_LogingEventData_V1 : public bz_EventData
{
public:
	bz_LogingEventData_V1()
	{
		eventType = bz_eLogingEvent;
		level = 0;
		eventTime = 0;
	}

	virtual ~bz_LogingEventData_V1(){};
	virtual void update (){bz_EventData::update();}

	double eventTime;
	int level;
	bz_ApiString message;
};

class bz_FlagTransferredEventData_V1 : public bz_EventData
{
public:
  enum Action {
    ContinueSteal = 0,
    CancelSteal = 1,
    DropThief = 2
  };

  bz_FlagTransferredEventData_V1()
  {
    eventType = bz_eFlagTransferredEvent;
    fromPlayerID = 0;
    toPlayerID = 0;
    flagType = NULL;
    action = ContinueSteal;
  }

  virtual ~bz_FlagTransferredEventData_V1(){};

  int fromPlayerID;
  int toPlayerID;
  const char *flagType;
  enum Action action;
};

class bz_FlagGrabbedEventData_V1 : public bz_EventData
{
public:

	bz_FlagGrabbedEventData_V1()
	{
		eventType = bz_eFlagGrabbedEvent;
		playerID = -1;
		flagID = -1;
		shotType = eNoShot;
		position[0] = position[1] = position[2] = 0;
	}

	virtual ~bz_FlagGrabbedEventData_V1(){};

	int playerID;
	int flagID;

	bz_eShotType shotType;

	const char *flagType;
	float	position[3];
};

class bz_FlagDroppedEvenData_V1 : public bz_EventData
{
public:

	bz_FlagDroppedEvenData_V1()
	{
		eventType = bz_eFlagDroppedEvent;
		playerID = -1;
		flagID = -1;
		position[0] = position[1] = position[2] = 0;
	}

	virtual ~bz_FlagDroppedEvenData_V1(){};

	int playerID;
	int flagID;

	const char *flagType;
	float	position[3];
};


class bz_ShotEndedEventData_V1 : public bz_EventData
{
public:

	bz_ShotEndedEventData_V1()
	{
		eventType = bz_eShotEndedEvent;
		playerID = -1;
		shotID = -1;
		exlpode = false;
	}

	virtual ~bz_ShotEndedEventData_V1(){};

	int playerID;
	int shotID;
	bool exlpode;
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

BZF_API bool bz_getPlayerIndexList ( bz_APIIntList *playerList );
BZF_API bz_APIIntList *bz_getPlayerIndexList ( void );
BZF_API bz_BasePlayerRecord *bz_getPlayerByIndex ( int index );
BZF_API bool bz_updatePlayerData ( bz_BasePlayerRecord *playerRecord );
BZF_API bool bz_hasPerm ( int playerID, const char* perm );
BZF_API bool bz_grantPerm ( int playerID, const char* perm );
BZF_API bool bz_revokePerm ( int playerID, const char* perm );
BZF_API bool bz_freePlayerRecord ( bz_BasePlayerRecord *playerRecord );

BZF_API const char* bz_getPlayerFlag( int playerID );

BZF_API bool bz_isPlayerPaused( int playerID );

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
		jitter = 0;
		packetloss = 0;

		wins = 0;
		losses = 0;
		version = 1;
		bzID = "";
	};

	~bz_BasePlayerRecord(){};

	void update ( void ){bz_updatePlayerData(this);}	// call to update with current data

	bool hasPerm ( const char* perm ){return bz_hasPerm(playerID,perm);}
	bool grantPerm ( const char* perm ){return bz_grantPerm(playerID,perm);}
	bool revokePerm ( const char* perm ){return bz_revokePerm(playerID,perm);}

	int version;
	int playerID;
	bz_ApiString callsign;
	bz_ApiString email;

	bz_eTeamType team;

	float pos[3];
	float rot;

	bz_ApiString ipAddress;

	bz_ApiString currentFlag;
	bz_APIStringList flagHistory;

	bool spawned;
	bool verified;
	bool globalUser;
	bz_ApiString bzID;
	bool admin;
	bool op;
	bz_APIStringList groups;

	int lag;
	int jitter;
	float packetloss;

	int wins;
	int losses;
	int teamKills;
};

BZF_API bool bz_setPlayerOperator ( int playerId );

// team info
BZF_API unsigned int bz_getTeamPlayerLimit ( bz_eTeamType team );

// player score
BZF_API bool bz_setPlayerWins (int playerId, int wins);
BZF_API bool bz_setPlayerLosses (int playerId, int losses);
BZF_API bool bz_setPlayerTKs (int playerId, int tks);

BZF_API bool bz_resetPlayerScore(int playerId);

// player shots
BZF_API bool bz_setPlayerShotType( int playerId, bz_eShotType shotType );

// player lag info
BZF_API int bz_getPlayerLag( int playerId );
BZF_API int bz_getPlayerJitter( int playerId );
BZF_API float bz_getPlayerPacketloss( int playerId );

// groups API
BZF_API bz_APIStringList* bz_getGroupList ( void );
BZF_API bz_APIStringList* bz_getGroupPerms ( const char* group );
BZF_API bool bz_groupAllowPerm ( const char* group, const char* perm );

// message API
BZF_API bool bz_sendTextMessage (int from, int to, const char* message);
BZF_API bool bz_sendTextMessage (int from, bz_eTeamType to, const char* message);
BZF_API bool bz_sendTextMessagef(int from, int to, const char* fmt, ...);
BZF_API bool bz_sendTextMessagef(int from, bz_eTeamType to, const char* fmt, ...);
BZF_API bool bz_sentFetchResMessage ( int playerID,  const char* URL );

// world weapons
BZF_API bool bz_fireWorldWep ( const char* flagType, float lifetime, float *pos, float tilt, float direction, int shotID , float dt );
BZF_API int bz_fireWorldGM ( int targetPlayerID, float lifetime, float *pos, float tilt, float direction, float dt);

// time API
BZF_API double bz_getCurrentTime ( void );
BZF_API float bz_getMaxWaitTime ( void );
BZF_API void bz_setMaxWaitTime ( float maxTime );

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
BZF_API bz_ApiString bz_getBZDBString( const char* variable );
BZF_API bool bz_getBZDBBool( const char* variable );
BZF_API int bz_getBZDBInt( const char* variable );

BZF_API int bz_getBZDBItemPerms( const char* variable );
BZF_API bool bz_getBZDBItemPesistent( const char* variable );
BZF_API bool bz_BZDBItemExists( const char* variable );

BZF_API bool bz_setBZDBDouble ( const char* variable, double val, int perms = 0, bool persistent = false );
BZF_API bool bz_setBZDBString( const char* variable, const char *val, int perms = 0, bool persistent = false  );
BZF_API bool bz_setBZDBBool( const char* variable, bool val, int perms = 0, bool persistent = false  );
BZF_API bool bz_setBZDBInt( const char* variable, int val, int perms = 0, bool persistent = false  );

BZF_API int bz_getBZDBVarList( bz_APIStringList	*varList );

// logging
BZF_API void bz_debugMessage ( int debugLevel, const char* message );
BZF_API void bz_debugMessagef( int debugLevel, const char* fmt, ... );
BZF_API int bz_getDebugLevel ( void );

// admin
BZF_API bool bz_kickUser ( int playerIndex, const char* reason, bool notify );
BZF_API bool bz_IPBanUser ( int playerIndex, const char* ip, int durration, const char* reason );
BZF_API bool bz_IPUnbanUser ( const char* ip );
BZF_API bz_APIStringList* bz_getReports( void );

// lagwarn
BZF_API int bz_getLagWarn( void );
BZF_API bool bz_setLagWarn( int lagwarn );

// timelimit
BZF_API bool bz_setTimeLimit( float timeLimit );
BZF_API float bz_getTimeLimit( void );
BZF_API bool bz_isTimeManualStart( void );

// countdown
BZF_API bool bz_isCountDownActive( void );
BZF_API bool bz_isCountDownInProgress( void );

// polls
BZF_API bool bz_pollVeto( void );

// help
BZF_API bz_APIStringList* bz_getHelpTopics( void );
BZF_API bz_APIStringList* bz_getHelpTopic( const char* name );

// custom commands

class bz_CustomSlashCommandHandler
{
public:
	virtual ~bz_CustomSlashCommandHandler(){};
	virtual bool handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params ) = 0;
	virtual const char* help ( bz_ApiString /* command */ ){return NULL;}
};

BZF_API bool bz_registerCustomSlashCommand ( const char* command, bz_CustomSlashCommandHandler *handler );
BZF_API bool bz_removeCustomSlashCommand ( const char* command );

// spawning
BZF_API bool bz_getStandardSpawn ( int playeID, float pos[3], float *rot );

// dying
BZF_API bool bz_killPlayer ( int playeID, bool spawnOnBase, int killerID = -1, const char* flagID = NULL );

// flags
BZF_API bool bz_givePlayerFlag ( int playeID, const char* flagType, bool force );
BZF_API bool bz_removePlayerFlag ( int playeID );
BZF_API void bz_resetFlags ( bool onlyUnused );

BZF_API unsigned int bz_getNumFlags( void );
BZF_API const bz_ApiString bz_getName( int flag );
BZF_API bool bz_resetFlag ( int flag );
BZF_API int bz_flagPlayer ( int flag );
BZF_API bool bz_getFlagPosition ( int flag, float* pos );
BZF_API bool bz_moveFlag ( int flag, float pos[3], bool reset = true );


// world
typedef struct
{
	bool	driveThru;
	bool	shootThru;
}bz_WorldObjectOptions;

typedef struct
{
	bz_ApiString		texture;
	bool		useAlpha;
	bool		useColorOnTexture;
	bool		useSphereMap;
	int			combineMode;
}bz_MaterialTexture;

class BZF_API bz_APITextureList
{
public:
	bz_APITextureList();
	bz_APITextureList(const bz_APITextureList	&r);

	~bz_APITextureList();

	void push_back ( bz_MaterialTexture &value );
	bz_MaterialTexture get ( unsigned int i );

	const bz_MaterialTexture& operator[] (unsigned int i) const;
	bz_APITextureList& operator = ( const bz_APITextureList& r );

	unsigned int size ( void );
	void clear ( void );

protected:
	class dataBlob;

	dataBlob *data;
};

typedef struct bz_MaterialInfo
{
	bz_ApiString name;
	bz_APITextureList textures;

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
BZF_API bool bz_addWorldWeapon( const char* flagType, float *pos, float rot, float tilt, float initDelay, bz_APIFloatList &delays );

BZF_API bool bz_setWorldSize( float size, float wallHeight = -1.0 );
BZF_API void bz_setClientWorldDowloadURL( const char* URL );
BZF_API const bz_ApiString bz_getClientWorldDowloadURL( void );
BZF_API bool bz_saveWorldCacheFile( const char* file );

// custom map objects

typedef struct bz_CustomMapObjectInfo
{
	bz_ApiString name;
	bz_APIStringList data;
}bz_CustomMapObjectInfo;

class bz_CustomMapObjectHandler
{
public:
	virtual ~bz_CustomMapObjectHandler(){};
	virtual bool handle ( bz_ApiString object, bz_CustomMapObjectInfo *data ) = 0;

};

BZF_API bool bz_registerCustomMapObject ( const char* object, bz_CustomMapObjectHandler *handler );
BZF_API bool bz_removeCustomMapObject ( const char* object );


// info about the world
typedef enum
{
	eNullObject,
	eSolidObject,
	eTeleporterField,
	eWorldWeapon
}bz_eWorldObjectType;

class BZF_API bz_APIBaseWorldObject
{
public:
	bz_APIBaseWorldObject(){type = eNullObject;}
	virtual ~bz_APIBaseWorldObject(){};
	bz_eWorldObjectType type;
};

class BZF_API bz_APISolidWorldObject_V1 : public bz_APIBaseWorldObject
{
public:
	bz_APISolidWorldObject_V1();
	virtual ~bz_APISolidWorldObject_V1();

	float center[3];
	float maxAABBox[3];
	float minAABBox[3];
	float rotation[3];
	float maxBBox[3];
	float minBBox[3];

	void update ( void );

	class		dataBlob;
	dataBlob	*data;
};

class BZF_API bz_APITeleporterField_V1 : public bz_APIBaseWorldObject
{
public:
	bz_APITeleporterField_V1(){};
	virtual ~bz_APITeleporterField_V1(){};

	float center[3];
	float maxAABBox[3];
	float minAABBox[3];
	float rotation[3];
	float maxBBox[3];
	float minBBox[3];

	bz_ApiString name;
	bz_APIStringList targets[2];
};

class BZF_API bz_APIWorldObjectList
{
public:
	bz_APIWorldObjectList();
	bz_APIWorldObjectList(const bz_APIWorldObjectList &r);
	~bz_APIWorldObjectList();
	void push_back ( bz_APIBaseWorldObject *value );
	bz_APIBaseWorldObject *get ( unsigned int i );
	const bz_APIBaseWorldObject* operator[] (unsigned int i) const;
	bz_APIWorldObjectList& operator = ( const bz_APIWorldObjectList& r );
	unsigned int size ( void );
	void clear ( void );

protected:
	class dataBlob;
	dataBlob *data;
};

BZF_API void bz_getWorldSize( float *size, float *wallHeight );
BZF_API int bz_getWorldObjectCount( void );
BZF_API bz_APIWorldObjectList* bz_getWorldObjectList( void );
BZF_API void bz_releaseWorldObjectList( bz_APIWorldObjectList* list );

// collision methods
typedef enum
{
	eNoCol,
	eInSolid,
	eInBase,
	eInTP
}bz_eAPIColType;

// these realy need to return the object they came from
bz_eAPIColType bz_cylinderInMapObject ( float pos[3], float height, float radius, bz_APIBaseWorldObject **object );
bz_eAPIColType bz_boxInMapObject ( float pos[3], float size[3], float angle, bz_APIBaseWorldObject **object );

void bz_freeWorldObjectPtr ( bz_APIBaseWorldObject *ptr );

// public server info
BZF_API bool bz_getPublic( void );
BZF_API bz_ApiString bz_getPublicAddr( void );
BZF_API bz_ApiString bz_getPublicDescription( void );

// custom client sounds
BZF_API bool bz_sendPlayCustomLocalSound ( int playerID, const char* soundName );

class bz_APIPluginHandler
{
public:
	virtual ~bz_APIPluginHandler(){};
	virtual bool handle ( bz_ApiString plugin, bz_ApiString param ) = 0;
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
BZF_API void bz_resetTeamScores ( void );

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
BZF_API bz_ApiString bz_filterPath ( const char* path );

// Record-Replay
BZF_API bool bz_saveRecBuf( const char * _filename, int seconds);
BZF_API bool bz_startRecBuf( void );
BZF_API bool bz_stopRecBuf( void );

// cheap Text Utils
BZF_API const char *bz_format(const char* fmt, ...);
BZF_API const char *bz_toupper(const char* val );
BZF_API const char *bz_tolower(const char* val );

// game countdown
BZF_API void bz_pauseCountdown ( const char *pausedBy );
BZF_API void bz_resumeCountdown ( const char *resumedBy );
BZF_API void bz_startCountdown ( int delay, float limit, const char *byWho );

// server control
BZF_API void bz_shutdown();
BZF_API bool bz_restart ( void );
BZF_API void bz_superkill();
BZF_API void bz_gameOver(int,int = -1);

BZF_API void bz_reloadLocalBans();
BZF_API void bz_reloadMasterBans();
BZF_API void bz_reloadGroups();
BZF_API void bz_reloadUsers();
BZF_API void bz_reloadHelp();

// rabbit control
BZF_API void bz_newRabbit( int player, bool swap );
BZF_API void bz_removeRabbit( int player );

// team control
BZF_API void bz_changeTeam( int player, bz_eTeamType team );

BZF_API bz_APIIntList* bz_getPlayerIndexList ( void );

// info about the world
BZF_API bz_eTeamType bz_checkBaseAtPoint ( float pos[3] );

// game info
BZF_API bz_eGameType bz_getGameType( void );

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

typedef struct
{
	int wins;
	int losses;
	int tks;
}bz_ScoreRecord;

typedef enum
{
	eRegularPlayer,
	eRobotPlayer,
	eObserverPlayer
}bz_ePlayerType;

typedef struct
{
	int index;
	bz_ePlayerType type;
	bz_eTeamType team;
	bz_ScoreRecord	score;
	char			callsign[32];
	char			email[128];
}bz_PlayerInfoUpdateRecord;

typedef struct
{
	int id;
	int size;
	int	wins;
	int losses;
}bz_TeamInfoRecord;

typedef enum
{
	eRejectBadRequest,
	eRejectBadTeam,
	eRejectBadType,
	eRejectBadEmail,
	eRejectTeamFull,
	eRejectServerFull,
	eRejectBadCallsign,
	eRejectRepeatCallsign,
	eRejectRejoinWaitTime,
	eRejectIPBanned,
	eRejectHostBanned,
	eRejectIDBanned
}bz_eRejectCodes;

typedef struct
{
	int player;
	int handycap;
}bz_HandycapUpdateRecord;

typedef enum
{
	eDead,		// not alive, not paused, etc.
	eAlive,				// player is alive
	ePaused,			// player is paused
	eExploding,			// currently blowing up
	eTeleporting		// teleported recently
}bz_ePlayerStatus;

typedef struct bz_PlayerUpdateState
{
	bz_ePlayerStatus	status;
	bool				falling;
	bool				crossingWall;
	float				pos[3];			// position of tank
	float				velocity[3];	// velocity of tank
	float				rotation;		// orientation of tank
	float				angVel;			// angular velocity of tank
	int					phydrv;			// physics driver

	bz_PlayerUpdateState()
	{
		status = eAlive;
		falling = false;
		crossingWall = false;
		pos[0] = pos[1] = pos[2] = 0;
		velocity[0] = velocity[1] = velocity[2] = 0;
		rotation = 0;
		angVel = 0;
		phydrv = -1;
	}
}bz_PlayerUpdateState;

typedef enum
{
	eGotKilled,
	eGotShot,
	eGotRunOver,
	eGotCaptured,
	eGenocideEffect,
	eSelfDestruct,
	eWaterDeath,
	ePhysicsDriverDeath
}bz_ePlayerDeathReason;

class BZF_API bz_ServerSidePlayerHandler
{
public:
  virtual ~bz_ServerSidePlayerHandler(){};

  // you must call setEntryData when this is called.
  virtual void added(int player) = 0;
  virtual void removed(void){};
  virtual void playerRemoved(int player);
  virtual void playerRejected(bz_eRejectCodes code, const char *reason);
  virtual void playerAccepted(void){};
  virtual void playerSpawned(int player, float pos[3], float rot);
  virtual void textMessage(int dest, int source, const char *text);
  virtual void playerKilledMessage(int victimIndex, int killerIndex,
				   bz_ePlayerDeathReason reason, int shotIndex,
				   const char *flagType, int phydrv);
  virtual void scoreLimitReached(int player, bz_eTeamType team);
  virtual void flagCaptured(int player, int flag, bz_eTeamType team);

  virtual void flagUpdate(int count, bz_FlagUpdateRecord **flagList);
  virtual void playerInfoUpdate(bz_PlayerInfoUpdateRecord *playerRecord);
  virtual void teamUpdate(int count, bz_TeamInfoRecord **teamList);
  virtual void handycapUpdate(int count,
			      bz_HandycapUpdateRecord **handycapList);
  virtual void playerIPUpdate(int player, const char *ipAddress);
  virtual void playerStateUpdate(int player, bz_PlayerUpdateState *playerState,
				 float timestamp);
  virtual void playerScoreUpdate(int player, int wins, int losses, int TKs);
  virtual void flagTransfer ( int from, int to, int flagID, bz_eShotType shotType );
  virtual void nearestFlag ( const char* flagName, float pos[3] );
  virtual void grabFlag ( int player, int flagID, const char* flagType, bz_eShotType shotType );
  virtual void setShotType ( int player, bz_eShotType shotType );
  virtual void shotFired ( int player, unsigned short shotID, bz_eShotType shotType );
  virtual void shotEnded( int player, unsigned short shotID, unsigned short reason );

  int playerID;

  // higher level functions
  virtual void spawned ( void );
  virtual bool think ( void );	// return true to kill and delete the bot;

protected:
  void setPlayerData(const char *callsign, const char *email,
		     const char *token, const char *clientVersion,
		     bz_eTeamType team);
  void joinGame(void);
  void updateState(bz_PlayerUpdateState *state);
  void dropFlag(float pos[3]);
  void sendChatMessage(const char* text, int targetPlayer = BZ_ALLUSERS);
  void sendTeamChatMessage(const char *text, bz_eTeamType targetTeam);
  void captureFlag(bz_eTeamType team);

  void setMovementInput ( float forward, float turn );
  bool fireShot ( void );
  bool jump ( void );

  void updatePhysics ( void );

  // info
  bool canJump ( void );
  bool canShoot ( void );
  bool canMove ( void );
  bz_eShotType getShotType ( void );

protected:
	typedef struct 
	{
		bool enabled;
		bz_eShotType shotType;
		float x,y,z;
		float vx,vy,vz;
		float fireTime;

		float cx,cy,cz;
	}Shot;

#define _BOT_MAX_SHOTS 100
	Shot shots[_BOT_MAX_SHOTS];

	float input[2];

	float pos[3];
	float vec[3];
	float rot;
	float rotVel;

	bool alive;

private:
	void computeVelsFromInput ( void );
};

// *** NOTE *** support for server side players in incomplete.
//              there WILL be crashes if you add one.
//              this message will be removed when the code is complete.
BZF_API int bz_addServerSidePlayer ( bz_ServerSidePlayerHandler *handler );
BZF_API bool bz_removeServerSidePlayer ( int playerID, bz_ServerSidePlayerHandler *handler ); // you have to pass in the handler to ensure you "own" the player


#endif //_BZFS_API_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
