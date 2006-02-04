// serverSideBotSample.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION


class SimpleBotHandler : public bz_ServerSidePlayerHandler
{
public:
	virtual void added ( int playerID );
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
  bz_debugMessage(0,"******* WARNING. THE SERVER SIDE BOT PLUGIN IS UNSTABLE******");
  bz_debugMessage(0,"******* IT CAN AND WILL CRASH YOUR SYSTEM ******");
  bz_debugMessage(0,"******* THE CODE IS UNDER DEVELOPMENT ******");
  bz_debugMessage(0,"******* DO NOT USE IT UNLESS YOU ARE SURE YOU KNOW WHAT YOU ARE DOING ******");
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

void SimpleBotHandler::added ( int playerID )
{
	bz_debugMessage(3,"SimpleBotHandler::added");
	setEntryData("someBot","bot@bzflag.org",NULL,"bot sample");
}

void SimpleBotHandler::removed ( void )
{
	bz_debugMessage(3,"SimpleBotHandler::removed");
}

void SimpleBotHandler::playerRemoved ( int playerID )
{
	bz_debugMessage(3,"SimpleBotHandler::playerRemoved");
}

void SimpleBotHandler::flagUpdate ( int count, bz_FlagUpdateRecord **flagList )
{
	bz_debugMessage(3,"SimpleBotHandler::flagUpdate");
}

void SimpleBotHandler::playerUpdate ( bz_PlayerUpdateRecord *playerRecord )
{
	bz_debugMessage(3,"SimpleBotHandler::playerUpdate");
}

void SimpleBotHandler::teamUpdate ( int count, bz_TeamInfoRecord **teamList )
{
	bz_debugMessage(3,"SimpleBotHandler::teamUpdate");
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

