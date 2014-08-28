/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// common-interface headers
#include "global.h"

// implementation wrappers for all the bzf_ API functions
#include "bzfsAPI.h"

#include "bzfs.h"
#include "WorldWeapons.h"
#include "WorldEventManager.h"
#include "GameKeeper.h"
#include "FlagInfo.h"
#include "VotingArbiter.h"
#include "commands.h"
#include "SpawnPosition.h"
#include "WorldInfo.h"
#include "BzMaterial.h"
#include "cURLManager.h"
#include "bzfsPlugins.h"
#include "CustomWorld.h"
#include "Permissions.h"
#include "CommandManager.h"
#include "md5.h"
#include "version.h"

TimeKeeper synct = TimeKeeper::getCurrent();
std::list<PendingChatMessages> pendingChatMessages;

std::map<std::string, std::vector<bz_ClipFieldNotifier*> > clipFieldMap;

void callClipFiledCallbacks ( const char* field );

class MasterBanURLHandler : public bz_BaseURLHandler
{
public:
  bool busy;
  unsigned int id;
  std::string theData;

  void doNext ( void )
  {
    if (id >= clOptions->masterBanListURL.size())
    {
      rescanForBans();
      busy = false;
      return;
    }
    theData = "";
    bz_addURLJob(clOptions->masterBanListURL[id].c_str(),this);
    id++;
  }

  void start ( void )
  {

    id = 0;
    busy = true;
    doNext();
  }

  virtual void URLDone ( const char*, const void * data, unsigned int size, bool complete )
  {
    if (!busy)
      return;

    if (data && size > 0)
    {
      char *p = (char*)malloc(size+1);
      memcpy(p,data,size);
      p[size] = 0;
      theData += p;
      free(p);
    }

    if (complete)
    {
      clOptions->acl.merge(theData);
      doNext();
    }
  }

  virtual void URLTimeout ( const char* UNUSED(URL), int UNUSED(errorCode) )
  {
    doNext();
  }

  virtual void URLError ( const char* UNUSED(URL), int UNUSED(errorCode), const char * UNUSED(errorString) )
  {
    doNext();
  }

  MasterBanURLHandler()
  {
    id = 0;
    busy = false;
  }
};

MasterBanURLHandler	masterBanHandler;

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
    bz_ApiString	name = material->textures[i].texture;

    bzmat.addTexture(std::string(name.c_str()));
    bzmat.setCombineMode(material->textures[i].combineMode);
    bzmat.setUseTextureAlpha(material->textures[i].useAlpha);
    bzmat.setUseColorOnTexture(material->textures[i].useColorOnTexture);
    bzmat.setUseSphereMap(material->textures[i].useSphereMap);
  }
}

bz_eTeamType convertTeam ( int _team )
{
  switch (_team)
  {
  default:
    return eNoTeam;
  case RogueTeam:
    return eRogueTeam;
  case RedTeam:
    return eRedTeam;
  case GreenTeam:
    return eGreenTeam;
  case BlueTeam:
    return eBlueTeam;
  case PurpleTeam:
    return ePurpleTeam;
  case ObserverTeam:
    return eObservers;
  case RabbitTeam:
    return eRabbitTeam;
  case HunterTeam:
    return eHunterTeam;
  }
}

int convertTeam( bz_eTeamType _team )
{
  if (_team > eObservers)
    return NoTeam;

  switch (_team)
  {
  default:
    return NoTeam;
  case eRogueTeam:
    return RogueTeam;
  case eRedTeam:
    return RedTeam;
  case eGreenTeam:
    return GreenTeam;
  case eBlueTeam:
    return BlueTeam;
  case ePurpleTeam:
    return PurpleTeam;
  case eObservers:
    return ObserverTeam;
  case eRabbitTeam:
    return RabbitTeam;
  case eHunterTeam:
    return HunterTeam;
  }
  return (TeamColor)_team;
}

void broadcastPlayerScoreUpdate ( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
  if (!player)
    return;

  void *buf, *bufStart;
  bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, 1);
  buf = nboPackUByte(buf, playerID);
  buf = player->score.pack(buf);
  broadcastMessage(MsgScore, (char*)buf-(char*)bufStart, bufStart);
}

//******************************Versioning********************************************
BZF_API int bz_APIVersion ( void )
{
  return BZ_API_VERSION;
}

//******************************bzApiString********************************************
class bz_ApiString::dataBlob
{
public:
  std::string str;
};

bz_ApiString::bz_ApiString()
{
  data = new dataBlob;
}

bz_ApiString::bz_ApiString(const char* c)
{
  data = new dataBlob;
  data->str = c;
}

bz_ApiString::bz_ApiString(const std::string &s)
{
  data = new dataBlob;
  data->str = s;
}

bz_ApiString::bz_ApiString(const bz_ApiString &r)
{
  data = new dataBlob;
  data->str = r.data->str;
}

bz_ApiString::~bz_ApiString()
{
  delete(data);
}

bz_ApiString& bz_ApiString::operator = ( const bz_ApiString& r )
{
  data->str = r.data->str;
  return *this;
}

bz_ApiString& bz_ApiString::operator = ( const std::string& r )
{
  data->str = r;
  return *this;
}

bz_ApiString& bz_ApiString::operator = ( const char* r )
{
  data->str = r;
  return *this;
}

bool bz_ApiString::operator == ( const bz_ApiString&r )
{
  return data->str == r.data->str;
}

bool bz_ApiString::operator == ( const std::string& r )
{
  return data->str == r;
}

bool bz_ApiString::operator == ( const char* r )
{
  return data->str == r;
}

bool bz_ApiString::operator != ( const bz_ApiString&r )
{
  return data->str != r.data->str;
}

bool bz_ApiString::operator != ( const std::string& r )
{
  return data->str != r;
}

bool bz_ApiString::operator != ( const char* r )
{
  return data->str != r;
}

unsigned int bz_ApiString::size ( void ) const
{
  return data->str.size();
}

const char* bz_ApiString::c_str(void) const
{
  return data->str.c_str();
}

void bz_ApiString::format(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  data->str = TextUtils::vformat(fmt, args);
  va_end(args);
}

void bz_ApiString::replaceAll ( const char* target, const char* with )
{
  if (!target)
    return;

  if (!with)
    return;

  data->str = TextUtils::replace_all(data->str,std::string(target),std::string(with));
}

void bz_ApiString::tolower ( void )
{
  data->str = TextUtils::tolower(data->str);
}

void bz_ApiString::urlEncode ( void )
{
  data->str = TextUtils::url_encode(data->str);
}

void bz_ApiString::toupper ( void )
{
  data->str = TextUtils::toupper(data->str);
}

//******************************bzAPIIntList********************************************
class bz_APIIntList::dataBlob
{
public:
  std::vector<int>	list;
};

bz_APIIntList::bz_APIIntList()
{
  data = new dataBlob;
}

bz_APIIntList::bz_APIIntList(const bz_APIIntList	&r)
{
  data = new dataBlob;
  data->list = r.data->list;
}

bz_APIIntList::bz_APIIntList(const std::vector<int>	&r)
{
  data = new dataBlob;
  data->list = r;
}

bz_APIIntList::~bz_APIIntList()
{
  delete(data);
}

void bz_APIIntList::push_back ( int value )
{
  data->list.push_back(value);
}

int bz_APIIntList::get ( unsigned int i )
{
  if (i >= data->list.size())
    return 0;

  return data->list[i];
}

const int& bz_APIIntList::operator[] (unsigned int i) const
{
  return data->list[i];
}

bz_APIIntList& bz_APIIntList::operator = ( const bz_APIIntList& r )
{
  data->list = r.data->list;
  return *this;
}

bz_APIIntList& bz_APIIntList::operator = ( const std::vector<int>& r )
{
  data->list = r;
  return *this;
}

unsigned int bz_APIIntList::size ( void )
{
  return data->list.size();
}

void bz_APIIntList::clear ( void )
{
  data->list.clear();
}

BZF_API bz_APIIntList* bz_newIntList ( void )
{
  return new bz_APIIntList;
}

BZF_API void bz_deleteIntList( bz_APIIntList * l )
{
  delete(l);
}

//******************************bzAPIFloatList********************************************
class bz_APIFloatList::dataBlob
{
public:
  std::vector<float>	list;
};

bz_APIFloatList::bz_APIFloatList()
{
  data = new dataBlob;
}

bz_APIFloatList::bz_APIFloatList(const bz_APIFloatList	&r)
{
  data = new dataBlob;
  data->list = r.data->list;
}

bz_APIFloatList::bz_APIFloatList(const std::vector<float>	&r)
{
  data = new dataBlob;
  data->list = r;
}

bz_APIFloatList::~bz_APIFloatList()
{
  delete(data);
}

void bz_APIFloatList::push_back ( float value )
{
  data->list.push_back(value);
}

float bz_APIFloatList::get ( unsigned int i )
{
  if (i >= data->list.size())
    return 0;

  return data->list[i];
}

const float& bz_APIFloatList::operator[] (unsigned int i) const
{
  return data->list[i];
}

bz_APIFloatList& bz_APIFloatList::operator = ( const bz_APIFloatList& r )
{
  data->list = r.data->list;
  return *this;
}

bz_APIFloatList& bz_APIFloatList::operator = ( const std::vector<float>& r )
{
  data->list = r;
  return *this;
}

unsigned int bz_APIFloatList::size ( void )
{
  return data->list.size();
}

void bz_APIFloatList::clear ( void )
{
  data->list.clear();
}

BZF_API bz_APIFloatList* bz_newFloatList ( void )
{
  return new bz_APIFloatList;
}

BZF_API void bz_deleteFloatList( bz_APIFloatList * l )
{
  if (l)
    delete(l);
}

BZF_API bz_APIStringList* bz_newStringList ( void )
{
  return new bz_APIStringList;
}

BZF_API void bz_deleteStringList( bz_APIStringList * l )
{
  if (l)
    delete(l);
}

//******************************bzApiStringList********************************************
class bz_APIStringList::dataBlob
{
public:
  std::vector<bz_ApiString> list;
};


bz_APIStringList::bz_APIStringList()
{
  data = new dataBlob;
}

bz_APIStringList::bz_APIStringList(const bz_APIStringList	&r)
{
  data = new dataBlob;
  data->list = r.data->list;
}

bz_APIStringList::bz_APIStringList(const std::vector<std::string>	&r)
{
  data = new dataBlob;

  for ( unsigned int i = 0; i < r.size(); i++)
  {
    std::string d = r[i];
    data->list.push_back(bz_ApiString(d));
  }
}

bz_APIStringList::~bz_APIStringList()
{
  delete(data);
}

void bz_APIStringList::push_back ( const bz_ApiString &value )
{
  data->list.push_back(value);
}

void bz_APIStringList::push_back ( const std::string &value )
{
  data->list.push_back(bz_ApiString(value));
}

bz_ApiString bz_APIStringList::get ( unsigned int i ) const
{
  if (i >= data->list.size())
    return bz_ApiString("");

  return data->list[i];
}

