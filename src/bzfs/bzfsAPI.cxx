/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag global header
#include "global.h"

// implementation wrappers for all the bzf_ API functions
#include "bzfsAPI.h"

#ifndef BZ_PLUGINS
#  include <iostream>
#endif

#include "bzfs.h"
#include "bzfsMessages.h"
#include "bzfsClientMessages.h"
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
#include "CustomWorld.h"
#include "Permissions.h"
#include "CommandManager.h"
#include "bzfsPlugins.h"
#include "ObstacleMgr.h"
#include "BaseBuilding.h"
#include "BufferedNetworkMessage.h"
#include "ServerIntangibilityManager.h"
#include "bz_md5.h"
#include "version.h"
#include "BZDBCache.h"
#include "MotionUtils.h"
#include "Reports.h"

TimeKeeper synct=TimeKeeper::getCurrent();

std::map<std::string, std::vector<bz_ClipFiledNotifier*> > clipFieldMap;

void callClipFiledCallbacks ( const char* field );

std::map<std::string,float> APIWaitTimes;

class MasterBanURLHandler: public bz_BaseURLHandler
{
public:
  bool busy;
  unsigned int id;
  std::string theData;

  void doNext(void)
  {
    if(id >= clOptions->masterBanListURL.size())
    {
      rescanForBans();
      busy=false;
      return ;
    } theData="";
    bz_addURLJob(clOptions->masterBanListURL[id].c_str(), this);
    id++;
  }

  void start(void)
  {

    id=0;
    busy=true;
    doNext();
  }

  virtual void done(const char * /*URL*/, void *data, unsigned int size, bool complete)
  {
    if(!busy)
      return ;

    if(data && size > 0)
    {
      char *p=(char*)malloc(size+1);
      memcpy(p, data, size);
      p[size]=0;
      theData+=p;
      free(p);
    }

    if(complete)
    {
      clOptions->acl.merge(theData);
      doNext();
    }
  }

  virtual void timeout(const char * /*URL*/, int /*errorCode*/)
  {
    doNext();
  }

  virtual void error(const char * /*URL*/, int /*errorCode*/, const char * /*errorString*/)
  {
    doNext();
  }

  MasterBanURLHandler()
  {
    id=0;
    busy=false;
  }
};

MasterBanURLHandler masterBanHandler;

//-------------------------------------------------------------------------

bz_eTeamType convertTeam(TeamColor _team)
{
  switch(_team)
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
    case AutomaticTeam:
      return eAutomaticTeam;
  }
}

//-------------------------------------------------------------------------

TeamColor convertTeam(bz_eTeamType _team)
{
  if(_team > eObservers)
    return NoTeam;

  switch(_team)
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
    case eAutomaticTeam:
      return AutomaticTeam;
  }
  return (TeamColor)_team;
}

//-------------------------------------------------------------------------

void broadcastPlayerScoreUpdate(int playerID)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if (!player)
    return;

  sendPlayerScoreUpdate(player);
}

//-------------------------------------------------------------------------
unsigned int buildObjectIDFromObstacle ( const Obstacle &obstacle );
void setSolidObjectFromObstacle ( bz_APISolidWorldObject_V1 &object, const Obstacle &obstacle );

bz_APISolidWorldObject_V1 *APISolidFromObsacle( const Obstacle *obs )
{
  bz_APISolidWorldObject_V1 * solid = new bz_APISolidWorldObject_V1;
  solid->id = buildObjectIDFromObstacle(*obs);
  setSolidObjectFromObstacle(*solid,*obs);
  solid->subID = obs->getListID();
  return solid;
}



//******************************Versioning********************************************
BZF_API int bz_APIVersion(void)
{
  return BZ_API_VERSION;
}

//******************************bz_ApiString********************************************
class bz_ApiString::dataBlob
{
public:
  std::string str;
};

bz_ApiString::bz_ApiString()
{
  data=new dataBlob;
}

//-------------------------------------------------------------------------

bz_ApiString::bz_ApiString(const char *c)
{
  data=new dataBlob;
  data->str=c;
}

//-------------------------------------------------------------------------

bz_ApiString::bz_ApiString(const std::string &s)
{
  data=new dataBlob;
  data->str=s;
}

//-------------------------------------------------------------------------

bz_ApiString::bz_ApiString(const bz_ApiString &r)
{
  data=new dataBlob;
  data->str=r.data->str;
}

//-------------------------------------------------------------------------

bz_ApiString::~bz_ApiString()
{
  delete (data);
}

//-------------------------------------------------------------------------

bz_ApiString &bz_ApiString::operator=(const bz_ApiString &r)
{
  data->str=r.data->str;
  return  *this;
}

//-------------------------------------------------------------------------

bz_ApiString &bz_ApiString::operator=(const std::string &r)
{
  data->str=r;
  return  *this;
}

//-------------------------------------------------------------------------

bz_ApiString &bz_ApiString::operator=(const char *r)
{
  data->str=r;
  return  *this;
}

//-------------------------------------------------------------------------

bool bz_ApiString::operator==(const bz_ApiString &r)
{
  return data->str==r.data->str;
}

//-------------------------------------------------------------------------

bool bz_ApiString::operator==(const std::string &r)
{
  return data->str==r;
}

//-------------------------------------------------------------------------

bool bz_ApiString::operator==(const char *r)
{
  return data->str==r;
}

//-------------------------------------------------------------------------

bool bz_ApiString::operator!=(const bz_ApiString &r)
{
  return data->str!=r.data->str;
}

//-------------------------------------------------------------------------

bool bz_ApiString::operator!=(const std::string &r)
{
  return data->str!=r;
}

//-------------------------------------------------------------------------

bool bz_ApiString::operator!=(const char *r)
{
  return data->str!=r;
}

//-------------------------------------------------------------------------

unsigned int bz_ApiString::size(void)const
{
  return data->str.size();
}

//-------------------------------------------------------------------------

const char *bz_ApiString::c_str(void)const
{
  return data->str.c_str();
}

//-------------------------------------------------------------------------

void bz_ApiString::format(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  data->str=TextUtils::vformat(fmt, args);
  va_end(args);
}

//-------------------------------------------------------------------------

void bz_ApiString::replaceAll(const char *target, const char *with)
{
  if(!target)
    return ;

  if(!with)
    return ;

  data->str=TextUtils::replace_all(data->str, std::string(target), std::string(with));
}

//-------------------------------------------------------------------------

void bz_ApiString::tolower(void)
{
  data->str=TextUtils::tolower(data->str);
}

//-------------------------------------------------------------------------

void bz_ApiString::toupper(void)
{
  data->str=TextUtils::toupper(data->str);
}

//-------------------------------------------------------------------------

void bz_ApiString::urlEncode(void)
{
  data->str=TextUtils::url_encode(data->str);
}

//******************************bz_APIIntList********************************************
class bz_APIIntList::dataBlob
{
public:
  std::vector < int > list;
};

bz_APIIntList::bz_APIIntList()
{
  data=new dataBlob;
}

//-------------------------------------------------------------------------

bz_APIIntList::bz_APIIntList(const bz_APIIntList &r)
{
  data=new dataBlob;
  data->list=r.data->list;
}

//-------------------------------------------------------------------------

bz_APIIntList::bz_APIIntList(const std::vector < int >  &r)
{
  data=new dataBlob;
  data->list=r;
}

//-------------------------------------------------------------------------

bz_APIIntList::~bz_APIIntList()
{
  delete (data);
}

//-------------------------------------------------------------------------

void bz_APIIntList::push_back(int value)
{
  data->list.push_back(value);
}

//-------------------------------------------------------------------------

int bz_APIIntList::get(unsigned int i)
{
  if(i >= data->list.size())
    return 0;

  return data->list[i];
}

//-------------------------------------------------------------------------

const int &bz_APIIntList::operator[](unsigned int i)const
{
  return data->list[i];
}

//-------------------------------------------------------------------------

bz_APIIntList &bz_APIIntList::operator=(const bz_APIIntList &r)
{
  data->list=r.data->list;
  return  *this;
}

//-------------------------------------------------------------------------

bz_APIIntList &bz_APIIntList::operator=(const std::vector < int >  &r)
{
  data->list=r;
  return  *this;
}

//-------------------------------------------------------------------------

unsigned int bz_APIIntList::size(void)
{
  return data->list.size();
}

//-------------------------------------------------------------------------

void bz_APIIntList::clear(void)
{
  data->list.clear();
}

//-------------------------------------------------------------------------

BZF_API bz_APIIntList *bz_newIntList(void)
{
  return new bz_APIIntList;
}

//-------------------------------------------------------------------------

BZF_API void bz_deleteIntList(bz_APIIntList *l)
{
  delete (l);
}

//******************************bz_APIFloatList********************************************
class bz_APIFloatList::dataBlob
{
public:
  std::vector < float > list;
};

bz_APIFloatList::bz_APIFloatList()
{
  data=new dataBlob;
}

//-------------------------------------------------------------------------

bz_APIFloatList::bz_APIFloatList(const bz_APIFloatList &r)
{
  data=new dataBlob;
  data->list=r.data->list;
}

//-------------------------------------------------------------------------

bz_APIFloatList::bz_APIFloatList(const std::vector < float >  &r)
{
  data=new dataBlob;
  data->list=r;
}

//-------------------------------------------------------------------------

bz_APIFloatList::~bz_APIFloatList()
{
  delete (data);
}

//-------------------------------------------------------------------------

void bz_APIFloatList::push_back(float value)
{
  data->list.push_back(value);
}

//-------------------------------------------------------------------------

float bz_APIFloatList::get(unsigned int i)
{
  if(i >= data->list.size())
    return 0;

  return data->list[i];
}

//-------------------------------------------------------------------------

const float &bz_APIFloatList::operator[](unsigned int i)const
{
  return data->list[i];
}

//-------------------------------------------------------------------------

bz_APIFloatList &bz_APIFloatList::operator=(const bz_APIFloatList &r)
{
  data->list=r.data->list;
  return  *this;
}

//-------------------------------------------------------------------------

bz_APIFloatList &bz_APIFloatList::operator=(const std::vector < float >  &r)
{
  data->list=r;
  return  *this;
}

//-------------------------------------------------------------------------

unsigned int bz_APIFloatList::size(void)
{
  return data->list.size();
}

//-------------------------------------------------------------------------

void bz_APIFloatList::clear(void)
{
  data->list.clear();
}

//-------------------------------------------------------------------------

BZF_API bz_APIFloatList *bz_newFloatList(void)
{
  return new bz_APIFloatList;
}

//-------------------------------------------------------------------------

BZF_API void bz_deleteFloatList(bz_APIFloatList *l)
{
  if(l)
    delete (l);
}

//-------------------------------------------------------------------------

BZF_API bz_APIStringList *bz_newStringList(void)
{
  return new bz_APIStringList;
}

//-------------------------------------------------------------------------

BZF_API void bz_deleteStringList(bz_APIStringList *l)
{
  if(l)
    delete (l);
}

//******************************bz_APIStringList********************************************
class bz_APIStringList::dataBlob
{
public:
  std::vector < bz_ApiString > list;
};


bz_APIStringList::bz_APIStringList()
{
  data=new dataBlob;
}

//-------------------------------------------------------------------------

bz_APIStringList::bz_APIStringList(const bz_APIStringList &r)
{
  data=new dataBlob;
  data->list=r.data->list;
}

//-------------------------------------------------------------------------

bz_APIStringList::bz_APIStringList(const std::vector < std::string >  &r)
{
  data=new dataBlob;

  for(unsigned int i=0; i < r.size(); i++)
  {
    std::string d=r[i];
    data->list.push_back(bz_ApiString(d));
  }
}

//-------------------------------------------------------------------------

bz_APIStringList::~bz_APIStringList()
{
  delete (data);
}

//-------------------------------------------------------------------------

void bz_APIStringList::push_back(const std::string &value)
{
  data->list.push_back(bz_ApiString(value));
}

//-------------------------------------------------------------------------
bz_ApiString emptyString;

const bz_ApiString& bz_APIStringList::get(unsigned int i)
{
  if(i >= data->list.size())
    return emptyString;

  return data->list[i];
}

//-------------------------------------------------------------------------

const bz_ApiString &bz_APIStringList::operator[](unsigned int i)const
{
  return data->list[i];
}

//-------------------------------------------------------------------------

bz_APIStringList &bz_APIStringList::operator=(const bz_APIStringList &r)
{
  data->list=r.data->list;
  return  *this;
}

//-------------------------------------------------------------------------

bz_APIStringList &bz_APIStringList::operator=(const std::vector < std::string >  &r)
{
  data->list.clear();

  for(unsigned int i=0; i < r.size(); i++)
    data->list.push_back(bz_ApiString(r[i]));

  return  *this;
}

//-------------------------------------------------------------------------

unsigned int bz_APIStringList::size(void)
{
  return data->list.size();
}

//-------------------------------------------------------------------------

void bz_APIStringList::clear(void)
{
  data->list.clear();
}

//-------------------------------------------------------------------------
bool bz_APIStringList::contains(const char* value)
{
  if (!value)
    return false;

  return std::find(data->list.begin(), data->list.end(), bz_ApiString(value)) != data->list.end();
}

//-------------------------------------------------------------------------

bool bz_APIStringList::contains(const std::string &value)
{
  return std::find(data->list.begin(), data->list.end(), bz_ApiString(value)) != data->list.end();
}

//-------------------------------------------------------------------------


//-------------------------------------------------------------------------

void bz_APIStringList::tokenize(const char *in, const char *delims, int maxTokens, bool useQuotes)
{
  clear();
  if(!in || !delims)
    return ;

  std::vector < std::string > list=TextUtils::tokenize(std::string(in), std::string(delims), maxTokens, useQuotes);

  for(unsigned int i=0; i < list.size(); i++)
    data->list.push_back(bz_ApiString(list[i]));
}

//******************************bz_APIWorldObjectList********************************************

class bz_APIWorldObjectList::dataBlob
{
public:
  std::vector < bz_APIBaseWorldObject * > list;
};

bz_APIWorldObjectList::bz_APIWorldObjectList()
{
  data=new dataBlob;
}

//-------------------------------------------------------------------------

bz_APIWorldObjectList::bz_APIWorldObjectList(const bz_APIWorldObjectList &r)
{
  data=new dataBlob;
  data->list=r.data->list;
}

//-------------------------------------------------------------------------

bz_APIWorldObjectList::~bz_APIWorldObjectList()
{
  delete (data);
}

//-------------------------------------------------------------------------

void bz_APIWorldObjectList::push_back(bz_APIBaseWorldObject *value)
{
  data->list.push_back(value);
}

//-------------------------------------------------------------------------

