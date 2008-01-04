// soundTest.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

class SoundCommand : public bz_CustomSlashCommandHandler
{
public:
	virtual ~SoundCommand(){};
	virtual bool handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params );
};

SoundCommand soundCommand;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"soundTest plugin loaded");
  bz_registerCustomSlashCommand("sound",&soundCommand);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_debugMessage(4,"soundTest plugin unloaded");
  bz_removeCustomSlashCommand("sound");
  return 0;
}

bool SoundCommand::handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params )
{
	if (!bz_hasPerm(playerID,bz_perm_setAll))
	{
		bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permission to play sounds (SETALL)");
		return true;
	}

	if (message.size() <= 0)
	{
		bz_sendTextMessage(BZ_SERVER, playerID, "Missing sound file name");
		return true;
	}

	bz_sendPlayCustomLocalSound(playerID,message.c_str());
	return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
