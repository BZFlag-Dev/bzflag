// webstats.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"

class WebStats : public BZFSHTTPServer
{
public:
  WebStats( const char * plugInName ): BZFSHTTPServer(plugInName){};

  virtual bool acceptURL ( const char *url ){return true;}
  virtual void getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get = true );
};

WebStats webStats("webstats");

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"webstats plugin loaded");
  webStats.startupHTTP();

  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  webStats.shutdownHTTP();
  bz_debugMessage(4,"webstats plugin unloaded");
  return 0;
}

std::string teamToString ( bz_eTeamType team )
{
  std::string name = "Unknown Team";
  switch(team)
  {
    case eRogueTeam:
      name = "Rogue";
      break;
    case eRedTeam:
      name = "Red";
      break;
    case eGreenTeam:
      name = "Green";
      break;
    case eBlueTeam:
      name = "Blue";
      break;
    case ePurpleTeam:
      name = "Purple";
      break;
    case eRabbitTeam:
      name = "Rabbit";
      break;
    case eHunterTeam:
      name = "Hunter";
      break;
  }

  return name;
}

void WebStats::getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get )
{
  std::string page;
  page = "<HTML><HEAD></HEAD><BODY>\n<BR>\n";

  std::string publicAddr = bz_getPublicAddr().c_str();
  page = "Stats for";
  page += publicAddr + "<br>\n";

  bz_APIIntList *players = bz_getPlayerIndexList();

  if ( !players->size() )
    page += "There are no players :(\n";

  for ( int i = 0; i < (int)players->size(); i++ )
  {
    int player = players->get(i);

    bz_BasePlayerRecord *rec = bz_getPlayerByIndex (player);
    if (rec)
    {
      page += rec->callsign.c_str();
      page += " ";
      if ( rec->team != eObservers )
      {
	page += teamToString(rec->team);
	page += format(" Wins=%d Losses=%d TKs=%d",rec->wins,rec->losses,rec->teamKills);
	if ( rec->admin )
	  page += " Admin";

	if ( rec->spawned )
	   page += " Spawned";

	if ( rec->verified )
	  page += " Verified";
      }
      else
	page += "Observer";

      page += "<br>\n";
      bz_freePlayerRecord (rec);
    }
  }
  bz_deleteIntList(players);

  page += "<BR></body></HTML>";

  setURLDocType(eHTML,requestID);
  setURLDataSize ( (unsigned int)page.size(), requestID );
  setURLData ( page.c_str(), requestID );

}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
