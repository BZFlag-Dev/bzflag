// serverSideBotSample.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION


class SimpleBotHandler : public bz_ServerSidePlayerHandler
{
public:
	virtual void removed ( void );
	virtual void playerRemoved ( int playerID );

	virtual void flagUpdate ( int count, bz_FlagUpdateRecord **flagList );
	virtual void playerUpdate ( bz_PlayerUpdateRecord *playerRecord );
	virtual void teamUpdate ( int count, bz_TeamInfoRecord **teamList );
};

SimpleBotHandler	bot;
int					botPlayerID;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"serverSideBotSample plugin loaded");
  bz_debugMessage(2,"adding one simple bot");
  botPlayerID = bz_addServerSidePlayer(&bot);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeServerSidePlayer ( botPlayerID, &bot );
  bz_debugMessage(2,"removing one simple bot");
  bz_debugMessage(4,"serverSideBotSample plugin unloaded");
  return 0;
}

void SimpleBotHandler::removed ( void )
{

}

void SimpleBotHandler::playerRemoved ( int playerID )
{

}


void SimpleBotHandler::flagUpdate ( int count, bz_FlagUpdateRecord **flagList )
{

}

void SimpleBotHandler::playerUpdate ( bz_PlayerUpdateRecord *playerRecord )
{

}

void SimpleBotHandler::teamUpdate ( int count, bz_TeamInfoRecord **teamList )
{

}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

