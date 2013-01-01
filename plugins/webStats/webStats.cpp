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

// WebStats.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "bzfsHTTPAPI.h"
#include "plugin_utils.h"
#include <map>
#include <string>

std::string templatesDir;

class WebStats : public bz_Plugin, public bzhttp_VDir, public bzhttp_TemplateCallback
{
public:
	WebStats();

	virtual const char* Name (){return "Web Statistics";}
	virtual void Init(const char* config);
	virtual void Cleanup();
	virtual void Event ( bz_EventData *eventData );

	void loadDefaultTemplates(void);

	virtual const char* GetTemplateKey(const char* /* key */);
	virtual bool GetTemplateLoop(const char* /* key */, const char* /*param*/);
	virtual bool GetTemplateIF(const char* /* key */, const char* /*param*/);

	virtual const char* VDirName(){return "WebStats";}
	virtual const char* VDirDescription(){return "View game and server stats on-line";}

	virtual bzhttp_ePageGenStatus GeneratePage ( const bzhttp_Request& request, bzhttp_Responce &responce );
	//virtual bool GenerateNoAuthPage ( const bzhttp_Request& request, bzhttp_Responce &responce);

	virtual bool AllowResourceDownloads ( void );

protected:
	std::string resourceDir;

	// globals for report
	int player;
	std::map<bz_eTeamType, std::vector<bz_BasePlayerRecord*> > teamSort;
	std::map<bz_eTeamType, std::vector<bz_BasePlayerRecord*> >::iterator teamSortItr;
	size_t playerInTeam;

	bz_BasePlayerRecord* playeRecord;
	int flagReportID;

	// default template
	std::string defaultMainTemplate;
	std::string defaultPlayerTemplate;
	std::string defaultFlagTemplate;

	unsigned int groupLoop;
	unsigned int flagHistoryLoop;

	double serverStartTime;

	std::map<int, int> registeredPlayerMap;
	std::map<std::string, int> nonRegedPlayerMap;

	int joins;
	int spawns;
	int deaths;

	size_t dataIn, dataOut;

	// reports

	void doFlagReport(std::string& page, int flagID);
	void doPlayerReport(std::string& page, int playerID);
	void doStatReport(std::string& page, std::string& action);

	void initReport(void);
	void finishReport(void);
};

BZ_PLUGIN(WebStats)

WebStats::WebStats() :bz_Plugin(), bzhttp_VDir(),bzhttp_TemplateCallback()
{

}

void WebStats::Init(const char *commandLine)
{
  bzhttp_RegisterVDir(this,this);
  this->RequiredAuthentiction = eBZID;
  this->CacheAuthentication = true;

  Register(bz_ePlayerJoinEvent);
  Register(bz_ePlayerSpawnEvent);
  Register(bz_ePlayerDieEvent);
  Register(bz_eNetDataSendEvent);
  Register(bz_eNetDataReceiveEvent);

  loadDefaultTemplates();

  if (commandLine)
    resourceDir = commandLine;
  if (resourceDir.size())
  {
    bzhttp_AddSearchPath("StatsTemplates",commandLine);
    this->ResourceDirs.push_back(std::string(commandLine));
  }
}

void WebStats::Event ( bz_EventData *eventData )
{
	switch (eventData->eventType) 
	{
	default:
		break;

	case bz_ePlayerJoinEvent:
		{
			joins++;
			bz_PlayerJoinPartEventData_V1* evtData = (bz_PlayerJoinPartEventData_V1*)eventData;
			if (evtData->record->verified) 
			{
				int bzID = atoi(evtData->record->bzID.c_str());
				if (registeredPlayerMap.find(bzID) == registeredPlayerMap.end())
					registeredPlayerMap[bzID] = 1;
				else 
					registeredPlayerMap[bzID]++;
			}
			else 
			{
				std::string name = bz_tolower(evtData->record->callsign.c_str());

				if (nonRegedPlayerMap.find(name) == nonRegedPlayerMap.end())
					nonRegedPlayerMap[name] = 1;
				else
					nonRegedPlayerMap[name]++;
			}
		}
		break;

	case bz_ePlayerSpawnEvent:
		spawns++;
		break;

	case bz_ePlayerDieEvent:
		deaths++;
		break;

	case bz_eNetDataSendEvent:
		{
			bz_NetTransferEventData_V1* evtData = (bz_NetTransferEventData_V1*)eventData;
			dataOut += evtData->iSize;
		}
		break;

	case bz_eNetDataReceiveEvent: 
		{
			bz_NetTransferEventData_V1* evtData = (bz_NetTransferEventData_V1*)eventData;
			dataIn += evtData->iSize;
		}
		break;
	}
}

