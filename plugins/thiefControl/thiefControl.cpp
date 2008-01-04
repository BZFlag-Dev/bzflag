// thiefControl.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

class ThiefControl:public bz_EventHandler
{
public:
  ThiefControl() {};
  virtual ~ThiefControl() {};
  virtual void process(bz_EventData * eventData);
};

ThiefControl thiefHandler;

BZF_PLUGIN_CALL int bz_Load(const char * /*commandLine */ )
{
  bz_debugMessage(4, "thiefControl plugin loaded");
  bz_registerEvent(bz_eFlagTransferredEvent, &thiefHandler);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void)
{
  bz_removeEvent(bz_eFlagTransferredEvent, &thiefHandler);
  bz_debugMessage(4, "thiefControl plugin unloaded");
  return 0;
}

void ThiefControl::process(bz_EventData * eventData)
{
  bz_FlagTransferredEventData_V1 *data = (bz_FlagTransferredEventData_V1 *) eventData;
  const std::string noStealMsg = "Flag dropped. Don't steal from teammates!";

  if (!data)
    return;

  if (data->eventType != bz_eFlagTransferredEvent)
    return;

  bz_BasePlayerRecord *playerFrom = bz_getPlayerByIndex(data->fromPlayerID);

  if (!playerFrom)
    return;

  bz_BasePlayerRecord *playerTo = bz_getPlayerByIndex(data->toPlayerID);

  if (!playerTo) {
    bz_freePlayerRecord(playerFrom);
    return;
  }

  switch (bz_getGameType()) {

  case eTeamFFAGame:
    if (playerTo->team == playerFrom->team && playerTo->team != eRogueTeam) {
      data->action = data->DropThief;
      bz_sendTextMessage(BZ_SERVER, data->toPlayerID, noStealMsg.c_str());
    }
    break;

  case eClassicCTFGame:
    {
      // Allow teammates to steal team flags
      // This will allow someone to steal a team flag in order
      // to possibly capture it faster.
      bool allowTransfer = false;

      if (playerTo->team == playerFrom->team && playerTo->team != eRogueTeam) {
	std::string flagT = data->flagType;

	// Allow theft of team flags only
	allowTransfer = (flagT == "R*" || flagT == "G*" || flagT == "B*" || flagT == "P*");

	if (!allowTransfer) {
	  data->action = data->DropThief;
	  bz_sendTextMessage(BZ_SERVER, data->toPlayerID, noStealMsg.c_str());
	}
      }
    }
    break;

  case eOpenFFAGame:
    break;

  case eRabbitGame:
    if (playerTo->team == playerFrom->team) {

      data->action = data->DropThief;
      bz_sendTextMessage(BZ_SERVER, data->toPlayerID, noStealMsg.c_str());
    }
    break;

  default:
    break;

  }

  bz_freePlayerRecord(playerTo);
  bz_freePlayerRecord(playerFrom);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
