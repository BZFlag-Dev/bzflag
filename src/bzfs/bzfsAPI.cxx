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

#include "bzfs.h"
#include "WorldWeapons.h"
#include "bzfsEvents.h"
#include "WorldEventManager.h"
#include "GameKeeper.h"
#include "FlagInfo.h"


extern void sendMessage(int playerIndex, PlayerId dstPlayer, const char *message);


bz_String::bz_String()
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


BZF_API bool bz_registerEvent ( bz_teEventType eventType, int team, bz_EventHandaler* eventHandaler )
{
	if (!eventHandaler)
		return false;
	
	worldEventManager.addEvent((teEventType)eventType,team,(BaseEventHandaler*)eventHandaler);
	return true;
}

bz_PlayerRecord::bz_PlayerRecord()
{
	playerID = -1;
	team = -1;

	pos[0] = pos[1] = pos[2] = 0;
	rot = 0;

	spawned = false;
	verified = false;
	globalUser = false;
}

bz_PlayerRecord::~bz_PlayerRecord()
{

}

void bz_PlayerRecord::update ( void )
{
	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
	if (!player)
		return;

	memcpy(pos,player->lastState->pos,sizeof(float)*3);

	rot = player->lastState->azimuth;

	int flagid = player->player.getFlag();
	FlagInfo *flagInfo = FlagInfo::get(flagid);

	currentFlag.set(flagInfo->flag.type->label().c_str());

	std::vector<FlagType*>	flagHistoryList = player->flagHistory.get();

	flagHistory.clear();
	for ( unsigned int i = 0; i < flagHistoryList.size(); i ++)
		flagHistory.push(flagHistoryList[i]->label().c_str());

	groups.clear();
	for ( unsigned int i = 0; i < player->accessInfo.groups.size(); i ++)
		groups.push(player->accessInfo.groups[i].c_str());
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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
