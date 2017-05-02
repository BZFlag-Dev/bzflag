// killall.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

class KillAll : public bz_Plugin, bz_CustomSlashCommandHandler
{
public:
	virtual const char* Name() {return "Kill All";}

	virtual void Init ( const char* /* config */ )
	{
		bz_registerCustomSlashCommand ( "killall", this );
	}

	virtual void Cleanup ( void )
	{
		bz_removeCustomSlashCommand ( "killall" );
	}

  virtual bool SlashCommand ( int playerID, bz_ApiString /*command*/, bz_ApiString /*message*/, bz_APIStringList* /*params*/ )
  {
    bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);
    if (!player)
      return true;

    if ( !player->admin )
    {
      bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permission to run /killall");
      bz_freePlayerRecord(player);
      return true;
    }

    std::string msg = player->callsign.c_str();
    msg += " has killed everyone";

    bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,msg.c_str());

    bz_APIIntList *playerList = bz_newIntList();

    bz_getPlayerIndexList ( playerList );

    for ( unsigned int i = 0; i < playerList->size(); i++ )
      bz_killPlayer(playerList->get(i),false);

    bz_freePlayerRecord(player);
    bz_deleteIntList(playerList);

    return true;
  }
};

BZ_PLUGIN(KillAll)



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
