// webstats.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "plugin_HTTPTemplates.h"


class WebStats : public BZFSHTTPServer, TemplateCallbackClass
{
public:
  WebStats( const char * plugInName ): BZFSHTTPServer(plugInName){};

  void init ( const char *commandLine );

  virtual bool acceptURL ( const char *url ){return true;}
  virtual void getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get = true );

  Templateiser	templateSystem;

  virtual void keyCallback ( std::string &data, const std::string &key );
  virtual bool loopCallback ( const std::string &key );
  virtual bool ifCallback ( const std::string &key );

  void initReport ( void );
  void finishReport ( void );

  void doStatReport ( std::string &page );
  void doPlayerReport ( std::string &page, int playerID );

  // globals for report
  int player;
  std::map<bz_eTeamType,std::vector<bz_BasePlayerRecord*> >  teamSort;
  std::map<bz_eTeamType,std::vector<bz_BasePlayerRecord*> >::iterator teamSortItr;
  size_t playerInTeam;

  bz_BasePlayerRecord *playeRecord;

  // default template
  std::string defaultMainTemplate;
  std::string defaultPlayerTemplate;
};

WebStats webStats("webstats");

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
  bz_debugMessage(4,"webstats plugin loaded");
  
  webStats.init(commandLine);
  webStats.startupHTTP();
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  webStats.shutdownHTTP();
  bz_debugMessage(4,"webstats plugin unloaded");
  return 0;
}

void WebStats::init ( const char *commandLine )
{
  templateSystem.addSearchPath("./");
  templateSystem.addSearchPath(commandLine);

  templateSystem.setPluginName("WebStats",getBaseServerURL());

  templateSystem.addKey("PlayerCount",this);
  templateSystem.addLoop("Players",this);
  templateSystem.addIF("NewTeam",this);
  templateSystem.addIF("Players",this);
  templateSystem.addKey("TeamName",this);
  templateSystem.addKey("Callsign",this);
  templateSystem.addKey("Wins",this);
  templateSystem.addKey("Losses",this);
  templateSystem.addKey("TeamKills",this);
  templateSystem.addKey("Status",this);
  templateSystem.addKey("PlayerID",this);
  templateSystem.addKey("PlayerFlag",this);

  templateSystem.addIF("Spawned",this);
  templateSystem.addIF("Verified",this);
  templateSystem.addIF("Global",this);
  templateSystem.addIF("Admin",this);
  templateSystem.addIF("Op",this);
  templateSystem.addIF("CanSpawn",this);

  templateSystem.addKey("BZID",this);

  defaultMainTemplate = "<html><head></head><body><h2>Players</h2>";
  defaultMainTemplate += "[*START Players][$Callsign]<br>[*END Players]None[*EMPTY Players]<hr></body></html>";
  
  defaultPlayerTemplate = "<html><head></head><body><h2>[$Callsign]</h2><b>[$TeamName]</b> [$Wins]/[$Losses]([$TeamKills]) [$Status]</body></html>";

}

void getStatus ( bz_BasePlayerRecord* rec, std::string &data )
{
  if ( rec->team != eObservers )
  {
    if ( rec->admin )
      data += "Admin";

    if ( rec->spawned )
    {
      if (rec->admin)
	data += "/";
      data += "Spawned";
    }

    if ( rec->verified )
    {
      if (rec->admin || rec->spawned ) 
	data += "/";

      data += "Verified";
    }
  }

  if (!data.size())
    data = "&nbsp;";
}