bool WebStats::AllowResourceDownloads ( void )
{
  return resourceDir.size() > 0;
}

void WebStats::Cleanup()
{
  bzhttp_RemoveAllVdirs(this);
  Flush();
}

void getStatus(bz_BasePlayerRecord* rec, std::string& data)
{
	if (rec->team != eObservers) 
	{
		if (rec->admin)
			data += "Admin";

		if (rec->spawned) 
		{
			if (rec->admin)
				data += "/";
			data += "Spawned";
		}

		if (rec->verified)
		{
			if (rec->admin || rec->spawned)
				data += "/";

			data += "Verified";
		}
	}

	if (!data.size())
		data = "&nbsp;";
}


const char* WebStats::GetTemplateKey(const char* _key)
{
	if (!_key)
		return "";

	bz_BasePlayerRecord* rec = NULL;
	if (playeRecord) 
		rec = playeRecord;
	else if (teamSortItr != teamSort.end() && playerInTeam < teamSortItr->second.size())
		rec = teamSortItr->second[playerInTeam];

	int flagID = flagReportID;
	if (rec) 
		flagID = rec->currentFlagID;

	static std::string data;
	data = "";
	std::string key = _key;

	double uptime = bz_getCurrentTime() - serverStartTime;

	if (key == "playercount") 
	{
		data = bz_format("%d", bz_getPlayerCount());
	}
	else if (key == "gametype")
	{
		switch (bz_getGameType()) 
		{
		case eFFAGame:
			data = "Team FFA";
			break;

		case eCTFGame:
			data = "Team Capture the Flag";
			break;

		case eRabbitGame:
			data = "Rabbit Hunt";
			break;

		case eOpenFFAGame:
			data = "Open FFA";
			break;

		default:
			data = "other";
			break;
		}
	}
	else if (key == "uptime") 
	{
		int days = (int)(uptime / 86400.0);
		uptime -= days * 86400.0;
		int hours = (int)(uptime / 3600.0);
		uptime -= hours * 3600.0;
		int min = (int)(uptime / 60.0);
		uptime -= min * 60.0;
		int seconds = (int)(uptime);
		data = bz_format("%d days %d hours %d minutes %d seconds", days, hours, min, seconds);
	}
	else if (key == "totalplayers") 
	{
		size_t count = registeredPlayerMap.size() + nonRegedPlayerMap.size();
		data = bz_format("%d", (int)count);
	}
	else if (key == "joins")
		data = bz_format("%d", joins);
	else if (key == "spawns") 
		data = bz_format("%d", spawns);
	else if (key == "deaths")
		data = bz_format("%d", deaths);
	else if (key == "uniquereged")
		data = bz_format("%d", (int)registeredPlayerMap.size());
	else if (key == "uniqueunreged")
		data = bz_format("%d", (int)nonRegedPlayerMap.size());
	else if (key == "datainb")
		data = bz_format("%d", (int)dataIn);
	else if (key == "dataoutb") 
		data = bz_format("%d", (int)dataOut);
	else if (key == "datainkb")
		data = bz_format("%f", dataIn / 1024.0);
	else if (key == "dataoutkb")
		data = bz_format("%f", dataOut / 1024.0);
	else if (key == "datainmb")
		data = bz_format("%f", dataIn / 1024.0 / 1024.0);
	else if (key == "dataoutmb")
		data = bz_format("%f", dataOut / 1024.0 / 1024.0);
	else if (key == "dataingb") 
		data = bz_format("%f", dataIn / 1024.0 / 1024.0 / 1024.0);
	else if (key == "dataoutgb")
		data = bz_format("%f", dataOut / 1024.0 / 1024.0 / 1024.0);
	else if (key == "totaltransferb") 
		data = bz_format("%f", (float)(dataOut + dataIn));
	else if (key == "totaltransferkb")
		data = bz_format("%f", (dataOut + dataIn) / 1024.0);
	else if (key == "totaltransfermb")
		data = bz_format("%f", (dataOut + dataIn) / 1024.0 / 1024.0);
	else if (key == "totaltransfergb")
		data = bz_format("%f", (dataOut + dataIn) / 1024.0 / 1024.0 / 1024.0);
	else if (key == "datainavgkb")
		data = bz_format("%f", (dataIn / uptime) / 1024.0);
	else if (key == "dataoputavgkb")
		data = bz_format("%f", (dataOut / uptime) / 1024.0);
	else if (key == "redteamcount")
		data = bz_format("%d", bz_getTeamCount(eRedTeam));
	else if (key == "blueteamcount")
		data = bz_format("%d", bz_getTeamCount(eBlueTeam));
	else if (key == "greenteamcount")
		data = bz_format("%d", bz_getTeamCount(eGreenTeam));
	else if (key == "rpurpleteamcount")
		data = bz_format("%d", bz_getTeamCount(ePurpleTeam));
	else if (key == "rogueteamcount")
		data = bz_format("%d", bz_getTeamCount(eRogueTeam));
	else if (key == "observerteamcount")
		data = bz_format("%d", bz_getTeamCount(eObservers));
	else if (key == "hunterteamcount")
		data = bz_format("%d", bz_getTeamCount(eHunterTeam));
	else if (key == "rabbitteamcount")
		data = bz_format("%d", bz_getTeamCount(eRabbitTeam));
	else if (key == "redteamscore")
		data = bz_format("%d", bz_getTeamScore(eRedTeam));
	else if (key == "redteamwins")
		data = bz_format("%d", bz_getTeamWins(eRedTeam));
	else if (key == "redteamlosses")
		data = bz_format("%d", bz_getTeamLosses(eRedTeam));
	else if (key == "blueteamscore")
		data = bz_format("%d", bz_getTeamScore(eBlueTeam));
	else if (key == "blueteamwins")
		data = bz_format("%d", bz_getTeamWins(eBlueTeam));
	else if (key == "blueteamlosses")
		data = bz_format("%d", bz_getTeamLosses(eBlueTeam));
	else if (key == "greenteamscore")
		data = bz_format("%d", bz_getTeamScore(eGreenTeam));
	else if (key == "greenteamwins")
		data = bz_format("%d", bz_getTeamWins(eGreenTeam));
	else if (key == "greenteamlosses")
		data = bz_format("%d", bz_getTeamLosses(eGreenTeam));
	else if (key == "purpleteamscore")
		data = bz_format("%d", bz_getTeamScore(ePurpleTeam));
	else if (key == "purpleteamwins")
		data = bz_format("%d", bz_getTeamWins(ePurpleTeam));
	else if (key == "purpleteamlosses")
		data = bz_format("%d", bz_getTeamLosses(ePurpleTeam));
	else if (key == "flagcount")
		data = bz_format("%d", bz_getNumFlags());
	else if (key == "playerid")
	{
		if (rec)
			data = format("%d", rec->playerID);
		else
			data = "-1";
	}
	else if (flagID >= 0)
	{
		if (key == "flagname")
			data = bz_getFlagName(flagID).c_str();
		else if (key == "flagplayer")
			data = bz_getPlayerCallsign(bz_flagPlayer(flagID));
		else if (key == "flagplayerid")
			data = bz_format("%d", bz_flagPlayer(flagID));
		else if (key == "flagx" || key == "flagy" || key == "flagz") 
		{
			float pos[3] = {0, 0, 0};
			if (bz_getFlagPosition(flagID, pos))
				data = bz_format("%.3f", pos[key[4] - 'x']);
		}
	}

	if (data.size() == 0 && rec)
	{
		if (key == "teamname")
			data = bzu_GetTeamName(rec->team);
		else if (key == "callsign") 
			data = rec->callsign.c_str();
		else if (key == "rank") 
			data = bz_format("%f%%", rec->rank);
		else if (key == "wins")
			data = bz_format("%d", rec->wins);
		else if (key == "losses")
			data = bz_format("%d", rec->losses);
		else if (key == "teamkills")
			data = bz_format("%d", rec->teamKills);
		else if (key == "status")
			getStatus(rec, data);
		else if (key == "playerflag") 
			data = rec->currentFlag.c_str();
		else if (key == "bzid") 
			data = rec->bzID.c_str();
		else if (key == "ipaddress")
			data = rec->ipAddress.c_str();
		else if (key == "lag")
			data = bz_format("%d", rec->lag);
		else if (key == "jitter") 
			data = bz_format("%d", rec->jitter);
		else if (key == "packetloss")
			data = bz_format("%f", rec->packetLoss);
		else if (key == "groupcount") 
			data = bz_format("%d", rec->groups.size());
		else if (key == "groupname" && groupLoop < rec->groups.size()) 
			data = rec->groups[groupLoop].c_str();
		else if (key == "flaghistorycount")
			data = bz_format("%d", rec->flagHistory.size());
		else if (key == "flaghistoryflag" && flagHistoryLoop < rec->flagHistory.size())
			data = rec->flagHistory[flagHistoryLoop].c_str();
		else if (key == "teamcount") 
			data = bz_format("%d", bz_getTeamCount(rec->team));
		else if (key == "teamscore")
			data = bz_format("%d", bz_getTeamScore(rec->team));
		else if (key == "teamwins")
			data = bz_format("%d", bz_getTeamWins(rec->team));
		else if (key == "teamlosses") 
			data = bz_format("%d", bz_getTeamLosses(rec->team));
	}
  return data.c_str();
}