const bz_ApiString& bz_APIStringList::operator[] (unsigned int i) const
{
  return data->list[i];
}

bz_APIStringList& bz_APIStringList::operator = ( const bz_APIStringList& r )
{
  data->list = r.data->list;
  return *this;
}

bz_APIStringList& bz_APIStringList::operator = ( const std::vector<std::string>& r )
{
  data->list.clear();

  for ( unsigned int i = 0; i < r.size(); i++)
    data->list.push_back(bz_ApiString(r[i]));

  return *this;
}

unsigned int bz_APIStringList::size ( void ) const
{
  return data->list.size();
}

void bz_APIStringList::clear ( void )
{
  data->list.clear();
}

void bz_APIStringList::tokenize ( const char* in, const char* delims, int maxTokens, bool useQuotes)
{
  clear();
  if (!in || !delims)
    return;

  std::vector<std::string> list = TextUtils::tokenize(std::string(in),std::string(delims),maxTokens,useQuotes);

  for ( unsigned int i = 0; i < list.size(); i++)
    data->list.push_back(bz_ApiString(list[i]));
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

void bz_deleteMaterial ( bz_MaterialInfo *material )
{
  if (material)
    delete(material);
}

// events

bz_Plugin::bz_Plugin()
{
  MaxWaitTime = -1;
  Unloadable = true;
}

bz_Plugin::~bz_Plugin()
{
}

bool bz_Plugin::Register (bz_eEventType eventType)
{
  return RegisterEvent(eventType,this);
}

bool bz_Plugin::Remove (bz_eEventType eventType)
{
  return RemoveEvent(eventType,this);
}

void bz_Plugin::Flush ()
{
  FlushEvents(this);
}

BZF_API bool bz_pluginExists(const char* name)
{
#ifdef BZ_PLUGINS
  return getPlugin(name) != NULL;
#else
  (void)name;
  return false;
#endif
}

BZF_API bz_Plugin* bz_getPlugin(const char* name)
{
#ifdef BZ_PLUGINS
  return getPlugin(name);
#else
  (void)name;
  return NULL;
#endif
}

BZF_API int bz_callPluginGenericCallback(const char* plugin, const char* name, void* data )
{
	bz_Plugin *p = bz_getPlugin(plugin);
	if (p == NULL)
		return 0;

	return p->GeneralCallback(name,data);
}

//-------------------------------------------------------------------------
//
// utility
//

static inline NetConnectedPeer* getNonPlayerPeer(int connID)
{
  std::map<int, NetConnectedPeer>::iterator it = netConnectedPeers.find(connID);
  if (it == netConnectedPeers.end()) {
    return NULL;
  }
  NetConnectedPeer* peer = &(it->second);
  if (peer->player != -1) {
    return NULL;
  }
  return peer;
}


BZF_API bool bz_registerNonPlayerConnectionHandler(int connectionID, bz_NonPlayerConnectionHandler* handler)
{
  if (!handler) {
    return false;
  }
  NetConnectedPeer* peer = getNonPlayerPeer(connectionID);
  if ((peer == NULL) || (peer->apiHandler != NULL)) {
    return false;
  }
  peer->apiHandler = handler;
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeNonPlayerConnectionHandler(int connectionID, bz_NonPlayerConnectionHandler *handler)
{
  if (!handler) {
    return false;
  }
  NetConnectedPeer* peer = getNonPlayerPeer(connectionID);
  if ((peer == NULL) || (peer->apiHandler != handler)) {
    return false;
  }
  peer->apiHandler = NULL;
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setNonPlayerDisconnectOnSend(int connectionID, bool bSet)
{
  NetConnectedPeer* peer = getNonPlayerPeer(connectionID);
  if (peer == NULL) {
    return false;
  }

  peer->deleteWhenDoneSending = bSet;
  return true;
}

BZF_API bool bz_setNonPlayerDataThrottle(int connectionID, double time)
{
  NetConnectedPeer* peer = getNonPlayerPeer(connectionID);
  if (peer == NULL) {
    return false;
  }

  peer->minSendTime = time;
  return true;
}

BZF_API bool bz_setNonPlayerInactivityTimeout(int connectionID, double time)
{
  NetConnectedPeer* peer = getNonPlayerPeer(connectionID);
  if (peer == NULL) {
    return false;
  }

  peer->inactivityTimeout = time;
  return true;
}

BZF_API bool bz_sendNonPlayerData(int connID, const void *data, unsigned int size)
{
  if (!data || !size) {
    return false;
  }

  NetConnectedPeer* peer = getNonPlayerPeer(connID);
  if (peer == NULL) {
    return false;
  }

  const bool sendOneNow = false; //peer->sendChunks.empty();

  unsigned int chunkSize = 0;

  for (unsigned int pos = 0; pos < size; pos += chunkSize) {
    const unsigned int left = (size - pos);

    chunkSize = (left < maxNonPlayerDataChunk) ? left : maxNonPlayerDataChunk;

    peer->sendChunks.push_back(std::string((const char*)data + pos, chunkSize));
  }

  // send off at least one now if it was empty
  if (sendOneNow) {
    sendBufferedNetDataForPeer(*peer);
  }

  return true;
}


BZF_API unsigned int bz_getNonPlayerConnectionOutboundPacketCount(int connectionID)
{
  NetConnectedPeer* peer = getNonPlayerPeer(connectionID);
  if (peer == NULL) {
    return 0;
  }
  return peer->sendChunks.size();
}


BZF_API const char* bz_getNonPlayerConnectionIP ( int connectionID )
{
  NetConnectedPeer* peer = getNonPlayerPeer(connectionID);
  if (peer == NULL) {
    return NULL;
  }

  unsigned int address = (unsigned int)peer->netHandler->getIPAddress().s_addr;
  unsigned char* a = (unsigned char*)&address;

  static std::string strRet;

  strRet = TextUtils::format("%d.%d.%d.%d",(int)a[0],(int)a[1],(int)a[2],(int)a[3]);

  return strRet.c_str();
}


BZF_API const char* bz_getNonPlayerConnectionHost ( int connectionID )
{
  NetConnectedPeer* peer = getNonPlayerPeer(connectionID);
  if (peer == NULL) {
    return NULL;
  }
  return peer->netHandler->getHostname();
}


//-------------------------------------------------------------------------

BZF_API bool bz_disconnectNonPlayerConnection(int connectionID)
{
  NetConnectedPeer* peer = getNonPlayerPeer(connectionID);
  if (peer == NULL) {
    return false;
  }

  // flush out the rest of it's sends
  while (!peer->sendChunks.empty()) {
    sendBufferedNetDataForPeer(*peer);
  }

  if (peer->apiHandler) {
    peer->apiHandler->disconnect(connectionID);
  }

  peer->netHandler->flushData();
  delete(peer->netHandler);
  peer->netHandler = NULL;
  peer->apiHandler = NULL;
  peer->sendChunks.clear();
  peer->deleteMe = true;

  return true;
}

BZF_API bool bz_updatePlayerData ( bz_BasePlayerRecord *playerRecord )
{
  if (!playerRecord)
    return false;

  GameKeeper::Player* player = GameKeeper::Player::getPlayerByIndex(playerRecord->playerID);
  if (!player)
    return false;

  playerStateToAPIState(playerRecord->lastKnownState, player->lastState);
  playerRecord->lastUpdateTime = player->player.getLastMsgTime().getSeconds();

  playerRecord->currentFlagID = player->player.getFlag();
  FlagInfo* flagInfo = FlagInfo::get(playerRecord->currentFlagID);

  std::string label;
  if (flagInfo && flagInfo->flag.type)
    label = flagInfo->flag.type->label();
  playerRecord->currentFlag = label;

  std::vector < FlagType* > flagHistoryList = player->flagHistory.getVec();

  playerRecord->flagHistory.clear();
  for (unsigned int i = 0; i < flagHistoryList.size(); i++)
    playerRecord->flagHistory.push_back(flagHistoryList[i]->label());

  playerRecord->groups.clear();
  playerRecord->groups = player->accessInfo.groups;

  playerRecord->admin = player->accessInfo.isAdmin();
  playerRecord->op = player->accessInfo.isOperator();

  playerRecord->verified = player->accessInfo.isVerified();

  playerRecord->spawned = player->player.isAlive();
  playerRecord->lag = player->lagInfo.getLag();
  playerRecord->jitter = player->lagInfo.getJitter();
  playerRecord->packetLoss=(float)player->lagInfo.getLoss();

  playerRecord->rank = player->score.ranking();
  playerRecord->wins = player->score.getWins();
  playerRecord->losses = player->score.getLosses();
  playerRecord->teamKills = player->score.getTKs();
  playerRecord->canSpawn = true;

  playerRecord->clientVersion = player->player.getClientVersion();

  if (playerRecord->version > 1)
  {
    bz_PlayerRecordV2 *r = (bz_PlayerRecordV2*)playerRecord;
    r->motto = player->player.getMotto();
  }
  return true;
}

BZF_API bool bz_hasPerm ( int playerID, const char* perm )
{
  if (!perm)
    return false;

  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
  if (!player)
    return false;

  std::string permName = perm;

  permName = TextUtils::toupper(permName);

  PlayerAccessInfo::AccessPerm realPerm =  permFromName(permName);

  if (realPerm != PlayerAccessInfo::lastPerm)
    return player->accessInfo.hasPerm(realPerm);
  else
    return player->accessInfo.hasCustomPerm(permName.c_str());
}

BZF_API bool bz_grantPerm ( int playerID, const char* perm  )
{
  if (!perm)
    return false;

  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return false;

  std::string permName = perm;

  permName = TextUtils::toupper(permName);

  PlayerAccessInfo::AccessPerm realPerm =  permFromName(permName);

  if (realPerm == PlayerAccessInfo::lastPerm)
    return false;

  player->accessInfo.grantPerm(realPerm);
  return true;
}

BZF_API bool bz_revokePerm ( int playerID, const char* perm  )
{
  if (!perm)
    return false;

  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return false;

  std::string permName = perm;

  permName = TextUtils::toupper(permName);

  PlayerAccessInfo::AccessPerm realPerm =  permFromName(permName);

  if (realPerm == PlayerAccessInfo::lastPerm)
    return false;

  player->accessInfo.revokePerm(realPerm);
  return true;
}

BZF_API bz_APIIntList *bz_getPlayerIndexList(void)
{
  bz_APIIntList *playerList = new bz_APIIntList;

  for (int i = 0; i < curMaxPlayers; i++) {
    if (GameKeeper::Player::getPlayerByIndex(i))
      playerList->push_back(i);
  }
  return playerList;
}

BZF_API int bz_getPlayerCount(void)
{
	int count = 0;
	for (int i = 0; i < curMaxPlayers; i++)
	{
		if (GameKeeper::Player::getPlayerByIndex(i))
			count++;
	}
	return count;
}

BZF_API bool bz_getPlayerIndexList ( bz_APIIntList *playerList )
{
  playerList->clear();

  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
    if (p == NULL)
      continue;

    playerList->push_back(i);
  }
  return playerList->size() > 0;
}

BZF_API bz_BasePlayerRecord * bz_getPlayerByIndex ( int index )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(index);

  bz_PlayerRecordV2 *playerRecord = new bz_PlayerRecordV2;

  if (!player || !playerRecord)
    return NULL;

  playerRecord->callsign = player->player.getCallSign();
  playerRecord->playerID = index;
  playerRecord->bzID = player->getBzIdentifier();
  playerRecord->team = convertTeam(player->player.getTeam());

  playerRecord->spawned = player->player.isAlive();
  playerRecord->verified = player->accessInfo.isVerified();
  playerRecord->globalUser = player->authentication.isGlobal();

  playerRecord->ipAddress = player->netHandler->getTargetIP();

  playerRecord->lag = player->lagInfo.getLag();
  playerRecord->update();

  return playerRecord;
}

BZF_API  bool bz_freePlayerRecord( bz_BasePlayerRecord *playerRecord )
{
  if (playerRecord)
    delete (playerRecord);

  return true;
}
//-------------------------------------------------------------------------

BZF_API bool bz_getAdmin ( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
  if (!player)
    return false;
  return player->accessInfo.isAdmin();
}

//-------------------------------------------------------------------------

BZF_API bool bz_validAdminPassword ( const char* passwd )
{
  if (!passwd || !clOptions->password.size())
    return false;

  return clOptions->password == passwd;
}

BZF_API const char* bz_getPlayerFlag( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return NULL;

  FlagInfo* flagInfo = FlagInfo::get(player->player.getFlag());
  if (!flagInfo)
    return NULL;

  return FlagInfo::get(player->player.getFlag())->flag.type->flagAbbv.c_str();
}

BZF_API bool bz_isPlayerPaused( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return false;

  return player->player.isPaused();
}

BZF_API int bz_getPausedTime( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return -1;

  return player->player.getPausedTime();
}

BZF_API int bz_getIdleTime( int playerID )
{
  GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!otherData)
    return -1;

  return otherData->player.getIdleTime();
}

