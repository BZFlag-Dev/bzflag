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
#include "WorldEventManager.h"
#include "GameKeeper.h"
#include "FlagInfo.h"

#include "commands.h"
#include "SpawnPosition.h"
#include "WorldInfo.h"

#include "BzMaterial.h"

#include "bzfsPlugins.h"

TimeKeeper synct = TimeKeeper::getCurrent();

extern void sendMessage(int playerIndex, PlayerId dstPlayer, const char *message);
extern void removePlayer(int playerIndex, const char *reason, bool notify);
extern void zapFlagByPlayer(int playerIndex);
extern void broadcastMessage(uint16_t code, int len, const void *msg);
extern void directMessage(int playerIndex, uint16_t code, int len, const void *msg);
extern char *getDirectMessageBuffer();

extern CmdLineOptions *clOptions;
extern uint16_t curMaxPlayers;
extern WorldInfo *world;
extern float pluginWorldSize;
extern float pluginWorldHeight;
extern float pluginMaxWait;

// utility functions
void setBZMatFromAPIMat (BzMaterial &bzmat, bz_MaterialInfo* material )
{
	if (!material)
		return;

	bzmat.setName(std::string(material->name.c_str()));
	bzmat.setAmbient(material->ambient);
	bzmat.setDiffuse(material->diffuse);
	bzmat.setSpecular(material->specular);
	bzmat.setEmission(material->emisive);
	bzmat.setShininess(material->shine);

	bzmat.setNoCulling(!material->culling);
	bzmat.setNoSorting(!material->sorting);
	bzmat.setAlphaThreshold(material->alphaThresh);

	for( unsigned int i = 0; i < material->textures.size();i++ )
	{
		bzApiString	name = material->textures[i].texture;

		bzmat.addTexture(std::string(name.c_str()));
		bzmat.setCombineMode(material->textures[i].combineMode);
		bzmat.setUseTextureAlpha(material->textures[i].useAlpha);
		bzmat.setUseColorOnTexture(material->textures[i].useColorOnTexture);
		bzmat.setUseSphereMap(material->textures[i].useSphereMap);
	}
}


//******************************Versioning********************************************
BZF_API int bz_APIVersion ( void )
{
	return BZ_API_VERSION;
}

//******************************bzApiString********************************************
class bzApiString::dataBlob
{
public:
	std::string str;
};

bzApiString::bzApiString()
{
	data = new dataBlob;
}

bzApiString::bzApiString(const char* c)
{
	data = new dataBlob;
	data->str = c;
}

bzApiString::bzApiString(const std::string &s)
{
	data = new dataBlob;
	data->str = s;
}

bzApiString::bzApiString(const bzApiString &r)
{
	data = new dataBlob;
	data->str = r.data->str;
}

bzApiString::~bzApiString()
{
	delete(data);
}

bzApiString& bzApiString::operator = ( const bzApiString& r )
{
	data->str = r.data->str;
	return *this;
}

bzApiString& bzApiString::operator = ( const std::string& r )
{
	data->str = r;
	return *this;
}

bzApiString& bzApiString::operator = ( const char* r )
{
	data->str = r;
	return *this;
}

bool bzApiString::operator == ( const bzApiString&r )
{
	return data->str == r.data->str;
}

bool bzApiString::operator == ( const std::string& r )
{
	return data->str == r;
}

bool bzApiString::operator == ( const char* r )
{
	return data->str == r;
}

bool bzApiString::operator != ( const bzApiString&r )
{
	return data->str != r.data->str;
}

bool bzApiString::operator != ( const std::string& r )
{
	return data->str != r;
}

bool bzApiString::operator != ( const char* r )
{
	return data->str != r;
}

unsigned int bzApiString::size ( void )
{
	return data->str.size();
}

const char* bzApiString::c_str(void)
{
	return data->str.c_str();
}

//******************************bzAPIIntList********************************************
class bzAPIIntList::dataBlob
{
public:
	std::vector<int>	list;
};

