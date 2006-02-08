// serverSideBotSample.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include <string>

BZ_GET_PLUGIN_VERSION


class SimpleBotHandler : public bz_ServerSidePlayerHandler , bz_EventHandler
{
public:
	virtual void added ( int playerIndex );
	virtual void removed ( void );
	virtual void playerRemoved ( int playerIndex );

	virtual void flagUpdate ( int count, bz_FlagUpdateRecord **flagList );
	virtual void playerUpdate ( bz_PlayerUpdateRecord *playerRecord );
	virtual void teamUpdate ( int count, bz_TeamInfoRecord **teamList );

	virtual void process ( bz_EventData *eventData );

protected:
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
  bz_debugMessage(2,"adding one simple bot, may the gods have mercy on your soul");
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

void SimpleBotHandler::added ( int playerIndex )
{
	bz_debugMessage(3,"SimpleBotHandler::added");
	std::string name = "dante_";
	name += bz_format("%d",playerID);
	setPlayerData(name.c_str(),"dante@inferno.org",NULL,"bot sample",eAutomaticTeam);
	bz_registerEvent ( bz_ePlayerSpawnEvent, this );
	joinGame();
}

void SimpleBotHandler::removed ( void )
{
	bz_removeEvent ( bz_ePlayerSpawnEvent, this );
	bz_debugMessage(3,"SimpleBotHandler::removed");
}

void SimpleBotHandler::playerRemoved ( int playerIndex )
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

void SimpleBotHandler::process ( bz_EventData *eventData )
{
	switch(eventData->eventType)
	{
		case bz_ePlayerSpawnEvent:
			{
				bz_debugMessage(3,"SimpleBotHandler::process bz_ePlayerSpawnEvent");

				bz_PlayerSpawnEventData_V1	*spawnEvent = (bz_PlayerSpawnEventData_V1*)eventData;

				if (spawnEvent->playerID == playerID)
					break;

				bz_BasePlayerRecord * player = bz_getPlayerByIndex ( spawnEvent->playerID );
				std::string message;
				message = bz_format("well, look who droped in...%s",player->callsign.c_str());
				bz_sendTextMessage(playerID,BZ_ALLUSERS,message.c_str());
				bz_freePlayerRecord(player);
			}
			break;
		default:
			return;
	}
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

