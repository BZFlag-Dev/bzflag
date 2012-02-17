// regFlag.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

class RegFlag : public bz_Plugin
{
public:
	virtual const char* Name (){return "RegFlag";}
	virtual void Init ( const char* config);

	virtual void Event ( bz_EventData *eventData );
};

BZ_PLUGIN(RegFlag)

void RegFlag::Init( const char* /*commandLine*/ )
{
  Register(bz_ePlayerUpdateEvent);
}


void RegFlag::Event ( bz_EventData *eventData )
{
  bz_BasePlayerRecord *player = NULL;
  int playerID = -1;

  switch (eventData->eventType)
    {
    case bz_ePlayerUpdateEvent:
      playerID = ((bz_PlayerUpdateEventData_V1*)eventData)->playerID;
      break;

    case bz_eShotFiredEvent:
      playerID = ((bz_PlayerUpdateEventData_V1*)eventData)->playerID;
      break;

    default:
      return;
    }

  player = bz_getPlayerByIndex(playerID);
  if (!player) return;
  if (player->globalUser) {
    bz_freePlayerRecord(player);
    return;
  }
  bz_freePlayerRecord(player);

  const char* flagAbrev = bz_getPlayerFlag(playerID);
  if (!flagAbrev)
    return;

  bz_removePlayerFlag(playerID);
  bz_sendTextMessage(BZ_SERVER,playerID, "Flags are for registered players only");

}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