BZF_API bz_eTeamType bz_getPlayerTeam( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return eNoTeam;

  return convertTeam(player->player.getTeam());
}

BZF_API const char* bz_getPlayerCallsign( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return NULL;

  return player->player.getCallSign();
}

BZF_API const char* bz_getPlayerMotto(int playerID)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return NULL;

  return player->player.getMotto();
}

BZF_API const char* bz_getPlayerIPAddress( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return NULL;

  return player->netHandler->getTargetIP();
}

BZF_API int bz_getPlayerLag( int playerId )
{
  if (!GameKeeper::Player::getPlayerByIndex(playerId))
    return 0;

  return GameKeeper::Player::getPlayerByIndex(playerId)->lagInfo.getLag();
}

BZF_API int bz_getPlayerJitter( int playerId )
{
  if (!GameKeeper::Player::getPlayerByIndex(playerId))
    return 0;

  return GameKeeper::Player::getPlayerByIndex(playerId)->lagInfo.getJitter();
}

BZF_API float bz_getPlayerPacketloss( int playerId )
{
  if (!GameKeeper::Player::getPlayerByIndex(playerId))
    return 0;

  return (float)GameKeeper::Player::getPlayerByIndex(playerId)->lagInfo.getLoss();
}


BZF_API unsigned int bz_getTeamPlayerLimit (bz_eTeamType _team)
{
  switch (_team)
  {
  case eRogueTeam:
  case eBlueTeam:
  case eRedTeam:
  case eGreenTeam:
  case ePurpleTeam:
  case eObservers:
    return clOptions->maxTeam[convertTeam(_team)];
    break;
  default:
    break;
  }
  return 0;
}

BZF_API void bz_computePlayerScore( bool enabled )
{
  Score::KeepPlayerScores = enabled;
}

BZF_API bool bz_computingPlayerScore( void )
{
  return Score::KeepPlayerScores;
}

BZF_API bool bz_setPlayerWins (int playerId, int wins)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return false;

  int oldWins = player->score.getWins();
  player->score.setWins(wins);
  bz_PlayerScoreChangeEventData_V1 eventData = bz_PlayerScoreChangeEventData_V1(playerId, bz_eWins, oldWins, wins);
  worldEventManager.callEvents(&eventData);
  broadcastPlayerScoreUpdate(playerId);
  return true;
}

BZF_API bool bz_setPlayerOperator (int playerId)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return false;

  player->accessInfo.setOperator();
  return true;
}

BZF_API bool bz_addPlayerToGame( int playerID )
{
	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

	if (!player || !player->addWasDelayed)
		return false;

	AddPlayer(playerID, player);

	return true;
}

BZF_API bool bz_getPlayerHumanity( int playerId )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return false;

  return player->player.isHuman();
}

BZF_API bool bz_setPlayerLosses (int playerId, int losses)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return false;

  int old = player->score.getLosses();
  player->score.setLosses(losses);
  bz_PlayerScoreChangeEventData_V1 eventData = bz_PlayerScoreChangeEventData_V1(playerId, bz_eLosses, old, losses);
  worldEventManager.callEvents(&eventData);

  broadcastPlayerScoreUpdate(playerId);
  return true;
}

BZF_API bool bz_setPlayerTKs(int playerId, int tks)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return false;

  int old = player->score.getTKs();
  player->score.setTKs(tks);
  bz_PlayerScoreChangeEventData_V1 eventData = bz_PlayerScoreChangeEventData_V1(playerId, bz_eTKs, old, tks);
  worldEventManager.callEvents(&eventData);

  broadcastPlayerScoreUpdate(playerId);
  return true;
}


//-------------------------------------------------------------------------
BZF_API float bz_getPlayerRank (int playerId)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return -1;

  return player->score.ranking();
}

//-------------------------------------------------------------------------
BZF_API int bz_getPlayerWins (int playerId)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return -1;

  return player->score.getWins();
}

//-------------------------------------------------------------------------
BZF_API int bz_getPlayerLosses (int playerId)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return -1;

  return player->score.getLosses();
}

//-------------------------------------------------------------------------
BZF_API int bz_getPlayerTKs (int playerId)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return -1;

  return player->score.getTKs();
}

BZF_API int bz_howManyTimesPlayerKilledBy(int playerId, int killerId)
{
	GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

	if (!player)
		return 0;

	return player->player.howManyTimesKilledBy(killerId);
}

BZF_API bool bz_resetPlayerScore(int playerId)
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerId);

  if (!player)
    return false;

  int old = player->score.getWins();
  player->score.setWins(0);
  bz_PlayerScoreChangeEventData_V1 eventData = bz_PlayerScoreChangeEventData_V1(playerId, bz_eWins, old, 0);
  worldEventManager.callEvents(&eventData);

  old = player->score.getLosses();
  player->score.setLosses(0);
  eventData = bz_PlayerScoreChangeEventData_V1(playerId, bz_eLosses, old, 0);
  worldEventManager.callEvents(&eventData);

  old = player->score.getTKs();
  player->score.setTKs(0);
  eventData = bz_PlayerScoreChangeEventData_V1(playerId, bz_eTKs, old, 0);
  worldEventManager.callEvents(&eventData);

  broadcastPlayerScoreUpdate(playerId);
  return true;
}

BZF_API bz_APIStringList* bz_getGroupList ( void )
{
  bz_APIStringList *groupList = new bz_APIStringList;

  PlayerAccessMap::iterator itr = groupAccess.begin();
  while (itr != groupAccess.end()) {
    groupList->push_back(itr->first);
    ++itr;
  }
  return groupList;
}

BZF_API bz_APIStringList* bz_getGroupPerms ( const char* group )
{
  bz_APIStringList *permList = new bz_APIStringList;

  std::string groupName = group;
  groupName = TextUtils::toupper(groupName);
  PlayerAccessMap::iterator itr = groupAccess.find(groupName);
  if (itr == groupAccess.end())
    return permList;

  for (int i = 0; i < PlayerAccessInfo::lastPerm; i++)
  {
    if (itr->second.explicitAllows.test(i) && !itr->second.explicitDenys.test(i) )
      permList->push_back(nameFromPerm((PlayerAccessInfo::AccessPerm)i));
  }

  for(unsigned int c = 0; c < itr->second.customPerms.size(); c++)
    permList->push_back(TextUtils::toupper(itr->second.customPerms[c]));

  return permList;
}

BZF_API bool bz_groupAllowPerm ( const char* group, const char* perm )
{
  std::string permName = perm;
  permName = TextUtils::toupper(permName);

  PlayerAccessInfo::AccessPerm realPerm =  permFromName(permName);

  // find the group
  std::string groupName = group;
  groupName = TextUtils::toupper(groupName);
  PlayerAccessMap::iterator itr = groupAccess.find(groupName);
  if (itr == groupAccess.end())
    return false;

  if (realPerm != PlayerAccessInfo::lastPerm)
    return itr->second.explicitAllows.test(realPerm);
  else
  {
    for(unsigned int i = 0; i < itr->second.customPerms.size(); i++)
    {
      if ( permName == TextUtils::toupper(itr->second.customPerms[i]) )
	return true;
    }
  }
  return false;
}


BZF_API bool bz_sendTextMessage(int from, int to, bz_eMessageType type, const char* message)
{
  if (!message)
    return false;

  int playerIndex;
  PlayerId dstPlayer = AllPlayers;
  if ( to != BZ_ALLUSERS)
    dstPlayer = (PlayerId)to;

  if (from == BZ_SERVER)
    playerIndex = ServerPlayer;
  else
    playerIndex = from;

  MessageType msgType = ChatMessage;

  if (type == eActionMessage)
    msgType = ActionMessage;

  pendingChatMessages.push_back(PendingChatMessages(dstPlayer,playerIndex,message,msgType));

  return true;
}

BZF_API bool bz_sendTextMessage(int from, int to, const char* message)
{
  return bz_sendTextMessage(from, to, eChatMessage, message);
}

BZF_API bool bz_sendTextMessage(int from, bz_eTeamType to, bz_eMessageType, const char* message)
{
  switch(to)
  {
  case eNoTeam:
    return false;

  default:
  case eRogueTeam:
  case eRedTeam:
  case eGreenTeam:
  case eBlueTeam:
  case ePurpleTeam:
  case eRabbitTeam:
  case eHunterTeam:
  case eObservers:
    return bz_sendTextMessage(from,FirstTeam-(int)convertTeam(to),message);

  case eAdministrators:
    return bz_sendTextMessage(from,AdminPlayers,message);
  }
}

BZF_API bool bz_sendTextMessage(int from, bz_eTeamType to, const char* message)
{
  return bz_sendTextMessage(from, to, eChatMessage, message);
}

BZF_API bool bz_sendTextMessagef (int from, bz_eTeamType to, bz_eMessageType type, const char* fmt, ...)
{
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 1024, fmt, args);
  va_end(args);
  return bz_sendTextMessage (from, to, type, buffer);
}

BZF_API bool bz_sendTextMessagef (int from, bz_eTeamType to, const char* fmt, ...)
{
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 1024, fmt, args);
  va_end(args);
  return bz_sendTextMessage (from, to, buffer);
}

