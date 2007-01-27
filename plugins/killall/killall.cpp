// killall.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION


class KillAll : public bz_CustomSlashCommandHandler
{
public:
  virtual bool handle ( int playerID, bz_ApiString /*command*/, bz_ApiString /*message*/, bz_APIStringList* /*params*/ )
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

KillAll killall;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
