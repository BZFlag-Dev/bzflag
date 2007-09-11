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
		#define strcasecmp stricmp
	#endif
#else
	#define BZF_API
	#define BZF_PLUGIN_CALL extern "C"
#endif

#define BZ_API_VERSION	16

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
	bz_ePlayerUpdateEvent,
	bz_eNetDataSendEvent,
	bz_eNetDataReceveEvent,
	bz_eLogingEvent,
	bz_eShotEndedEvent,
	bz_eFlagTransferredEvent,
	bz_eFlagGrabbedEvent,
	bz_eFlagDroppedEvent,
	bz_eFlagResetEvent,
	bz_eAllowCTFCapEvent,
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

typedef enum
{
	eFFAGame= 0,
	eCTFGame,
	eRabbitGame
}bz_eGameType;

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
	void urlEncode ( void );

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
    teamCapped = eNoTeam;
    teamCapping = eNoTeam;
    playerCapping = -1;
  }
  virtual ~bz_CTFCaptureEventData(){};

  bz_eTeamType teamCapped;
  bz_eTeamType teamCapping;
  int playerCapping;

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
		team = eNoTeam;
		killerID = -1;
		killerTeam = eNoTeam;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}
	virtual ~bz_PlayerDieEventData(){};

	int playerID;
	bz_eTeamType team;
	int killerID;
	bz_eTeamType killerTeam;
	bzApiString flagKilledWith;
        int shotID;

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
		team = eNoTeam;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}

	virtual ~bz_PlayerSpawnEventData(){};

	int playerID;
	bz_eTeamType team;

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
		team = eNoTeam;
	}

	virtual ~bz_ChatEventData(){};

	int from;
	int to;
	bz_eTeamType	team;
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
		team = eNoTeam;
		time = 0.0;
	}
	virtual ~bz_PlayerJoinPartEventData(){};

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
		team = eNoTeam;

		handled = false;

		pos[0] = pos[1] = pos[2] = 0.0f;
		rot = 0.0f;
		time = 0.0;
	}

	virtual ~bz_GetPlayerSpawnPosEventData(){};

	int playerID;
	bz_eTeamType team;

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
		eventType = bz_eGetWorldEvent;
		generated = false;
		openFFA = rabbit = ctf = false;
		eventTime = 0.0;

	}
	virtual ~bz_GenerateWorldEventData(){};
	
	bool generated;
	bool ctf;
	bool rabbit;
	bool openFFA;

	bzApiString	worldFile;

	double eventTime;
};

class bz_GetPlayerInfoEventData : public bz_EventData
{
public:
	bz_GetPlayerInfoEventData()
	{
		eventType = bz_eGetPlayerInfoEvent;
		playerID = -1;
		team = eNoTeam;
		admin = false;
		verified = false;
		registered = false;
		time = 0.0;
	}
	virtual ~bz_GetPlayerInfoEventData(){};

	int playerID;
	bzApiString callsign;
	bzApiString ipAddress;

	bz_eTeamType team;

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
		team = eNoTeam;
		handled = false;
	}

	virtual ~bz_GetAutoTeamEventData(){};

	int playeID;
	bzApiString callsign;
	bz_eTeamType team;

	bool handled;
};

class  bz_AllowSpawnData : public bz_EventData
{
public:
	bz_AllowSpawnData()
	{
		eventType = bz_eAllowSpawn;
		playerID = -1;
		team = eNoTeam;

		handled = false;
		allow = true;

		time = 0.0;
	}

	virtual ~bz_AllowSpawnData(){};

	int playerID;
	bz_eTeamType team;

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

class bz_BanEventData : public bz_EventData
{
public:
	bz_BanEventData()
	{
		eventType = bz_eBanEvent;
		bannerID = -1;
		banneeID = -1;
		duration = -1;
	}
	virtual ~bz_BanEventData(){};

	int bannerID;
	int banneeID;
	int duration;
	bzApiString ipAddress;
	bzApiString reason;
};

class bz_HostBanEventData : public bz_EventData
{
public:
	bz_HostBanEventData()
	{
		eventType = bz_eHostBanEvent;
		bannerID = -1;
		duration = -1;
	}
	virtual ~bz_HostBanEventData(){};

	int bannerID;
	int duration;
	bzApiString hostPattern;
	bzApiString reason;
};

class bz_KickEventData : public bz_EventData
{
public:
	bz_KickEventData()
	{
		eventType = bz_eKickEvent;
		kickerID = -1;
		kickedID = -1;
	}
	virtual ~bz_KickEventData(){};

