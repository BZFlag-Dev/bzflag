// thiefControl.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

class ThiefControl : public bz_EventHandler
{
public:
  ThiefControl() {} ;
  virtual ~ThiefControl() {};
  virtual void process( bz_EventData *eventData );
};

ThiefControl thiefHandler;

BZF_PLUGIN_CALL int bz_Load ( const char* /*commandLine*/ )
{
  bz_debugMessage(4,"thiefControl plugin loaded");
  bz_registerEvent(bz_eFlagTransferredEvent, &thiefHandler);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent(bz_eFlagTransferredEvent, &thiefHandler);
  bz_debugMessage(4,"thiefControl plugin unloaded");
  return 0;
}

void ThiefControl::process( bz_EventData *eventData )
{
  bz_FlagTransferredEventData_V1 *data = (bz_FlagTransferredEventData_V1 *) eventData;
  bz_BasePlayerRecord *playerFrom = bz_getPlayerByIndex(data->fromPlayerID);
  bz_BasePlayerRecord *playerTo = bz_getPlayerByIndex(data->toPlayerID);

  if (eventData) {
    switch (eventData->eventType) {
    case bz_eFlagTransferredEvent:

      if (playerFrom && playerTo) {
	if ((playerTo->team != eRogueTeam || bz_getGameType() == eRabbitGame) && playerFrom->team == playerTo->team) {
	  data->action = data->DropThief;
	  bz_sendTextMessage(BZ_SERVER, data->toPlayerID, "Flag dropped. Don't steal from teammates!");
	}
      }
      break;
    default:
      break;
    }
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
