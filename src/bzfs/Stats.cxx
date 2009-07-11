/* bzflag
* Copyright (c) 1993 - 2009 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "Stats.h"
#include "StateDatabase.h"
#include "TextUtils.h"
#include "global.h"
#include "GameKeeper.h"

StatsLink::StatsLink()
{
}

void StatsLink::init( void )
{
  sentAdd = false;
  bz_registerEvent(bz_eWorldFinalized,this);
  bz_registerEvent(bz_eListServerUpdateEvent,this);
  bz_registerEvent(bz_ePlayerPartEvent,this);
  bz_registerEvent(bz_eGetWorldEvent,this);

  if (BZDB.isSet("_statURL"))
    url = BZDB.get("_statURL");
  if (!url.size())
    url = "http://stattrack.bzflag.org/track/";
}

StatsLink::~StatsLink()
{
}

const char* getTeamName ( TeamColor team )
{
  switch(team) {
  default:
    break;
  case RogueTeam:
    return "Rogue";
  case RedTeam:
    return "Red";
  case GreenTeam:
    return "Green";
  case BlueTeam:
    return "Blue";
  case PurpleTeam:
    return "Purple";
  case ObserverTeam:
    return "Observer";
  case RabbitTeam:
    return "Rabbit";
  case HunterTeam:
    return "Hunter";
  }
  return "unknown";
}

void StatsLink::buildXMLPlayer ( std::string &params, int playerID )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
  if (player) {
    params += "\t<GamePlayer>\n";
    params += TextUtils::format("\t\t<callsign>%s</callsign>\n",player->player.getCallSign());
    if (player->hasCustomField("motto") && player->customData["motto"].size())
      params += TextUtils::format("\t\t<motto>%s</motto>\n", player->customData["motto"].c_str());
    else
      params += "\t\t<motto />\n";

    params += TextUtils::format("\t\t<team>%s</team>\n",getTeamName(player->player.getTeam()));

    std::string BZID = player->getBzIdentifier();
    if (!BZID.size())
      BZID = "none";

    params += TextUtils::format("\t\t<bzID>%s</bzID>\n",BZID.c_str());

    if (player->player.getTeam() == ObserverTeam) {
      params += "\t\t<wins>0</wins>\n";
      params += "\t\t<losses>0</losses>\n";
      params += "\t\t<teamkills>0</teamkills>\n";
    } else {
      params += TextUtils::format("\t\t<wins>%d</wins>\n",player->score.getWins());
      params += TextUtils::format("\t\t<losses>%d</losses>\n",player->score.getLosses());
      params += TextUtils::format("\t\t<teamkills>%d</teamkills>\n",player->score.getTKs());

    }
    params += "\t</GamePlayer>\n";
  }
}

void StatsLink::buildXMLPlayerList ( std::string &params )
{
  bz_APIIntList *players = bz_getPlayerIndexList();
  if (players && players->size()) {
    params += "&players=";
    params += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    params += "<ArrayOfGamePlayer xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n";

    for( unsigned int i = 0; i < players->size(); i++ )
      buildXMLPlayer(params,players->get(i));
    params += "</ArrayOfGamePlayer>\n";
  }
  bz_deleteIntList(players);
}

void StatsLink::buildHTMLPlayer ( std::string &params, int playerID, int index )
{
  GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
  if (player)
  {
    params += TextUtils::format("&callsign%d=%s",index,TextUtils::url_encode(std::string(player->player.getCallSign())).c_str());

    if (player->hasCustomField("motto") && player->customData["motto"].size())
      params += TextUtils::format("&motto%d=%s",index, TextUtils::url_encode(player->customData["motto"]).c_str());

    params += TextUtils::format("&team%d=%s",index,TextUtils::url_encode(std::string(getTeamName(player->player.getTeam()))).c_str());

    std::string BZID = player->getBzIdentifier();
    if (!BZID.size())
      BZID = "none";
    params += TextUtils::format("&bzID%d=%s",index,TextUtils::url_encode(BZID).c_str());

    std::string Token = player->player.getToken();
    if (!Token.size())
      Token = "none";
    params += TextUtils::format("&token%d=%s",index,TextUtils::url_encode(Token).c_str());

    if (player->player.getTeam() != ObserverTeam) 
    {
      params += TextUtils::format("&wins%d=%d",index,player->score.getWins());
      params += TextUtils::format("&losses%d=%d",index,player->score.getLosses());
      params += TextUtils::format("&teamkills%d=%d",index,player->score.getTKs());
    }
    params += TextUtils::format("&version%d=%s",index,TextUtils::url_encode(std::string(player->player.getClientVersion())).c_str());
  }
}

void StatsLink::buildHTMLPlayerList ( std::string &params, int skip )
{
  bz_APIIntList *players = bz_getPlayerIndexList();
  if (players && players->size())
  {
    int count = (int)players->size();
    if (skip > 0)
      count--;

    params += TextUtils::format("&playercount=%d", count);
    int index = 0;
    for( unsigned int i = 0; i < players->size(); i++ )
    {
      int playerID = players->get(i);
      if (playerID != skip)
      {
	buildHTMLPlayer(params,playerID,index);
	index ++;
      }
    }
  }
  bz_deleteIntList(players);
}

bool StatsLink::getPushHeader(std::string &header)
{
  bz_ApiString host = bz_getPublicAddr();
  int port = bz_getPublicPort();
  if (port == 0)
    port = 5154;

  bz_ApiString desc = bz_getPublicDescription();

  header += "&isgameserver=1";

  header+= "&host=";
  if (host.size())
    header += host.c_str();
  else
    return false;

  header += TextUtils::format("&port=%d",port);
  if (desc.size())
    header += "&desc=" + std::string(desc.c_str());

  if (mapFile.size())
    header += "&map=" + mapFile;

  // game mode
   header += "&game=";
  switch(bz_getGameType())
  {
    default:
      header +="TeamFFA";
      break;
    case eClassicCTFGame:
      header +="CTF";
      break;
    case eRabbitGame:
      header +="Rabbit";
      break;
    case eOpenFFAGame:
      header +="OpenFFA";
      break;
  }

  // team scores
  header += TextUtils::format("&redteamscore=%d",bz_getTeamScore(eRedTeam));
  header += TextUtils::format("&redteamwins=%d",bz_getTeamWins(eRedTeam));
  header += TextUtils::format("&redteamlosses=%d",bz_getTeamLosses(eRedTeam));
  header += TextUtils::format("&greenteamscore=%d",bz_getTeamScore(eGreenTeam));
  header += TextUtils::format("&greenteamwins=%d",bz_getTeamWins(eGreenTeam));
  header += TextUtils::format("&greenteamlosses=%d",bz_getTeamLosses(eGreenTeam));
  header += TextUtils::format("&blueteamscore=%d",bz_getTeamScore(eBlueTeam));
  header += TextUtils::format("&blueteamwins=%d",bz_getTeamWins(eBlueTeam));
  header += TextUtils::format("&blueteamlosses=%d",bz_getTeamLosses(eBlueTeam));
  header += TextUtils::format("&purpleteamscore=%d",bz_getTeamScore(ePurpleTeam));
  header += TextUtils::format("&purpleteamwins=%d",bz_getTeamWins(ePurpleTeam));
  header += TextUtils::format("&purpleteamlosses=%d",bz_getTeamLosses(ePurpleTeam));
  return true;
}

int sumString( const std::string &str )
{
  int i = 0;
  std::string::const_iterator itr = str.begin();
  while (itr != str.end())
    i += *itr++;
  return i;
}

void StatsLink::buildStateHash(std::string &params)
{
  int hash = sumString(mapFile);

  int i = 0;
  i += bz_getTeamScore(eRedTeam);
  i += bz_getTeamScore(eGreenTeam);
  i += bz_getTeamScore(eBlueTeam);
  i += bz_getTeamScore(ePurpleTeam);
  i += bz_getTeamWins(eRedTeam);
  i += bz_getTeamWins(eGreenTeam);
  i += bz_getTeamWins(eBlueTeam);
  i += bz_getTeamWins(ePurpleTeam);
  i += bz_getTeamLosses(eRedTeam);
  i += bz_getTeamLosses(eGreenTeam);
  i += bz_getTeamLosses(eBlueTeam);
  i += bz_getTeamLosses(ePurpleTeam);

  hash += (i * 1000);

  i = 0;
  bz_APIIntList *players = bz_getPlayerIndexList();
  if (players && players->size())
  {
    for (size_t p = 0; p < players->size(); p++ )
    {
      int playerID = players->get(p);
      GameKeeper::Player *player = GameKeeper::Player::getPlayerByIndex(playerID);
      if (player)
      {
	std::string BZID = player->getBzIdentifier();
	if (BZID.size())
	  i += sumString(BZID);
	else
	  i += sumString(std::string(player->player.getCallSign()));

	i += sumString(player->player.getToken());

	i += player->score.getWins();
	i += player->score.getLosses();
	i += player->score.getTKs();
      }
    }
  }
  bz_deleteIntList(players);

  hash += (i * 100000);

  params += TextUtils::format("&hash=%d",hash);
}


void StatsLink::process(bz_EventData *eventData)
{
  if (!eventData || !bz_getPublic())
    return;

  if (eventData->eventType == bz_eGetWorldEvent)
  {
    bz_GetWorldEventData_V1 *data = (bz_GetWorldEventData_V1*)eventData;
    mapFile = data->worldFile.c_str();
    if (!mapFile.size())
    {
      if (data->worldBlob)
	mapFile = "Custom";
      else
	mapFile = "Random";
    }
  }
  else
  {
    if (eventData->eventType == bz_eListServerUpdateEvent || eventData->eventType == bz_eWorldFinalized) 
    {
      std::string params = "action=add&";
      getPushHeader(params);

      buildHTMLPlayerList(params);

      buildStateHash(params);

      bz_addURLJob(url.c_str(),NULL,params.c_str());
    }
    else if (eventData->eventType == bz_ePlayerPartEvent) 
    {
      bz_PlayerJoinPartEventData_V1 *data = (bz_PlayerJoinPartEventData_V1*)eventData;
      std::string params = "action=part";
      getPushHeader(params);

      if (data->playerID) // we use -1 for the parted player, then skip them in the player list. this way we always get all player data on a part
	  buildHTMLPlayer(params,data->playerID,-1);

      buildHTMLPlayerList(params, data->playerID);

      bz_addURLJob(url.c_str(),NULL,params.c_str());
    }
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