	int kickerID;
	int kickedID;
	bzApiString reason;
};

class bz_KillEventData : public bz_EventData
{
public:
	bz_KillEventData()
	{
		eventType = bz_eKillEvent;
		killerID = -1;
		killedID = -1;
	}
	virtual ~bz_KillEventData(){};

	int killerID;
	int killedID;
	bzApiString reason;
};

class bz_PlayerPausedEventData : public bz_EventData
{
public:
	bz_PlayerPausedEventData()
	{
		eventType = bz_ePlayerPausedEvent;
		player = -1;
		time = 0.0;
		pause = false;
	}
	virtual ~bz_PlayerPausedEventData(){};

	int player;
	double time;
	bool pause;
};

class bz_MessageFilteredEventData : public bz_EventData
{
public:
	bz_MessageFilteredEventData()
	{
		eventType = bz_eMessagFilteredEvent;
		player = -1;
		time = 0.0;
	}
	virtual ~bz_MessageFilteredEventData(){};

	int player;
	double time;

	bzApiString		rawMessage;
	bzApiString		filteredMessage;
};

class bz_GameStartEndEventData : public bz_EventData
{
public:
	bz_GameStartEndEventData()
	{
		eventType = bz_eGameStartEvent;
		time = 0.0;
		duration = 0.0;
	}
	virtual ~bz_GameStartEndEventData(){};

	double time;
	double duration;
};

class bz_SlashCommandEventData : public bz_EventData
{
public:
	bz_SlashCommandEventData()
	{
		eventType = bz_eSlashCommandEvent;
		from = -1;
		time = 0;
	}

	virtual ~bz_SlashCommandEventData(){};

	int from;

	bzApiString message;

	double time;
};


class bz_PlayerAuthEventData : public bz_EventData
{
public:
	bz_PlayerAuthEventData()
	{
		eventType = bz_ePlayerAuthEvent;
		playerID = -1;
	}

	virtual ~bz_PlayerAuthEventData(){};

	int playerID;
};

class bz_ServerMsgEventData : public bz_EventData
{
public:
	bz_ServerMsgEventData()
	{
		eventType = bz_eServerMsgEvent;

		to = -1;
		time = 0.0;
		team = eNoTeam;
	}

	virtual ~bz_ServerMsgEventData(){};

	int to;
	bz_eTeamType team;
	bzApiString message;

	double time;
};

class bz_ShotFiredEventData : public bz_EventData
{
public:
	bz_ShotFiredEventData()
	{
		eventType = bz_eShotFiredEvent;
		pos[0] = pos[1] = pos[2] = 0;
		changed = false;
		playerID = -1;
	}

	virtual ~bz_ShotFiredEventData(){};

	bool		changed;
	float		pos[3];
	bzApiString	type;
	int		playerID;
};

class bz_PlayerUpdateEventData : public bz_EventData
{
public:
	bz_PlayerUpdateEventData()
	{
		eventType = bz_ePlayerUpdateEvent;
		pos[0] = pos[1] = pos[2] = 0;
		velocity[0] = velocity[1] = velocity[2] = 0;
		azimuth = angVel = 0.0f;
		phydrv = 0;
		time = 0;
		playerID = -1;
	}

	virtual ~bz_PlayerUpdateEventData(){};

	float	pos[3];
	float	velocity[3];
	float	azimuth;	
	float	angVel;
	int	phydrv;		
	int	playerID;

	double time;
};

class bz_NetTransferEventData : public bz_EventData
{
public:
	bz_NetTransferEventData()
	{
		eventType = bz_eNetDataReceveEvent;
		send = false;
		udp = false;
		iSize = 0;
		data = NULL;

		time = 0;
	}

	virtual ~bz_NetTransferEventData(){};

	bool send;
	bool udp;
	unsigned int iSize;

	double time;

	// DON'T CHANGE THIS!!!
	unsigned char* data;
};

class bz_LogingEventData : public bz_EventData
{
public:
	bz_LogingEventData()
	{
		eventType = bz_eLogingEvent;
		level = 0;
		time = 0;
	}

	virtual ~bz_LogingEventData(){};

	double time;
	int level;
	bzApiString message;
};

class bz_ShotEndedEventData : public bz_EventData
{
public:

	bz_ShotEndedEventData()
	{
		eventType = bz_eShotEndedEvent;
		playerID = -1;
		shotID = -1;
		explode = false;
	}

