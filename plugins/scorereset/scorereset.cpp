// chathistory.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "TextUtils.h"
#include <string>
#include <map>
#include <vector>

BZ_GET_PLUGIN_VERSION

class ScoreResetCommand : public bz_CustomSlashCommandHandler
{
public:
	virtual ~ScoreResetCommand(){};
	virtual bool handle ( int playerID, bzApiString command, bzApiString message, bzAPIStringList *param );
};

ScoreResetCommand	scoreResetCommand;


BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"ScoreReset plugin loaded");


	bz_registerCustomSlashCommand("scorereset",&scoreResetCommand);

	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeCustomSlashCommand("scorereset");
	bz_debugMessage(4,"ScoreReset plugin unloaded");
	return 0;
}


bool ScoreResetCommand::handle ( int playerID, bzApiString _command, bzApiString _message, bzAPIStringList *_param )
{
	std::string command = _command.c_str();
	std::string message = _message.c_str();


	bz_PlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);

	if ( !fromPlayer->admin )
	{
		bz_sendTextMessage(BZ_SERVER,playerID,"You must be admin to use the ScoreReset plugin");
		return true;
	}

	if ( command == "scorereset")
	{

		bz_PlayerRecord *dude = bz_getPlayerByIndex ( playerID );
		if (dude)
			bz_sendTextMessage(BZ_SERVER,BZ_ALL_USERS,TextUtils::format("%s has reset all scores",dude->callsign.c_str()).c_str());
		
		bzAPIIntList	*playerList = bz_newIntList();

		bz_getPlayerIndexList(playerList);

		bz_resetTeamScore(-1);

		for( int i = 0; i < (int)playerList->size(); i ++)
			bz_resetPlayerScore(i);

		return true;
	}
	return false;
}