BZF_API bool bz_sendTextMessagef (int from, int to, bz_eMessageType type, const char* fmt, ...)
{
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 1024, fmt, args);
  va_end(args);
  return bz_sendTextMessage (from, to, type, buffer);
}

BZF_API bool bz_sendTextMessagef (int from, int to, const char* fmt, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 1024, fmt, args);
  va_end(args);
  return bz_sendTextMessage (from, to, buffer);
}

BZF_API bool bz_sentFetchResMessage ( int playerID,  const char* URL )
{
  if (playerID == BZ_SERVER || !URL)
    return false;

  teResourceType resType = eFile;

  std::vector<std::string> temp = TextUtils::tokenize(TextUtils::tolower(std::string(URL)),std::string("."));

  std::string ext = temp[temp.size()-1];
  if (ext == "wav")
    resType = eSound;


  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUShort (bufStart, 1);    // the count
  buf = nboPackUShort(buf, (short)resType);
  buf = nboPackUShort(buf, (unsigned short)strlen(URL));
  buf = nboPackString(buf, URL,strlen(URL));

  if (playerID == BZ_ALLUSERS)
    broadcastMessage(MsgFetchResources, (char*)buf - (char*)bufStart, bufStart);
  else
    directMessage(playerID,MsgFetchResources, (char*)buf - (char*)bufStart, bufStart);

  return true;
}

BZF_API bool bz_fireWorldWep ( const char* flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, int shotID, float dt, bz_eTeamType shotTeam )
{
  if (!pos || !flagType)
    return false;

  FlagTypeMap &flagMap = FlagType::getFlagMap();
  if (flagMap.find(std::string(flagType)) == flagMap.end())
    return false;

  FlagType *flag = flagMap.find(std::string(flagType))->second;

  PlayerId player;
  if ( fromPlayer == BZ_SERVER )
    player = ServerPlayer;
  else
    player = fromPlayer;

  int realShotID = shotID;
  if ( realShotID == 0)
    realShotID = world->getWorldWeapons().getNewWorldShotID();

  return fireWorldWep(flag,lifetime,player,pos,tilt,direction,-1,realShotID,dt,(TeamColor)convertTeam(shotTeam)) == realShotID;
}

BZF_API bool bz_fireWorldWep ( const char* flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, float speed, int* shotID, float dt, bz_eTeamType shotTeam )
{
	if (!pos || !flagType)
		return false;

	FlagTypeMap &flagMap = FlagType::getFlagMap();
	if (flagMap.find(std::string(flagType)) == flagMap.end())
		return false;

	FlagType *flag = flagMap.find(std::string(flagType))->second;

	PlayerId player;
	if ( fromPlayer == BZ_SERVER )
		player = ServerPlayer;
	else
		player = fromPlayer;

	int realShotID = world->getWorldWeapons().getNewWorldShotID(player);

	if (shotID != NULL)
		*shotID = realShotID;

	return fireWorldWep(flag,lifetime,player,pos,tilt,direction, speed,realShotID,dt,(TeamColor)convertTeam(shotTeam)) == realShotID;
}

BZF_API bool bz_fireWorldWep( const char* flagType, float lifetime, int fromPlayer, float *pos, float tilt, float direction, int* shotID , float dt, bz_eTeamType shotTeam )
{
	return bz_fireWorldWep(flagType, lifetime, fromPlayer, pos, tilt, direction, -1, shotID , dt, shotTeam );
}

BZF_API int bz_fireWorldGM ( int targetPlayerID, float lifetime, float *pos, float tilt, float direction, float dt, bz_eTeamType shotTeam)
{
  const char* flagType = "GM";

  if (!pos || !flagType)
    return false;

  FlagTypeMap &flagMap = FlagType::getFlagMap();
  if (flagMap.find(std::string(flagType)) == flagMap.end())
    return false;

  FlagType *flag = flagMap.find(std::string(flagType))->second;

  PlayerId player = ServerPlayer;

  int shotID =  world->getWorldWeapons().getNewWorldShotID();

  fireWorldGM(flag,targetPlayerID, lifetime,player,pos,tilt,direction,
    shotID, dt, (TeamColor)convertTeam(shotTeam));

  return shotID;
}

BZF_API uint32_t bz_getShotMetaData (int fromPlayer, int shotID , const char* name)
{
	uint32_t shotGUId = ShotManager.FindShotGUID(fromPlayer,shotID);

	if (shotGUId == 0 || name == NULL)
		return 0;

	Shots::ShotRef shot = ShotManager.FindShot(shotGUId);

	std::string n = name;
	if (shot->MetaData.find(n) == shot->MetaData.end())
		return 0;

	return shot->MetaData[n];
}

BZF_API void bz_setShotMetaData (int fromPlayer, int shotID , const char* name, uint32_t value)
{
	uint32_t shotGUId = ShotManager.FindShotGUID(fromPlayer,shotID);

	if (shotGUId == 0 || name == NULL)
		return;

	Shots::ShotRef shot = ShotManager.FindShot(shotGUId);

	std::string n = name;
	shot->MetaData[n] = value;
}

BZF_API bool bz_shotHasMetaData (int fromPlayer, int shotID , const char* name)
{
	uint32_t shotGUId = ShotManager.FindShotGUID(fromPlayer,shotID);

	if (shotGUId == 0 || name == NULL)
		return false;

	Shots::ShotRef shot = ShotManager.FindShot(shotGUId);

	std::string n = name;
	if (shot->MetaData.find(n) == shot->MetaData.end())
		return false;

	return true;
}

BZF_API uint32_t bz_getShotGUID (int fromPlayer, int shotID)
{
	return ShotManager.FindShotGUID(fromPlayer,shotID);
}

// time API
BZF_API double bz_getCurrentTime ( void )
{
  return TimeKeeper::getCurrent().getSeconds();
}

BZF_API void bz_getLocaltime ( bz_Time	*ts )
{
  if (!ts)
    return;

  TimeKeeper::localTime(&ts->year,&ts->month,&ts->day,&ts->hour,&ts->minute,&ts->second,&ts->daylightSavings);
}

BZF_API void bz_getUTCtime ( bz_Time	*ts )
{
  if (!ts)
    return;

  TimeKeeper::UTCTime(&ts->year, &ts->month, &ts->day,&ts->dayofweek, &ts->hour, &ts->minute, &ts->second, &ts->daylightSavings);
}

// info
BZF_API double bz_getBZDBDouble ( const char* variable )
{
  if (!variable)
    return 0.0;

  return BZDB.eval(std::string(variable));
}

BZF_API bz_ApiString bz_getBZDBString( const char* variable )
{
  if (!variable)
    return bz_ApiString("");

  return bz_ApiString(BZDB.get(std::string(variable)));
}

BZF_API bool bz_getBZDBBool( const char* variable )
{
  if (!variable)
    return false;

  return BZDB.eval(std::string(variable)) > 0.0;
}

BZF_API int bz_getBZDBInt( const char* variable )
{
  return (int)BZDB.eval(std::string(variable));
}

BZF_API int bz_getBZDBItemPerms( const char* variable )
{
  if (!bz_BZDBItemExists(variable))
    return BZ_BZDBPERM_NA;

  switch(BZDB.getPermission(std::string(variable)))
  {
  case StateDatabase::ReadWrite:
    return BZ_BZDBPERM_USER;

  case StateDatabase::Locked:
    return BZ_BZDBPERM_SERVER;

  case StateDatabase::ReadOnly:
    return BZ_BZDBPERM_CLIENT;

  default:
    return BZ_BZDBPERM_NA;
  }
}

BZF_API bool bz_getBZDBItemPesistent( const char* variable )
{
  if (!bz_BZDBItemExists(variable))
    return false;

  return BZDB.isPersistent(std::string(variable));
}

BZF_API bool bz_BZDBItemExists( const char* variable )
{
  if (!variable)
    return false;

  return BZDB.isSet(std::string(variable));
}

BZF_API bool bz_BZDBItemHasValue( const char* variable )
{
  if (!variable)
    return false;

  return BZDB.isSet(std::string(variable)) && BZDB.get(std::string(variable)).size() > 0;
}


void setVarPerms ( const char* variable, int perms, bool persistent)
{
  if (perms != BZ_BZDBPERM_NA)
  {
    switch(perms) {
case BZ_BZDBPERM_USER:
  BZDB.setPermission(std::string(variable),StateDatabase::ReadWrite);
  break;
case BZ_BZDBPERM_SERVER:
  BZDB.setPermission(std::string(variable),StateDatabase::Locked);
  break;
default:
  BZDB.setPermission(std::string(variable),StateDatabase::ReadOnly);
  break;
    }
  }
  BZDB.setPersistent(std::string(variable),persistent);
}

BZF_API bool bz_setBZDBDouble ( const char* variable, double val, int perms, bool persistent)
{
  if (!variable)
    return false;

  bool exists = BZDB.isSet(std::string(variable));

  BZDB.set(std::string(variable),TextUtils::format("%f",val));
  setVarPerms(variable,perms,persistent);

  return !exists;
}

BZF_API bool bz_setBZDBString( const char* variable, const char *val, int perms, bool persistent )
{
  if (!variable || !val)
    return false;

  bool exists = BZDB.isSet(std::string(variable));

  BZDB.set(std::string(variable),std::string(val));
  setVarPerms(variable,perms,persistent);

  return !exists;
}

BZF_API bool bz_setBZDBBool( const char* variable, bool val, int perms, bool persistent )
{
  if (!variable)
    return false;

  bool exists = BZDB.isSet(std::string(variable));

  BZDB.set(std::string(variable),TextUtils::format("%d",val));
  setVarPerms(variable,perms,persistent);

  return !exists;
}

BZF_API bool bz_setBZDBInt( const char* variable, int val, int perms, bool persistent )
{
  if (!variable)
    return false;

  bool exists = BZDB.isSet(std::string(variable));

  BZDB.set(std::string(variable),TextUtils::format("%d",val));
  setVarPerms(variable,perms,persistent);

  return !exists;
}


//-------------------------------------------------------------------------