	virtual ~bz_ShotEndedEventData(){};

	int playerID;
	int shotID;
	bool explode;
};


class bz_FlagTransferredEventData : public bz_EventData
{
public:
  enum Action {
    ContinueSteal = 0,
    CancelSteal = 1,
    DropThief = 2 
  };

  bz_FlagTransferredEventData()
  {
    eventType = bz_eFlagTransferredEvent;
    fromPlayerID = 0;
    toPlayerID = 0;
    flagType = NULL;
    action = ContinueSteal;
  }
  
  virtual ~bz_FlagTransferredEventData(){};

  int fromPlayerID;
  int toPlayerID;
  const char *flagType;
  enum Action action;
};

class bz_FlagGrabbedEventData : public bz_EventData
{
public:

	bz_FlagGrabbedEventData()
	{
		eventType = bz_eFlagGrabbedEvent;
		playerID = -1;
		flagID = -1;
		pos[0] = pos[1] = pos[2] = 0;
	}

	virtual ~bz_FlagGrabbedEventData(){};

	int playerID;
	int flagID;

	const char *flagType;
	float	pos[3];
};

class bz_FlagDroppedEventData : public bz_EventData
{
public:

	bz_FlagDroppedEventData()
	{
	  eventType = bz_eFlagDroppedEvent;
	  playerID = -1;
	  flagID = -1;
	  pos[0] = pos[1] = pos[2] = 0;
	}

	virtual ~bz_FlagDroppedEventData(){};

	int playerID;
	int flagID;

	const char *flagType;
	float	pos[3];
};

class bz_FlagResetEventData : public bz_EventData
{
public:
  bz_FlagResetEventData()
  {
    eventType = bz_eFlagResetEvent;
    flagID = -1;
    pos[0] = pos[1] = pos[2] = 0;
    changed = false;
    teamIsEmpty = false;
  }

  virtual ~bz_FlagResetEventData(){};

  int flagID;
  bool teamIsEmpty;
  const char *flagType;
  bool	changed;
  float	pos[3];
};

class bz_AllowCTFCapEventData : public bz_EventData
{
public:
  bz_AllowCTFCapEventData()
  {
    eventType = bz_eAllowCTFCapEvent;
    teamCapped = eNoTeam;
    teamCapping = eNoTeam;
    playerCapping = -1;
    allow = false;
  }
  virtual ~bz_AllowCTFCapEventData(){};

  bz_eTeamType teamCapped;
  bz_eTeamType teamCapping;
  int playerCapping;

  float	    pos[3];
  float	    rot;
  double    time;
  
  bool	    allow;
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

class bz_PlayerRecord;

BZF_API bool bz_getPlayerIndexList ( bzAPIIntList *playerList );
BZF_API bz_PlayerRecord *bz_getPlayerByIndex ( int index );
BZF_API bool bz_updatePlayerData ( bz_PlayerRecord *playerRecord );
BZF_API bool bz_hasPerm ( int playerID, const char* perm );
BZF_API bool bz_grantPerm ( int playerID, const char* perm );
BZF_API bool bz_revokePerm ( int playerID, const char* perm );
BZF_API bool bz_freePlayerRecord ( bz_PlayerRecord *playerRecord );

BZF_API const char* bz_getPlayerFlag( int playerID );

BZF_API bool bz_isPlayerPaused( int playerID );

// player lag info
BZF_API int bz_getPlayerLag( int playerId );
BZF_API int bz_getPlayerJitter( int playerId );
BZF_API float bz_getPlayerPacketloss( int playerId );

class bz_PlayerRecord
{
public:
	bz_PlayerRecord()
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
		bzID = "";
	}

	~bz_PlayerRecord(){};

	void update ( void ){bz_updatePlayerData(this);}	// call to update with current data

	bool hasPerm ( const char* perm ){return bz_hasPerm(playerID,perm);}
	bool grantPerm ( const char* perm ){return bz_grantPerm(playerID,perm);}
	bool revokePerm ( const char* perm ){return bz_revokePerm(playerID,perm);}

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
        bzApiString bzID;
	bool admin;
	bool op;
	bzAPIStringList groups;

	int lag;

