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

#include "Stats.h"
#include "StateDatabase.h"
#include "TextUtils.h"
#include "global.h"
#include "GameKeeper.h"

StatsLink::StatsLink()
{
  sentAdd = false;
  bz_registerEvent(bz_eWorldFinalized,this);
  bz_registerEvent(bz_eListServerUpdateEvent,this);
  bz_registerEvent(bz_ePlayerPartEvent,this);

  if (BZDB.isSet("_statURL"))
    url = BZDB.get("_statURL");
  if (!url.size())
    url = "http://stattrack.bzflag.bz:88";
}

StatsLink::~StatsLink()
{
}

const char* getTeamName ( TeamColor team )
{
    switch(team)
    {
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
  if (player)
  {
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

    if (player->player.getTeam() == ObserverTeam)
    {
      params += "\t\t<wins>0</wins>\n";
      params += "\t\t<losses>0</losses>\n";
      params += "\t\t<teamkills>0</teamkills>\n";
    }
    else
    {
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
  if (players && players->size())
  {
    params += "&players=";
    params += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    params += "<ArrayOfGamePlayer xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\n";

    for( unsigned int i = 0; i < players->size(); i++ )
     buildXMLPlayer(params,players->get(i));
    params += "</ArrayOfGamePlayer>\n";
  }
  bz_deleteIntList(players);
}

void StatsLink::process(bz_EventData *eventData)
{
  if (!eventData)
    return;
  if (eventData->eventType == bz_eWorldFinalized)
    sentAdd = false;
  else if (eventData->eventType == bz_eListServerUpdateEvent)
  {
    if (bz_getPublic())
    {
      bz_ApiString host = bz_getPublicAddr();
      int port = bz_getPublicPort();
      if (port == 0)
	port = 5154;

      bz_ApiString desc = bz_getPublicDescription();

      std::string params = "action=add&isgameserver=1";
      
      params+= "&host=";
      if (host.size())
	params += host.c_str();
      else
	return;

      params += TextUtils::format("&port=%d",port);
      if (desc.size())
	params += "&desc=" + std::string(desc.c_str());

      buildXMLPlayerList(params);

      bz_addURLJob(url.c_str(),NULL,params.c_str());
    }
    else if (eventData->eventType == bz_ePlayerPartEvent)
    {
      bz_PlayerJoinPartEventData_V1 *data = (bz_PlayerJoinPartEventData_V1*)eventData;
      std::string params = "action=part&isgameserver=1&player=";
      buildXMLPlayerList(params);
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