bool WebStats::GetTemplateLoop(const char* _key, const char* /*_param*/)
{
	if (!_key)
		return false;

	bz_BasePlayerRecord* rec = NULL;
	if (playeRecord) 
		rec = playeRecord;
	else if (teamSortItr != teamSort.end() && playerInTeam < teamSortItr->second.size())
		rec = teamSortItr->second[playerInTeam];

	std::string key = _key;

	if (key == "players")
	{
		if (playeRecord || !teamSort.size()) 
			return false;

		if (teamSortItr == teamSort.end())
		{
			teamSortItr = teamSort.begin();
			playerInTeam = 0;
		}
		else
		{
			playerInTeam++;
			if (playerInTeam >= teamSortItr->second.size())
			{
				teamSortItr++;
				playerInTeam = 0;
			}
		}
		return teamSortItr != teamSort.end();
	}
	else if (key == "groups" && rec)
	{
		if (!groupLoop)
			groupLoop++;
		else 
			groupLoop = 0;

		if (groupLoop >= rec->groups.size())
		{
			groupLoop = 0;
			return false;
		}
		return true;
	}
	else if (key == "flaghistory" && rec)
	{
		if (!flagHistoryLoop) 
			flagHistoryLoop++;
		else
			flagHistoryLoop = 0;

		if (flagHistoryLoop >= rec->flagHistory.size())
		{
			flagHistoryLoop = 0;
			return false;
		}
		return true;
	}
	else if (key == "flaghistory" && rec)
	{
		if (flagReportID != -1)
			flagReportID++;
		else
			flagReportID = 0;

		if (flagReportID >= (int)bz_getNumFlags())
		{
			flagReportID = -1;
			return false;
		}
		return true;
	}
	return false;
}

