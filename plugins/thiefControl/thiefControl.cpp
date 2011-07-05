// thiefControl.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

class ThiefControl : public bz_Plugin
{
public:
  virtual const char* Name () {return "Thief Control";}
  virtual void Init (const char* config);
  virtual void Event( bz_EventData *eventData );
};

BZ_PLUGIN(ThiefControl);

void ThiefControl::Init (const char* /*config*/ )
{
  Register(bz_eFlagTransferredEvent);
}

void ThiefControl::Event( bz_EventData *eventData )
{
  if (eventData) {
    switch (eventData->eventType) {
    case bz_eFlagTransferredEvent: {
      if (bz_getGameType() == eOpenFFAGame)
	break;

      bz_FlagTransferredEventData_V1 *data = (bz_FlagTransferredEventData_V1 *) eventData;
      bz_BasePlayerRecord *playerFrom = bz_getPlayerByIndex(data->fromPlayerID);
      bz_BasePlayerRecord *playerTo = bz_getPlayerByIndex(data->toPlayerID);

      if (playerFrom && playerTo) {
	if ((playerTo->team != eRogueTeam || bz_getGameType() == eRabbitGame) && playerFrom->team == playerTo->team) {
	  data->action = data->DropThief;
	  bz_sendTextMessage(BZ_SERVER, data->toPlayerID, "Flag dropped. Don't steal from teammates!");
	}
      }
      break;
    }
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