bz_APIBaseWorldObject *bz_APIWorldObjectList::get(unsigned int i)
{
  return data->list[i];
}

//-------------------------------------------------------------------------

const bz_APIBaseWorldObject *bz_APIWorldObjectList::operator[](unsigned int i)const
{
  return data->list[i];
}

//-------------------------------------------------------------------------

bz_APIWorldObjectList &bz_APIWorldObjectList::operator=(const bz_APIWorldObjectList &r)
{
  data->list=r.data->list;
  return  *this;
}

//-------------------------------------------------------------------------

unsigned int bz_APIWorldObjectList::size(void)
{
  return data->list.size();
}

//-------------------------------------------------------------------------

void bz_APIWorldObjectList::clear(void)
{
  data->list.clear();
}

//-------------------------------------------------------------------------

void bz_releaseWorldObjectList(bz_APIWorldObjectList *objectList)
{
  if(objectList)
    delete (objectList);
}



// events
BZF_API bool bz_registerEvent(bz_eEventType eventType, bz_EventHandler *eventHandler)
{
  if(!eventHandler)
    return false;

  worldEventManager.addEvent(eventType, eventHandler);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeEvent(bz_eEventType eventType, bz_EventHandler *eventHandler)
{
  if(!eventHandler)
    return false;

  worldEventManager.removeEvent(eventType, eventHandler);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_registerNonPlayerConnectionHandler(int connectionID, bz_NonPlayerConnectionHandler *handler)
{
  if(!handler || netConnectedPeers.find(connectionID)==netConnectedPeers.end() || netConnectedPeers[connectionID].player!=-1)
    return false;

  netConnectedPeers[connectionID].notifyList.push_back(handler);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeNonPlayerConnectionHandler(int connectionID, bz_NonPlayerConnectionHandler *handler)
{
  if(!handler || netConnectedPeers.find(connectionID)==netConnectedPeers.end() || netConnectedPeers[connectionID].player!=-1)
    return false;

  for(unsigned int i=0; i < netConnectedPeers[connectionID].notifyList.size(); i++)
  {
    if(netConnectedPeers[connectionID].notifyList[i]==handler)
    {
      netConnectedPeers[connectionID].notifyList.erase(netConnectedPeers[connectionID].notifyList.begin()+i);
      return true;
    }
  }
  return false;
}

//-------------------------------------------------------------------------

BZF_API bool bz_sendNonPlayerData(int connectionID, const void *data, unsigned int size)
{
  if(!data || size==0 || netConnectedPeers.find(connectionID)==netConnectedPeers.end() || netConnectedPeers[connectionID].player!=-1 || !netConnectedPeers[connectionID].handler)
    return false;

  bool sendOneNow = netConnectedPeers[connectionID].pendingSendChunks.size() == 0;
  unsigned int pos=0;
  while(pos < size)
  {
    char *dataBlob;
    unsigned int thisSize;

    dataBlob=(char*)data+pos;
    if(size-pos < maxNonPlayerDataChunk)
      thisSize=size-pos;
    else
      thisSize=maxNonPlayerDataChunk;

    netConnectedPeers[connectionID].pendingSendChunks.push_back(NonPlayerDataChunk(dataBlob, thisSize));

    pos+=thisSize;
  }

  // send off at least one now if it was empty
  if (sendOneNow)
    sendBufferedNetDataForPeer(netConnectedPeers[connectionID]);

  return true;
}

BZF_API unsigned int bz_getNonPlayerConnectionOutboundPacketCount ( int connectionID )
{
  if( netConnectedPeers.find(connectionID)==netConnectedPeers.end() || netConnectedPeers[connectionID].player!=-1 || !netConnectedPeers[connectionID].handler)
    return 0;

  return  netConnectedPeers[connectionID].pendingSendChunks.size();
}

BZF_API const char* bz_getNonPlayerConnectionIP ( int connectionID )
{
  if( netConnectedPeers.find(connectionID)==netConnectedPeers.end() || netConnectedPeers[connectionID].player!=-1 || !netConnectedPeers[connectionID].handler)
    return 0;

  unsigned int address = (unsigned int)netConnectedPeers[connectionID].handler->getIPAddress().s_addr;

  unsigned char *a = (unsigned char*)&address;
  return TextUtils::format("%d.%d.%d.%d",a[0],a[1],a[2],a[3]).c_str();
}

BZF_API const char* bz_getNonPlayerConnectionHost ( int connectionID )
{
  if( netConnectedPeers.find(connectionID)==netConnectedPeers.end() || netConnectedPeers[connectionID].player!=-1 || !netConnectedPeers[connectionID].handler)
    return 0;

  return netConnectedPeers[connectionID].handler->getHostname();
}


//-------------------------------------------------------------------------

BZF_API bool bz_disconnectNonPlayerConnection(int connectionID)
{
  if(netConnectedPeers.find(connectionID)==netConnectedPeers.end() || netConnectedPeers[connectionID].player!=-1)
    return false;

  // flush out the rest of it's sends
  while(netConnectedPeers[connectionID].pendingSendChunks.size())
    sendBufferedNetDataForPeer(netConnectedPeers[connectionID]);

  for(unsigned int i=0; i < netConnectedPeers[connectionID].notifyList.size(); i++)
    netConnectedPeers[connectionID].notifyList[i]->disconnect(connectionID);

  netConnectedPeers[connectionID].handler->flushData();
  delete(netConnectedPeers[connectionID].handler);
  netConnectedPeers[connectionID].handler = NULL;
  netConnectedPeers[connectionID].notifyList.clear();
  netConnectedPeers[connectionID].pendingSendChunks.clear();
  netConnectedPeers[connectionID].deleteMe = true;
  return true;
}

class APIPendingPacket
{
public:
  unsigned int	size;
  char		*data;

  APIPendingPacket(unsigned int s, const char *d)
  {
    size = s;
    if (!s)
      data = NULL;
    else
    {
      data = (char*)malloc(s);
      memcpy(data,d,s);
    }
  }

  APIPendingPacket(const APIPendingPacket& p)
  {
    size = p.size;
    if (!size)
      data = NULL;
    else
    {
      data = (char*)malloc(size);
      memcpy(data,p.data,size);
    }
  }

 ~APIPendingPacket()
 {
   if (data)
     free(data);
 }
};


BZF_API const char *bz_BasePlayerRecord::getCustomData ( const char* key )
{
  GameKeeper::Player* player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player || !key)
    return NULL;

  if (player->customData.find(std::string(key)) == player->customData.end())
    return NULL;

  return player->customData[std::string(key)].c_str();
}

BZF_API bool bz_BasePlayerRecord::setCustomData ( const char* key, const char* data ) 
{
  return bz_setPayerCustomData(playerID,key,data);
}

BZF_API bool bz_setPayerCustomData(int playerID, const char* key, const char* data )
{
  GameKeeper::Player* player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player || !key)
    return false;

  std::string k,v;
  k = key;
  if(data)
    v = data;

  bool found = false;
  if (player->customData.find(k) != player->customData.end())
    return found = true;

  player->customData[k] = v;
  sendPlayerCustomDataPair(playerID,k,v);

  // notify on the change
  bz_PlayerSentCustomData_V1 eventData;
  eventData.playerID = playerID;
  eventData.key = k;
  eventData.data = v;

  eventData.eventType =bz_ePlayerCustomDataChanged;
  worldEventManager.callEvents(eventData);

  return found;
}

//-------------------------------------------------------------------------

BZF_API bool bz_updatePlayerData(bz_BasePlayerRecord* playerRecord)
{
  if(!playerRecord)
    return false;

  GameKeeper::Player* player=GameKeeper::Player::getPlayerByIndex(playerRecord->playerID);
  if(!player)
    return false;

  playerStateToAPIState(playerRecord->lastKnownState, player->lastState);
  playerRecord->lastUpdateTime=player->lastState.lastUpdateTime;

  playerStateToAPIState(playerRecord->currentState, player->getCurrentStateAsState());

  playerRecord->currentFlagID=player->player.getFlag();
  FlagInfo* flagInfo=FlagInfo::get(playerRecord->currentFlagID);

  std::string label;
  if(flagInfo && flagInfo->flag.type)
    label=flagInfo->flag.type->label();
  playerRecord->currentFlag=label;

  std::vector < FlagType* > flagHistoryList=player->flagHistory.get();

  playerRecord->flagHistory.clear();
  for(unsigned int i=0; i < flagHistoryList.size(); i++)
    playerRecord->flagHistory.push_back(flagHistoryList[i]->label());

  playerRecord->groups.clear();
  playerRecord->groups=player->accessInfo.groups;

  playerRecord->admin=player->accessInfo.isAdmin();
  playerRecord->op=player->accessInfo.isOperator();

  playerRecord->verified=player->accessInfo.isVerified();

  playerRecord->spawned=player->player.isAlive();
  playerRecord->lag=player->lagInfo.getLag();
  playerRecord->jitter=player->lagInfo.getJitter();
  playerRecord->packetLoss=(float)player->lagInfo.getLoss();

  playerRecord->rank=player->score.ranking();
  playerRecord->wins=player->score.getWins();
  playerRecord->losses=player->score.getLosses();
  playerRecord->teamKills=player->score.getTKs();
  playerRecord->canSpawn = player->isSpawnable();
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_getAdmin ( int playerID )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player)
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

//-------------------------------------------------------------------------

BZF_API bool bz_hasPerm(int playerID, const char *perm)
{
  if(!perm)
    return false;

  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player)
    return false;

  std::string permName=perm;

  permName=TextUtils::toupper(permName);

  PlayerAccessInfo::AccessPerm realPerm=permFromName(permName);

  if(realPerm!=PlayerAccessInfo::lastPerm)
    return player->accessInfo.hasPerm(realPerm);
  else
    return player->accessInfo.hasCustomPerm(permName.c_str());
}

//-------------------------------------------------------------------------

BZF_API bool bz_grantPerm(int playerID, const char *perm)
{
  if(!perm)
    return false;

  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return false;

  std::string permName=perm;

  permName=TextUtils::toupper(permName);

  PlayerAccessInfo::AccessPerm realPerm=permFromName(permName);

  if(realPerm==PlayerAccessInfo::lastPerm)
    return false;

  player->accessInfo.grantPerm(realPerm);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_revokePerm(int playerID, const char *perm)
{
  if(!perm)
    return false;

  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return false;

  std::string permName=perm;

  permName=TextUtils::toupper(permName);

  PlayerAccessInfo::AccessPerm realPerm=permFromName(permName);

  if(realPerm==PlayerAccessInfo::lastPerm)
    return false;

  player->accessInfo.revokePerm(realPerm);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_getPlayerIndexList(bz_APIIntList *playerList)
{
  playerList->clear();

  for(int i=0; i < curMaxPlayers; i++)
  {
    if(GameKeeper::Player::getPlayerByIndex(i))
      playerList->push_back(i);
  }
  return playerList->size() > 0;
}

BZF_API int bz_getPlayerCount ( void )
{
  return curMaxPlayers;
}

BZF_API bool bz_anyPlayers ( void )
{
  return curMaxPlayers > 0;
}


//-------------------------------------------------------------------------

BZF_API bz_APIIntList *bz_getPlayerIndexList(void)
{
  bz_APIIntList *playerList=new bz_APIIntList;

  for(int i=0; i < curMaxPlayers; i++)
  {
    if(GameKeeper::Player::getPlayerByIndex(i))
      playerList->push_back(i);
  }
  return playerList;
}

bz_BasePlayerRecord *APIPlayerFromRecord ( GameKeeper::Player *player )
{
  if (!player)
    return NULL;

  bz_BasePlayerRecord *playerRecord=new bz_BasePlayerRecord;

  if (!playerRecord)
    return NULL;

  playerRecord->callsign=player->player.getCallSign();
  playerRecord->playerID=player->getIndex();
  playerRecord->bzID=player->getBzIdentifier();
  playerRecord->team=convertTeam(player->player.getTeam());

  playerRecord->spawned=player->player.isAlive();
  playerRecord->verified=player->accessInfo.isVerified();
  playerRecord->globalUser=player->authentication.isGlobal();


  playerRecord->clientVersion = player->player.getClientVersion();

  if(player->netHandler)
    playerRecord->ipAddress=player->netHandler->getTargetIP();
  else
    playerRecord->ipAddress = "localhost";

  playerRecord->update();
  return playerRecord;
}

//-------------------------------------------------------------------------

BZF_API bz_BasePlayerRecord *bz_getPlayerByIndex(int index)
{
  return APIPlayerFromRecord(GameKeeper::Player::getPlayerByIndex(index));
}

//-------------------------------------------------------------------------

BZF_API bz_BasePlayerRecord *bz_getPlayerByCallsign(const char* callsign)
{
  if (!callsign)
    return NULL;

  std::string upperName = TextUtils::toupper(std::string(callsign));

  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
    if ((p == NULL))
      continue;

    if (upperName == TextUtils::toupper(std::string(p->player.getCallSign())) )
      return APIPlayerFromRecord(p);
  }

  return NULL;
}

//-------------------------------------------------------------------------

BZF_API bz_BasePlayerRecord * bz_getPlayerByBZID ( int BZID )
{
  for (int i = 0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
    if ((p == NULL))
      continue;

    if (BZID == atoi(p->getBzIdentifier().c_str()))
      return APIPlayerFromRecord(p);
  }

  return NULL;
}

//-------------------------------------------------------------------------

BZF_API bool bz_freePlayerRecord(bz_BasePlayerRecord *playerRecord)
{
  if(playerRecord)
    delete (playerRecord);

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_isPlayerPaused(int playerID)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return false;

  return player->player.isPaused();
}

BZF_API bool bz_canPlayerSpawn( int playerID )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return false;

  return player->isSpawnable();
}

BZF_API bz_eTeamType bz_getPlayerTeam( int playerID )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return eNoTeam;

  return convertTeam(player->player.getTeam());
}

BZF_API const char* bz_getPlayerCallsign( int playerID )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return NULL;

  return player->player.getCallSign();
}

BZF_API const char* bz_getPlayerIPAddress( int playerID )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return NULL;

  return player->netHandler->getTargetIP();
}

BZF_API bool bz_setPlayerSpawnable( int playerID, bool spawn )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return false;

  player->setSpawnable(spawn);

  return true;
}

BZF_API bool bz_setPlayerLimboMessage( int playerID, const char* text )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return false;

  std::string realText;
  if ( text )
    realText = text;

  sendMsgLimboMessage(playerID,realText);
  return true;
}



//-------------------------------------------------------------------------

BZF_API const char *bz_getPlayerFlag(int playerID)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return NULL;

  FlagInfo *flagInfo=FlagInfo::get(player->player.getFlag());
  if(!flagInfo)
    return NULL;

  return FlagInfo::get(player->player.getFlag())->flag.type->flagAbbv.c_str();
}