bool WebStats::GetTemplateIF(const char* _key, const char* /*_param*/)
{
	if (!_key)
		return false;

	bz_BasePlayerRecord* rec = NULL;
	if (playeRecord)
		rec = playeRecord;
	else if (teamSortItr != teamSort.end() && playerInTeam < teamSortItr->second.size()) 
		rec = teamSortItr->second[playerInTeam];

	std::string key = _key;

	if (key == "newteam")
		return teamSortItr != teamSort.end() && playerInTeam == 0;
	else if (key == "players") 
		return playeRecord != NULL ? playeRecord != NULL : teamSort.size() > 0;
	else if (key == "redteam") 
		return bz_getTeamCount(eRedTeam) > 0;
	else if (key == "greenteam") 
		return bz_getTeamCount(eGreenTeam) > 0;
	else if (key == "blueteam") 
		return bz_getTeamCount(eBlueTeam) > 0;
	else if (key == "purpleteam")
		return bz_getTeamCount(ePurpleTeam) > 0;
	else if (key == "observerteam")
		return bz_getTeamCount(eObservers) > 0;
	else if (key == "rogueteam")
		return bz_getTeamCount(eRogueTeam) > 0;
	else if (key == "hunterteam")
		return bz_getTeamCount(eHunterTeam) > 0;
	else if (key == "rabbitteam")
		return bz_getTeamCount(eRabbitTeam) > 0;
	else if (key == "teamffa") 
		return bz_getGameType() == eFFAGame;
	else if (key == "openffa") 
		return bz_getGameType() == eOpenFFAGame;
	else if (key == "ctf") 
		return bz_getGameType() == eCTFGame;
	else if (key == "rabbitchase") 
		return bz_getGameType() == eRabbitGame;
	else if (key == "flags") 
		return bz_getNumFlags() > 0;
	else if (rec)
	{
		if (key == "spawned") 
			return rec->spawned;
		else if (key == "verified")
			return rec->verified;
		else if (key == "global")
			return rec->globalUser;
		else if (key == "admin")
			return rec->admin;
		else if (key == "op")
			return rec->op;
		else if (key == "canspawn")
			return rec->canSpawn;
		else if (key == "groups")
			return rec->groups.size() > 0;
		else if (key == "flaghistory")
			return rec->flagHistory.size() > 0;
	}
	return false;
}