bzAPIIntList::bzAPIIntList()
{
	data = new dataBlob;
}

bzAPIIntList::bzAPIIntList(const bzAPIIntList	&r)
{
	data = new dataBlob;
	data->list = r.data->list;
}

bzAPIIntList::bzAPIIntList(const std::vector<int>	&r)
{
	data = new dataBlob;
	data->list = r;
}

bzAPIIntList::~bzAPIIntList()
{
	delete(data);
}

void bzAPIIntList::push_back ( int value )
{
	data->list.push_back(value);
}

int bzAPIIntList::get ( unsigned int i )
{
	if (i >= data->list.size())
		return 0;

	return data->list[i];
}

const int& bzAPIIntList::operator[] (unsigned int i) const
{
	return data->list[i];
}

bzAPIIntList& bzAPIIntList::operator = ( const bzAPIIntList& r )
{
	data->list = r.data->list;
	return *this;
}

bzAPIIntList& bzAPIIntList::operator = ( const std::vector<int>& r )
{
	data->list = r;
	return *this;
}

unsigned int bzAPIIntList::size ( void )
{
	return data->list.size();
}

void bzAPIIntList::clear ( void )
{
	data->list.clear();
}

//******************************bzAPIFloatList********************************************
class bzAPIFloatList::dataBlob
{
public:
	std::vector<float>	list;
};

bzAPIFloatList::bzAPIFloatList()
{
	data = new dataBlob;
}

bzAPIFloatList::bzAPIFloatList(const bzAPIFloatList	&r)
{
	data = new dataBlob;
	data->list = r.data->list;
}

bzAPIFloatList::bzAPIFloatList(const std::vector<float>	&r)
{
	data = new dataBlob;
	data->list = r;
}

bzAPIFloatList::~bzAPIFloatList()
{
	delete(data);
}

void bzAPIFloatList::push_back ( float value )
{
	data->list.push_back(value);
}

float bzAPIFloatList::get ( unsigned int i )
{
	if (i >= data->list.size())
		return 0;

	return data->list[i];
}

const float& bzAPIFloatList::operator[] (unsigned int i) const
{
	return data->list[i];
}

bzAPIFloatList& bzAPIFloatList::operator = ( const bzAPIFloatList& r )
{
	data->list = r.data->list;
	return *this;
}

bzAPIFloatList& bzAPIFloatList::operator = ( const std::vector<float>& r )
{
	data->list = r;
	return *this;
}

unsigned int bzAPIFloatList::size ( void )
{
	return data->list.size();
}

void bzAPIFloatList::clear ( void )
{
	data->list.clear();
}

BZF_API bzAPIFloatList* bz_newFloatList ( void )
{
	return new bzAPIFloatList;
}

BZF_API void bz_deleteFloatList( bzAPIFloatList * l )
{
	if (l)
		delete(l);
}


//******************************bzApiStringList********************************************
class bzAPIStringList::dataBlob
{
public:
	std::vector<bzApiString> list;
};


bzAPIStringList::bzAPIStringList()
{
	data = new dataBlob;
}

bzAPIStringList::bzAPIStringList(const bzAPIStringList	&r)
{
	data = new dataBlob;
	data->list = r.data->list;
}

bzAPIStringList::bzAPIStringList(const std::vector<std::string>	&r)
{
	data = new dataBlob;

	for ( unsigned int i = 0; i < r.size(); i++)
	{
		std::string d = r[i];
		data->list.push_back(bzApiString(d));
	}
}

bzAPIStringList::~bzAPIStringList()
{
	delete(data);
}

void bzAPIStringList::push_back ( const bzApiString &value )
{
	data->list.push_back(value);
}

void bzAPIStringList::push_back ( const std::string &value )
{
	data->list.push_back(bzApiString(value));
}

bzApiString bzAPIStringList::get ( unsigned int i )
{
	if (i >= data->list.size())
		return bzApiString("");

	return data->list[i];
}

