// pushstats.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"

std::string url;

class StatPush : public bz_Plugin
{
public:
  virtual const char* Name () {return "Stats Pusher";}
  virtual void Init ( const char* /* config */)
  {
    Register(bz_eListServerUpdateEvent);
    Register(bz_ePlayerPartEvent);
    Register(bz_eGetWorldEvent);

    if (bz_BZDBItemExists("_statURL"))
      url = bz_getBZDBString("_statURL").c_str();
    if (!url.size())
      url = "http://stattrack.bzflag.org/track/";
  }

  const char* getTeamName ( bz_eTeamType team )
  {
    switch (team) {
     default:
      break;
     case eRogueTeam:
      return "Rogue";
     case eRedTeam:
      return "Red";
     case eGreenTeam:
      return "Green";
     case eBlueTeam:
      return "Blue";
     case ePurpleTeam:
      return "Purple";
     case eObservers:
      return "Observer";
     case eRabbitTeam:
      return "Rabbit";
     case eHunterTeam:
      return "Hunter";
     }
    return "unknown";
  }

  void buildHTMLPlayer ( std::string &params, int playerID, int index )
  {
    bz_BasePlayerRecord	*player = bz_getPlayerByIndex(playerID);
    if (player)
    {
      params += format("&callsign%d=%s",index,bz_urlEncode(player->callsign.c_str()));

      params += format("&team%d=%s",index,bz_urlEncode(getTeamName(player->team)));

      std::string BZID = player->bzID.c_str();
      if (!BZID.size())
	BZID = "none";
      params += format("&bzID%d=%s",index,bz_urlEncode(BZID.c_str()));

      params += format("&token%d=V2",index);

      if (player->team != eObservers)
      {
	params += format("&wins%d=%d",index,player->wins);
	params += format("&losses%d=%d",index,player->losses);
	params += format("&teamkills%d=%d",index,player->teamKills);
      }
      params += format("&version%d=2.0.x",index);
      bz_freePlayerRecord(player);
    }
  }

  void buildHTMLPlayerList ( std::string &params, int skip = -1 )
  {
    bz_APIIntList *players = bz_newIntList();
    bz_getPlayerIndexList(players);;
    if (players && players->size())
    {
      int count = (int)players->size();
      if (skip > 0)
	count--;

      params += format("&playercount=%d", count);
      int index = 0;
      for ( unsigned int i = 0; i < players->size(); i++ )
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

  bool getPushHeader(std::string &header)
  {
    bz_ApiString host = bz_getPublicAddr();
    bz_ApiString desc = bz_getPublicDescription();

    header += "&isgameserver=1";

    header+= "&host=";
    if (host.size())
      header += host.c_str();
    else
      return false;

    header += format("&port=%s",port.c_str());
    if (desc.size())
      header += "&desc=" + std::string(desc.c_str());

    if (mapFile.size())
      header += "&map=" + mapFile;

    // game mode
    header += "&game=";
    switch (bz_getGameType())
    {
     default:
      header +="TeamFFA";
      break;
     case eCTFGame:
      header +="CTF";
      break;
     case eRabbitGame:
      header +="Rabbit";
      break;
    }

    // team scores
    header += format("&redteamscore=%d",bz_getTeamScore(eRedTeam));
    header += format("&redteamwins=%d",bz_getTeamWins(eRedTeam));
    header += format("&redteamlosses=%d",bz_getTeamLosses(eRedTeam));
    header += format("&greenteamscore=%d",bz_getTeamScore(eGreenTeam));
    header += format("&greenteamwins=%d",bz_getTeamWins(eGreenTeam));
    header += format("&greenteamlosses=%d",bz_getTeamLosses(eGreenTeam));
    header += format("&blueteamscore=%d",bz_getTeamScore(eBlueTeam));
    header += format("&blueteamwins=%d",bz_getTeamWins(eBlueTeam));
    header += format("&blueteamlosses=%d",bz_getTeamLosses(eBlueTeam));
    header += format("&purpleteamscore=%d",bz_getTeamScore(ePurpleTeam));
    header += format("&purpleteamwins=%d",bz_getTeamWins(ePurpleTeam));
    header += format("&purpleteamlosses=%d",bz_getTeamLosses(ePurpleTeam));
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

  void buildStateHash(std::string &params)
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
    bz_APIIntList *players = bz_newIntList();
    bz_getPlayerIndexList(players);;
    if (players && players->size())
    {
      for (unsigned int p = 0; p < players->size(); p++ )
      {
	bz_BasePlayerRecord	*player = bz_getPlayerByIndex(players->get(p));

	//int playerID = players->get(p);
	if (player)
	{
	  std::string BZID = player->bzID.c_str();
	  if (BZID.size())
	    i += sumString(BZID);
	  else
	    i += sumString(std::string(player->callsign.c_str()));

	  i += sumString("NONE");

	  i += player->wins;
	  i += player->losses;
	  i += player->teamKills;

	  bz_freePlayerRecord(player);
	}
      }
    }
    bz_deleteIntList(players);

    hash += (i * 100000);

    params += format("&hash=%d",hash);
  }

  virtual void Event ( bz_EventData *eventData )
  {
    {
      if (!eventData || !bz_getPublic())
	return;

      if (eventData->eventType == bz_eGetWorldEvent)
      {
	bz_GetWorldEventData_V1 *data = (bz_GetWorldEventData_V1*)eventData;
	mapFile = data->worldFile.c_str();
	if (!mapFile.size())
	  mapFile = "Random";
      }
      else
      {
	if (eventData->eventType == bz_eListServerUpdateEvent)
	{
	  bz_ListServerUpdateEvent_V1 *data = (bz_ListServerUpdateEvent_V1*)eventData;

	  const char *c = strrchr(data->address.c_str(),':');
	  if (!c)
	    port = "5154";
	  else
	    port = c+1;

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

  }

  std::string mapFile;
  std::string port;
};

BZ_PLUGIN(StatPush)


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
