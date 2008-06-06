// webstats.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "plugin_HTTPTemplates.h"

class WebStats : public BZFSHTTPServer, TemplateCallbackClass
{
public:
  WebStats(const char *plugInName): BZFSHTTPServer(plugInName) {};

  void init(const char *commandLine);

  virtual bool acceptURL(const char *url) { return true; }
  virtual void getURLData(const char* url, int requestID,
			  const URLParams &paramaters, bool get = true);

  Templateiser	templateSystem;

  virtual void keyCallback(std::string &data, const std::string &key);
  virtual bool loopCallback(const std::string &key);
  virtual bool ifCallback(const std::string &key);

  void initReport(void);
  void finishReport(void);

  void doStatReport(std::string &page, std::string &action);
  void doPlayerReport(std::string &page, int playerID);
  void doFlagReport(std::string &page, int flagID);

  // globals for report
  int player;
  std::map<bz_eTeamType, std::vector<bz_BasePlayerRecord*> > teamSort;
  std::map<bz_eTeamType, std::vector<bz_BasePlayerRecord*> >::iterator teamSortItr;
  size_t playerInTeam;

  bz_BasePlayerRecord *playeRecord;
  int flagReportID;

  // default template
  std::string defaultMainTemplate;
  std::string defaultPlayerTemplate;
  std::string defaultFlagTemplate;

  unsigned int groupLoop;
  unsigned int flagHistoryLoop;


  // plugin computed stats
  class ComputedStats : public bz_EventHandler
  {
  public:
    virtual void process ( bz_EventData *eventData );
    double serverStartTime;
    
    std::map<int,int> registeredPlayerMap;
    std::map<std::string,int> nonRegedPlayerMap;

    int joins;
    int spawns;
    int deaths;

    size_t dataIn,dataOut;
  };

  ComputedStats stats;
 
};

WebStats webStats("webstats");

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
  bz_debugMessage(4, "webstats plugin loaded");

  bz_registerEvent(bz_ePlayerJoinEvent,&webStats.stats);
  bz_registerEvent(bz_ePlayerSpawnEvent,&webStats.stats);
  bz_registerEvent(bz_ePlayerDieEvent,&webStats.stats);

  bz_registerEvent(bz_eNetDataSendEvent,&webStats.stats);
  bz_registerEvent(bz_eNetDataReceveEvent,&webStats.stats);

  webStats.init(commandLine);
  webStats.startupHTTP();
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void)
{
  bz_removeEvent(bz_ePlayerJoinEvent,&webStats.stats);
  bz_removeEvent(bz_ePlayerSpawnEvent,&webStats.stats);
  bz_removeEvent(bz_ePlayerDieEvent,&webStats.stats);

  bz_removeEvent(bz_eNetDataSendEvent,&webStats.stats);
  bz_removeEvent(bz_eNetDataReceveEvent,&webStats.stats);

  webStats.shutdownHTTP();
  bz_debugMessage(4, "webstats plugin unloaded");
  return 0;
}