const bzApiString& bzAPIStringList::operator[] (unsigned int i) const
{
	return data->list[i];
}

bzAPIStringList& bzAPIStringList::operator = ( const bzAPIStringList& r )
{
	data->list = r.data->list;
	return *this;
}

bzAPIStringList& bzAPIStringList::operator = ( const std::vector<std::string>& r )
{
	data->list.clear();

	for ( unsigned int i = 0; i < r.size(); i++)
		data->list.push_back(bzApiString(r[i]));

	return *this;
}

unsigned int bzAPIStringList::size ( void )
{
	return data->list.size();
}

void bzAPIStringList::clear ( void )
{
	data->list.clear();
}

//******************************bzApiTextreList********************************************

class bzAPITextureList::dataBlob
{
public:
	std::vector<bz_MaterialTexture> list;
};

bzAPITextureList::bzAPITextureList()
{
	data = new dataBlob;
}

bzAPITextureList::bzAPITextureList(const bzAPITextureList	&r)
{
	data = new dataBlob;
	data->list = r.data->list;
}

bzAPITextureList::~bzAPITextureList()
{
	delete(data);
}

void bzAPITextureList::push_back ( bz_MaterialTexture &value )
{
	data->list.push_back(value);
}

bz_MaterialTexture bzAPITextureList::get ( unsigned int i )
{
	return data->list[i];
}

const bz_MaterialTexture& bzAPITextureList::operator[] (unsigned int i) const
{
	return data->list[i];
}

bzAPITextureList& bzAPITextureList::operator = ( const bzAPITextureList& r )
{
	data->list = r.data->list;
	return *this;
}

unsigned int bzAPITextureList::size ( void )
{
	return data->list.size();
}

void bzAPITextureList::clear ( void )
{
	data->list.clear();
}

bz_MaterialInfo* bz_anewMaterial ( void )
{
	return new bz_MaterialInfo;
}

void bz_adeleteMaterial ( bz_MaterialInfo *material )
{
	if (material)
		delete(material);
}

// events
BZF_API bool bz_registerEvent ( bz_eEventType eventType, int team, bz_EventHandler* eventHandler )
{
	if (!eventHandler)
		return false;
	
	worldEventManager.addEvent(eventType,team,eventHandler);
	return true;
}

BZF_API bool bz_registerGeneralEvent ( bz_eEventType eventType, bz_EventHandler* eventHandler )
{
	return bz_registerEvent(eventType,-1,eventHandler);
}

BZF_API bool bz_removeEvent ( bz_eEventType eventType, int team, bz_EventHandler* eventHandler )
{
	if (!eventHandler)
		return false;

	worldEventManager.removeEvent(eventType,team,eventHandler);
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

//	playerRecord->flagHistory.clear();
	for ( unsigned int i = 0; i < flagHistoryList.size(); i ++)
		playerRecord->flagHistory.push_back(flagHistoryList[i]->label());

	playerRecord->groups.clear();
	playerRecord->groups = player->accessInfo.groups;

	playerRecord->admin = player->accessInfo.isVerified();

	playerRecord->wins = player->score.getWins();
	playerRecord->losses = player->score.getLosses();

	return true;
}