BZF_API bool bz_updateBZDBDouble(const char *variable, double val)
{
  if (!variable)
    return false;

  if (!BZDB.isSet(std::string(variable)))
    return false;

  BZDB.set(std::string(variable), TextUtils::format("%f", val));

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_updateBZDBString(const char *variable, const char *val)
{
  if (!variable || !val)
    return false;

  if (!BZDB.isSet(std::string(variable)))
    return false;

  BZDB.set(std::string(variable), std::string(val));

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_updateBZDBBool(const char *variable, bool val)
{
  if (!variable)
    return false;

  if (!BZDB.isSet(std::string(variable)))
    return false;

  BZDB.set(std::string(variable), TextUtils::format("%d", val));

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_updateBZDBInt(const char *variable, int val)
{
  if (!variable)
    return false;

  if (!BZDB.isSet(std::string(variable)))
    return false;

  BZDB.set(std::string(variable), TextUtils::format("%d", val));

  return true;
}


void bzdbIterator (const std::string& name, void* userData)
{
  bz_APIStringList	* varList = static_cast<bz_APIStringList*>(userData);
  varList->push_back(name);
}

BZF_API int bz_getBZDBVarList( bz_APIStringList	*varList )
{
  if (!varList)
    return -1;

  varList->clear();
  BZDB.iterate(&bzdbIterator,varList);
  return (int)varList->size();
}

BZF_API void bz_resetBZDBVar( const char* variable )
{
  std::string command = "reset ";
  if ( variable && strlen(variable) )
    command += variable;
  else
    command += "*";

  CMDMGR.run(command);
}

BZF_API void bz_resetALLBZDBVars( void )
{
  bz_resetBZDBVar(NULL);
}



// logging
BZF_API void bz_debugMessage ( int _debugLevel, const char* message )
{
  if (!message)
    return;
  logDebugMessage(_debugLevel,"%s\n",message);
}

BZF_API void bz_debugMessagef ( int _debugLevel, const char* fmt, ... )
{
  char buffer[4096];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 4096, fmt, args);
  va_end(args);
  bz_debugMessage (_debugLevel, buffer);
}

BZF_API int bz_getDebugLevel ( void )
{
  return debugLevel;
}

BZF_API int bz_setDebugLevel ( int debug )
{
  if (debug >= 0  && debug <= 9)
    debugLevel = debug;

  return debugLevel;
}

// admin
BZF_API bool bz_kickUser ( int playerIndex, const char* reason, bool notify )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (!player || !reason)
    return false;

  if (notify)
  {
    std::string msg = std::string("You have been kicked from the server for: ") + reason;
    sendMessage(ServerPlayer, playerIndex, msg.c_str());

    msg = player->player.getCallSign();
    msg += std::string(" was kicked for:") + reason;
    sendMessage(ServerPlayer, AdminPlayers, msg.c_str());
  }
  removePlayer(playerIndex,reason);
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

BZF_API bool bz_IPUnbanUser ( const char* ip )
{
  // reload the banlist in case anyone else has added
  clOptions->acl.load();

  if (clOptions->acl.unban(ip))
    clOptions->acl.save();
  else
    return false;

  return true;
}


BZF_API bool bz_HostBanUser(const char* hostmask, const char* source, int duration, const char* reason)
{
  if (!reason || !hostmask)
    return false;

  std::string banner = "server";
  if (source)
    banner = source;

  // reload the banlist in case anyone else has added
  clOptions->acl.load();
  clOptions->acl.hostBan(hostmask, banner.c_str(), duration, reason);
  clOptions->acl.save();

  return true;

}
//-------------------------------------------------------------------------

BZF_API bool bz_IDUnbanUser ( const char* bzID )
{
  if (!bzID)
    return false;

  clOptions->acl.load();

  if (clOptions->acl.idUnban(bzID))
    clOptions->acl.save();
  else
    return false;

  return true;
}

BZF_API bool bz_HostUnbanUser(const char* hostmask)
{
  if (!hostmask)
    return false;

  clOptions->acl.load();

  if (clOptions->acl.hostUnban(hostmask))
    clOptions->acl.save();
  else
    return false;

  return true;
}

BZF_API unsigned int bz_getBanListSize( bz_eBanListType listType )
{
  switch(listType) {
    default:
    case eIPList:
      return (unsigned int)clOptions->acl.banList.size();

    case eHostList:
      return (unsigned int)clOptions->acl.hostBanList.size();

    case eIDList:
      return (unsigned int)clOptions->acl.idBanList.size();
  }

  return 0;
}

BZF_API const char* bz_getBanItem ( bz_eBanListType listType, unsigned int item )
{
  if (item > bz_getBanListSize(listType))
    return NULL;

  static std::string API_BAN_ITEM;

  API_BAN_ITEM = "";
  switch(listType) {
    default:
    case eIPList:
      API_BAN_ITEM = clOptions->acl.getBanMaskString(clOptions->acl.banList[item].addr).c_str();
      break;

    case eHostList:
      API_BAN_ITEM = clOptions->acl.hostBanList[item].hostpat.c_str();
      break;

    case eIDList:
      API_BAN_ITEM = clOptions->acl.idBanList[item].idpat.c_str();
      break;
  }

  if (API_BAN_ITEM.size())
    return API_BAN_ITEM.c_str();
  return NULL;
}

BZF_API const char* bz_getBanItemReason ( bz_eBanListType listType, unsigned int item )
{
  if (item > bz_getBanListSize(listType))
    return NULL;

  switch(listType) {
  default:
  case eIPList:
    return clOptions->acl.banList[item].reason.c_str();

  case eHostList:
    return clOptions->acl.hostBanList[item].reason.c_str();

  case eIDList:
    return clOptions->acl.idBanList[item].reason.c_str();
  }

  return NULL;
}

BZF_API const char* bz_getBanItemSource ( bz_eBanListType listType, unsigned int item )
{
  if (item > bz_getBanListSize(listType))
    return NULL;

  switch(listType) {
  default:
  case eIPList:
    return clOptions->acl.banList[item].bannedBy.c_str();

  case eHostList:
    return clOptions->acl.hostBanList[item].bannedBy.c_str();

  case eIDList:
    return clOptions->acl.idBanList[item].bannedBy.c_str();
  }

  return NULL;
}

BZF_API double bz_getBanItemDuration ( bz_eBanListType listType, unsigned int item )
{
  if (item > bz_getBanListSize(listType))
    return 0.0;

  TimeKeeper end = TimeKeeper::getCurrent();

  switch(listType) {
  default:
  case eIPList:
    end = clOptions->acl.banList[item].banEnd;
    break;

  case eHostList:
    end = clOptions->acl.hostBanList[item].banEnd;
    break;

  case eIDList:
    end = clOptions->acl.idBanList[item].banEnd;
    break;
  }

  if (end.getSeconds() > 30000000.0) // it's basicly forever
    return -1.0;

  return end.getSeconds() - TimeKeeper::getCurrent().getSeconds();
}


BZF_API bool bz_getBanItemIsFromMaster ( bz_eBanListType listType, unsigned int item )
{
  if (item > bz_getBanListSize(listType))
    return false;

  switch(listType) {
  default:
  case eIPList:
    return clOptions->acl.banList[item].fromMaster;

  case eHostList:
    return clOptions->acl.hostBanList[item].fromMaster;

  case eIDList:
    return clOptions->acl.idBanList[item].fromMaster;
  }

  return false;
}


BZF_API  bz_APIStringList *bz_getReports( void )
{
  bz_APIStringList *buffer = new bz_APIStringList;

  // Are we reporting to a file?
  if (clOptions->reportFile.size() == 0)
    return buffer;

  std::ifstream ifs(clOptions->reportFile.c_str(), std::ios::in);
  if (ifs.fail()) {
    return buffer;
  }

  std::string line;

  while (std::getline(ifs, line)) {
    buffer->push_back(line);
  }

  return buffer;
}


BZF_API int bz_getLagWarn( void ) {
  return int(clOptions->lagwarnthresh * 1000 + 0.5);
}

BZF_API bool bz_setLagWarn( int lagwarn ) {
  clOptions->lagwarnthresh = (float) (lagwarn / 1000.0);
  LagInfo::setThreshold(clOptions->lagwarnthresh,(float)clOptions->maxlagwarn);

  return true;
}

BZF_API bool bz_setTimeLimit( float timeLimit )
{
  if (timeLimit <= 0.0f)
    return false;
  clOptions->timeLimit = timeLimit;
  return true;
}

BZF_API float bz_getTimeLimit( void )
{
  return clOptions->timeLimit;
}

BZF_API bool bz_isTimeManualStart( void )
{
  return clOptions->timeManualStart;
}

BZF_API bool bz_isCountDownActive( void )
{
  return countdownActive;
}

BZF_API bool bz_isCountDownInProgress( void )
{
  return countdownDelay > 0;
}

BZF_API bool bz_isCountDownPaused( void )
{
  return clOptions->countdownPaused;
}

BZF_API bool bz_pollActive( void )
{
  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll"))
    return false;

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  /* make sure there is an unexpired poll */
  return ((arbiter != NULL) && !arbiter->knowsPoll());
}

BZF_API bool bz_pollVeto( void )
{
  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll"))
    return false;

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  /* make sure there is an unexpired poll */
  if ((arbiter != NULL) && !arbiter->knowsPoll())
    return false;
  /* poof */
  arbiter->forgetPoll();

  return true;
}

BZF_API bz_APIStringList *bz_getHelpTopics( void )
{
  return new bz_APIStringList(clOptions->textChunker.getChunkNames());
}

BZF_API bz_APIStringList *bz_getHelpTopic(std::string name)
{
  return new bz_APIStringList(*clOptions->textChunker.getTextChunk(name));
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

BZF_API bool bz_getStandardSpawn ( int playerID, float pos[3], float *rot )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
  if (!player)
    return false;

  // get the spawn position
  SpawnPosition* spawnPosition = new SpawnPosition(playerID,
    (!clOptions->respawnOnBuildings) || (player->player.isBot()),
    clOptions->gameType == ClassicCTF);

  pos[0] = spawnPosition->getX();
  pos[1] = spawnPosition->getY();
  pos[2] = spawnPosition->getZ();
  if (rot)
    *rot = spawnPosition->getAzimuth();

  return true;
}

BZF_API bool bz_killPlayer ( int playerID, bool spawnOnBase, int killerID, const char* flagType  )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
  if (!player)
    return false;

  if (!player->player.isAlive())
    return false;

  if (killerID == -1)
    killerID = ServerPlayer;

  FlagType *flag = NULL;
  if ( flagType )
  {
    FlagTypeMap &flagMap = FlagType::getFlagMap();
    if (flagMap.find(std::string(flagType)) == flagMap.end())
      return false;

    flag = flagMap.find(std::string(flagType))->second;
  }

  playerKilled(playerID, killerID, 0, -1, flag ? flag : Flags::Null, -1,spawnOnBase);

  return true;
}

BZF_API bool bz_givePlayerFlag ( int playerID, const char* flagType, bool force )
{
  FlagInfo* fi = NULL;
  GameKeeper::Player* gkPlayer = GameKeeper::Player::getPlayerByIndex(playerID);

  if (gkPlayer != NULL)
  {
    FlagType* ft = Flag::getDescFromAbbreviation(flagType);
    if (ft != Flags::Null)
    {
      // find unused and forced candidates
      FlagInfo* unused = NULL;
      FlagInfo* forced = NULL;
      for (int i = 0; i < numFlags; i++)
      {
	FlagInfo* fi2 = FlagInfo::get(i);
	if ((fi2 != NULL) && (fi2->flag.type == ft))
	{
	  forced = fi2;
	  if (fi2->player < 0)
	  {
	    unused = fi2;
	    break;
	  }
	}
      }

      // see if we need to force it
      if (unused != NULL)
	fi = unused;
      else if (forced != NULL)
      {
	if (force)
	  fi = forced;
	else  //all flags of this type are in use and force is set to false
	  return false;
      }
      else
      {
	//none of these flags exist in the game
	return false;
      }
    }
    else
    {
      //bogus flag
      return false;
    }
  }
  else //invald player
    return false;

  if (gkPlayer && fi)
  {
    // do not give flags to dead players
    if (!gkPlayer->player.isAlive())
      return false; //player is dead

    // deal with the player's current flag (if applicable)
    const int flagId = gkPlayer->player.getFlag();
    if (flagId >= 0)
    {
      FlagInfo& currentFlag = *FlagInfo::get(flagId);
      if (currentFlag.flag.type->flagTeam != NoTeam)
	dropFlag(currentFlag, gkPlayer->lastState.pos);// drop team flags
      else
	resetFlag(currentFlag);// reset non-team flags
    }

    grabFlag(gkPlayer->getIndex(), *fi, false);

    //flag successfully given to player
    return true;
  }
  //just in case? (a "wtf" case)
  return false;
}

BZF_API bool bz_removePlayerFlag ( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
  if (!player)
    return false;

  if (!player->player.isAlive())
    return false;

  zapFlagByPlayer(playerID);

  return true;
}

BZF_API void bz_resetFlags ( bool onlyUnused, bool keepTeamFlags )
{
  for (int i = 0; i < numFlags; i++)
  {
    FlagInfo &flag = *FlagInfo::get(i);
    // see if someone had grabbed flag,
    // and if it belongs to a certain team
    const int playerIndex = flag.player;
    const TeamColor flagTeam = flag.teamIndex();
    if ((!onlyUnused || (playerIndex == -1)) && (!keepTeamFlags || flagTeam == NoTeam))
      resetFlag(flag);
  }
}


BZF_API unsigned int bz_getNumFlags( void )
{
  return numFlags;
}

BZF_API const bz_ApiString bz_getName( int flag )
{
  FlagInfo *pFlag = FlagInfo::get(flag);
  if (!pFlag)
    return bz_ApiString("");

  return bz_ApiString(pFlag->flag.type->flagAbbv);
}

BZF_API const bz_ApiString bz_getFlagName( int flag )
{
	return bz_getName(flag);
}

BZF_API bool bz_resetFlag ( int flag )
{
  FlagInfo *pFlag = FlagInfo::get(flag);
  if(!pFlag)
    return false;

  resetFlag(*pFlag);

  return true;
}

BZF_API bool bz_moveFlag ( int flag, float pos[3] )
{
  FlagInfo *pFlag = FlagInfo::get(flag);
  if(!pFlag)
    return false;

  TeamColor teamIndex = pFlag->teamIndex();
  bool teamIsEmpty = true;
  if (teamIndex != ::NoTeam)
    teamIsEmpty = (team[teamIndex].team.size == 0);

  pFlag->resetFlag(pos, teamIsEmpty);
  sendFlagUpdate(*pFlag);

  return true;
}

BZF_API int bz_getPlayerFlagID ( int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);

  if (!player)
    return -1;

  if (!player->player.haveFlag())
    return -1;

  return player->player.getFlag();
}