//-------------------------------------------------------------------------

BZF_API bool bz_getPlayerCurrentState(int playerID, bz_PlayerUpdateState &state)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player)
    return false;

  playerStateToAPIState(state, player->getCurrentStateAsState());

  return true;
}

//-------------------------------------------------------------------------

BZF_API unsigned int bz_getTeamPlayerLimit(bz_eTeamType _team)
{
  switch(_team)
  {
    case eRogueTeam:
    case eBlueTeam:
    case eRedTeam:
    case eGreenTeam:
    case ePurpleTeam:
    case eObservers:
      return clOptions->maxTeam[convertTeam(_team)];
    default:
      break;
  }

  return 0;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setPlayerWins(int playerId, int wins)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerId);

  if(!player)
    return false;

  player->score.setWins(wins);
  broadcastPlayerScoreUpdate(playerId);
  return true;
}

//-------------------------------------------------------------------------
BZF_API bool bz_setPlayerLosses(int playerId, int losses)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerId);

  if(!player)
    return false;

  player->score.setLosses(losses);
  broadcastPlayerScoreUpdate(playerId);
  return true;
}

//-------------------------------------------------------------------------
BZF_API bool bz_setPlayerTKs(int playerId, int tks)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerId);

  if(!player)
    return false;

  player->score.setTKs(tks);
  broadcastPlayerScoreUpdate(playerId);
  return true;
}

//-------------------------------------------------------------------------
BZF_API float bz_getPlayerRank (int playerId)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerId);

  if(!player)
    return -1;

  return player->score.ranking();
}

//-------------------------------------------------------------------------
BZF_API int bz_getPlayerWins (int playerId)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerId);

  if(!player)
    return -1;

  return player->score.getWins();
}

//-------------------------------------------------------------------------
BZF_API int bz_getPlayerLosses (int playerId)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerId);

  if(!player)
    return -1;

  return player->score.getLosses();
}

//-------------------------------------------------------------------------
BZF_API int bz_getPlayerTKs (int playerId)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerId);

  if(!player)
    return -1;

  return player->score.getTKs();
}

//-------------------------------------------------------------------------

BZF_API bool bz_setPlayerOperator(int playerId)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerId);

  if(!player)
    return false;

  player->accessInfo.setOperator();
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_resetPlayerScore(int playerId)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerId);

  if(!player)
    return false;

  player->score.setWins(0);
  player->score.setLosses(0);
  player->score.setTKs(0);
  broadcastPlayerScoreUpdate(playerId);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setPlayerShotType(int playerId, bz_eShotType shotType)
{
  if(!GameKeeper::Player::getPlayerByIndex(playerId))
    return false;

  sendSetShotType(playerId, (ShotType)shotType);
  return true;
}

//-------------------------------------------------------------------------

BZF_API int bz_getPlayerLag(int playerId)
{
  if(!GameKeeper::Player::getPlayerByIndex(playerId))
    return 0;

  return GameKeeper::Player::getPlayerByIndex(playerId)->lagInfo.getLag();
}

//-------------------------------------------------------------------------

BZF_API int bz_getPlayerJitter(int playerId)
{
  if(!GameKeeper::Player::getPlayerByIndex(playerId))
    return 0;

  return GameKeeper::Player::getPlayerByIndex(playerId)->lagInfo.getJitter();
}

//-------------------------------------------------------------------------

BZF_API float bz_getPlayerPacketLoss(int playerId)
{
  if(!GameKeeper::Player::getPlayerByIndex(playerId))
    return 0;

  return (float)GameKeeper::Player::getPlayerByIndex(playerId)->lagInfo.getLoss();
}

//-------------------------------------------------------------------------

BZF_API bz_APIStringList *bz_getGroupList(void)
{
  bz_APIStringList *groupList=new bz_APIStringList;

  PlayerAccessMap::iterator itr=groupAccess.begin();
  while(itr!=groupAccess.end())
  {
    groupList->push_back(itr->first);
    itr++;
  }
  return groupList;
}

//-------------------------------------------------------------------------

BZF_API bz_APIStringList *bz_getGroupPerms(const char *group)
{
  bz_APIStringList *permList=new bz_APIStringList;

  std::string groupName=group;
  groupName=TextUtils::toupper(groupName);
  PlayerAccessMap::iterator itr=groupAccess.find(groupName);
  if(itr==groupAccess.end())
    return permList;

  for(int i=0; i < PlayerAccessInfo::lastPerm; i++)
  {
    if(itr->second.explicitAllows.test(i) && !itr->second.explicitDenys.test(i))
      permList->push_back(nameFromPerm((PlayerAccessInfo::AccessPerm)i));
  }

  for(unsigned int c=0; c < itr->second.customPerms.size(); c++)
    permList->push_back(TextUtils::toupper(itr->second.customPerms[c]));

  return permList;
}

//-------------------------------------------------------------------------

BZF_API bool bz_groupAllowPerm(const char *group, const char *perm)
{
  std::string permName=perm;
  permName=TextUtils::toupper(permName);

  PlayerAccessInfo::AccessPerm realPerm=permFromName(permName);

  // find the group
  std::string groupName=group;
  groupName=TextUtils::toupper(groupName);
  PlayerAccessMap::iterator itr=groupAccess.find(groupName);
  if(itr==groupAccess.end())
    return false;

  if(realPerm!=PlayerAccessInfo::lastPerm)
    return itr->second.explicitAllows.test(realPerm);
  else
  {
    for(unsigned int i=0; i < itr->second.customPerms.size(); i++)
    {
      if(permName==TextUtils::toupper(itr->second.customPerms[i]))
	return true;
    }
  }
  return false;
}

BZF_API bz_APIStringList* bz_getStandardPermList ( void )
{
  bz_APIStringList* perms = bz_newStringList();

  for (int i = 0; i < (int)PlayerAccessInfo::lastPerm; i++)
    perms->push_back(nameFromPerm((PlayerAccessInfo::AccessPerm)i));

  return perms;
}

//-------------------------------------------------------------------------


BZF_API bool bz_sendTextMessage(int from, int to, const char *message)
{
  if(!message)
    return false;

  int playerIndex;
  PlayerId dstPlayer=AllPlayers;
  if(to!=BZ_ALLUSERS)
    dstPlayer=(PlayerId)to;

  if(from==BZ_SERVER)
    playerIndex=ServerPlayer;
  else
    playerIndex=from;

  sendChatMessage(playerIndex,dstPlayer,message);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_sendTextMessage(int from, bz_eTeamType to, const char *message)
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
      return bz_sendTextMessage(from, 250-(int)convertTeam(to), message);

    case eAdministrators:
      return bz_sendTextMessage(from, AdminPlayers, message);
  }
}

//-------------------------------------------------------------------------


BZF_API bool bz_sendTextMessagef(int from, bz_eTeamType to, const char *fmt, ...)
{
  char buffer[1024];

  if (!fmt) return false;

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 1024, fmt, args);
  va_end(args);
  return bz_sendTextMessage(from, to, buffer);
}

//-------------------------------------------------------------------------

BZF_API bool bz_sendTextMessagef(int from, int to, const char *fmt, ...)
{
  char buffer[1024];

  if (!fmt) return false;

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 1024, fmt, args);
  va_end(args);
  return bz_sendTextMessage(from, to, buffer);
}

//-------------------------------------------------------------------------