BZF_API bool bz_getPlayerIndexList ( bzAPIIntList *playerList )
{
	playerList->clear();

	for (int i = 0; i < curMaxPlayers; i++)
	{
		GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
		if ((p == NULL))
			continue;

		playerList->push_back(i);
	}
	return playerList->size() > 0;
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

BZF_API bool bz_fireWorldWep ( const char* flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, int shotID, float dt )
{
	if (!pos || !flagType)
		return false;

	FlagTypeMap &flagMap = FlagType::getFlagMap();
	if (flagMap.find(flagType) == flagMap.end())
		return false;

	FlagType *flag = flagMap.find(std::string(flagType))->second;

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

BZF_API float bz_getMaxWaitTime ( void )
{
	return pluginMaxWait;
}

BZF_API void bz_setMaxWaitTime ( float time )
{
	if ( pluginMaxWait > time)
		pluginMaxWait = time;
}


// info
BZF_API double bz_getBZDBDouble ( const char* variable )
{
	if (!variable)
		return 0.0;

	return BZDB.eval(std::string(variable));
}

BZF_API bzApiString bz_getBZDString( const char* variable )
{
	if (!variable)
		return bzApiString("");

	return bzApiString(BZDB.get(std::string(variable)));
}

BZF_API bool bz_getBZDBool( const char* variable )
{
	if (!variable)
		return false;

	return BZDB.eval(std::string(variable)) > 0.0;
}

BZF_API int bz_getBZDInt( const char* variable )
{
	return (int)BZDB.eval(std::string(variable));
}

BZF_API bool bz_setBZDBDouble ( const char* variable, double val )
{
	if (!variable)
		return false;

	bool exists = BZDB.isSet(std::string(variable));

	BZDB.set(std::string(variable),TextUtils::format("%f",val));

	return !exists;
}

BZF_API bool bz_setBZDString( const char* variable, const char *val )
{
	if (!variable || !val)
		return false;

	bool exists = BZDB.isSet(std::string(variable));

	BZDB.set(std::string(variable),std::string(val));

	return !exists;
}

BZF_API bool bz_setBZDBool( const char* variable, bool val )
{
	if (!variable)
		return false;

	bool exists = BZDB.isSet(std::string(variable));

	BZDB.set(std::string(variable),TextUtils::format("%d",val));

	return !exists;
}

BZF_API bool bz_setBZDInt( const char* variable, int val )
{
	if (!variable)
		return false;

	bool exists = BZDB.isSet(std::string(variable));

	BZDB.set(std::string(variable),TextUtils::format("%d",val));

	return !exists;
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

BZF_API bool bz_registerCustomSlashCommand ( const char* command, bz_CustomSlashCommandHandler *handler )
{
	if (!command || !handler)
		return false;

	registerCustomSlashCommand(std::string(command),handler);
	return true;
}

BZF_API bool bz_removeCustomSlashCommand ( const char* command )
{
	if (!command)
		return false;

	removeCustomSlashCommand(std::string(command));
	return true;
}

BZF_API bool bz_getStandardSpawn ( int playeID, float pos[3], float *rot )
{
	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playeID);
	if (!player)
		return false;

	// get the spawn position
	SpawnPosition* spawnPosition = new SpawnPosition(playeID,
		(!clOptions->respawnOnBuildings) || (player->player.isBot()),
		clOptions->gameStyle & TeamFlagGameStyle);

	pos[0] = spawnPosition->getX();
	pos[1] = spawnPosition->getY();
	pos[2] = spawnPosition->getZ();
	if (rot)
		*rot = spawnPosition->getAzimuth();

	return true;
}

BZF_API bool bz_killPlayer ( int playeID, bool spawnOnBase )
{
	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playeID);
	if (!player)
		return false;

	if (!player->player.isAlive())
		return false;

	player->player.setDead();
	player->player.setRestartOnBase(spawnOnBase);
	zapFlagByPlayer(playeID);

	return true;
}

BZF_API bool bz_removePlayerFlag ( int playeID )
{
	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playeID);
	if (!player)
		return false;

	if (!player->player.isAlive())
		return false;

	zapFlagByPlayer(playeID);

	return true;
}

BZF_API bool bz_addWorldBox ( float *pos, float rot, float* scale, bz_WorldObjectOptions options )
{
	if (!world || world->isFinisihed() || !pos || !scale)
		return false;

	world->addBox(pos[0],pos[1],pos[2],rot,scale[0],scale[1],scale[2],options.driveThru,options.shootThru);
	return true;
}

BZF_API bool bz_addWorldPyramid ( float *pos, float rot, float* scale, bool fliped, bz_WorldObjectOptions options )
{
	if (!world || world->isFinisihed() || !pos || !scale)
		return false;

	world->addPyramid(pos[0],pos[1],pos[2],rot,scale[0],scale[1],scale[2],options.driveThru,options.shootThru,fliped);
	return true;
}