	int wins;
	int losses;
	int teamKills;
};

BZF_API bool bz_setPlayerOperator ( int playerId );

// team info
BZF_API unsigned int bz_getTeamPlayerLimit ( bz_eTeamType team );

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

BZF_API void bz_resetBZDBVar( const char* variable );
BZF_API void bz_resetALLBZDBVars( void );

// logging
BZF_API void bz_debugMessage ( int debugLevel, const char* message );
BZF_API void bz_debugMessagef( int debugLevel, const char* fmt, ... );
BZF_API int bz_getDebugLevel ( void );

// admin
BZF_API bool bz_kickUser ( int playerIndex, const char* reason, bool notify );
BZF_API bool bz_IPBanUser ( int playerIndex, const char* ip, int time, const char* reason );
BZF_API bool bz_IPUnbanUser ( const char* ip );
BZF_API  bzAPIStringList *bz_getReports( void );

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
BZF_API bzAPIStringList *bz_getHelpTopics( void );
BZF_API bzAPIStringList *bz_getHelpTopic( std::string name );

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
BZF_API bool bz_getStandardSpawn ( int playerID, float pos[3], float *rot );

// dying
BZF_API bool bz_killPlayer ( int playerID, bool spawnOnBase, int killerID = -1, const char* flagID = NULL );

// flags
BZF_API bool bz_givePlayerFlag ( int playerID, const char* flagType, bool force );
BZF_API bool bz_removePlayerFlag ( int playerID );
BZF_API void bz_resetFlags ( bool onlyUnused );

BZF_API unsigned int bz_getNumFlags( void );
BZF_API const bzApiString bz_getName( int flag );
BZF_API bool bz_resetFlag ( int flag );
BZF_API bool bz_moveFlag ( int flag, float pos[3] );
BZF_API int bz_flagPlayer ( int flag );
BZF_API bool bz_getFlagPosition ( int flag, float* pos );

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
BZF_API void bz_setClientWorldDowloadURL( const char* URL );
BZF_API const bzApiString bz_getClientWorldDowloadURL( void );
BZF_API bool bz_saveWorldCacheFile( const char* file );


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
BZF_API void bz_resetTeamScores ( void );

// list server
BZF_API void bz_updateListServer ( void );

// url API
class bz_URLHandler
{
public:
	virtual ~bz_URLHandler(){};
	virtual void done ( const char* URL, void * data, unsigned int size, bool complete ) = 0;
	virtual void timeout ( const char* /*URL*/, int /*errorCode*/ ){};
	virtual void error ( const char* /*URL*/, int /*errorCode*/, const char * /*errorString*/ ){};
};

BZF_API bool bz_addURLJob ( const char* URL, bz_URLHandler* handler = NULL, const char* postData = NULL );
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

class bz_ClipFiledNotifier
{
public:
  virtual ~bz_ClipFiledNotifier(){};
  virtual void fieldChange ( const char* /*field*/) = 0;
};

BZF_API bool bz_addClipFieldNotifier ( const char *name, bz_ClipFiledNotifier *cb );
BZF_API bool bz_removeClipFieldNotifier ( const char *name, bz_ClipFiledNotifier *cb );

// path checks
BZF_API bzApiString bz_filterPath ( const char* path );

// Record-Replay
BZF_API bool bz_saveRecBuf( const char * _filename, int seconds  = 0);
BZF_API bool bz_startRecBuf( void );
BZF_API bool bz_stopRecBuf( void );

// cheap Text Utils
BZF_API const char *bz_format(const char* fmt, ...);
BZF_API const char *bz_toupper(const char* val );
BZF_API const char *bz_tolower(const char* val );
BZF_API const char *bz_urlEncode(const char* val );

// game countdown
BZF_API void bz_pauseCountdown ( const char *pausedBy );
BZF_API void bz_resumeCountdown ( const char *resumedBy );
BZF_API void bz_startCountdown ( int delay, float limit, const char *byWho );

// server control
BZF_API void bz_shutdown();
BZF_API void bz_superkill();
BZF_API void bz_gameOver(int playerID, bz_eTeamType team = eNoTeam);
BZF_API bool bz_restart ( void );

BZF_API void bz_reloadLocalBans();
BZF_API void bz_reloadMasterBans();
BZF_API void bz_reloadGroups();
BZF_API void bz_reloadUsers();
BZF_API void bz_reloadHelp();

// info about the world
BZF_API bz_eTeamType bz_checkBaseAtPoint ( float pos[3] );

bz_eTeamType convertTeam ( int team );
int convertTeam( bz_eTeamType team );

// game type info
BZF_API	bz_eGameType bz_getGameType ( void  );

#endif //_BZFS_API_H_

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