void WebStats::keyCallback ( std::string &data, const std::string &key )
{
  bz_BasePlayerRecord *rec = NULL;
  if ( !playeRecord && teamSortItr != teamSort.end())
    rec = teamSortItr->second[playerInTeam];
  else
    rec = playeRecord;

  data = "";

  if (key == "playercount")
    data = format("%d",bz_getPlayerCount());
  else if (key == "teamname")
  {
    if (rec)
      data = bzu_GetTeamName(rec->team);
  }
  else if (key == "callsign")
  {
    if (rec)
      data = rec->callsign.c_str();
  }
  else if (key == "wins")
  {
    if (rec)
      data = format("%d",rec->wins);
  }
  else if (key == "losses")
  {
    if (rec)
      data = format("%d",rec->losses);
  }
  else if (key == "teamkills")
  {
    if (rec)
      data = format("%d",rec->teamKills);
  }
  else if (key == "status")
  {
    if (rec)
      getStatus(rec,data);
  }
  else if (key == "playerid")
  {
    if (rec)
      data = format("%d",rec->playerID);
    else
      data = "-1";
  }
  else if (key == "playerflag")
  {
    if (rec)
      data = rec->currentFlag.c_str();
  }
  else if (key == "bzid")
  {
    if (rec)
      data = rec->bzID.c_str();
  }
}

bool WebStats::loopCallback ( const std::string &key )
{
  if (key == "players")
  {
    if (playeRecord || !teamSort.size())
      return false;

    if ( teamSortItr == teamSort.end())
    {
      teamSortItr = teamSort.begin();
      playerInTeam;
    }
    else
    {
      playerInTeam++;
      if ( playerInTeam > teamSortItr->second.size())
      {
	teamSortItr++;
	playerInTeam = 0;
      }
    }
    return teamSortItr != teamSort.end();
  }
  return false;
}

bool WebStats::ifCallback ( const std::string &key )
{
  bz_BasePlayerRecord *rec = NULL;
  if ( !playeRecord && teamSortItr != teamSort.end())
    rec = teamSortItr->second[playerInTeam];
  else
    rec = playeRecord;

  if (key == "newteam")
    return teamSortItr != teamSort.end() && playerInTeam == 0;
  else if (key == "players")
    return playeRecord != NULL ? playeRecord!= NULL : teamSort.size() > 0;
  else if (rec)
  {
    if (key == "spawned")
      return playeRecord->spawned;
    else if (key == "verified")
      return playeRecord->verified;
    else if (key == "global")
      return playeRecord->globalUser;
    else if (key == "admin")
      return playeRecord->admin;
    else if (key == "op")
      return playeRecord->op;
    else if (key == "canspawn")
      return playeRecord->canSpawn;
  }

  return false;
}

void WebStats::initReport ( void )
{
  templateSystem.startTimer();

  bz_APIIntList *players = bz_getPlayerIndexList();

  for ( int i = 0; i < (int)players->size(); i++ )
  {
    int player = players->get(i);
    bz_BasePlayerRecord *rec = bz_getPlayerByIndex (player);

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

void WebStats::finishReport ( void )
{
  teamSortItr = teamSort.begin();
  while (teamSortItr != teamSort.end())
  {
    for ( size_t i = 0; i < teamSortItr->second.size(); i++)
      bz_freePlayerRecord(teamSortItr->second[i]);

    teamSortItr++;
  }
  teamSort.clear();
}

void WebStats::doStatReport ( std::string &page )
{
  playeRecord = NULL;
  initReport();
  if (!templateSystem.processTemplateFile(page,"stats.tmpl"))
    templateSystem.processTemplate(page,defaultMainTemplate);
  finishReport();
}

void WebStats::doPlayerReport ( std::string &page, int playerID )
{ 
  templateSystem.startTimer();
  playeRecord = bz_getPlayerByIndex(playerID);
  if (!playeRecord)
    page += "Invalid Player";
  else
  {
    if (!templateSystem.processTemplateFile(page,"player.tmpl"))
      templateSystem.processTemplate(page,defaultPlayerTemplate);
    bz_freePlayerRecord(playeRecord);
    playeRecord = NULL;
  }
} 


void WebStats::getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get )
{
  bool evenLine = false;
  playeRecord = NULL;

  std::string page;

  std::string action = getParam(paramaters,"action");
  std::string playerID = getParam(paramaters,"playerid");
  if ( action == "player" && playerID.size())
    doPlayerReport(page,atoi(playerID.c_str()));
  else
   doStatReport(page);

  setURLDocType(eHTML,requestID);
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