BZF_API int bz_flagPlayer ( int flag )
{
  FlagInfo *pFlag = FlagInfo::get(flag);
  if(!pFlag)
    return -1;

  return pFlag->player;
}

BZF_API bool bz_getFlagPosition ( int flag, float* pos )
{
  FlagInfo *pFlag = FlagInfo::get(flag);
  if(!pFlag || !pos)
    return false;

  if (pFlag->player != -1)
  {
    GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(pFlag->player);

    if (!player)
      return false;

    memcpy(pos,player->lastState.pos,sizeof(float)*3);
  }
  else
    memcpy(pos,pFlag->flag.position,sizeof(float)*3);

  return true;
}


//-------------------------------------------------------------------------

BZF_API bool bz_setWorldSize(float size, float wallHeight)
{
  pluginWorldHeight = wallHeight;
  pluginWorldSize = size;

  return true;
}

//-------------------------------------------------------------------------

BZF_API void bz_setClientWorldDownloadURL(const char *URL)
{
  clOptions->cacheURL.clear();
  if (URL)
    clOptions->cacheURL = URL;
}

//-------------------------------------------------------------------------

BZF_API const bz_ApiString bz_getClientWorldDownloadURL(void)
{
  bz_ApiString URL;
  if (clOptions->cacheURL.size())
    URL = clOptions->cacheURL;
  return URL;
}

//-------------------------------------------------------------------------

BZF_API bool bz_saveWorldCacheFile(const char *file)
{
  if (!file)
    return false;
  return saveWorldCache(file);
}

BZF_API unsigned int bz_getWorldCacheSize ( void )
{
  return worldDatabaseSize;
}

