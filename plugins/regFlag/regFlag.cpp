// regFlag.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"

BZ_GET_PLUGIN_VERSION

class RegFlag : public bz_EventHandler
{
public:
  virtual void process ( bz_EventData *eventData );
};

RegFlag RegFlagHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"regFlag plugin loaded");
  bz_registerEvent(bz_ePlayerUpdateEvent, &RegFlagHandler);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_debugMessage(4,"regFlag plugin unloaded");
  bz_removeEvent(bz_ePlayerUpdateEvent, &RegFlagHandler);
  return 0;
}

void RegFlag::process ( bz_EventData *eventData )
{
  bz_PlayerRecord *player = NULL;
  int playerID = -1;

  switch (eventData->eventType)
    {
    case bz_ePlayerUpdateEvent:
      playerID = ((bz_PlayerUpdateEventData*)eventData)->playerID;
      break;

    case bz_eShotFiredEvent:
      playerID = ((bz_ShotFiredEventData*)eventData)->playerID;
      break;

    default:
      return;
    }

  player = bz_getPlayerByIndex(playerID);
  if (!player || player->globalUser)
    return;

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