BZF_API bool bz_addWorldBase( float *pos, float rot, float* scale, int team, bz_WorldObjectOptions options )
{
	if (!world || world->isFinisihed() || !pos || !scale)
		return false;

	world->addBase(pos,rot,scale,team,options.driveThru,options.shootThru);
	return true;
}

BZF_API bool bz_addWorldTeleporter ( float *pos, float rot, float* scale, float border, bz_WorldObjectOptions options )
{
	if (!world || world->isFinisihed() || !pos || !scale)
		return false;

	world->addTeleporter(pos[0],pos[1],pos[2],rot,scale[0],scale[1],scale[2],border,false,options.driveThru,options.shootThru);
	return true;
}

BZF_API bool bz_addWorldLink( int from, int to )
{
	if (!world || world->isFinisihed() )
		return false;

	world->addLink(from,to);
	return true;
}

BZF_API bool bz_addWorldWaterLevel( float level, bz_MaterialInfo *material )
{
	if (!world || world->isFinisihed() )
		return false;

	if (!material)
	{
		world->addWaterLevel(level,NULL);
		return true;
	}

	BzMaterial	bzmat;
	setBZMatFromAPIMat(bzmat,material);
	world->addWaterLevel(level,MATERIALMGR.addMaterial(&bzmat));
	return true;
}

BZF_API bool bz_addWorldWeapon( std::string flagType, float *pos, float rot, float tilt, float initDelay, bzAPIFloatList &delays )
{
	if (!world || world->isFinisihed() )
		return false;

	FlagTypeMap &flagMap = FlagType::getFlagMap();
	if (flagMap.find(flagType) == flagMap.end())
		return false;

	FlagType *flag = flagMap.find(flagType)->second;

	std::vector<float> realDelays;

	for(unsigned int i = 0; i < delays.size(); i++)
		realDelays.push_back(delays.get(i));

	world->addWeapon(flag, pos, rot, tilt, initDelay, realDelays, synct);
	return true;
}


BZF_API bool bz_setWorldSize( float size, float wallHeight )
{
	pluginWorldHeight = wallHeight;
	pluginWorldSize = size;

	return true;
}

BZF_API bool bz_getPublic( void )
{
	return clOptions->publicizeServer;
}

BZF_API bzApiString bz_getPublicAddr( void )
{
	if (!clOptions->publicizeServer)
		return bzApiString("");

	return bzApiString(clOptions->publicizedAddress);
}

BZF_API bzApiString bz_getPublicDescription( void )
{
	if (!clOptions->publicizeServer)
		return bzApiString("");

	return bzApiString(clOptions->publicizedTitle);
}

BZF_API bool bz_sendPlayCustomLocalSound ( int playerID, const char* soundName )
{
	if (playerID == BZ_SERVER || !soundName)
		return false;

	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUShort(bufStart, LocalCustomSound);
	buf = nboPackUShort(buf, (unsigned short)strlen(soundName));
	buf = nboPackString(buf, soundName,strlen(soundName));
	if (playerID == BZ_ALL_USERS)
		broadcastMessage(MsgCustomSound, (char*)buf - (char*)bufStart, bufStart);
	else
		directMessage(playerID,MsgCustomSound, (char*)buf - (char*)bufStart, bufStart);

	return true;
}

// custom pluginHandaler
bool bz_registerCustomPluginHandaler ( const char* extension, bz_APIPluginHandaler *handaler )
{
	if (!extension || !handaler)
		return false;

	std::string ext = extension;

	return registerCustomPluginHandaler( ext,handaler);
}

bool bz_removeCustomPluginHandaler ( const char* extension, bz_APIPluginHandaler *handaler )
{
	if (!extension || !handaler)
		return false;

	std::string ext = extension;

	return removeCustomPluginHandaler( ext,handaler);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