std::string GetParamater(const bzhttp_Request& request, const char* name)
{
	std::string p;
	const char*a = request.GetParamater(name);
	if (a)
		p = a;
	return p;
}

 bzhttp_ePageGenStatus WebStats::GeneratePage ( const bzhttp_Request& request, bzhttp_Responce &responce )
{
  responce.ReturnCode = e200OK;
  responce.DocumentType = eHTML;
 
  groupLoop = 0;
  flagHistoryLoop = 0;
  playeRecord = NULL;
  flagReportID = -1;

  std::string page;

  std::string action, playerID, flagID;
  action = GetParamater(request,"action");
  playerID = GetParamater(request,"playerid");
  flagID = GetParamater(request,"flagid");

  if (action == "player" && playerID.size()) 
	  doPlayerReport(page, atoi(playerID.c_str()));
  else if (action == "flag" && flagID.size()) 
	  doFlagReport(page, atoi(flagID.c_str()));
  else
	  doStatReport(page, action);

  responce.AddBodyData(page.c_str());
  return ePageDone;
}

 void WebStats::doFlagReport(std::string& page, int flagID)
 {
	 flagReportID = flagID;
	 if (flagReportID < 0 || flagReportID > (int)bz_getNumFlags())
		 page += "Invalid Flag";
	 else
	 {
		 const char* file = bzhttp_FindFile("StatsTemplates","flag.tmpl");
		 if (file)
			 page += bzhttp_RenderTemplate(file,this).c_str();
		 else
			 page += bzhttp_RenderTemplateFromText(defaultFlagTemplate.c_str(),this).c_str();
	 }
	 flagReportID = -1;
 }

 void WebStats::doPlayerReport(std::string& page, int playerID) 
 {
	 playeRecord = bz_getPlayerByIndex(playerID);
	 if (!playeRecord) 
	 {
		 page += "Invalid Player";
	 }
	 else
	 {
		 const char* file = bzhttp_FindFile("StatsTemplates","player.tmpl");
		 if (file)
			 page += bzhttp_RenderTemplate(file,this).c_str();
		 else
			 page += bzhttp_RenderTemplateFromText(defaultPlayerTemplate.c_str(),this).c_str();
	 }
	 playeRecord = NULL;
 }

 void WebStats::doStatReport(std::string& page, std::string& action) 
 {
	 playeRecord = NULL;
	 initReport();

	 if (action.size())
		 action += ".tmpl";

	 const char* file = bzhttp_FindFile("StatsTemplates",action.c_str());

	 if (!action.size() || !file)
		 page += bzhttp_RenderTemplateFromText(defaultMainTemplate.c_str(),this).c_str();
	 else
		 page += bzhttp_RenderTemplate(file,this).c_str();

	 finishReport();
 }
 void WebStats::initReport(void)
 {
	 bz_APIIntList* players = bz_getPlayerIndexList();

	 for (int i = 0; i < (int)players->size(); i++)
	 {
		 int playerID = players->get(i);
		 bz_BasePlayerRecord* rec = bz_getPlayerByIndex(playerID);

		 if (rec)
		 {
			 if (teamSort.find(rec->team) == teamSort.end())
			 {
				 std::vector<bz_BasePlayerRecord*> temp;
				 teamSort[rec->team] = temp;
			 }
			 teamSort[rec->team].push_back(rec);
		 }
	 }
	 teamSortItr = teamSort.end();
	 playerInTeam = 0;
 }

 void WebStats::finishReport(void)
 {
	 teamSortItr = teamSort.begin();
	 while (teamSortItr != teamSort.end())
	{
		 for (size_t i = 0; i < teamSortItr->second.size(); i++)
			 bz_freePlayerRecord(teamSortItr->second[i]);

		 teamSortItr++;
	 }
	 teamSort.clear();
 }

void WebStats::loadDefaultTemplates(void)
{
	defaultMainTemplate = "<html><head></head><body><h2>Players</h2>";
	defaultMainTemplate += "[*START Players][$Callsign]<br>[*END Players]None[*EMPTY Players]<hr></body></html>";

	defaultPlayerTemplate = "<html><head></head><body><h2>[$Callsign]</h2><b>[$TeamName]</b> [$Wins]/[$Losses]([$TeamKills]) [$Status]</body></html>";
	defaultFlagTemplate = "<html><head></head><body><h2>[$FlagType]</h2><b>Player [$FlagPlayer]</b> [$FlagX],[$FlagY],[$FlagZ]</body></html>";
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
