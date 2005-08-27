// killall.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION


class KillAll : public bz_CustomSlashCommandHandler
{
public:
	virtual bool handle ( int playerID, bzApiString command, bzApiString message, bzAPIStringList *params )
	{
		bz_PlayerRecord *player = bz_getPlayerByIndex(playerID);
		if (!player)
			return true;

		if ( !player->admin )
		{
			bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permission to run /killall");
			return true;
		}

		std::string msg = player->callsign.c_str();
		msg += " has killed everyone";

		bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,msg.c_str());

		bzAPIIntList *playerList = bz_newIntList();

		bz_getPlayerIndexList ( playerList );

		for ( unsigned int i = 0; i < playerList->size(); i++ )
			bz_killPlayer(playerList->get(i),false);

		return true;
	}
};

KillAll	killall;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"killall plugin loaded");
	bz_registerCustomSlashCommand ( "killall", &killall );
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeCustomSlashCommand ( "killall" );
	bz_debugMessage(4,"killall plugin unloaded");
	return 0;
}

