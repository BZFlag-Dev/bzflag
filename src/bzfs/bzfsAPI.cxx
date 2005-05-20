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

// implementation wrapers for all the bza_ API functions
#include "bzfsAPI.h"

#include "common.h"
#include "bzfs.h"
#include "WorldWeapons.h"
#include "bzfsEvents.h"
#include "WorldEventManager.h"
#include "GameKeeper.h"
#include "FlagInfo.h"

#include "commands.h"

#define BZ_API_VERSION	1

extern void sendMessage(int playerIndex, PlayerId dstPlayer, const char *message);
extern void removePlayer(int playerIndex, const char *reason, bool notify);
extern CmdLineOptions *clOptions;


// versioning
BZF_API int bz_APIVersion ( void )
{
	return BZ_API_VERSION;
}

/*bz_String::bz_String()
{
	data = (void*) new std::string;
}

bz_String::bz_String(const char* c)
{
	data = (void*) new std::string(c);
}

bz_String::~bz_String()
{
	delete((std::string*)data);
}

const char* bz_String::c_str ( void )
{
	return ((std::string*)data)->c_str();
}

void bz_String::set ( const char* c)
{
	*((std::string*)data) = c;
}

bz_StringList::bz_StringList()
{
	data = (void*)new std::vector<std::string>;
}

bz_StringList::~bz_StringList()
{
	delete((std::vector<std::string>*)data);
}

unsigned int bz_StringList::size ( void )
{
	return ((std::vector<std::string>*)data)->size();
}

const char* bz_StringList::get ( int i )
{
	if ( i< 0 || i >= (int)size())
		return NULL;

	return (*((std::vector<std::string>*)data))[i].c_str();
}

void bz_StringList::push ( const char* c )
{
	std::string str = c;
	((std::vector<std::string>*)data)->push_back(str);
}

void bz_StringList::clear ( void )
{
	((std::vector<std::string>*)data)->clear();
}
*/

BZF_API bool bz_registerEvent ( bz_teEventType eventType, int team, bz_EventHandaler* eventHandaler )
{
	if (!eventHandaler)
		return false;
	
	worldEventManager.addEvent((teEventType)eventType,team,(BaseEventHandaler*)eventHandaler);
	return true;
}

BZF_API bool bz_updatePlayerData ( bz_PlayerRecord *playerRecord )
{
	if (!playerRecord)
		return false;

	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerRecord->playerID);
	if (!player)
		return false;

	memcpy(playerRecord->pos,player->lastState->pos,sizeof(float)*3);

	playerRecord->rot = player->lastState->azimuth;

	int flagid = player->player.getFlag();
	FlagInfo *flagInfo = FlagInfo::get(flagid);

	playerRecord->currentFlag = flagInfo->flag.type->label();

	std::vector<FlagType*>	flagHistoryList = player->flagHistory.get();

	playerRecord->flagHistory.clear();
	for ( unsigned int i = 0; i < flagHistoryList.size(); i ++)
		playerRecord->flagHistory.push_back(flagHistoryList[i]->label());

	playerRecord->groups.clear();
	playerRecord->groups = player->accessInfo.groups;

	playerRecord->admin = player->accessInfo.isVerified();
	return true;
}

BZF_API bool bz_getPlayerByIndex ( int index, bz_PlayerRecord *playerRecord )
{
	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(index);

	if (!player || !playerRecord)
		return false;

	playerRecord->callsign = player->player.getCallSign();
	playerRecord->playerID = index;
	playerRecord->team = player->player.getTeam();

	playerRecord->spawned = player->player.isAlive();
	playerRecord->verified = player->accessInfo.isVerified();
	playerRecord->globalUser = player->authentication.isGlobal();

	playerRecord->ipAddress = player->netHandler->getTargetIP();
	playerRecord->update();
	return true;
}

BZF_API bool bz_sendTextMessage(int from, int to, const char* message)
{
	if (!message)
		return false;

	int playerIndex;
	PlayerId dstPlayer;

	if (to == BZ_ALL_USERS)
		dstPlayer = AllPlayers;
	else
		dstPlayer = (PlayerId)to;

	if (from == BZ_SERVER)
		playerIndex = ServerPlayer;
	else
		playerIndex = from;

	sendMessage(playerIndex, dstPlayer, message);
	return true;
}

BZF_API bool bz_fireWorldWep ( std::string flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, int shotID, float dt )
{
	if (!pos || !flagType.size())
		return false;

	FlagTypeMap &flagMap = FlagType::getFlagMap();
	if (flagMap.find(flagType) == flagMap.end())
		return false;

	FlagType *flag = flagMap.find(flagType)->second;

	PlayerId player;
	if ( fromPlayer == BZ_SERVER )
		player = ServerPlayer;
	else
		player = fromPlayer;

	return fireWorldWep(flag,lifetime,player,pos,tilt,direction,shotID,dt) == shotID;
}

// time API
BZF_API double bz_getCurrentTime ( void )
{
	return TimeKeeper::getCurrent().getSeconds();
}

// info
BZF_API double bz_getBZDBDouble ( const char* variable )
{
	if (!variable)
		return 0.0;

	return BZDB.eval(std::string(variable));
}

// loging
BZF_API void bz_debugMessage ( int _debugLevel, const char* message )
{
	if (!message)
		return;

	if (debugLevel >= _debugLevel)
		formatDebug("%s\n",message);
}

// admin
BZF_API bool bz_kickUser ( int playerIndex, const char* reason, bool notify )
{
	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerIndex);
	if (!player || !reason)
		return false;

	removePlayer(playerIndex,reason,notify);
	return true;
}

BZF_API bool bz_IPBanUser ( int playerIndex, const char* ip, int time, const char* reason )
{
	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerIndex);
	if (!player || !reason || !ip)
		return false;

	// reload the banlist in case anyone else has added
	clOptions->acl.load();

	if (clOptions->acl.ban(ip, player->player.getCallSign(), time,reason))
		clOptions->acl.save();
	else
		return false;

	return true;
}

BZF_API bool bz_registerCustomSlashCommand ( const char* command, bz_CustomSlashCommandHandaler *handaler )
{
	if (!command || !handaler)
		return false;

	registerCustomSlashCommand(std::string(command),(CustomSlashCommandHandaler*)handaler);
	return true;
}

BZF_API bool bz_removeCustomSlashCommand ( const char* command )
{
	if (!command)
		return false;

	removeCustomSlashCommand(std::string(command));
	return true;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