BZF_API unsigned int bz_getWorldCacheData ( unsigned char *data )
{
  if (!data)
    return 0;

  memcpy(data,worldDatabase,worldDatabaseSize);
  return worldDatabaseSize;
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

BZF_API bool bz_addWorldBase( float *pos, float rot, float* scale, int teamIndex, bz_WorldObjectOptions options )
{
  if (!world || world->isFinisihed() || !pos || !scale)
    return false;

  world->addBase(pos,rot,scale,teamIndex,options.driveThru,options.shootThru);
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

BZF_API bool bz_addWorldWeapon( const char* _flagType, float *pos, float rot, float tilt, float initDelay, bz_APIFloatList &delays )
{
  if (!world || world->isFinisihed() || !_flagType )
    return false;

  std::string flagType = _flagType;

  FlagTypeMap &flagMap = FlagType::getFlagMap();
  if (flagMap.find(std::string(flagType)) == flagMap.end())
    return false;

  FlagType *flag = flagMap.find(std::string(flagType))->second;

  std::vector<float> realDelays;

  for(unsigned int i = 0; i < delays.size(); i++)
    realDelays.push_back(delays.get(i));

  world->addWeapon(flag, pos, rot, tilt, RogueTeam, initDelay, realDelays, synct);
  return true;
}

BZF_API bool bz_registerCustomMapObject ( const char* object, bz_CustomMapObjectHandler *handler )
{
  if (!object || !handler)
    return false;

  registerCustomMapObject(object,handler);
  return true;
}

BZF_API bool bz_removeCustomMapObject ( const char* object )
{
  if (!object)
    return false;

  removeCustomMapObject(object);
  return true;
}

BZF_API bool bz_getPublic( void )
{
  return clOptions->publicizeServer;
}

BZF_API bz_ApiString bz_getPublicAddr( void )
{
  if (!clOptions->publicizeServer)
    return bz_ApiString("");

  return bz_ApiString(clOptions->publicizedAddress);
}

BZF_API bz_ApiString bz_getPublicDescription( void )
{
  if (!clOptions->publicizeServer)
    return bz_ApiString("");

  return bz_ApiString(clOptions->publicizedTitle);
}

BZF_API int bz_getPublicPort( void )
{
  if (clOptions->useGivenPort)
    return clOptions->wksPort;

  return ServerPort;
}


BZF_API int bz_getLoadedPlugins( bz_APIStringList * list )
{
#ifdef BZ_PLUGINS
  std::vector<std::string>  pList = getPluginList();
  for (unsigned int i = 0; i < pList.size(); i++ )
    list->push_back(pList[i]);

  return (int)pList.size();
#else
  return -1;
  list = list; // quell unused var warning
#endif
}

BZF_API bool bz_loadPlugin( const char* path, const char *params )
{
#ifdef BZ_PLUGINS
  if (!path)
    return false;
  std::string config;
  if (params)
    config = params;

  logDebugMessage(2,"bz_loadPlugin: %s \n",path);

  return loadPlugin(std::string(path),config);
#else
  return false;
  path = path; // quell unused var warning
  params = params; // quell unused var warning
#endif
}

BZF_API bool bz_unloadPlugin( const char* path )
{
#ifdef BZ_PLUGINS
  if (!path)
    return false;

  return unloadPlugin(std::string(path));
#else
  return false;
  path = path; // quell unused var warning
#endif
}

BZF_API const char* bz_pluginBinPath(void)
{
#ifdef BZ_PLUGINS
  return lastPluginDir.c_str();
#else
  return "";
#endif
}


BZF_API bool bz_sendPlayCustomLocalSound ( int UNUSED(playerID), const char* UNUSED(soundName) )
{
  return false;
//   if (playerID == BZ_SERVER || !soundName)
//     return false;
//
//   void *buf, *bufStart = getDirectMessageBuffer();
//   buf = nboPackUShort(bufStart, LocalCustomSound);
//   buf = nboPackUShort(buf, (unsigned short)strlen(soundName));
//   buf = nboPackString(buf, soundName,strlen(soundName));
//   if (playerID == BZ_ALLUSERS)
//     broadcastMessage(MsgCustomSound, (char*)buf - (char*)bufStart, bufStart);
//   else
//     directMessage(playerID,MsgCustomSound, (char*)buf - (char*)bufStart, bufStart);
//
//   return true;
}

// custom pluginHandler
BZF_API bool bz_registerCustomPluginHandler ( const char* extension, bz_APIPluginHandler *handler )
{
  if (!extension || !handler)
    return false;

#ifdef BZ_PLUGINS
  std::string ext = extension;

  return registerCustomPluginHandler( ext,handler);
#else
  std::cerr << "This BZFlag server does not support plugins." << std::endl;
  return false;
#endif
}

BZF_API bool bz_removeCustomPluginHandler ( const char* extension, bz_APIPluginHandler *handler )
{
  if (!extension || !handler)
    return false;

#ifdef BZ_PLUGINS
  std::string ext = extension;

  return removeCustomPluginHandler( ext,handler);
#else
  std::cerr << "This BZFlag server does not support plugins." << std::endl;
  return false;
#endif
}

// team info
BZF_API int bz_getTeamCount ( bz_eTeamType _team )
{
  int teamIndex = (int)convertTeam(_team);

  int count = 0;
  if ( teamIndex < 0 || teamIndex >= NumTeams)
    return 0;

  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
    if (p == NULL)
      continue;

    if (p->player.getTeam() == teamIndex)
      count++;
  }

  return count;
}

BZF_API void bz_computeTeamScore( bool enabled )
{
  Score::KeepTeamScores = enabled;
}

BZF_API bool bz_computingTeamScore( void )
{
  return Score::KeepTeamScores;
}

BZF_API int bz_getTeamScore ( bz_eTeamType _team )
{
  int teamIndex = (int)convertTeam(_team);

  if ( teamIndex < 0 || teamIndex >= NumTeams)
    return 0;

  return team[teamIndex].team.getWins() - team[teamIndex].team.getLosses();
}

BZF_API int bz_getTeamWins ( bz_eTeamType _team )
{
  int teamIndex = (int)convertTeam(_team);

  if ( teamIndex < 0 || teamIndex >= NumTeams)
    return 0;

  return team[teamIndex].team.getWins() ;
}

BZF_API int bz_getTeamLosses ( bz_eTeamType _team )
{
  int teamIndex = (int)convertTeam(_team);

  if ( teamIndex < 0 || teamIndex >= NumTeams)
    return 0;

  return team[teamIndex].team.getLosses();
}

BZF_API void bz_setTeamWins (bz_eTeamType _team, int wins )
{
  int teamIndex = (int)convertTeam(_team);

  if ( teamIndex < 0 || teamIndex >= NumTeams)
    return ;

  int old = team[teamIndex].team.getWins();
  team[teamIndex].team.setWins(wins);
  bz_TeamScoreChangeEventData_V1 eventData = bz_TeamScoreChangeEventData_V1(_team, bz_eWins, old, wins);
  worldEventManager.callEvents(&eventData);

  checkTeamScore(-1, teamIndex);
  sendTeamUpdate(-1,teamIndex);
}

BZF_API void bz_setTeamLosses (bz_eTeamType _team, int losses )
{
  int teamIndex = (int)convertTeam(_team);

  if ( teamIndex < 0 || teamIndex >= NumTeams)
    return ;

  int old = team[teamIndex].team.getLosses();
  team[teamIndex].team.setLosses(losses);
  bz_TeamScoreChangeEventData_V1 eventData = bz_TeamScoreChangeEventData_V1(_team, bz_eWins, old, losses);
  worldEventManager.callEvents(&eventData);
  sendTeamUpdate(-1,teamIndex);
}

BZF_API void bz_resetTeamScore (bz_eTeamType _team )
{
  int teamIndex = (int)convertTeam(_team);

  if ( teamIndex >= NumTeams)
    return ;

  if ( teamIndex >= 0 )
  {
    int old = team[teamIndex].team.getWins();
    team[teamIndex].team.setWins(0);
    bz_TeamScoreChangeEventData_V1 eventData = bz_TeamScoreChangeEventData_V1(_team, bz_eWins, old, 0);
    worldEventManager.callEvents(&eventData);

    team[teamIndex].team.getLosses();
    team[teamIndex].team.setLosses(0);
    // Reinitialize eventData because we don't know that it wasn't changed above.
    eventData = bz_TeamScoreChangeEventData_V1(_team, bz_eWins, old, 0);
    worldEventManager.callEvents(&eventData);

    sendTeamUpdate(-1,teamIndex);
  }
  else
  {
    for ( int i =0; i < NumTeams; i++)
    {
      int old = team[i].team.getWins();
      team[i].team.setWins(0);
      bz_TeamScoreChangeEventData_V1 eventData = bz_TeamScoreChangeEventData_V1(convertTeam(i), bz_eWins, old, 0);
      worldEventManager.callEvents(&eventData);

      team[i].team.getLosses();
      team[i].team.setLosses(0);
      worldEventManager.callEvents(&eventData);

      sendTeamUpdate(-1,i);
    }
  }
}

BZF_API void bz_updateListServer ( void )
{
  publicize();
}

//-------------------------------------------------------------------------
//
//  URLFetch
//

class URLFetchTask
{
public:
  std::string url;
  std::string postData;
  size_t id;
  bz_BaseURLHandler *handler;
  double lastTime;
  void* token;
};

//-------------------------------------------------------------------------
//
//  URLFetchHandler
//

size_t urlWriteFunction(void *data, size_t size, size_t count, void *param)
{
  std::string& jobData = *(std::string*)param;

  jobData += std::string((char*)data,size*count);
  return size*count;
}

class URLFetchHandler
{
private:
  CURLM* curlHandle;
  CURL *currentJob;
  std::vector<URLFetchTask> Tasks;

  std::string bufferedJobData;

  size_t LastJob;

  double HTTPTimeout;
public:

  URLFetchHandler()
  {
    curlHandle = NULL;
    currentJob = NULL;
    LastJob = 1;
    HTTPTimeout = 60;
  }

  ~URLFetchHandler()
  {
    removeAllJobs();
    if (curlHandle) {
      curl_multi_cleanup(curlHandle);
    }
  }

  void Tick ( void )
  {
	  if (Tasks.empty())
		  return;

	  URLFetchTask &task = Tasks[0];

	  // check for jobs being done
	  if (currentJob)
	  {
		  int running;
		  curl_multi_perform(curlHandle, &running);

		  if (running == 0)
		  {
			  int      msgs_in_queue;
			  CURLMsg *pendingMsg = curl_multi_info_read(curlHandle, &msgs_in_queue);
			  if (currentJob == pendingMsg->easy_handle)
			  {
				  if (Tasks[0].handler->version >= 2)
				  {
					  ((bz_URLHandler_V2*)Tasks[0].handler)->token = Tasks[0].token;
				  }
				  if (bufferedJobData.size())
					  Tasks[0].handler->URLDone(Tasks[0].url.c_str(),bufferedJobData.c_str(),bufferedJobData.size(),true);
				  else
					  Tasks[0].handler->URLError(Tasks[0].url.c_str(),1,"Error");

				  bufferedJobData = "";
				  KillCurrentJob(false);
			  }
		  }

		  if (Tasks.size() &&(TimeKeeper::getCurrent().getSeconds() > Tasks[0].lastTime +HTTPTimeout))
		  {
			  Tasks[0].handler->URLTimeout(Tasks[0].url.c_str(),1);
			  KillCurrentJob(false);
		  }
	  }

	  if (!currentJob && Tasks.size())
	  {
		  currentJob = curl_easy_init();
		  curl_easy_setopt(currentJob, CURLOPT_URL, task.url.c_str());

		  if (task.postData.size())
			  curl_easy_setopt(currentJob, CURLOPT_POSTFIELDS, task.postData.c_str());

		  curl_easy_setopt(currentJob, CURLOPT_WRITEFUNCTION, urlWriteFunction);
		  curl_easy_setopt(currentJob, CURLOPT_WRITEDATA, &bufferedJobData);

		  curl_multi_add_handle(curlHandle, currentJob);
		  Tasks[0].lastTime = TimeKeeper::getCurrent().getSeconds();

		  int running;
		  curl_multi_perform(curlHandle, &running);
	  }
  }

  size_t addJob(const char *URL, bz_BaseURLHandler *handler,
    const char *postData, void* token)
  {
    if (!curlHandle)
    {
      curlHandle = curl_multi_init();
    }

    URLFetchTask newTask;
	newTask.token = token;
    newTask.handler = handler;
    newTask.url = URL;
    if (postData)
      newTask.postData = postData;
    newTask.id = ++LastJob;
    newTask.lastTime = TimeKeeper::getCurrent().getSeconds();
    Tasks.push_back(newTask);

    if (Tasks.size() == 1)
      Tick();
    return LastJob;
  }

  bool removeJob(size_t id)
  {
    if (Tasks.empty())
      return false;

    if (Tasks[0].id == id)
    {
      KillCurrentJob(true);
      Tasks.erase(Tasks.begin());
      return true;
    }
    else
    {
      for (size_t i = 0; i < Tasks.size(); i++)
      {
	if (Tasks[i].id == id)
	{
	  Tasks.erase(Tasks.begin()+i);
	  return true;
	}
      }
    }
    return false;
  }

  bool removeJob(const char* url)
  {
    if (Tasks.empty())
      return false;

    if (Tasks[0].url == url)
    {
      KillCurrentJob(true);
      Tasks.erase(Tasks.begin());
      return true;
    }
    else
    {
      for (size_t i = 0; i < Tasks.size(); i++)
      {
	if (Tasks[i].url == url)
	{
	  Tasks.erase(Tasks.begin()+i);
	  return true;
	}
      }
    }
    return false;
  }

  bool removeAllJobs()
  {
    KillCurrentJob(true);
    Tasks.clear();
    return true;
  }

private:

  void KillCurrentJob ( bool notify )
  {
    if (notify && Tasks.size())
      Tasks[0].handler->URLError(Tasks[0].url.c_str(),1,"Canceled");

    if (currentJob) {
      curl_multi_remove_handle(curlHandle, currentJob);
      curl_easy_cleanup(currentJob);
      currentJob = NULL;
    }

    if (!Tasks.empty())
      Tasks.erase(Tasks.begin());
  }
};

static URLFetchHandler urlFetchHandler;

void ApiTick ( void )
{
	urlFetchHandler.Tick();
}


BZF_API bool bz_addURLJob(const char *URL, bz_BaseURLHandler *handler, const char *postData)
{
  if (!URL) {
    return false;
  }

  return (urlFetchHandler.addJob(URL, handler, postData,NULL) != 0);
}

BZF_API bool bz_addURLJob(const char* URL, bz_URLHandler_V2* handler, void* token, const char* postData)
{
	if (!URL)
		return false;

	return (urlFetchHandler.addJob(URL, handler, postData,token) != 0);
}

//-------------------------------------------------------------------------

BZF_API size_t bz_addURLJobForID(const char *URL,
				 bz_BaseURLHandler *handler,
				 const char *postData)
{
  if (!URL) {
    return false;
  }

  return urlFetchHandler.addJob(URL, handler, postData,NULL);
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeURLJob(const char *URL)
{
  if (!URL) {
    return false;
  }
  return urlFetchHandler.removeJob(URL);
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeURLJobByID(size_t id)
{
  if (id == 0) {
    return false;
  }
  return urlFetchHandler.removeJob(id);
}

//-------------------------------------------------------------------------

BZF_API bool bz_stopAllURLJobs(void)
{
  urlFetchHandler.removeAllJobs();
  return true;
}

// inter plugin communication
std::map<std::string,std::string>	globalPluginData;

BZF_API bool bz_clipFieldExists ( const char *_name )
{
  if (!_name)
    return false;

  std::string name = _name;

  return globalPluginData.find(name) != globalPluginData.end();
}

BZF_API const char* bz_getclipFieldString ( const char *_name )
{
  if (!bz_clipFieldExists(_name))
    return NULL;

  std::string name = _name;

  return globalPluginData[name].c_str();
}

BZF_API float bz_getclipFieldFloat ( const char *_name )
{
  if (!bz_clipFieldExists(_name))
    return 0.0f;

  std::string name = _name;

  return (float)atof(globalPluginData[name].c_str());

}

BZF_API int bz_getclipFieldInt( const char *_name )
{
  if (!bz_clipFieldExists(_name))
    return 0;

  std::string name = _name;

  return atoi(globalPluginData[name].c_str());
}

BZF_API bool bz_setclipFieldString ( const char *_name, const char* data )
{
  bool existed = bz_clipFieldExists(_name);
  if (!data)
    return false;

  std::string name = _name;

  globalPluginData[name] = std::string(data);
  callClipFiledCallbacks(name.c_str());
  return existed;
}

BZF_API bool bz_setclipFieldFloat ( const char *_name, float data )
{
  bool existed = bz_clipFieldExists(_name);
  if (!data)
    return false;

  std::string name = _name;

  globalPluginData[name] = TextUtils::format("%f",data);
  callClipFiledCallbacks(name.c_str());
  return existed;
}

BZF_API bool bz_setclipFieldInt( const char *_name, int data )
{
  bool existed = bz_clipFieldExists(_name);
  if (!data)
    return false;

  std::string name = _name;

  globalPluginData[name] = TextUtils::format("%d",data);
  callClipFiledCallbacks(name.c_str());
  return existed;
}

void callClipFiledCallbacks ( const char* field )
{
  if (!field)
    return;

  std::string name = field;

  if (clipFieldMap.find(name) != clipFieldMap.end())
  {
    std::vector<bz_ClipFieldNotifier*> &vec = clipFieldMap[name];
    for ( unsigned int i = 0; i < (unsigned int)vec.size(); i++ )
      vec[i]->fieldChange(field);
  }

  if (clipFieldMap.find(std::string("")) != clipFieldMap.end())
  {
    std::vector<bz_ClipFieldNotifier*> &vec = clipFieldMap[std::string("")];
    for ( unsigned int i = 0; i < (unsigned int)vec.size(); i++ )
      vec[i]->fieldChange(field);
  }
}

BZF_API bool bz_addclipFieldNotifier ( const char *name, bz_ClipFieldNotifier *cb )
{
  if (!cb)
    return false;

  if (clipFieldMap.find(name) == clipFieldMap.end())
  {
    std::vector<bz_ClipFieldNotifier*> vec;
    clipFieldMap[name] = vec;
  }
  clipFieldMap[name].push_back(cb);

  return true;
}

BZF_API bool bz_removeclipFieldNotifier ( const char *name, bz_ClipFieldNotifier *cb )
{
  if (!cb)
    return false;

  if (clipFieldMap.find(name) == clipFieldMap.end())
    return false;

  std::vector<bz_ClipFieldNotifier*> &vec = clipFieldMap[name];
  for ( unsigned int i = 0; i < (unsigned int)vec.size(); i++ )
  {
    if ( vec[i] == cb)
    {
      vec.erase(vec.begin()+i);
      return true;
    }
  }
  return false;
}


BZF_API bz_ApiString bz_filterPath ( const char* path )
{
  if (!path)
    return bz_ApiString("");

  char *temp;
  temp = (char*)malloc(strlen(path)+1);

  strcpy(temp,path);

  // replace anything but alphanumeric charcters or dots in filename by '_'
  // should be safe on every supported platform

  char * buf = temp;
  while (*buf != '\0')
  {
    if ( !isalnum(*buf) ||  *buf != '.' )
      *buf = '_';

    buf++;
  }
  bz_ApiString ret(temp);
  free(temp);
  return ret;
}

BZF_API bool bz_saveRecBuf( const char * _filename, int seconds )
{
  if (!Record::enabled() || !_filename)
    return false;

  bool result = Record::saveBuffer( ServerPlayer, _filename, seconds);
  return result;
}

BZF_API bool bz_startRecBuf( void )
{
  if (Record::enabled())
    return false;

  return Record::start(ServerPlayer);
}

BZF_API bool bz_stopRecBuf( void )
{
  if (!Record::enabled())
    return false;

  return Record::stop(ServerPlayer);
}

BZF_API const char *bz_format(const char* fmt, ...)
{
  static std::string result;
  va_list args;
  va_start(args, fmt);
  result = TextUtils::vformat(fmt, args);
  va_end(args);
  return result.c_str();
}

BZF_API const char *bz_toupper(const char* val )
{
  static std::string temp;
  if (!val)
    return NULL;

  temp	 =	TextUtils::toupper(std::string(val));
  return temp.c_str();
}

BZF_API const char *bz_tolower(const char* val )
{
  static std::string temp;
  if (!val)
    return NULL;

  temp	 =	TextUtils::tolower(std::string(val));
  return temp.c_str();
}

BZF_API const char *bz_urlEncode(const char* val )
{
  static std::string temp;
  if (!val)
    return NULL;

  temp	 =	TextUtils::url_encode(std::string(val));
  return temp.c_str();
}


// server control
BZF_API bool bz_getShotMismatch( void )
{
  return checkShotMismatch;
}

BZF_API void bz_setShotMismatch ( bool value )
{
  checkShotMismatch = value;
}

BZF_API bool bz_isAutoTeamEnabled ( void )
{
    return clOptions->autoTeam;
}

BZF_API void bz_shutdown ( void )
{
  shutdownCommand(NULL,NULL);
}

BZF_API bool bz_restart ( void )
{
  if (clOptions->replayServer)
    return false;

  // close out the game, and begin anew
  // tell players to quit
  for (int i = 0; i < curMaxPlayers; i++)
    removePlayer(i,"Server Reset");

  delete world;
  world = NULL;
  delete[] worldDatabase;
  worldDatabase = NULL;

  gameOver = false;

  if (clOptions->timeManualStart)
  {
    countdownActive = false;
    countdownPauseStart = TimeKeeper::getNullTime();
    clOptions->countdownPaused = false;
  }

  bz_stopRecBuf();

  // start up all new and stuff
  if (!defineWorld())
  {
    shutdownCommand(NULL,NULL);
    return false;
  }

  return true;
}

BZF_API void bz_reloadLocalBans()
{
  // reload the banlist
  logDebugMessage(3,"Reloading bans\n");
  clOptions->acl.load();

  rescanForBans();
}

BZF_API void bz_reloadMasterBans()
{
  if (masterBanHandler.busy)
    return;

  // reload the banlist
  logDebugMessage(3,"Reloading master bans\n");
  clOptions->acl.purgeMasters();

  masterBanHandler.start();
}

BZF_API void bz_reloadGroups()
{
  logDebugMessage(3,"Reloading groups\n");
  groupAccess.clear();
  initGroups();
}

BZF_API void bz_reloadUsers()
{
  logDebugMessage(3,"Reloading users and passwords\n");
  userDatabase.clear();
  passwordDatabase.clear();

  if (userDatabaseFile.size())
    PlayerAccessInfo::readPermsFile(userDatabaseFile);
  GameKeeper::Player::reloadAccessDatabase();
}

BZF_API void bz_reloadHelp()
{
  // reload the text chunks
  logDebugMessage(3,"Reloading helpfiles\n");
  clOptions->textChunker.reload();
}

BZF_API void bz_superkill()
{
  superkillCommand(NULL,NULL);
}

BZF_API void bz_gameOver(int playerID, bz_eTeamType _team )
{
  void *buf, *bufStart = getDirectMessageBuffer();

  buf = nboPackUByte(bufStart, playerID);
  buf = nboPackUShort(buf, uint16_t( convertTeam(_team) == -1 ? NoTeam : convertTeam(_team) ));
  broadcastMessage(MsgScoreOver, (char*)buf - (char*)bufStart, bufStart);

  gameOver = true;

  if (clOptions->timeManualStart)
  {
    countdownActive = false;
    countdownPauseStart = TimeKeeper::getNullTime();
    clOptions->countdownPaused = false;
  }

  // fire off a game end event
  bz_GameStartEndEventData_V1	gameData;
  gameData.eventType = bz_eGameEndEvent;
  gameData.duration = clOptions->timeLimit;
  worldEventManager.callEvents(bz_eGameEndEvent,&gameData);
}

BZF_API bz_eTeamType bz_checkBaseAtPoint ( float pos[3] )
{
  return convertTeam(whoseBase(pos[0],pos[1],pos[2]));
}

BZF_API void bz_pauseCountdown ( const char *pausedBy )
{
  pauseCountdown(pausedBy);
}

BZF_API void bz_resumeCountdown ( const char *resumedBy )
{
  resumeCountdown(resumedBy);
}

BZF_API void bz_resetTeamScores ( void )
{
  resetTeamScores();
}

BZF_API void bz_startCountdown ( int delay, float limit, const char *byWho )
{
  startCountdown(delay,limit,byWho);
}

BZF_API	bz_eGameType bz_getGameType ( void  )
{
  if (clOptions->gameType == ClassicCTF)
    return eCTFGame;
  else if (clOptions->gameType == OpenFFA)
    return eOpenFFAGame;
  else if (clOptions->gameType == RabbitChase)
    return eRabbitGame;

  return eFFAGame;
}


// utility
BZF_API const char* bz_MD5 ( const char * str )
{
  if (!str)
    return NULL;
  return bz_MD5(str,strlen(str));
}

BZF_API const char* bz_MD5 ( const void * data, size_t size )
{
  static std::string hex;
  MD5 md5;
  md5.update((const unsigned char*)data, size);
  md5.finalize();
  hex = md5.hexdigest();
  return hex.c_str();
}

BZF_API const char* bz_getServerVersion ( void )
{
  return getAppVersion();
}

BZF_API const char* bz_getProtocolVersion ( void )
{
  return getProtocolVersion();
}

BZF_API bool bz_RegisterCustomFlag(const char* abbr, const char* name,
				   const char* help, bz_eShotType shotType,
				   bz_eFlagQuality quality)
{
  // require defined fields
  if (!abbr || !name || !help)
    return false;

  // length limits
  if ((strlen(abbr) > 2) || (strlen(name) > 32) || (strlen(help) > 128))
    return false;

  // don't register an existing flag (i.e. can't override builtins)
  if (Flag::getDescFromAbbreviation(abbr) != Flags::Null)
    return false;

  FlagEndurance e = FlagUnstable;
  switch(quality) {
    case eGoodFlag: e = FlagUnstable; break;
    case eBadFlag: e = FlagSticky; break;
    default: return false; // shouldn't happen
  }

  /* let this pointer dangle.  the constructor has taken care of all
   * the real work on the server side.
   */
  FlagType* tmp = new FlagType(name, abbr, e, (ShotType)shotType, (FlagQuality)quality, NoTeam, help, true);

  /* default the shot limit.  note that -sl will still take effect, if
   * this plugin is loaded from the command line or config file, since
   * it's processed in finalization
   */
  clOptions->flagLimit[tmp] = -1;

  /* notify existing players (if any) about the new flag type.  this
   * behavior is a bit questionable, but seems to be the Right
   * Thing(tm) to do.  new clients will get the notification during
   * flag negotiation, which is better.
   */
  char* buf = getDirectMessageBuffer();
  char* bufStart = buf;
  buf = (char*)tmp->packCustom(buf);
  broadcastMessage(MsgFlagType, buf-bufStart, bufStart);

  return true;
}

BZF_API bool bz_ChatFiltered(void)
{
  return clOptions->filterChat;
}

BZF_API bool bz_CallsignsFiltered(void)
{
  return clOptions->filterCallsigns;
}

BZF_API void bz_SetFiltering(bool chat, bool callsigns)
{
  clOptions->filterChat = chat;
  clOptions->filterCallsigns = callsigns;
}

BZF_API void bz_LoadFilterDefFile(const char* fileName)
{
  if (!fileName)
    return;

  clOptions->filterChat = true;
  clOptions->filterFilename = fileName;
  clOptions->filter.loadFromFile(fileName, true);

}

BZF_API void bz_AddFilterItem(const char* word, const char* expression)
{
  if (!word || !expression)
    return;

  clOptions->filterChat = true;
  std::string filterWord = word;
  std::string filterExpression = "";
  if (expression != NULL)
    filterExpression = expression;

  std::transform (filterWord.begin(),filterWord.end(), filterWord.begin(), tolower);

  clOptions->filter.addToFilter(filterWord,filterExpression);
}

BZF_API void bz_ClearFilter(void)
{
  clOptions->filter.clear();
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
