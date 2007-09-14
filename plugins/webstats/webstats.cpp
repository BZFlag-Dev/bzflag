// webstats.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_HTTP.h"
#include "statTemplates.h"

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

void WebStats::getURLData ( const char* url, int requestID, const URLParams &paramaters, bool get )
{
  bool evenLine = false;

  std::string page;
  page = getFileHeader();

  bz_APIIntList *players = bz_getPlayerIndexList();

  std::map<bz_eTeamType,std::vector<bz_BasePlayerRecord*> >  teamSort;

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

  // generate the list of players
  std::map<bz_eTeamType,std::vector<bz_BasePlayerRecord*> >::iterator itr = teamSort.begin();

  page += getPlayersHeader();

  if ( !players->size() )
    page += getPlayersNoPlayers();

  while (itr != teamSort.end())
  {
    for ( int i = 0; i < (int)itr->second.size(); i++)
    {
      bz_BasePlayerRecord *rec = itr->second[i];
      int player = rec->playerID;
      if (rec)
      {
	if (rec->callsign.size() == 0) continue;
	page += getPlayersLineItem ( rec, evenLine );
	evenLine = !evenLine;

	bz_freePlayerRecord (rec);
      }
    }
    itr++;
  }
  bz_deleteIntList(players);

  page += getPlayersFooter();
  // end player list

  // TODO, do the team scores, flag stats, do the last chat lines, etc..

  // finish the document
  page += getFileFooter();

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