void WebStats::ComputedStats::process ( bz_EventData *eventData )
{
  switch(eventData->eventType)
  {
    default:
      break;

    case bz_ePlayerJoinEvent:
      {
	joins++;
	bz_PlayerJoinPartEventData_V1 *data = (bz_PlayerJoinPartEventData_V1*)eventData;
	if (data->record->verified)
	{
	  int bzID = atoi(data->record->bzID.c_str());
	  if (registeredPlayerMap.find(bzID) == registeredPlayerMap.end())
	    registeredPlayerMap[bzID] = 1;
	  else
	    registeredPlayerMap[bzID]++;
	}
	else
	{
	  std::string name = tolower(data->record->callsign.c_str());

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
	bz_NetTransferEventData_V1 *data = (bz_NetTransferEventData_V1*)eventData;
	dataOut += data->iSize;
      }
      break;

    case bz_eNetDataReceveEvent:
      {
	bz_NetTransferEventData_V1 *data = (bz_NetTransferEventData_V1*)eventData;
	dataIn += data->iSize;
      }
      break;
  }
}

void WebStats::init(const char *commandLine)
{
  stats.serverStartTime = bz_getCurrentTime();

  stats.joins = stats.spawns = stats.deaths = 0;
  stats.dataIn = stats.dataOut = 0;

  templateSystem.addSearchPath("./");
  templateSystem.addSearchPath(commandLine);

  templateSystem.setPluginName("WebStats", getBaseServerURL());

  templateSystem.addKey("PlayerCount", this);
  templateSystem.addLoop("Players", this);
  templateSystem.addIF("NewTeam", this);
  templateSystem.addIF("Players", this);
  templateSystem.addKey("TeamName", this);
  templateSystem.addKey("Callsign", this);
  templateSystem.addKey("Wins", this);
  templateSystem.addKey("Losses", this);
  templateSystem.addKey("TeamKills", this);
  templateSystem.addKey("Status", this);
  templateSystem.addKey("PlayerID", this);
  templateSystem.addKey("PlayerFlag", this);

  templateSystem.addIF("Spawned", this);
  templateSystem.addIF("Verified", this);
  templateSystem.addIF("Global", this);
  templateSystem.addIF("Admin", this);
  templateSystem.addIF("Op", this);
  templateSystem.addIF("CanSpawn", this);

  templateSystem.addKey("BZID", this);

  // groups
  templateSystem.addKey("GroupCount", this);
  templateSystem.addIF("Groups", this);
  templateSystem.addLoop("Groups", this);
  templateSystem.addKey("GroupName", this);

  // flagHistory
  templateSystem.addKey("FlagHistoryCount", this);
  templateSystem.addIF("FlagHistory", this);
  templateSystem.addLoop("FlagHistory", this);
  templateSystem.addKey("FlagHistoryFlag", this);

  // lag info
  templateSystem.addKey("Lag", this);
  templateSystem.addKey("Jitter", this);
  templateSystem.addKey("PacketLoss", this);

  templateSystem.addKey("IPAddress", this);

  // player team info
  templateSystem.addKey("TeamCount", this);
  templateSystem.addKey("TeamScore", this);
  templateSystem.addKey("TeamWins", this);
  templateSystem.addKey("TeamLosses", this);

  // global info
  templateSystem.addKey("RedTeamCount", this);
  templateSystem.addIF("RedTeam", this);
  templateSystem.addKey("RedTeamScore", this);
  templateSystem.addKey("RedTeamWins", this);
  templateSystem.addKey("RedTeamLosses", this);

  templateSystem.addKey("BlueTeamCount", this);
  templateSystem.addIF("BlueTeam", this);
  templateSystem.addKey("BlueTeamScore", this);
  templateSystem.addKey("BlueTeamWins", this);
  templateSystem.addKey("BlueTeamLosses", this);

  templateSystem.addKey("GreenTeamCount", this);
  templateSystem.addIF("GreenTeam", this);
  templateSystem.addKey("GreenTeamScore", this);
  templateSystem.addKey("GreenTeamWins", this);
  templateSystem.addKey("GreenTeamLosses", this);

  templateSystem.addKey("PurpleTeamCount", this);
  templateSystem.addIF("PurpleTeam", this);
  templateSystem.addKey("PurpleTeamScore", this);
  templateSystem.addKey("PurpleTeamWins", this);
  templateSystem.addKey("PurpleTeamLosses", this);

  templateSystem.addKey("RabbitTeamCount", this);
  templateSystem.addIF("RabbitTeam", this);

  templateSystem.addKey("RogueTeamCount", this);
  templateSystem.addIF("RogueTeam", this);

  templateSystem.addKey("HunterTeamCount", this);
  templateSystem.addIF("HunterTeam", this);

  templateSystem.addKey("ObserversTeamCount", this);
  templateSystem.addIF("ObserversTeam", this);

  templateSystem.addKey("GameType", this);
  templateSystem.addIF("TeamFFA", this);
  templateSystem.addIF("OpenFFA", this);
  templateSystem.addIF("CTF", this);
  templateSystem.addIF("RabbitChase", this);

  templateSystem.addKey("FlagCount", this);
  templateSystem.addIF("Flags", this);
  templateSystem.addLoop("Flags", this);
  templateSystem.addKey("FlagName", this);
  templateSystem.addKey("FlagPlayer", this);
  templateSystem.addKey("FlagPlayerID", this);
  templateSystem.addKey("FlagX", this);
  templateSystem.addKey("FlagY", this);
  templateSystem.addKey("FlagZ", this);

  // server info
  templateSystem.addKey("Uptime",this);
  templateSystem.addKey("TotalPlayers",this);
  templateSystem.addKey("Joins",this);
  templateSystem.addKey("Spawns",this);
  templateSystem.addKey("Deaths",this);
  templateSystem.addKey("UniqueReged",this);
  templateSystem.addKey("UniqueUnReged",this);
  templateSystem.addKey("DataInB",this);
  templateSystem.addKey("DataOutB",this);
  templateSystem.addKey("DataInKB",this);
  templateSystem.addKey("DataOutKB",this);
  templateSystem.addKey("DataInMB",this);
  templateSystem.addKey("DataOutMB",this);
  templateSystem.addKey("DataInGB",this);
  templateSystem.addKey("DataOutGB",this);
  templateSystem.addKey("TotalTransferB",this);
  templateSystem.addKey("TotalTransferKB",this);
  templateSystem.addKey("TotalTransferMB",this);
  templateSystem.addKey("TotalTransferGB",this);
  templateSystem.addKey("DataInAvgKBS",this);
  templateSystem.addKey("DataOutAvgKBS",this);

  defaultMainTemplate = "<html><head></head><body><h2>Players</h2>";
  defaultMainTemplate += "[*START Players][$Callsign]<br>[*END Players]None[*EMPTY Players]<hr></body></html>";

  defaultPlayerTemplate = "<html><head></head><body><h2>[$Callsign]</h2><b>[$TeamName]</b> [$Wins]/[$Losses]([$TeamKills]) [$Status]</body></html>";
  defaultFlagTemplate = "<html><head></head><body><h2>[$FlagType]</h2><b>Player [$FlagPlayer]</b> [$FlagX],[$FlagY],[$FlagZ]</body></html>";
}

void getStatus(bz_BasePlayerRecord* rec, std::string &data)
{
  if (rec->team != eObservers) {
    if ( rec->admin )
      data += "Admin";

    if (rec->spawned) {
      if (rec->admin)
	data += "/";
      data += "Spawned";
    }

    if (rec->verified) {
      if (rec->admin || rec->spawned)
	data += "/";

      data += "Verified";
    }
  }

  if (!data.size())
    data = "&nbsp;";
}

void WebStats::keyCallback(std::string &data, const std::string &key)
{
  bz_BasePlayerRecord *rec = NULL;
  if (playeRecord)
    rec = playeRecord;
  else if (teamSortItr != teamSort.end() && playerInTeam < teamSortItr->second.size())
    rec = teamSortItr->second[playerInTeam];

  int flagID = flagReportID;
  if (rec)
    flagID = rec->currentFlagID;

  data = "";

  double uptime = bz_getCurrentTime()-stats.serverStartTime;

  if (key == "playercount") {
    data = format("%d", bz_getPlayerCount());
  } else if (key == "gametype") {
    switch(bz_getGameType()) {
    case TeamFFAGame:
      data = "Team FFA";
      break;

    case ClassicCTFGame:
      data = "Team Capture the Flag";
      break;

    case eRabbitGame:
      data = "Rabbit Hunt";
      break;

    case OpenFFAGame:
      data = "Open FFA";
      break;

    default:
      data = "other";
      break;
    }
  }
  else if (key == "uptime")
  {
    int days = uptime/86400.0;
    uptime -= days*86400.0;
    int hours = uptime/3600.0;
    uptime -= hours*3600.0;
    int min = uptime/60.0;
    uptime -= min*60.0;
    int seconds = uptime;
    data = format("%d days %d hours %d minutes %d seconds",days,hours,min,seconds);
  }
  else if ( key == "totalplayers" )
  {
    size_t count = stats.registeredPlayerMap.size() + stats.nonRegedPlayerMap.size();
    data = format("%d",count);
  }
  else if ( key == "joins")
    data = format("%d",stats.joins);
  else if ( key == "spawns")
    data = format("%d",stats.spawns);
  else if ( key == "deaths")
    data = format("%d",stats.deaths);
  else if ( key == "uniquereged")
    data = format("%d",stats.registeredPlayerMap.size());
  else if ( key == "uniqueunreged")
    data = format("%d",stats.nonRegedPlayerMap.size());
  else if ( key == "datainb")
    data = format("%d",stats.dataIn);
  else if ( key == "dataoutb")
    data = format("%d",stats.dataOut);
  else if ( key == "datainkb")
    data = format("%f",stats.dataIn/1024.0);
  else if ( key == "dataoutkb")
    data = format("%f",stats.dataOut/1024.0);
  else if ( key == "datainmb")
    data = format("%f",stats.dataIn/1024.0/1024.0);
  else if ( key == "dataoutmb")
    data = format("%f",stats.dataOut/1024.0/1024.0);
  else if ( key == "dataingb")
    data = format("%f",stats.dataIn/1024.0/1024.0/1024.0);
  else if ( key == "dataoutgb")
    data = format("%f",stats.dataOut/1024.0/1024.0/1024.0);
  else if ( key == "totaltransferb")
    data = format("%f",(stats.dataOut+stats.dataIn));
  else if ( key == "totaltransferkb")
    data = format("%f",(stats.dataOut+stats.dataIn)/1024.0);
  else if ( key == "totaltransfermb")
    data = format("%f",(stats.dataOut+stats.dataIn)/1024.0/1024.0);
  else if ( key == "totaltransfergb")
    data = format("%f",(stats.dataOut+stats.dataIn)/1024.0/1024.0/1024.0);
  else if ( key == "datainavgkb")
    data = format("%f",(stats.dataIn/uptime)/1024.0);
  else if ( key == "dataoputavgkb")
    data = format("%f",(stats.dataOut/uptime)/1024.0);
  else if (key == "teamname") {
    if (rec)
      data = bzu_GetTeamName(rec->team);
  } else if (key == "callsign") {
    if (rec)
      data = rec->callsign.c_str();
  } else if (key == "wins") {
    if (rec)
      data = format("%d", rec->wins);
  } else if (key == "losses") {
    if (rec)
      data = format("%d", rec->losses);
  } else if (key == "teamkills") {
    if (rec)
      data = format("%d", rec->teamKills);
  } else if (key == "status") {
    if (rec)
      getStatus(rec, data);
  } else if (key == "playerid") {
    if (rec)
      data = format("%d", rec->playerID);
    else
      data = "-1";
  } else if (key == "playerflag") {
    if (rec)
      data = rec->currentFlag.c_str();
  } else if (key == "bzid") {
    if (rec)
      data = rec->bzID.c_str();
  } else if (key == "ipaddress") {
    if (rec)
      data = rec->ipAddress.c_str();
  } else if (key == "lag") {
    if (rec)
      data = format("%d", rec->lag);
  } else if (key == "jitter") {
    if (rec)
      data = format("%d", rec->jitter);
  } else if (key == "packetloss") {
    if (rec)
      data = format("%f", rec->packetloss);
  } else if (key == "groupcount") {
    if (rec)
      data = format("%d", rec->groups.size());
  } else if (key == "groupname") {
    if (rec && groupLoop < rec->groups.size())
      data = rec->groups[groupLoop].c_str();
  } else if (key == "flaghistorycount") {
    if (rec)
      data = format("%d", rec->flagHistory.size());
  } else if (key == "flaghistoryflag") {
    if (rec && flagHistoryLoop < rec->flagHistory.size())
      data = rec->flagHistory[flagHistoryLoop].c_str();
  } else if (key == "teamcount") {
    if (rec)
      data = format("%d", bz_getTeamCount(rec->team));
  } else if (key == "teamscore") {
    if (rec)
      data = format("%d", bz_getTeamScore(rec->team));
  } else if (key == "teamwins") {
    if (rec)
      data = format("%d", bz_getTeamWins(rec->team));
  } else if (key == "teamlosses") {
    if (rec)
      data = format("%d", bz_getTeamLosses(rec->team));
  } else if (key == "redteamcount") {
    data = format("%d", bz_getTeamCount(eRedTeam));
  } else if (key == "blueteamcount") {
    data = format("%d", bz_getTeamCount(eBlueTeam));
  } else if (key == "greenteamcount") {
    data = format("%d", bz_getTeamCount(eGreenTeam));
  } else if (key == "rpurpleteamcount") {
    data = format("%d", bz_getTeamCount(ePurpleTeam));
  } else if (key == "rogueteamcount") {
    data = format("%d", bz_getTeamCount(eRogueTeam));
  } else if (key == "observerteamcount") {
    data = format("%d", bz_getTeamCount(eObservers));
  } else if (key == "hunterteamcount") {
    data = format("%d", bz_getTeamCount(eHunterTeam));
  } else if (key == "rabbitteamcount") {
    data = format("%d", bz_getTeamCount(eRabbitTeam));
  } else if (key == "redteamscore") {
    data = format("%d", bz_getTeamScore(eRedTeam));
  } else if (key == "redteamwins") {
    data = format("%d", bz_getTeamWins(eRedTeam));
  } else if (key == "redteamlosses") {
    data = format("%d", bz_getTeamLosses(eRedTeam));
  } else if (key == "blueteamscore") {
    data = format("%d", bz_getTeamScore(eBlueTeam));
  } else if (key == "blueteamwins") {
    data = format("%d", bz_getTeamWins(eBlueTeam));
  } else if (key == "blueteamlosses") {
    data = format("%d", bz_getTeamLosses(eBlueTeam));
  } else if (key == "greenteamscore") {
    data = format("%d", bz_getTeamScore(eGreenTeam));
  } else if (key == "greenteamwins") {
    data = format("%d", bz_getTeamWins(eGreenTeam));
  } else if (key == "greenteamlosses") {
    data = format("%d", bz_getTeamLosses(eGreenTeam));
  } else if (key == "purpleteamscore") {
    data = format("%d", bz_getTeamScore(ePurpleTeam));
  } else if (key == "purpleteamwins") {
    data = format("%d", bz_getTeamWins(ePurpleTeam));
  } else if (key == "purpleteamlosses") {
    data = format("%d", bz_getTeamLosses(ePurpleTeam));
  } else if (key == "flagcount") {
    data = format("%d", bz_getNumFlags());
  } else if (flagID >= 0) {
    if ( key == "flagname") {
      data = bz_getFlagName(flagID).c_str();
    } else if ( key == "flagplayer") {
      data = bz_getPlayerCallsign(bz_flagPlayer(flagID));
    } else if (key == "flagplayerid") {
      data = format("%d", bz_flagPlayer(flagID));
    } else if (key == "flagx" || key =="flagy" || key == "flagz") {
      float pos[3] = {0, 0, 0};
      if(bz_getFlagPosition ( flagID, pos ))
	data = format("%.3f", pos[key[4] - 'x']);
    }
  }
}

bool WebStats::loopCallback(const std::string &key)
{
  bz_BasePlayerRecord *rec = NULL;
  if (playeRecord)
    rec = playeRecord;
  else if (teamSortItr != teamSort.end() && playerInTeam < teamSortItr->second.size())
    rec = teamSortItr->second[playerInTeam];

  if (key == "players")
  {
    if (playeRecord || !teamSort.size())
      return false;

    if ( teamSortItr == teamSort.end())
    {
      teamSortItr = teamSort.begin();
      playerInTeam = 0;
    }
    else
    {
      playerInTeam++;
      if ( playerInTeam >= teamSortItr->second.size())
      {
	teamSortItr++;
	playerInTeam = 0;
      }
    }
    return teamSortItr != teamSort.end();
  }
  else if ( key == "groups" && rec) {
    if (!groupLoop)
      groupLoop++;
    else
      groupLoop = 0;

    if (groupLoop >= rec->groups.size()) {
      groupLoop = 0;
      return false;
    }
    return true;
  } else if ( key == "flaghistory" && rec) {
    if (!flagHistoryLoop)
      flagHistoryLoop++;
    else
      flagHistoryLoop = 0;

    if (flagHistoryLoop >= rec->flagHistory.size()) {
      flagHistoryLoop = 0;
      return false;
    }
    return true;
  } else if ( key == "flaghistory" && rec) {
    if (flagReportID != -1)
      flagReportID++;
    else
      flagReportID = 0;

    if (flagReportID >= (int)bz_getNumFlags()) {
      flagReportID = -1;
      return false;
    }
    return true;
  }
  return false;
}

bool WebStats::ifCallback(const std::string &key)
{
  bz_BasePlayerRecord *rec = NULL;
  if (playeRecord)
    rec = playeRecord;
  else if (teamSortItr != teamSort.end() && playerInTeam < teamSortItr->second.size())
    rec = teamSortItr->second[playerInTeam];

  if (key == "newteam") {
    return teamSortItr != teamSort.end() && playerInTeam == 0;
  } else if (key == "players") {
    return playeRecord != NULL ? playeRecord!= NULL : teamSort.size() > 0;
  } else if (key == "redteam") {
    return bz_getTeamCount(eRedTeam) > 0;
  } else if (key == "greenteam") {
    return bz_getTeamCount(eGreenTeam) > 0;
  } else if (key == "blueteam") {
    return bz_getTeamCount(eBlueTeam) > 0;
  } else if (key == "purpleteam") {
    return bz_getTeamCount(ePurpleTeam) > 0;
  } else if (key == "observerteam") {
    return bz_getTeamCount(eObservers) > 0;
  } else if (key == "rogueteam") {
    return bz_getTeamCount(eRogueTeam) > 0;
  } else if (key == "hunterteam") {
    return bz_getTeamCount(eHunterTeam) > 0;
  } else if (key == "rabbitteam") {
    return bz_getTeamCount(eRabbitTeam) > 0;
  } else if (key == "teamffa") {
    return bz_getGameType() == TeamFFAGame;
  } else if (key == "openffa") {
    return bz_getGameType() == OpenFFAGame;
  } else if (key == "ctf") {
    return bz_getGameType() == ClassicCTFGame;
  } else if (key == "rabbitchase") {
    return bz_getGameType() == eRabbitGame;
  } else if (key == "flags") {
    return bz_getNumFlags() > 0;
  } else if (rec) {
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

void WebStats::initReport(void)
{
  bz_APIIntList *players = bz_getPlayerIndexList();

  for (int i = 0; i < (int)players->size(); i++) {
    int player = players->get(i);
    bz_BasePlayerRecord *rec = bz_getPlayerByIndex (player);

    if (rec) {
      if (teamSort.find(rec->team) == teamSort.end()) {
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
  while (teamSortItr != teamSort.end()) {
    for ( size_t i = 0; i < teamSortItr->second.size(); i++)
      bz_freePlayerRecord(teamSortItr->second[i]);

    teamSortItr++;
  }
  teamSort.clear();
}

void WebStats::doStatReport(std::string &page, std::string &action)
{
  playeRecord = NULL;
  initReport();

  if(action.size())
    action += ".tmpl";
  if (!action.size() || !templateSystem.processTemplateFile(page, action.c_str())) {
    if (!templateSystem.processTemplateFile(page, "stats.tmpl"))
      templateSystem.processTemplate(page, defaultMainTemplate);
  }
  finishReport();
}

void WebStats::doPlayerReport(std::string &page, int playerID)
{
  playeRecord = bz_getPlayerByIndex(playerID);
  if (!playeRecord) {
    page += "Invalid Player";
  } else {
    if (!templateSystem.processTemplateFile(page, "player.tmpl"))
      templateSystem.processTemplate(page, defaultPlayerTemplate);
      bz_freePlayerRecord(playeRecord);
  }
  playeRecord = NULL;
}

void WebStats::doFlagReport(std::string &page, int flagID)
{
  flagReportID = flagID;
  if (flagReportID < 0 || flagReportID > (int)bz_getNumFlags()) {
    page += "Invalid Flag";
  } else {
    if (!templateSystem.processTemplateFile(page, "flag.tmpl"))
      templateSystem.processTemplate(page, defaultFlagTemplate);
  }
  flagReportID = -1;
}

void WebStats::getURLData(const char* url, int requestID, const URLParams &paramaters, bool get)
{
  bool evenLine = false;
  groupLoop = 0;
  flagHistoryLoop = 0;
  playeRecord = NULL;
  flagReportID = -1;

  std::string page;
  templateSystem.startTimer();

  std::string action = getParam(paramaters, "action");
  std::string playerID = getParam(paramaters, "playerid");
  std::string flagID = getParam(paramaters, "flagid");

  if ( action == "player" && playerID.size())
    doPlayerReport(page, atoi(playerID.c_str()));
  if ( action == "flag" && flagID.size())
    doFlagReport(page, atoi(flagID.c_str()));
  else
    doStatReport(page, action);

  setURLDocType(eHTML, requestID);
  setURLDataSize ( (unsigned int)page.size(), requestID );
  setURLData ( page.c_str(), requestID );
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