BZF_API bool bz_sendFetchResMessage(int playerID, const char *URL)
{
  if(playerID==BZ_SERVER || !URL)
    return false;

  teResourceType resType=eFile;

  std::vector < std::string > temp=TextUtils::tokenize(TextUtils::tolower(std::string(URL)), std::string("."));

  std::string ext=temp[temp.size()-1];
  if(ext=="wav")
    resType=eSound;

  NetMsg   msg = MSGMGR.newMessage();

  msg->packUShort(1); // the count
  msg->packUShort((short)resType);
  msg->packUShort((unsigned short)strlen(URL));
  msg->packString(URL, strlen(URL));

  if(playerID==BZ_ALLUSERS)
  {
    for(int i=0; i < curMaxPlayers; i++)
    {
      GameKeeper::Player *p=GameKeeper::Player::getPlayerByIndex(i);
      if(p && p->caps.canDownloadResources)
	MSGMGR.newMessage(msg)->send(p->netHandler, MsgFetchResources);
    }
  }
  else
  {
    GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
    if(player && player->caps.canDownloadResources)
      MSGMGR.newMessage(msg)->send(player->netHandler, MsgFetchResources);
  }
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_fireWorldWep(const char *flagType, float lifetime, float *pos, float tilt, float direction, int shotID, float dt)
{
  if(!pos || !flagType)
    return false;

  FlagTypeMap &flagMap=FlagType::getFlagMap();
  if(flagMap.find(std::string(flagType))==flagMap.end())
    return false;

  FlagType *flag=flagMap.find(std::string(flagType))->second;

  int realShotID=shotID;
  if(realShotID <= 0)
    realShotID=world->getWorldWeapons().getNewWorldShotID();

  if (lifetime <= 0)
    lifetime =BZDB.eval(StateDatabase::BZDB_RELOADTIME);

  if (dt <= 0)
    dt = 0;

  return fireWorldWep(flag, lifetime, ServerPlayer, pos, tilt, direction, realShotID, dt)==realShotID;
}

//-------------------------------------------------------------------------

BZF_API int bz_fireWorldGM(int targetPlayerID, float lifetime, float *pos, float tilt, float direction, float dt)
{
  const char *flagType="GM";

  if(!pos || !flagType)
    return false;

  FlagTypeMap &flagMap=FlagType::getFlagMap();
  if(flagMap.find(std::string(flagType))==flagMap.end())
    return false;

  FlagType *flag=flagMap.find(std::string(flagType))->second;

  PlayerId player=ServerPlayer;

  int shotID=world->getWorldWeapons().getNewWorldShotID();

  fireWorldGM(flag, targetPlayerID, lifetime, player, pos, tilt, direction, shotID, dt);

  return shotID;
}

// time API
BZF_API double bz_getCurrentTime(void)
{
  return TimeKeeper::getCurrent().getSeconds();
}

//-------------------------------------------------------------------------

BZF_API float bz_getMaxWaitTime ( const char *name )
{
  std::string timeName;
  if (name)
    timeName = name;

  if ( APIWaitTimes.find(timeName) != APIWaitTimes.end() )
    return APIWaitTimes[timeName];

  return -1;
}

//-------------------------------------------------------------------------

BZF_API void bz_setMaxWaitTime ( float maxTime, const char *name )
{
  std::string timeName;
  if (name)
    timeName = name;

  APIWaitTimes[timeName] = maxTime;
}

BZF_API void bz_clearMaxWaitTime ( const char *name )
{
  std::string timeName;
  if (name)
    timeName = name;

  if ( APIWaitTimes.find(timeName) == APIWaitTimes.end() )
    return;

  APIWaitTimes.erase(APIWaitTimes.find(timeName));
}

//-------------------------------------------------------------------------

BZF_API void bz_getLocaltime(bz_Time *ts)
{
  if(!ts)
    return ;

  TimeKeeper::localTimeDOW(&ts->year, &ts->month, &ts->day,&ts->dayofweek, &ts->hour, &ts->minute, &ts->second, &ts->daylightSavings);
}

BZF_API void bz_getUTCtime ( bz_Time *ts )
{
  TimeKeeper::UTCTime(&ts->year, &ts->month, &ts->day,&ts->dayofweek, &ts->hour, &ts->minute, &ts->second, &ts->daylightSavings);
}


// info
BZF_API double bz_getBZDBDouble(const char *variable)
{
  if(!variable)
    return 0.0;

  return BZDB.eval(std::string(variable));
}

//-------------------------------------------------------------------------

BZF_API bz_ApiString bz_getBZDBString(const char *variable)
{
  if(!variable)
    return bz_ApiString("");

  return bz_ApiString(BZDB.get(std::string(variable)));
}

//-------------------------------------------------------------------------

BZF_API bool bz_getBZDBBool(const char *variable)
{
  if(!variable)
    return false;

  return BZDB.eval(std::string(variable)) > 0.0;
}

//-------------------------------------------------------------------------

BZF_API int bz_getBZDBInt(const char *variable)
{
  return BZDB.evalInt(std::string(variable));
}

//-------------------------------------------------------------------------

BZF_API int bz_getBZDBItemPerms(const char *variable)
{
  if(!bz_BZDBItemExists(variable))
    return BZ_BZDBPERM_NA;

  switch(BZDB.getPermission(std::string(variable)))
  {
    case StateDatabase::ReadWrite: return BZ_BZDBPERM_USER;

    case StateDatabase::Locked: return BZ_BZDBPERM_SERVER;

    case StateDatabase::ReadOnly: return BZ_BZDBPERM_CLIENT;

    default:
      return BZ_BZDBPERM_NA;
  }
}

//-------------------------------------------------------------------------

BZF_API bool bz_getBZDBItemPersistent(const char *variable)
{
  if(!bz_BZDBItemExists(variable))
    return false;

  return BZDB.isPersistent(std::string(variable));
}

//-------------------------------------------------------------------------

BZF_API bool bz_BZDBItemExists(const char *variable)
{
  if(!variable)
    return false;

  return BZDB.isSet(std::string(variable));
}

//-------------------------------------------------------------------------

void setVarPerms(const char *variable, int perms, bool persistent)
{
  if(perms!=BZ_BZDBPERM_NA)
  {
    switch(perms)
    {
      case BZ_BZDBPERM_USER:
	BZDB.setPermission(std::string(variable), StateDatabase::ReadWrite);
	break;
      case BZ_BZDBPERM_SERVER:
	BZDB.setPermission(std::string(variable), StateDatabase::Locked);
	break;
      default:
	BZDB.setPermission(std::string(variable), StateDatabase::ReadOnly);
	break;
	;
    }
  }
  BZDB.setPersistent(std::string(variable), persistent);
}

//-------------------------------------------------------------------------

BZF_API bool bz_setBZDBDouble(const char *variable, double val, int perms, bool persistent)
{
  if(!variable)
    return false;

  bool exists=BZDB.isSet(std::string(variable));

  BZDB.set(std::string(variable), TextUtils::format("%f", val));
  setVarPerms(variable, perms, persistent);

  return !exists;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setBZDBString(const char *variable, const char *val, int perms, bool persistent)
{
  if(!variable || !val)
    return false;

  bool exists=BZDB.isSet(std::string(variable));

  BZDB.set(std::string(variable), std::string(val));
  setVarPerms(variable, perms, persistent);

  return !exists;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setBZDBBool(const char *variable, bool val, int perms, bool persistent)
{
  if(!variable)
    return false;

  bool exists=BZDB.isSet(std::string(variable));

  BZDB.set(std::string(variable), TextUtils::format("%d", val));
  setVarPerms(variable, perms, persistent);

  return !exists;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setBZDBInt(const char *variable, int val, int perms, bool persistent)
{
  if(!variable)
    return false;

  bool exists=BZDB.isSet(std::string(variable));

  BZDB.set(std::string(variable), TextUtils::format("%d", val));
  setVarPerms(variable, perms, persistent);

  return !exists;
}

//-------------------------------------------------------------------------

BZF_API bool bz_updateBZDBDouble(const char *variable, double val, int /*perms*/, bool /*persistent*/)
{
  if(!variable)
    return false;

  if (!BZDB.isSet(std::string(variable)))
    return false;
 
  BZDB.set(std::string(variable), TextUtils::format("%f", val));

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_updateBZDBString(const char *variable, const char *val)
{
  if(!variable || !val)
    return false;

  if (!BZDB.isSet(std::string(variable)))
    return false;

  BZDB.set(std::string(variable), std::string(val));

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_updateBZDBBool(const char *variable, bool val)
{
  if(!variable)
    return false;

  if (!BZDB.isSet(std::string(variable)))
    return false;

  BZDB.set(std::string(variable), TextUtils::format("%d", val));

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_updateBZDBInt(const char *variable, int val)
{
  if(!variable)
    return false;

  if (!BZDB.isSet(std::string(variable)))
    return false;

  BZDB.set(std::string(variable), TextUtils::format("%d", val));

  return true;
}


//-------------------------------------------------------------------------

void bzdbIterator(const std::string &name, void *userData)
{
  bz_APIStringList *varList=static_cast < bz_APIStringList * > (userData);
  varList->push_back(name);
}

//-------------------------------------------------------------------------

BZF_API int bz_getBZDBVarList(bz_APIStringList *varList)
{
  if(!varList)
    return -1;

  varList->clear();
  BZDB.iterate(&bzdbIterator, varList);
  return (int)varList->size();
}

//-------------------------------------------------------------------------

BZF_API void bz_resetBZDBVar(const char *variable)
{
  std::string command="reset ";
  if(variable && strlen(variable))
    command+=variable;
  else
    command+="*";

  CMDMGR.run(command);
}

//-------------------------------------------------------------------------

BZF_API void bz_resetALLBZDBVars(void)
{
  bz_resetBZDBVar(NULL);
}

// logging
BZF_API void bz_debugMessage(int _level, const char *message)
{
  if(!message)
    return ;

  logDebugMessage(_level, "%s\n", message);
}

//-------------------------------------------------------------------------

BZF_API void bz_debugMessagef(int _level, const char *fmt, ...)
{
  char buffer[4096];

  if (!fmt) return;

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, 4096, fmt, args);
  va_end(args);
  bz_debugMessage(_level, buffer);
}

//-------------------------------------------------------------------------

BZF_API int bz_getDebugLevel(void)
{
  return debugLevel;
}

// admin
BZF_API bool bz_kickUser(int playerIndex, const char *reason, bool notify)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerIndex);
  if(!player || !reason)
    return false;

  if(notify)
  {
    std::string msg=std::string("You have been kicked from the server for: ")+reason;
    sendMessage(ServerPlayer, playerIndex, msg.c_str());

    msg=player->player.getCallSign();
    msg+=std::string(" was kicked for:")+reason;
    sendMessage(ServerPlayer, AdminPlayers, msg.c_str());
  }
  removePlayer(playerIndex, reason);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_IPBanUser(const char *ip, const char* source, int duration, const char *reason)
{
  if(!reason || !ip)
    return false;

  std::string banner = "server";
  if (source)
    banner = source;

  // reload the banlist in case anyone else has added
  clOptions->acl.load();

  if(clOptions->acl.ban(ip,banner.c_str(), duration, reason))
    clOptions->acl.save();
  else
    return false;

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_IDBanUser(const char *bzID, const char* source , int duration, const char *reason)
{
  if(!reason || !bzID || std::string(bzID).size() <= 0)
    return false;

  std::string banner = "server";
  if (source)
    banner = source;

  // reload the banlist in case anyone else has added
  clOptions->acl.load();
  clOptions->acl.idBan(bzID,banner.c_str(), duration, reason);
  clOptions->acl.save();

  return true;
}

BZF_API bool bz_HostBanUser(const char* hostmask, const char* source, int duration, const char* reason)
{
  if(!reason || !hostmask)
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

BZF_API bool bz_IPUnbanUser ( const char* ip )
{
  if(!ip)
    return false;
 
  clOptions->acl.load();

  if(clOptions->acl.unban(ip))
    clOptions->acl.save();
  else
    return false;

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_IDUnbanUser ( const char* bzID )
{
  if(!bzID)
    return false;
  
  clOptions->acl.load();

  if(clOptions->acl.idUnban(bzID))
    clOptions->acl.save();
  else
    return false;

  return true;
}

BZF_API bool bz_HostUnbanUser(const char* hostmask)
{
  if(!hostmask)
    return false;

  clOptions->acl.load();
  
  if(clOptions->acl.hostUnban(hostmask))
    clOptions->acl.save();
  else
    return false;

  return true;
}


BZF_API unsigned int bz_getBanListSize( bz_eBanListType listType )
{
  switch(listType)
  {
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
  switch(listType)
  {
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

  switch(listType)
  {
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

  switch(listType)
  {
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

BZF_API double bz_getBanItemDurration ( bz_eBanListType listType, unsigned int item )
{
  if (item > bz_getBanListSize(listType))
    return 0;

  TimeKeeper end = TimeKeeper::getCurrent();

  switch(listType)
  {
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
    return -1;

  return end.getSeconds() - TimeKeeper::getCurrent().getSeconds();
}

BZF_API bool bz_getBanItemIsFromMaster ( bz_eBanListType listType, unsigned int item )
{
  if (item > bz_getBanListSize(listType))
    return false;

  switch(listType)
  {
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


//-------------------------------------------------------------------------

BZF_API bz_APIStringList *bz_getReports(void)
{
  bz_APIStringList *list=new bz_APIStringList;

  std::vector<std::string> reports;
  REPORTS.getLines(reports);

  for ( size_t s = 0; s < reports.size(); s++ )
    list->push_back(reports[s]);

  return list;
}

BZF_API unsigned int bz_getReportCount(void)
{
  return (unsigned int)REPORTS.count();
}

BZF_API const char* bz_getReportSource(unsigned int id)
{
  Reports::Report report = REPORTS.get(id);
  return report.from.c_str();
}

BZF_API const char* bz_getReportBody(unsigned int id)
{
  Reports::Report report = REPORTS.get(id);
  return report.message.c_str();
}

BZF_API const char* bz_getReportTime(unsigned int id)
{
  Reports::Report report = REPORTS.get(id);
  return report.time.c_str();
}

BZF_API bool bz_clearReport(unsigned int id)
{
  return REPORTS.clear(id);
}

BZF_API bool bz_clearAllReports(void)
{
  return REPORTS.clear();
}


//-------------------------------------------------------------------------

BZF_API bool bz_fileReport(const char* message, const char * from)
{
  if ( !message )
    return false;

  std::string f = "server";
  if (from)
    f = from;

  return REPORTS.file(std::string(message),from);
}

//-------------------------------------------------------------------------

BZF_API int bz_getLagWarn(void)
{
  return (int)(clOptions->lagwarnthresh *1000+0.5);
}

//-------------------------------------------------------------------------

BZF_API bool bz_setLagWarn(int lagwarn)
{
  clOptions->lagwarnthresh=(float)(lagwarn/1000.0);
  LagInfo::setThreshold(clOptions->lagwarnthresh, (float)clOptions->maxlagwarn);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setTimeLimit(float timeLimit)
{
  if(timeLimit <= 0.0f)
    return false;
  clOptions->timeLimit=timeLimit;
  return true;
}

//-------------------------------------------------------------------------

BZF_API float bz_getTimeLimit(void)
{
  return clOptions->timeLimit;
}

//-------------------------------------------------------------------------

BZF_API bool bz_isTimeManualStart(void)
{
  return clOptions->timeManualStart;
}

//-------------------------------------------------------------------------

BZF_API bool bz_isCountDownActive(void)
{
  return countdownActive;
}

//-------------------------------------------------------------------------

BZF_API bool bz_isCountDownInProgress(void)
{
  return countdownDelay > 0;
}

//-------------------------------------------------------------------------

BZF_API bool bz_pollActive(void)
{
  if (votingArbiter != NULL && votingArbiter->knowsPoll())
    return true;
  else
    return false;
}

//-------------------------------------------------------------------------

BZF_API bool bz_pollVeto(void)
{
  /* make sure that there is a poll arbiter */
  if(votingArbiter==NULL)
    return false;

  /* make sure there is an unexpired poll */
  if(!votingArbiter->knowsPoll())
    return false;

  /* poof */
  votingArbiter->forgetPoll();

  return true;
}

//-------------------------------------------------------------------------

BZF_API bz_APIStringList *bz_getHelpTopics(void)
{
  bz_APIStringList *list=new bz_APIStringList;

  const std::vector < std::string >  &r=clOptions->textChunker.getChunkNames();
  for(unsigned int i=0; i < r.size(); i++)
    list->push_back(r[i]);
  return list;
}

//-------------------------------------------------------------------------

BZF_API bz_APIStringList *bz_getHelpTopic(const char *name)
{
  bz_APIStringList *list=new bz_APIStringList;

  std::string topicName;
  if(name)
    topicName=name;

  const std::vector < std::string >  *r=clOptions->textChunker.getTextChunk(topicName);
  for(unsigned int i=0; i < r->size(); i++)
    list->push_back((*r)[i]);
  return list;
}

//-------------------------------------------------------------------------

BZF_API bool bz_registerCustomSlashCommand(const char *command, bz_CustomSlashCommandHandler *handler)
{
  if(!command || !handler)
    return false;

  registerCustomSlashCommand(std::string(command), handler);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeCustomSlashCommand(const char *command)
{
  if(!command)
    return false;

  removeCustomSlashCommand(std::string(command));
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_getStandardSpawn(int playerID, float pos[3], float *rot)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player)
    return false;

  // get the spawn position
  SpawnPosition *spawnPosition=new SpawnPosition(playerID, (!clOptions->respawnOnBuildings) || (player->player.isBot()), clOptions->gameType==ClassicCTF);

  pos[0]=spawnPosition->getX();
  pos[1]=spawnPosition->getY();
  pos[2]=spawnPosition->getZ();
  if(rot)
    *rot=spawnPosition->getAzimuth();

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_killPlayer(int playerID, bool spawnOnBase, int killerID, const char *flagType)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player)
    return false;

  if(!player->player.isAlive())
    return false;

  if(killerID==-1)
    killerID = ServerPlayer;

  FlagType *flag=NULL;
  if(flagType)
  {
    FlagTypeMap &flagMap=FlagType::getFlagMap();
    if(flagMap.find(std::string(flagType))==flagMap.end())
      return false;

    flag=flagMap.find(std::string(flagType))->second;
  }

  playerKilled(playerID, killerID, GotKilledMsg, -1, flag ? flag : Flags::Null, -1, spawnOnBase);

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_givePlayerFlag(int playerID, const char *flagType, bool force)
{
  FlagInfo *fi=NULL;
  GameKeeper::Player *gkPlayer=GameKeeper::Player::getPlayerByIndex(playerID);

  if(gkPlayer!=NULL)
  {
    FlagType *ft=Flag::getDescFromAbbreviation(flagType);
    if(ft!=Flags::Null)
    {
      // find unused and forced candidates
      FlagInfo *unused=NULL;
      FlagInfo *forced=NULL;
      for(int i=0; i < numFlags; i++)
      {
	FlagInfo *fi2=FlagInfo::get(i);
	if((fi2!=NULL) && (fi2->flag.type==ft))
	{
	forced=fi2;
	if(fi2->player < 0)
	{
	unused=fi2;
	break;
	}
	}
      }

      // see if we need to force it
      if(unused!=NULL)
	fi=unused;
      else if(forced!=NULL)
      {
	if(force)
	fi=forced;
	else
	//all flags of this type are in use and force is set to false
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
  else
  //invald player
    return false;

  if(gkPlayer && fi)
  {
    // do not give flags to dead players
    if(!gkPlayer->player.isAlive())
      return false;
    //player is dead

    // deal with the player's current flag (if applicable)
    const int flagId=gkPlayer->player.getFlag();
    if(flagId >= 0)
    {
      FlagInfo &currentFlag= *FlagInfo::get(flagId);
      if(currentFlag.flag.type->flagTeam!=NoTeam)
	dropFlag(currentFlag, gkPlayer->currentPos);
      // drop team flags
      else
	resetFlag(currentFlag);
      // reset non-team flags
    }
    // setup bzfs' state
    fi->grab(gkPlayer->getIndex());
    gkPlayer->player.setFlag(fi->getIndex());

    // send MsgGrabFlag
    return sendGrabFlagMessage(gkPlayer->getIndex(),  *fi);
  }
  //just in case? (a "wtf" case)
  return false;
}

//-------------------------------------------------------------------------

BZF_API bool bz_removePlayerFlag(int playerID)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player)
    return false;

  if(!player->player.isAlive())
    return false;

  zapFlagByPlayer(playerID);

  return true;
}

//-------------------------------------------------------------------------

BZF_API void bz_resetFlags(bool onlyUnused)
{
  for(int i=0; i < numFlags; i++)
  {
    FlagInfo &flag= *FlagInfo::get(i);
    // see if someone had grabbed flag,
    const int playerIndex=flag.player;
    if(!onlyUnused || (playerIndex==-1))
    {
      if(playerIndex!=-1)
	dropFlag(flag);

      resetFlag(flag);
    }
  }
}

//-------------------------------------------------------------------------

BZF_API unsigned int bz_getNumFlags(void)
{
  return numFlags;
}

//-------------------------------------------------------------------------

BZF_API const bz_ApiString bz_getFlagName(int flag)
{
  FlagInfo *pFlag=FlagInfo::get(flag);
  if(!pFlag)
    return bz_ApiString("");

  return bz_ApiString(pFlag->flag.type->flagAbbv);
}

//-------------------------------------------------------------------------

BZF_API bool bz_resetFlag(int flag)
{
  FlagInfo *pFlag=FlagInfo::get(flag);
  if(!pFlag)
    return false;

  // if somone has it, drop it
  if(pFlag->player!=-1)
    dropFlag(*pFlag);

  resetFlag(*pFlag);

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_moveFlag(int flag, float pos[3], bool reset)
{
  FlagInfo *pFlag=FlagInfo::get(flag);
  if(!pFlag)
    return false;

  // if somone has it, drop it
  if(pFlag->player!=-1)
    dropFlag(*pFlag);

  if(reset)
    pFlag->resetFlag(pos, true);
  else
    memcpy(pFlag->flag.position, pos, sizeof(float) *3);

  sendFlagUpdateMessage(*pFlag);

  return true;
}

//-------------------------------------------------------------------------

BZF_API int bz_flagPlayer(int flag)
{
  FlagInfo *pFlag=FlagInfo::get(flag);
  if(!pFlag)
    return -1;

  return pFlag->player;
}

//-------------------------------------------------------------------------

BZF_API bool bz_getFlagPosition(int flag, float *pos)
{
  FlagInfo *pFlag=FlagInfo::get(flag);
  if(!pFlag || !pos)
    return false;

  if(pFlag->player!=-1)
  {
    GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(pFlag->player);

    if(!player)
      return false;

    memcpy(pos, player->currentPos, sizeof(float) *3);
  }
  else
    memcpy(pos, pFlag->flag.position, sizeof(float) *3);

  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setWorldSize(float size, float wallHeight)
{
  pluginWorldHeight=wallHeight;
  pluginWorldSize=size;

  return true;
}

//-------------------------------------------------------------------------

BZF_API void bz_setClientWorldDownloadURL(const char *URL)
{
  clOptions->cacheURL.clear();
  if(URL)
    clOptions->cacheURL=URL;
}

//-------------------------------------------------------------------------

BZF_API const bz_ApiString bz_getClientWorldDownloadURL(void)
{
  bz_ApiString URL;
  if(clOptions->cacheURL.size())
    URL=clOptions->cacheURL;
  return URL;
}

//-------------------------------------------------------------------------

BZF_API bool bz_saveWorldCacheFile(const char *file)
{
  if(!file)
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

//-------------------------------------------------------------------------

BZF_API bool bz_registerCustomMapObject(const char *object, bz_CustomMapObjectHandler *handler)
{
  if(!object || !handler)
    return false;

  return registerCustomMapObject(object, handler);
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeCustomMapObject(const char *object)
{
  if(!object)
    return false;

  return removeCustomMapObject(object);
}

//-------------------------------------------------------------------------

BZF_API void bz_getWorldSize(float *size, float *wallHeight)
{
  if(wallHeight)
    *wallHeight=pluginWorldHeight;
  if(size)
    *size=pluginWorldSize;
}

//-------------------------------------------------------------------------

BZF_API unsigned int bz_getWorldObjectCount(void)
{
  return (unsigned int)(OBSTACLEMGR.getWalls().size() + OBSTACLEMGR.getBoxes().size() + OBSTACLEMGR.getPyrs().size() + OBSTACLEMGR.getBases().size() + OBSTACLEMGR.getMeshes().size() + OBSTACLEMGR.getArcs().size() + OBSTACLEMGR.getCones().size() + OBSTACLEMGR.getSpheres().size() + OBSTACLEMGR.getTetras().size());
}

bz_eSolidWorldObjectType solidTypeFromObstacleType ( int type )
{
  switch(type)
  {
    case wallType:
	return eWallObject;
    case boxType:
      return eBoxObject;
    case pyrType:
      return ePyramidObject;
    case baseType:
      return eBaseObject;
    case meshType:
      return eMeshObject;
    case arcType:
      return eArcObject;
    case coneType:
      return eConeObject;
    case sphereType:
      return eSphereObject;
    case tetraType:
      return eTetraObject;
  }
  return eUnknownObject;
}

unsigned int buildObjectIDFromObstacle ( const Obstacle &obstacle )
{
  unsigned short p[2];
  p[0] = obstacle.getTypeID();
  p[1] = obstacle.getListID();

  return *force_cast<unsigned int *>(&p[0]);
}

const ObstacleList* obstacleListFromObstacleType ( int type )
{
  switch(type)
  {
  case wallType:
    return &OBSTACLEMGR.getWalls();
  case boxType:
    return &OBSTACLEMGR.getBoxes();
  case pyrType:
    return &OBSTACLEMGR.getPyrs();
  case baseType:
    return &OBSTACLEMGR.getBases();
  case meshType:
    return &OBSTACLEMGR.getMeshes();
  case arcType:
    return &OBSTACLEMGR.getArcs();
  case coneType:
    return &OBSTACLEMGR.getCones();
  case sphereType:
    return &OBSTACLEMGR.getSpheres();
  case tetraType:
    return &OBSTACLEMGR.getTetras();
  }
  return NULL;
}

unsigned char makeTangibilityMask ( const bz_SolidObjectPassableAtributes &atribs )
{
  if (atribs.allFalse())
    return 0;

  if ( atribs.allTrue())
    return 1;

  unsigned char m = 0;
  if ( atribs.red )
    m |= _RED_PASSABLE;
  if ( atribs.green )
    m |= _GREEN_PASSABLE;
  if ( atribs.blue )
    m |= _BLUE_PASSABLE;
  if ( atribs.purple )
    m |= _PURPLE_PASSABLE;
  if ( atribs.rogue )
    m |= _ROGUE_PASSABLE;

  return m;
}

void readTangibilityMask ( unsigned char m, bz_SolidObjectPassableAtributes &atribs )
{
  atribs.setAll(false);
  if (m == 0)
    return;

  if ( m == 1)
  {
    atribs.setAll(true);
    return;
  }

  atribs.red = ((m & _RED_PASSABLE) != 0);
  atribs.green = ((m & _GREEN_PASSABLE) != 0);
  atribs.blue = ((m & _BLUE_PASSABLE) != 0);
  atribs.purple = ((m & _PURPLE_PASSABLE) != 0);
  atribs.rogue = ((m & _ROGUE_PASSABLE) != 0);
}

void setSolidObjectFromObstacle ( bz_APISolidWorldObject_V1 &object, const Obstacle &obstacle )
{
  object.solidType = solidTypeFromObstacleType(obstacle.getTypeID());

  memcpy(object.center,obstacle.getPosition(),sizeof(float)*3);
  object.rotation[0] = object.rotation[1] = 0;
  object.rotation[2] = obstacle.getRotation();
  const Extents &extents = obstacle.getExtents();
  memcpy(object.maxAABBox,extents.maxs,sizeof(float)*3);
  memcpy(object.minAABBox,extents.mins,sizeof(float)*3);

  readTangibilityMask(ServerIntangibilityManager::instance().getWorldObjectTangibility(obstacle.getGUID()),object.driveThru);
  readTangibilityMask(obstacle.isShootThrough(),object.shootThru);
}

//-------------------------------------------------------------------------

void addObjectsToListsFromObstacleList ( bz_APIWorldObjectList *solidList, const ObstacleList &list )
{
  for ( unsigned int i = 0; i < list.size(); i++ )
  {
    bz_APISolidWorldObject_V1 *solid = new bz_APISolidWorldObject_V1;
    solid->id = buildObjectIDFromObstacle(*list[i]);
    setSolidObjectFromObstacle(*solid,*list[i]);
    solidList->push_back(solid);
  }
}

void addCTFBasesToListsFromObstacleList ( bz_APIWorldObjectList *solidList, const ObstacleList &list )
{
  for ( unsigned int i = 0; i < list.size(); i++ )
  {
    bz_CTFBaseWorldObject_V1 *solid = new bz_CTFBaseWorldObject_V1;
    solid->id = buildObjectIDFromObstacle(*list[i]);
    setSolidObjectFromObstacle(*solid,*list[i]);
    BaseBuilding* base = (BaseBuilding*)list[i];
    solid->team = convertTeam((TeamColor)base->getTeam());
    solidList->push_back(solid);
  }
}

BZF_API bz_APIWorldObjectList *bz_getWorldObjectList(void)
{
  bz_APIWorldObjectList *worldList = new bz_APIWorldObjectList;

  addObjectsToListsFromObstacleList(worldList,OBSTACLEMGR.getWalls());
  addObjectsToListsFromObstacleList(worldList,OBSTACLEMGR.getBoxes());
  addObjectsToListsFromObstacleList(worldList,OBSTACLEMGR.getPyrs());
  addCTFBasesToListsFromObstacleList(worldList,OBSTACLEMGR.getBases());
  addObjectsToListsFromObstacleList(worldList,OBSTACLEMGR.getTeles());
  addObjectsToListsFromObstacleList(worldList,OBSTACLEMGR.getMeshes());
  addObjectsToListsFromObstacleList(worldList,OBSTACLEMGR.getArcs());
  addObjectsToListsFromObstacleList(worldList,OBSTACLEMGR.getCones());
  addObjectsToListsFromObstacleList(worldList,OBSTACLEMGR.getSpheres());
  addObjectsToListsFromObstacleList(worldList,OBSTACLEMGR.getTetras());

  return worldList;
}

//-----------------------bz_APISolidWorldObject-----------------


bz_APISolidWorldObject_V1::bz_APISolidWorldObject_V1()
{
  type = eSolidObject;
  memset(center, 0, sizeof(float) *3);
  memset(maxAABBox, 0, sizeof(float) *3);
  memset(minAABBox, 0, sizeof(float) *3);
  memset(rotation, 0, sizeof(float) *3);
  memset(maxBBox, 0, sizeof(float) *3);
  memset(minBBox, 0, sizeof(float) *3);
}

bool bz_APISolidWorldObject_V1::collide(float /*pos*/[3], float /*rad*/, float* /*hit*/)
{
  return false;
}


//-------------------------------------------------------------------------

bz_APISolidWorldObject_V1::~bz_APISolidWorldObject_V1()
{
}

bz_CTFBaseWorldObject_V1::bz_CTFBaseWorldObject_V1() : bz_APISolidWorldObject_V1()
{
}

bz_CTFBaseWorldObject_V1::~bz_CTFBaseWorldObject_V1()
{
}

unsigned int findFirstNameInList ( const ObstacleList &list, unsigned short baseType, const std::string &name )
{
  unsigned short p[2];

  p[0] = baseType;
  for ( unsigned int i = 0; i < list.size(); i++ )
  {
    p[1] = (unsigned short)i;

    Obstacle *obstacle = list[i];
    if (obstacle->getName() == name )
      return *force_cast<unsigned int *>(&p[0]);
  }
  return 0;
}

BZF_API unsigned int bz_findWorldObject ( const char *name )
{
  if (!name)
    return 0;

  std::string nameStr = name;
  unsigned int id = findFirstNameInList(OBSTACLEMGR.getWalls(),eWallObject,nameStr);
  if (id)
    return id;

  id = findFirstNameInList(OBSTACLEMGR.getBoxes(),eBoxObject,nameStr);
  if (id)
    return id;

  id = findFirstNameInList(OBSTACLEMGR.getPyrs(),ePyramidObject,nameStr);
  if (id)
    return id;

  id = findFirstNameInList(OBSTACLEMGR.getBases(),eBaseObject,nameStr);
  if (id)
    return id;

  id = findFirstNameInList(OBSTACLEMGR.getMeshes(),eMeshObject,nameStr);
  if (id)
    return id;

  id = findFirstNameInList(OBSTACLEMGR.getArcs(),eArcObject,nameStr);
  if (id)
    return id;

  id = findFirstNameInList(OBSTACLEMGR.getSpheres(),eSphereObject,nameStr);
  if (id)
    return id;

  id = findFirstNameInList(OBSTACLEMGR.getTetras(),eTetraObject,nameStr);
  if (id)
    return id;

  return 0;
}

BZF_API bz_APIBaseWorldObject* bz_getWorldObjectByID ( unsigned int id )
{
  if (id == 0)
    return NULL;

  unsigned short p[2];
  memcpy(p,&id,sizeof(int));
  const ObstacleList  *list = obstacleListFromObstacleType(p[0]);
  if ( p[1] >= list->size())
    return NULL;

  bz_APISolidWorldObject_V1 *solid = new bz_APISolidWorldObject_V1;
  solid->id = id;
  Obstacle *obj = (*list)[p[1]];
  setSolidObjectFromObstacle(*solid,*obj);

  return solid;
}

BZF_API bool bz_SetWorldObjectTangibility ( int id, const bz_SolidObjectPassableAtributes &atribs )
{
  ServerIntangibilityManager::instance().instance().setWorldObjectTangibility(id,makeTangibilityMask(atribs));
  return true;
}

BZF_API bool bz_GetWorldObjectTangibility ( int id, bz_SolidObjectPassableAtributes &atribs )
{
  atribs.setAll(false);

  unsigned char val =  ServerIntangibilityManager::instance().instance().getWorldObjectTangibility(id);
  if ( val == _INVALID_TANGIBILITY )
    return false;

  readTangibilityMask(val,atribs);
  
  return true;
}

BZF_API void bz_ResetWorldObjectTangibilities ( void )
{
  ServerIntangibilityManager::instance().resetTangibility();
}

//-------------------------------------------------------------------------

bz_eAPIColType getAPIMapObject(InBuildingType colType, const Obstacle *obs, bz_APIBaseWorldObject **object)
{
  if(!obs)
    return eNoCol;

  bz_freeWorldObjectPtr(*object);

  bool base=false;
  switch(colType)
  {
    case NOT_IN_BUILDING:
    case IN_BOX_DRIVETHROUGH:
      return eNoCol;

    case IN_BASE:
      base=true;
    case IN_BOX_NOTDRIVETHROUGH:
    case IN_MESH:
    case IN_MESHFACE:
    case IN_PYRAMID:
    case IN_TETRA:
      if(object)
	*object = APISolidFromObsacle(obs);
      return base ? eInBase : eInSolid;

    case IN_TELEPORTER:
      return eInTP;
  }
  return eNoCol;
}

//-------------------------------------------------------------------------

bz_eAPIColType bz_cylinderInMapObject(float pos[3], float height, float radius, bz_APIBaseWorldObject **object)
{
  if(!world)
    return eNoCol;

  const Obstacle *obs=NULL;
  return getAPIMapObject(world->cylinderInBuilding(&obs, pos, radius, height), obs, object);
}

//-------------------------------------------------------------------------

bz_eAPIColType bz_boxInMapObject(float pos[3], float size[3], float angle, bz_APIBaseWorldObject **object)
{
  if(!world)
    return eNoCol;
  const Obstacle *obs=NULL;
  return getAPIMapObject(world->boxInBuilding(&obs, pos, angle, size[0], size[1], size[2]), obs, object);
}

//-------------------------------------------------------------------------

void bz_freeWorldObjectPtr(bz_APIBaseWorldObject *ptr)
{
  if(ptr)
    delete (ptr);

  ptr=NULL;
}

//-------------------------------------------------------------------------

BZF_API bool bz_getPublic(void)
{
  return clOptions->publicizeServer;
}

//-------------------------------------------------------------------------

BZF_API bz_ApiString bz_getPublicAddr(void)
{
  if(!clOptions->publicizeServer)
    return bz_ApiString("");

  return bz_ApiString(clOptions->publicizedAddress);
}

BZF_API int bz_getPublicPort( void )
{
  if(clOptions->useGivenPort)
    return clOptions->wksPort;

  return ServerPort;
}

//-------------------------------------------------------------------------

BZF_API bz_ApiString bz_getPublicDescription(void)
{
  if(!clOptions->publicizeServer)
    return bz_ApiString("");

  return bz_ApiString(clOptions->publicizedTitle);
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
  return lastPluginDir.c_str();
}


//-------------------------------------------------------------------------

BZF_API bool bz_sendPlayCustomLocalSound(int playerID, const char *soundName)
{
  if(playerID==BZ_SERVER || !soundName)
    return false;

  NetMsg   msg = MSGMGR.newMessage();

  msg->packUShort(LocalCustomSound);
  msg->packUShort((unsigned short)strlen(soundName));
  msg->packString(soundName, strlen(soundName));

  if(playerID==BZ_ALLUSERS)
  {
    for(int i=0; i < curMaxPlayers; i++)
    {
      GameKeeper::Player *p=GameKeeper::Player::getPlayerByIndex(i);
      if(p && p->caps.canPlayRemoteSounds)
	MSGMGR.newMessage(msg)->send(p->netHandler, MsgCustomSound);
    }
  }
  else
  {
    GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
    if(player && player->caps.canPlayRemoteSounds)
      MSGMGR.newMessage(msg)->send(player->netHandler, MsgCustomSound);
  }

  return true;
}

// custom pluginHandler
BZF_API bool bz_registerCustomPluginHandler(const char *extension, bz_APIPluginHandler *handler)
{
  if(!extension || !handler)
    return false;

  std::string ext=extension;

#ifdef BZ_PLUGINS
  return registerCustomPluginHandler(ext, handler);
#else
  std::cerr<<"This BZFlag server does not support plugins."<<std::endl;
  return false;
#endif
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeCustomPluginHandler(const char *extension, bz_APIPluginHandler *handler)
{
  if(!extension || !handler)
    return false;

  std::string ext=extension;

#ifdef BZ_PLUGINS
  return removeCustomPluginHandler(ext, handler);
#else
  std::cerr<<"This BZFlag server does not support plugins."<<std::endl;
  return false;
#endif
}

// generic callback system
std::map<std::string, bz_GenericCallback*> callbackClasses;
std::map<std::string, bz_GenericCallbackFunc> callbackFunctions;

BZF_API bool bz_registerCallBack ( const char* name, bz_GenericCallback *callback )
{
  if (!name || ! callback)
    return false;

  std::string callbackName = name;

  std::map<std::string, bz_GenericCallback*>::iterator itr = callbackClasses.find(callbackName);
  if (itr != callbackClasses.end())
    return false;

  callbackClasses[callbackName] = callback;
  return true;
}

BZF_API bool bz_registerCallBack ( const char* name, bz_GenericCallbackFunc callback )
{
  if (!name || ! callback)
    return false;

  std::string callbackName = name;

  std::map<std::string, bz_GenericCallbackFunc>::iterator itr = callbackFunctions.find(callbackName);
  if (itr != callbackFunctions.end())
    return false;

  callbackFunctions[callbackName] = callback;
  return true;
}

BZF_API bool bz_removeCallBack ( const char* name, bz_GenericCallback *callback )
{
  if (!name || ! callback)
    return false;

  std::string callbackName = name;

  std::map<std::string, bz_GenericCallback*>::iterator itr = callbackClasses.find(callbackName);
  if (itr == callbackClasses.end())
    return false;

  callbackClasses.erase(itr);
  return true;
}

BZF_API bool bz_removeCallBack ( const char* name, bz_GenericCallbackFunc callback )
{
  if (!name || ! callback)
    return false;

  std::string callbackName = name;

  std::map<std::string, bz_GenericCallbackFunc>::iterator itr = callbackFunctions.find(callbackName);
  if (itr == callbackFunctions.end())
    return false;

  callbackFunctions.erase(itr);
  return true;
}

BZF_API bool bz_callCallback ( const char* name, void *param )
{
  if (!name)
    return false;

  std::string callbackName = name;

  std::map<std::string, bz_GenericCallback*>::iterator classItr = callbackClasses.find(callbackName);
  if (classItr != callbackClasses.end())
    return classItr->second->call(param);

  std::map<std::string, bz_GenericCallbackFunc>::iterator funcItr = callbackFunctions.find(callbackName);
  if (funcItr != callbackFunctions.end())
    return (*funcItr->second)(param);
  return false;
}

BZF_API bool bz_callbackExists ( const char* name )
{
  if (!name)
    return false;

  std::string callbackName = name;

  std::map<std::string, bz_GenericCallback*>::iterator classItr = callbackClasses.find(callbackName);
  if (classItr != callbackClasses.end())
    return true;

  std::map<std::string, bz_GenericCallbackFunc>::iterator funcItr = callbackFunctions.find(callbackName);
  if (funcItr != callbackFunctions.end())
    return true;
  return false;
}

// team info
BZF_API int bz_getTeamCount(bz_eTeamType _team)
{
  int teamIndex=(int)convertTeam(_team);

  int count=0;
  if(teamIndex < 0 || teamIndex >= NumTeams)
    return 0;

  for(int i=0; i < curMaxPlayers; i++)
  {
    GameKeeper::Player *p=GameKeeper::Player::getPlayerByIndex(i);
    if((p==NULL))
      continue;

    if(p->player.getTeam()==teamIndex)
      count++;
  }

  return count;
}

//-------------------------------------------------------------------------

BZF_API int bz_getTeamScore(bz_eTeamType _team)
{
  int teamIndex=(int)convertTeam(_team);

  if(teamIndex < 0 || teamIndex >= NumTeams)
    return 0;

  return team[teamIndex].team.won-team[teamIndex].team.lost;
}

//-------------------------------------------------------------------------

BZF_API int bz_getTeamWins(bz_eTeamType _team)
{
  int teamIndex=(int)convertTeam(_team);

  if(teamIndex < 0 || teamIndex >= NumTeams)
    return 0;

  return team[teamIndex].team.won;
}

//-------------------------------------------------------------------------

BZF_API int bz_getTeamLosses(bz_eTeamType _team)
{
  int teamIndex=(int)convertTeam(_team);

  if(teamIndex < 0 || teamIndex >= NumTeams)
    return 0;

  return team[teamIndex].team.lost;
}

//-------------------------------------------------------------------------

BZF_API void bz_setTeamWins(bz_eTeamType _team, int wins)
{
  int teamIndex=(int)convertTeam(_team);

  if(teamIndex < 0 || teamIndex >= NumTeams)
    return ;

  team[teamIndex].team.won=wins;
  sendTeamUpdateMessageBroadcast(teamIndex);
}

//-------------------------------------------------------------------------

BZF_API void bz_setTeamLosses(bz_eTeamType _team, int losses)
{
  int teamIndex=(int)convertTeam(_team);

  if(teamIndex < 0 || teamIndex >= NumTeams)
    return ;

  team[teamIndex].team.lost=losses;
  sendTeamUpdateMessageBroadcast(teamIndex);
}

//-------------------------------------------------------------------------

BZF_API void bz_resetTeamScore(bz_eTeamType _team)
{
  int teamIndex=(int)convertTeam(_team);

  if(teamIndex >= NumTeams)
    return ;

  if(teamIndex >= 0)
  {
    team[teamIndex].team.won=0;
    team[teamIndex].team.lost=0;
    sendTeamUpdateMessageBroadcast(teamIndex);
  }
  else
  {
    for(int i=0; i < NumTeams; i++)
    {
      team[i].team.won=0;
      team[i].team.lost=0;
      sendTeamUpdateMessageBroadcast(i);
    }
  }
}

//-------------------------------------------------------------------------

BZF_API void bz_updateListServer(void)
{
  publicize();
}

//-------------------------------------------------------------------------

typedef struct
{
  std::string url;
  bz_BaseURLHandler *handler;
  std::string postData;
} trURLJob;

class BZ_APIURLManager: private cURLManager
{
public:
  BZ_APIURLManager()
  {
    doingStuff=false;
  }

  virtual ~BZ_APIURLManager()
  {
    flush();
  }

  void addJob(const char *URL, bz_BaseURLHandler *handler, const char *_postData)
  {
    if(!URL)
      return ;

    trURLJob job;
    job.url=URL;
    job.handler=handler;
    if(_postData)
      job.postData=_postData;

    jobs.push_back(job);

    if(!doingStuff)
      doJob();
  }

  void removeJob(const char *URL)
  {
    if(!URL)
      return ;

    std::string url=URL;

    for(unsigned int i=0; i < jobs.size(); i++)
    {
      if(jobs[i].url==url)
      {
	if(i==0)
	{
	  removeHandle();
	}
	jobs.erase(jobs.begin()+i);
	i=jobs.size()+1;
      }
    }
  }

  void flush(void)
  {
    removeHandle();
    jobs.clear();
    doingStuff=false;
  }

  int jobCount(void)
  {
    return jobs.size();
  }

  virtual void finalization(char *data, unsigned int length, bool good)
  {
    if(!jobs.size() || !doingStuff)
      return ;
    // we are suposed to be done

    // this is who we are suposed to be geting
    trURLJob job=jobs[0];
    jobs.erase(jobs.begin());
    if(good && job.handler)
      job.handler->done(job.url.c_str(), data, length, good);
    else if(job.handler)
      job.handler->error(job.url.c_str(), 1, "badness");

    // free it
    removeHandle();

    // do the next one if we must
    doJob();
  }

protected:
  void doJob(void)
  {
    if(!jobs.size())
      doingStuff=false;
    else
    {
      trURLJob job=jobs[0];
      doingStuff=true;
      setURL(job.url);

      if(job.postData.size())
      {
	setHTTPPostMode();
	setPostMode(job.postData);
      }
      else
	setGetMode();

      addHandle();
    }
  }

  std::vector < trURLJob > jobs;
  bool doingStuff;
};

BZ_APIURLManager *bz_apiURLManager=NULL;

BZF_API bool bz_addURLJob(const char *URL, bz_BaseURLHandler *handler, const char *postData)
{
  if(!URL)
    return false;

  if(!bz_apiURLManager)
    bz_apiURLManager=new BZ_APIURLManager;

  bz_apiURLManager->addJob(URL, handler, postData);
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeURLJob(const char *URL)
{
  if(!URL)
    return false;

  if(!bz_apiURLManager)
    return true;

  bz_apiURLManager->removeJob(URL);
  if (bz_apiURLManager->jobCount() == 0)
  {
    delete bz_apiURLManager;
    bz_apiURLManager = NULL;
  }
  return true;
}

//-------------------------------------------------------------------------

BZF_API bool bz_stopAllURLJobs(void)
{
  if(!bz_apiURLManager)
    return true;

  // flush is done automatically during d'tor
  delete bz_apiURLManager;
  bz_apiURLManager = NULL;
  return true;
}

// inter plugin communication
std::map < std::string, std::string > globalPluginData;

BZF_API bool bz_clipFieldExists(const char *_name)
{
  if(!_name)
    return false;

  std::string name=_name;

  return globalPluginData.find(name)!=globalPluginData.end();
}

//-------------------------------------------------------------------------

BZF_API const char *bz_getclipFieldString(const char *_name)
{
  if(!bz_clipFieldExists(_name))
    return NULL;

  std::string name=_name;

  return globalPluginData[name].c_str();
}

//-------------------------------------------------------------------------

BZF_API float bz_getclipFieldFloat(const char *_name)
{
  if(!bz_clipFieldExists(_name))
    return 0.0f;

  std::string name=_name;

  return (float)atof(globalPluginData[name].c_str());

}

//-------------------------------------------------------------------------

BZF_API int bz_getclipFieldInt(const char *_name)
{
  if(!bz_clipFieldExists(_name))
    return 0;

  std::string name=_name;

  return atoi(globalPluginData[name].c_str());
}

//-------------------------------------------------------------------------

BZF_API bool bz_setclipFieldString(const char *_name, const char *data)
{
  bool existed=bz_clipFieldExists(_name);
  if(!data)
    return false;

  std::string name=_name;

  globalPluginData[name]=std::string(data);
  callClipFiledCallbacks(name.c_str());
  return existed;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setclipFieldFloat(const char *_name, float data)
{
  bool existed=bz_clipFieldExists(_name);
  if(!data)
    return false;

  std::string name=_name;

  globalPluginData[name]=TextUtils::format("%f", data);
  callClipFiledCallbacks(name.c_str());
  return existed;
}

//-------------------------------------------------------------------------

BZF_API bool bz_setclipFieldInt(const char *_name, int data)
{
  bool existed=bz_clipFieldExists(_name);
  if(!data)
    return false;

  std::string name=_name;

  globalPluginData[name]=TextUtils::format("%d", data);
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
    std::vector<bz_ClipFiledNotifier*> &vec = clipFieldMap[name];
    for ( unsigned int i = 0; i < (unsigned int)vec.size(); i++ )
      vec[i]->fieldChange(field);
  }

  if (clipFieldMap.find(std::string("")) != clipFieldMap.end())
  {
    std::vector<bz_ClipFiledNotifier*> &vec = clipFieldMap[std::string("")];
    for ( unsigned int i = 0; i < (unsigned int)vec.size(); i++ )
      vec[i]->fieldChange(field);
  }
}

BZF_API bool bz_addclipFieldNotifier ( const char *name, bz_ClipFiledNotifier *cb )
{
  if (!cb)
    return false;

  std::string field;
  if (name)
    field = name;

  if (clipFieldMap.find(name) == clipFieldMap.end())
  {
    std::vector<bz_ClipFiledNotifier*> vec;
    clipFieldMap[name] = vec;
  }
  clipFieldMap[name].push_back(cb);

  return true;
}

BZF_API bool bz_removeclipFieldNotifier ( const char *name, bz_ClipFiledNotifier *cb )
{
  if (!cb)
    return false;

  std::string field;
  if (name)
    field = name;

  if (clipFieldMap.find(name) == clipFieldMap.end())
    return false;
 
  std::vector<bz_ClipFiledNotifier*> &vec = clipFieldMap[name];
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

//-------------------------------------------------------------------------

BZF_API bz_ApiString bz_filterPath(const char *path)
{
  if(!path)
    return bz_ApiString("");

  char *temp;
  temp=(char*)malloc(strlen(path)+1);
  if (temp) {
    strncpy(temp, path, strlen(path)+1);
  }

  // replace anything but alphanumeric charcters or dots in filename by '_'
  // should be safe on every supported platform

  char *buf=temp;
  while(buf && *buf!='\0')
  {
    if(!isalnum(*buf) ||  *buf!='.')
      *buf='_';

    buf++;
  }
  bz_ApiString ret(temp);
  if (temp) free(temp);
  return ret;
}

//-------------------------------------------------------------------------

BZF_API bool bz_saveRecBuf(const char *_filename, int seconds)
{
  if(!Record::enabled() || !_filename)
    return false;

  bool result=Record::saveBuffer(ServerPlayer, _filename, seconds);
  return result;
}

//-------------------------------------------------------------------------

BZF_API bool bz_startRecBuf(void)
{
  if(Record::enabled())
    return false;

  return Record::start(AllPlayers);
}

//-------------------------------------------------------------------------

BZF_API bool bz_stopRecBuf(void)
{
  if(!Record::enabled())
    return false;

  return Record::stop(ServerPlayer);
}

//-------------------------------------------------------------------------

BZF_API const char *bz_format(const char *fmt, ...)
{
  static std::string result = "";

  if (!fmt) return result.c_str();

  va_list args;
  va_start(args, fmt);
  result=TextUtils::vformat(fmt, args);
  va_end(args);
  return result.c_str();
}

//-------------------------------------------------------------------------

BZF_API const char *bz_toupper(const char *val)
{
  static std::string temp;
  if(!val)
    return NULL;

  temp=TextUtils::toupper(std::string(val));
  return temp.c_str();
}

//-------------------------------------------------------------------------

BZF_API const char *bz_tolower(const char *val)
{
  static std::string temp;
  if(!val)
    return NULL;

  temp=TextUtils::tolower(std::string(val));
  return temp.c_str();
}

//-------------------------------------------------------------------------

BZF_API const char *bz_urlEncode(const char *val)
{
  static std::string temp;
  if(!val)
    return NULL;

  temp=TextUtils::url_encode(std::string(val));
  return temp.c_str();
}

// server control
BZF_API void bz_shutdown()
{
  shutdownCommand(NULL, NULL);
}

//-------------------------------------------------------------------------

BZF_API bool bz_restart(void)
{
  if(clOptions->replayServer)
    return false;

  // close out the game, and begin anew
  // tell players to quit
  for(int i=0; i < curMaxPlayers; i++)
    removePlayer(i, "Server Reset");

  delete world;
  world=NULL;
  delete []worldDatabase;
  worldDatabase=NULL;

  bz_stopRecBuf();

  ServerIntangibilityManager::instance().resetTangibility();

  // start up all new and stuff
  if(!defineWorld())
  {
    shutdownCommand(NULL, NULL);
    return false;
  }

  for(int i=0; i < numFlags; i++)
  {
    FlagInfo &flag= *FlagInfo::get(i);
    zapFlag(flag);
  }

  return true;
}

//-------------------------------------------------------------------------

BZF_API void bz_reloadLocalBans()
{
  // reload the banlist
  logDebugMessage(3, "Reloading bans\n");
  clOptions->acl.load();

  rescanForBans();
}

//-------------------------------------------------------------------------

BZF_API void bz_reloadMasterBans()
{
  if(masterBanHandler.busy)
    return ;

  // reload the banlist
  logDebugMessage(3, "Reloading master bans\n");
  clOptions->acl.purgeMasters();

  masterBanHandler.start();
}

//-------------------------------------------------------------------------

BZF_API void bz_reloadGroups()
{
  logDebugMessage(3, "Reloading groups\n");
  groupAccess.clear();
  initGroups();
}

//-------------------------------------------------------------------------

BZF_API void bz_reloadUsers()
{
  logDebugMessage(3, "Reloading users and passwords\n");
  userDatabase.clear();

  if(userDatabaseFile.size())
    PlayerAccessInfo::readPermsFile(userDatabaseFile);
  GameKeeper::Player::reloadAccessDatabase();
}

//-------------------------------------------------------------------------

BZF_API void bz_reloadHelp()
{
  // reload the text chunks
  logDebugMessage(3, "Reloading helpfiles\n");
  clOptions->textChunker.reload();
  
  // check for new files in helpdirs
  std::list<OSDir>::iterator i, end = clOptions->helpDirs.end();
  OSFile f;
  
  for (i = clOptions->helpDirs.begin(); i != end; i++)
    while (i->getNextFile(f, "*.txt", false))
      if(clOptions->textChunker.parseFile(f.getFullOSPath(), f.getFileName(), 50, MessageLen))
	logDebugMessage(3, "Loaded help message: %s", f.getFileName().c_str());
}

//-------------------------------------------------------------------------

BZF_API void bz_superkill()
{
  superkillCommand(NULL, NULL);
}

//-------------------------------------------------------------------------

BZF_API void bz_gameOver(int playerID, bz_eTeamType _team )
{
  sendScoreOverMessage(playerID,convertTeam(_team));

  gameOver=true;

  if(clOptions->timeManualStart)
  {
    countdownActive=false;
    countdownPauseStart=TimeKeeper::getNullTime();
    clOptions->countdownPaused=false;
  }
}

//-------------------------------------------------------------------------

BZF_API bz_eTeamType bz_checkBaseAtPoint(float pos[3])
{
  return convertTeam(whoseBase(pos[0], pos[1], pos[2]));
}

//-------------------------------------------------------------------------

BZF_API bz_eGameType bz_getGameType(void)
{
  if(clOptions->gameType==ClassicCTF)
    return eClassicCTFGame;
  else if(clOptions->gameType==RabbitChase)
    return eRabbitGame;
  else if(clOptions->gameType==OpenFFA)
    return eOpenFFAGame;

  return eTeamFFAGame;
}

BZF_API bool bz_allowJumping(void)
{
  return clOptions->gameOptions & JumpingGameStyle ? true : false;
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
  MD5 md5;
  md5.update((const unsigned char*)data,size);
  return md5.hexdigest().c_str();
}

BZF_API const char* bz_getServerVersion ( void )
{
  return getAppVersion();
}

// server side bot API

bz_ServerSidePlayerHandler::bz_ServerSidePlayerHandler() : playerID(-1), wantToJump(false), autoSpawn(true), flaps(0), alive(false)
{
  input[0] = input[1] = 0;
  lastUpdate.rotVel = 0;
  lastUpdate.vec[0] = 0;
  lastUpdate.vec[1] = 0;
  lastUpdate.vec[2] = 0;
  lastUpdate.rot = 0;
  lastUpdate.pos[0] = lastUpdate.pos[1] = lastUpdate.pos[2] = 0;
  currentState = lastUpdate;

}

// higher level logic API
void bz_ServerSidePlayerHandler::spawned(void)
{
}

bool bz_ServerSidePlayerHandler::think(void)
{
  updatePhysics();
  return false;
}

void bz_ServerSidePlayerHandler::died ( int /*killer*/ )
{
  alive = false;
}

void bz_ServerSidePlayerHandler::smote ( SmiteReason /*reason*/ )
{
  alive = false;
}

void bz_ServerSidePlayerHandler::update ( void )
{
  think();
  updatePhysics();
}


// lower level message API
void bz_ServerSidePlayerHandler::playerRemoved(int){}

void bz_ServerSidePlayerHandler::playerRejected(bz_eRejectCodes, const char*){}

void bz_ServerSidePlayerHandler::playerSpawned(int id, float _pos[3], float _rot)
{
  if(id==playerID)
  {
    // it was me, I'm not in limbo
    alive=true;

    // update the current state
    lastUpdate.time = bz_getCurrentTime();
    // get where I am;
    memcpy(lastUpdate.pos, _pos, sizeof(float) *3);
    lastUpdate.rotVel = 0;
    lastUpdate.vec[0] = 0;
    lastUpdate.vec[1] = 0;
    lastUpdate.vec[2] = 0;
    lastUpdate.rot = _rot;

    currentState = lastUpdate;

    input[0] = input[1] = 0;
    updatePhysics();

    flaps = 0;
    // tell the high level API that we done spawned;
    spawned();
  }
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::textMessage(int, int, const char*){}

void bz_ServerSidePlayerHandler::playerKilledMessage(int, int, bz_ePlayerDeathReason, int, const char *, int){}

void bz_ServerSidePlayerHandler::scoreLimitReached(int, bz_eTeamType){}

void bz_ServerSidePlayerHandler::flagCaptured(int, int, bz_eTeamType){}

void bz_ServerSidePlayerHandler::flagUpdate(int, bz_FlagUpdateRecord **){}

void bz_ServerSidePlayerHandler::playerInfoUpdate(bz_PlayerInfoUpdateRecord*){}

void bz_ServerSidePlayerHandler::teamUpdate(int, bz_TeamInfoRecord **){}

void bz_ServerSidePlayerHandler::handicapUpdate(int, bz_HandicapUpdateRecord **){}

void bz_ServerSidePlayerHandler::playerIPUpdate(int, const char*){}

void bz_ServerSidePlayerHandler::playerStateUpdate(int, bz_PlayerUpdateState *, double){}

void bz_ServerSidePlayerHandler::playerScoreUpdate(int, float, int, int, int){}

void bz_ServerSidePlayerHandler::flagTransfer(int, int, int, bz_eShotType){}

void bz_ServerSidePlayerHandler::nearestFlag(const char *, float[3]){}

void bz_ServerSidePlayerHandler::grabFlag(int player, int /*flagID*/, const char* flagType, bz_eShotType shotType)
{
  if (player == playerID)
  {
    if (alive)	// it's for us so notify the AI that events happened
    {
      shotChange(shotType);
      flagPickup(flagType);
    }
  }
}

void bz_ServerSidePlayerHandler::setShotType(int player, bz_eShotType shotType )
{
  if (player == playerID)
  {
    if (alive)	// it's for us so notify the AI that events happened
      shotChange(shotType);
  }
}

void bz_ServerSidePlayerHandler::shotFired(int, unsigned short, bz_eShotType ){}

void bz_ServerSidePlayerHandler::shotEnded(int, unsigned short, unsigned short){}

void bz_ServerSidePlayerHandler::playerTeleported( int, unsigned short, unsigned short ){}

void bz_ServerSidePlayerHandler::playerAutopilot( int, bool ){}

void bz_ServerSidePlayerHandler::allowSpawn( bool ){}

void bz_ServerSidePlayerHandler::setPlayerData(const char *callsign, const char *token, const char *clientVersion, bz_eTeamType _team)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(!player || player->playerHandler!=this)
    return ;

  player->player.setType(TankPlayer); // because we like to lie :)
  player->player.setTeam(convertTeam(_team));
  player->player.setCallsign(callsign);
  player->player.setToken(token);
  player->player.setClientVersion(clientVersion);

  uint16_t code=0;
  char reason[512] = {0};
  if(!player->player.processEnter(code, reason))
    playerRejected((bz_eRejectCodes)code, reason);

  alive=player->player.isAlive();
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::joinGame(void)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player)
    return ;

  if(player->player.isAlive() || player->player.isPlaying())
    return ;

  player->lastState.order=0;

  // set our state to signing on so we can join
  player->player.signingOn();
  playerAlive(playerID);
  player->player.setAlive();
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::getCurrentState(bz_PlayerUpdateState *state)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!state || !player)
    return ;

  // grab the current state
  playerStateToAPIState(*state,player->getCurrentStateAsState());
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::dropFlag(void)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player)
    return ;

  float p[3],r;
  player->getPlayerCurrentPosRot(p,r);

  dropPlayerFlag(*player, p);
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::sendChatMessage(const char *text, int targetPlayer)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player || !text)
    return ;

  if(targetPlayer > LastRealPlayer)
    return ;

  PlayerId dstPlayer=targetPlayer==BZ_ALLUSERS ? AllPlayers : targetPlayer;

  ::sendChatMessage(player->getIndex(),dstPlayer,text);
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::sendTeamChatMessage(const char *text, bz_eTeamType targetTeam)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player || !text)
    return ;

  PlayerId dstPlayer=AllPlayers;

  switch(targetTeam)
  {
    case eRogueTeam:
    case eRedTeam:
    case eGreenTeam:
    case eBlueTeam:
    case ePurpleTeam:
    case eRabbitTeam:
    case eHunterTeam:
      dstPlayer=250+(int)targetTeam;
      break;

    case eAdministrators:
      dstPlayer=AdminPlayers;
      break;
    default:
      break;
  }

  ::sendChatMessage(player->getIndex(),dstPlayer,text);
}

//-------------------------------------------------------------------------

void bz_ServerSidePlayerHandler::setMovement(float forward, float turn)
{
  if(input[0]==turn && input[1]==forward)
    return ;

  input[0]=turn;
  input[1]=forward;

  if(input[0] > 1.0f)
    input[0]=1.0f;
  if(input[1] > 1.0f)
    input[1]=1.0f;
  if(input[0] < -1.0f)
    input[0]=-1.0f;
  if(input[1] < -1.0f)
    input[1]=-1.0f;
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::fireShot(void)
{
  return false;
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::jump(void)
{
  if(canMove() && canJump())
   wantToJump = true;
  return wantToJump;
}

//-------------------------------------------------------------------------

const Obstacle* hitBuilding ( const bz_ServerSidePlayerHandler::UpdateInfo &oldPos, bz_ServerSidePlayerHandler::UpdateInfo &newPos, float width, float breadth, float height, bool directional, bool checkWalls = true )
{
  return NULL;

  // check and see if this path goes thru a building.
  const Obstacle* hit = world->hitBuilding(oldPos.pos, oldPos.rot, newPos.pos, newPos.rot, width, breadth, breadth, directional,checkWalls);
  if (!hit) // if it does not, it's clear
    return NULL;
  else
  {
    // we assume that the start point of the path is always clear, so if we get here, then the endpoint passed thru a building.

    // compute the distance that the center point moves.
   float len = 0;
   float vec[3];
    for (int i =0; i < 3; i++)
      vec[i] = newPos.pos[i] - oldPos.pos[i];
    len = sqrtf(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);

    // if the distance is less then our tolerance, and the END point is IN an object, assume that the start point since we know that is clear
    // return this as the collision value
    float collideTol = 0.1f;
    if ( len <= collideTol)
    {
      newPos = oldPos;
      return hit;
    }

    //if (len < 10) //something is wrong
  //  {
  //    return hit;
  //  }

    // compute a midpoint to test, so we can do a binary search to find what "side" of the midpoint is clear
    bz_ServerSidePlayerHandler::UpdateInfo midpoint;

    for (int i =0; i < 3; i++)
      midpoint.pos[i] = oldPos.pos[i] + vec[i]*0.5f;

    midpoint.rot = oldPos.rot + ((oldPos.rot-newPos.rot)*0.5f);

    // test from the start point to the midpoint
    hit = world->hitBuilding(oldPos.pos, oldPos.rot, midpoint.pos, midpoint.rot, width, breadth, breadth, directional,checkWalls);

    // if we have a hit, then the midpoint is "in" a building, so the valid section is the oldPos -> midpoint section.
    // so set the endpoint to the midpoint and retest.
    // if we don't have a hit. then the midpoint is outside of the building, and we want to test from the midpoint to the newpos and get closer
    if (hit)
    {
      newPos = midpoint;
      midpoint = oldPos;
    }

    // recurse to get closer to the actual collision point
    return hitBuilding(midpoint,newPos,width,breadth,height,directional,checkWalls);
  }

  return NULL;
}

bool closeFloat ( float f1, float f2 )
{
  return fabs(f1)-fabs(f2) < 0.0001f;
}

bool checkBounds ( float pos[3], float rad )
{
  float x,y;
  world->getSize(x,y);

  if ( pos[0] + rad > x)
    return false;
  if ( pos[0] - rad < -x)
    return false;

  if ( pos[1] + rad > y)
    return false;
  if ( pos[1] - rad < -y)
    return false;

  return true;
}

void clampBounds ( float pos[3], float rad )
{
  float x,y;
  world->getSize(x,y);

  if ( pos[0] + rad > x)
    pos[0] = x-rad;
  if ( pos[0] - rad < -x)
    pos[0] = -x+rad;

  if ( pos[1] + rad > y)
    pos[1] = y-rad;
  if ( pos[1] - rad < -y)
    pos[1] = -y+rad;
}

void bz_ServerSidePlayerHandler::updatePhysics(void)
{
  if(!alive)
    return;

  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  player->player.state = PlayerAlive;

  UpdateInfo newState(currentState); // where we think we are

  double now = bz_getCurrentTime();
  float fullDT = (float)(now - currentState.time);

  float maxDelta = 0.25f;
  int count = 1;
  if ( fullDT > maxDelta)	// break up the dt into small chunks
    count = (int)(fullDT/maxDelta);

  for (int i = 0; i < count; i++ )
  {
    float delta = fullDT/count;

    int flagID = player->player.getFlag();
    FlagType *flag = NULL;
    bool hasOO = false;
    
    if (player->player.haveFlag())
    {
      flag = FlagInfo::get(flagID)->flag.type;
      if (flag == Flags::OscillationOverthruster)
	hasOO = true;
    }

    bool doUpdate = false;

    // if we can't move, don't try to move
    if (!canMove())
    {
      input[0] = input[1] = 0;
      wantToJump = false;
    }

    if (falling())  // we are falling... so the input dosn't matter at all, just move us
    {
      newState.vec[2] += BZDBCache::gravity * delta;

      newState.pos[0] += newState.vec[0] * delta;
      newState.pos[1] += newState.vec[1] * delta;
      newState.pos[2] += newState.vec[2] * delta;

      // clamp us to in bounds
      if (!checkBounds(newState.pos,BZDBCache::tankRadius))
      {
	clampBounds(newState.pos,BZDBCache::tankRadius);
	outOfBounds(newState.pos);
      }

      // see if we hit anything
      const Obstacle *target = hitBuilding(currentState,newState,BZDBCache::tankWidth,BZDBCache::tankLength,BZDBCache::tankHeight,!hasOO,false);

      if (target)
      {
	// something was hit, so no mater what we have to update, cus our DR WILL be off
	doUpdate = true;
	// find out if we landed, or if we just hit something

	// see if the thing we hit was below us.
	newState.pos[2] += 0.1f;

	// if we move the postion up a tad and test again, if we laned on something we should not be in contact with it.
	if(!target->inMovingBox(currentState.pos,currentState.rot,newState.pos,newState.rot,BZDBCache::tankWidth*0.5f,BZDBCache::tankLength*0.5f,BZDBCache::tankHeight))
	{
	  newState.pos[2] -= 0.1f;

	  // we landed, so our speed in z is 0
	  currentState = newState;
	  currentState.vec[2] = 0;

	  // get our facing and apply it to our vector so we keep the components that are along our facing.
	  float facing[3];
	  vecFromAngle2d(newState.rot,facing);
  	
	  currentState.vec[0] *= facing[0];
	  currentState.vec[1] *= facing[1];

	  // set the state to have landed
	  player->lastState.status &= ~PlayerState::Falling;

	  // tell the AI that we landed
	  landed();
	}
	else
	{
	  // we hit something normal, just slide down it
	  newState.pos[2] -= 0.1f;

	  currentState = newState;
	  currentState.vec[0] = currentState.vec[1] = 0;

	  bz_APISolidWorldObject_V1 *solid = APISolidFromObsacle(target);
	  collide(solid,currentState.pos);
	  delete(solid);
	}

	newState = currentState;
      }
    }

    // we are driving, so apply the input 
    if (!falling() && canMove())
    {
      // see what the input wants us to do
      float desiredTurn = input[1] * getMaxRotSpeed();

      // clamp to the rotation speed for the frame.
      newState.rotVel = computeAngleVelocity(newState.rotVel,desiredTurn,delta);

      float currentSpeed = input[1] * getMaxLinSpeed();

      // compute the momentum
      float lastSpeed = getMagnitude(newState.vec);
      if (lastSpeed < 0.001)
	lastSpeed = 0;
      
      computeMomentum(delta, flag, currentSpeed, newState.rotVel, getMagnitude(newState.vec), currentState.rotVel );

      // compute our new rotation;
      newState.rot += newState.rotVel * delta;

      // compute our new velocity
      vecFromAngle2d(newState.rot,newState.vec,currentSpeed);

      // clamp the speed to acceleration and the world
      computeFriction(delta,flag,currentState.vec,newState.vec);

      // compute our new position
      for (int j =0; j < 3; j++)
	newState.pos[j] += newState.vec[j] * delta;

      // clamp us to in bounds
      if (!checkBounds(newState.pos,BZDBCache::tankRadius))
      {
	clampBounds(newState.pos,BZDBCache::tankRadius);
	outOfBounds(newState.pos);
      }

      // clamp the pos to what we hit
      const Obstacle *target = hitBuilding(currentState,newState,BZDBCache::tankWidth,BZDBCache::tankLength,BZDBCache::tankHeight,!hasOO,false);
      if (target)
      {
	doUpdate = true;
	newState.vec[0] = newState.vec[1] = newState.vec[2] = 0;
	currentState = newState;

	bz_APISolidWorldObject_V1 *solid = APISolidFromObsacle(target);
	collide(solid,currentState.pos);
	delete(solid);
      }

      // now check for jumping
      if (wantToJump && canJump())
      {
	doUpdate = true;
	newState.vec[2] += computeJumpVelocity(flag);
	currentState = newState;
	player->lastState.status &= PlayerState::Falling;
	jumped();
      }
      else  // see if we fall off something
      {
	if (newState.pos[2] <= computeGroundLimit(flag))
	  newState.pos[2] = computeGroundLimit(flag);
	else
	{
	  // do a collision test to see if what is below the tank is empty
	  float temp[3];
	  memcpy(temp,newState.pos,sizeof(float)*3);
	  temp[2] -= 0.1f;

// 	  if (bz_cylinderInMapObject(temp, 0.1f, BZDBCache::tankRadius, NULL) == eNoCol)
// 	  {
// 	    // down we go
// 	    doUpdate = true;
// 	    newState.vec[2] = -BZDBCache::gravity;
// 	    currentState = newState;
// 	    player->lastState.status &= PlayerState::Falling;
// 	    jumped();
// 	  }
	}
      }
    }

    // the new state is where we will be
    currentState = newState;

    // if they jumped, they jumped.
    wantToJump = false;

    if (!doUpdate) // if we aren't forcing an update due to a collision then check to see if we are far enough away
    {
      if (lastUpdate.getDelta(newState) > 0.5f)
	doUpdate = true;
    }

    if (now - player->lastState.lastUpdateTime > 10.0f)
      doUpdate = true;


    if (doUpdate)
    {
      // send out the current state as an update

      PlayerState state = player->lastState;

      state.order++;
      state.angVel = currentState.rotVel;
      state.azimuth = currentState.rot;
      memcpy(state.velocity,currentState.vec,sizeof(float)*3);
      memcpy(state.pos,currentState.pos,sizeof(float)*3);
      updatePlayerState(player, state, TimeKeeper(now), false);

      lastUpdate = currentState;
    }
   }
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::canJump(void)
{
  return canMove() && bz_allowJumping();
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::canShoot(void)
{
  return false;
}

//-------------------------------------------------------------------------

bool bz_ServerSidePlayerHandler::canMove(void)
{
  return alive && !falling();
}

bool bz_ServerSidePlayerHandler::falling (void)
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if (!player)
    return false;

  return player->lastState.status & PlayerState::Falling ? true : false;
}

//-------------------------------------------------------------------------

bz_eShotType bz_ServerSidePlayerHandler::getShotType(void)
{
  return (bz_eShotType)GameKeeper::Player::getPlayerByIndex(playerID)->efectiveShotType;
}

void bz_ServerSidePlayerHandler::getPosition ( float *p )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player || !p)
    return;

  float r;
  player->getPlayerCurrentPosRot(p,r);
}

void bz_ServerSidePlayerHandler::getVelocity ( float *v )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player ||!v)
    return;

  memcpy(v,player->getCurrentStateAsState().velocity,sizeof(float)*3);
}

float bz_ServerSidePlayerHandler::getFacing ( void )
{
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);
  if(!player)
    return 0.0;

  float p[3],r;
  player->getPlayerCurrentPosRot(p,r);

  return r;
}

float bz_ServerSidePlayerHandler::getMaxLinSpeed ( void )
{

  FlagType *flag = NULL;
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(player && player->player.haveFlag())
    flag = FlagInfo::get(player->player.getFlag())->flag.type;

  return computeMaxLinVelocity(flag,currentState.pos[2]);
}

float bz_ServerSidePlayerHandler::getMaxRotSpeed ( void )
{
  FlagType *flag = NULL;
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerID);

  if(player && player->player.haveFlag())
    flag = FlagInfo::get(player->player.getFlag())->flag.type;

  return computeMaxAngleVelocity(flag,currentState.pos[2]);
}

float bz_ServerSidePlayerHandler::UpdateInfo::getDelta( const UpdateInfo & state)
{
  // plot where we think we are now based on the current time
  double dt = state.time - time;

  float newPos[3];
  newPos[0] = pos[0] + (float)(vec[0] *dt);
  newPos[1] = pos[1] + (float)(vec[1] *dt);
  newPos[2] = pos[2] + (float)(vec[2] *dt);

  // that's where we thing we'll be based on movement

  float dx = newPos[0] - state.pos[0];
  float dy = newPos[1] - state.pos[1];
  float dz = newPos[1] - state.pos[2];

  // return the distance between where our projection is, and where state is
  return sqrt(dx*dx+dy*dy+dz*dz);
}


//-------------------------------------------------------------------------

BZF_API int bz_addServerSidePlayer(bz_ServerSidePlayerHandler *handler)
{
  handler->setPlayerID(-1);

  PlayerId playerIndex=getNewPlayerID();
  if(playerIndex >= 0xFF)
    return -1;

  // make the player, check the game stuff, and don't do DNS stuff
  GameKeeper::Player *player=new GameKeeper::Player(playerIndex, handler);
  checkGameOn();
  player->_LSAState=GameKeeper::Player::notRequired;

  handler->setPlayerID(playerIndex);

  handler->added(playerIndex);
  return playerIndex;
}

//-------------------------------------------------------------------------

BZF_API bool bz_removeServerSidePlayer(int playerID, bz_ServerSidePlayerHandler *handler)
{
  if(playerID < 0)
    return false;

  PlayerId playerIndex=(PlayerId)playerID;
  GameKeeper::Player *player=GameKeeper::Player::getPlayerByIndex(playerIndex);

  if(player->playerHandler!=handler)
    return false;

  removePlayer(playerIndex, NULL, true);
  return true;
}

//-------------------------------------------------------------------------

BZF_API void bz_pauseCountdown(const char *pausedBy)
{
  pauseCountdown(pausedBy);
}

//-------------------------------------------------------------------------

BZF_API void bz_resumeCountdown(const char *resumedBy)
{
  resumeCountdown(resumedBy);
}

//-------------------------------------------------------------------------

BZF_API void bz_resetTeamScores(void)
{
  resetTeamScores();
}

//-------------------------------------------------------------------------

BZF_API void bz_startCountdown(int delay, float limit, const char *byWho)
{
  startCountdown(delay, limit, byWho);
}

//-------------------------------------------------------------------------

BZF_API void bz_newRabbit(int player, bool swap)
{
  if(player < 0)
    return ;

  PlayerId playerIndex=(PlayerId)player;
  GameKeeper::Player *playerRec=GameKeeper::Player::getPlayerByIndex(playerIndex);

  if(!playerRec)
    return ;

  if(swap)
  {
    GameKeeper::Player *rabbit=GameKeeper::Player::getPlayerByIndex(rabbitIndex);
    if(rabbit)
      rabbit->player.wasARabbit();
  }

  playerRec->player.setTeam(RabbitTeam);
  rabbitIndex=playerIndex;

  sendRabbitUpdate(playerIndex, swap ? 0 : 1);
}

//-------------------------------------------------------------------------

BZF_API void bz_removeRabbit(int player)
{
  if(player < 0)
    return ;

  PlayerId playerIndex=(PlayerId)player;
  GameKeeper::Player *playerRec=GameKeeper::Player::getPlayerByIndex(playerIndex);

  if(!player)
    return ;

  playerRec->player.wasARabbit();

  playerRec->player.setTeam(HunterTeam);

  if(playerIndex==rabbitIndex)
    rabbitIndex=NoPlayer;

  sendRabbitUpdate(playerIndex, 2);
}

//-------------------------------------------------------------------------

BZF_API void bz_changeTeam(int player, bz_eTeamType _team)
{
  if(player < 0)
    return ;

  PlayerId playerIndex=(PlayerId)player;
  GameKeeper::Player *playerRec=GameKeeper::Player::getPlayerByIndex(playerIndex);

  if(!playerRec)
    return ;

  TeamColor realTeam=convertTeam(_team);
  playerRec->player.setTeam(realTeam);

  sendSetTeam(playerIndex, realTeam);
}

BZF_API bool bz_zapPlayer(int player ) // forces a respawn
{
  if(player < 0)
    return false;

  PlayerId playerIndex=(PlayerId)player;
  GameKeeper::Player *playerRec=GameKeeper::Player::getPlayerByIndex(playerIndex);

  if(!playerRec || !playerRec->player.isAlive())
    return false;

  spawnPlayer(player);
  return true;
}


//-------------------------------------------------------------------------

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
  NetMsg msg = MSGMGR.newMessage();
  tmp->packCustom(msg);
  msg->broadcast(MsgFlagType);

  return true;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
