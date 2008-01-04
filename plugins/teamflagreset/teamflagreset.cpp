// teamflagreset.cpp : Defines the entry point for the DLL application.

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

// event handler callback
class TFR
{
public:
  TFR()
  {
    idleTime = 300;
    redLastTouched = bz_getCurrentTime();
    greenLastTouched = bz_getCurrentTime();
    blueLastTouched = bz_getCurrentTime();
    purpleLastTouched = bz_getCurrentTime();
    redFlagWasHeld = false;
    greenFlagWasHeld = false;
    blueFlagWasHeld = false;
    purpleFlagWasHeld = false;
    OKToReset = false;
    timerOff = false;
    flagTouched = "";
  }
  double idleTime;
  double redLastTouched;
  double greenLastTouched;
  double blueLastTouched;
  double purpleLastTouched;
  bool redFlagWasHeld;
  bool greenFlagWasHeld;
  bool blueFlagWasHeld;
  bool purpleFlagWasHeld;
  bool OKToReset;
  bool timerOff;
  const char *flagTouched;
};

TFR tfr;

class TeamFlagResetHandler:public bz_EventHandler
{
public:
  virtual void process(bz_EventData * eventData);
};

class TeamFlagResetIOHandler:public bz_CustomSlashCommandHandler
{
public:
  virtual ~TeamFlagResetIOHandler() {};
  virtual bool handle(int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList * param);
};

TeamFlagResetHandler teamflagresethandler;
TeamFlagResetIOHandler teamflagresetiohandler;

double ConvertToInteger(std::string msg)
{
  int msglength = (int) msg.length();

  if (msglength > 0 && msglength < 4) {
    double msgvalue = 0;
    double tens = 1;

    for (int i = (msglength - 1); i >= 0; i--) {
      if (msg[i] < '0' || msg[i] > '9')	// got something other than a number
	return 0;

      tens *= 10;
      msgvalue += (((double) msg[i] - '0') / 10) * tens;
    }

    // we're ok so far, check limits:
    if (msgvalue >= 1 && msgvalue <= 120)
      return msgvalue;
  }
  return 0;
}

BZF_PLUGIN_CALL int bz_Load(const char *commandLineParameter)
{
  std::string param = commandLineParameter;
  double timelimitparam = ConvertToInteger(param);

  if (timelimitparam > 0)
    tfr.idleTime = timelimitparam * 60;

  bz_debugMessage(4, "teamflagreset plugin loaded");
  bz_registerEvent(bz_eTickEvent, &teamflagresethandler);
  bz_registerCustomSlashCommand("tfrtime", &teamflagresetiohandler);
  bz_registerCustomSlashCommand("tfroff", &teamflagresetiohandler);
  bz_registerCustomSlashCommand("tfron", &teamflagresetiohandler);
  bz_registerCustomSlashCommand("tfrstatus", &teamflagresetiohandler);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload(void)
{
  bz_debugMessage(4, "teamflagreset plugin unloaded");
  bz_removeEvent(bz_eTickEvent, &teamflagresethandler);
  bz_removeCustomSlashCommand("tfrtime");
  bz_removeCustomSlashCommand("tfroff");
  bz_removeCustomSlashCommand("tfron");
  bz_removeCustomSlashCommand("tfrstatus");
  return 0;
}

void ResetFlagData()
{
  tfr.redLastTouched = bz_getCurrentTime();
  tfr.greenLastTouched = bz_getCurrentTime();
  tfr.blueLastTouched = bz_getCurrentTime();
  tfr.purpleLastTouched = bz_getCurrentTime();
  tfr.redFlagWasHeld = false;
  tfr.greenFlagWasHeld = false;
  tfr.blueFlagWasHeld = false;
  tfr.purpleFlagWasHeld = false;
}

void resetTeamFlag(bz_ApiString flagSent)
{
  for (unsigned int i = 0; i < bz_getNumFlags(); i++) {
    if (flagSent == bz_getFlagName(i))
      bool resetOK = bz_resetFlag(i);
  }
}

void TeamFlagResetHandler::process(bz_EventData * eventData)
{
  if (eventData->eventType != bz_eTickEvent)
    return;

  if (tfr.timerOff == true)
    return;

  bz_APIIntList *playerList = bz_newIntList();
  bz_getPlayerIndexList(playerList);

  // check to see if anyone has picked up a team flag & count players per team

  for (unsigned int i = 0; i < playerList->size(); i++) {
    bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i));

    if (player) {

      tfr.flagTouched = bz_getPlayerFlag(player->playerID);

      if (tfr.flagTouched) {

	if (strcmp(tfr.flagTouched, "R*") == 0) {
	  tfr.redLastTouched = bz_getCurrentTime();
	  tfr.redFlagWasHeld = true;
	}
	if (strcmp(tfr.flagTouched, "G*") == 0) {
	  tfr.greenLastTouched = bz_getCurrentTime();
	  tfr.greenFlagWasHeld = true;
	}
	if (strcmp(tfr.flagTouched, "B*") == 0) {
	  tfr.blueLastTouched = bz_getCurrentTime();
	  tfr.blueFlagWasHeld = true;
	}
	if (strcmp(tfr.flagTouched, "P*") == 0) {
	  tfr.purpleLastTouched = bz_getCurrentTime();
	  tfr.purpleFlagWasHeld = true;
	}
      }

      bz_freePlayerRecord(player);
    }
  }

  bz_deleteIntList(playerList);

  // if no teamplay, no need to reset flags

  tfr.OKToReset = false;

  if (bz_getTeamCount(eRedTeam) * bz_getTeamCount(eGreenTeam) > 0)
    tfr.OKToReset = true;
  if (bz_getTeamCount(eRedTeam) * bz_getTeamCount(eBlueTeam) > 0)
    tfr.OKToReset = true;
  if (bz_getTeamCount(eRedTeam) * bz_getTeamCount(ePurpleTeam) > 0)
    tfr.OKToReset = true;
  if (bz_getTeamCount(eGreenTeam) * bz_getTeamCount(eBlueTeam) > 0)
    tfr.OKToReset = true;
  if (bz_getTeamCount(eGreenTeam) * bz_getTeamCount(ePurpleTeam) > 0)
    tfr.OKToReset = true;
  if (bz_getTeamCount(eBlueTeam) * bz_getTeamCount(ePurpleTeam) > 0)
    tfr.OKToReset = true;

  if (tfr.OKToReset == false) {
    ResetFlagData();
    return;
  }
  // check if time's up on flags and reset (if they were held at least once after last reset)

  if (bz_getCurrentTime() - tfr.redLastTouched > tfr.idleTime && tfr.redFlagWasHeld) {
    if (bz_getTeamCount(eRedTeam) > 0) {
      resetTeamFlag("R*");
      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Red flag sat idle too long - reset by server.");
    }
    tfr.redFlagWasHeld = false;
    tfr.redLastTouched = bz_getCurrentTime();
  }

  if (bz_getCurrentTime() - tfr.greenLastTouched > tfr.idleTime && tfr.greenFlagWasHeld) {
    if (bz_getTeamCount(eGreenTeam) > 0) {
      resetTeamFlag("G*");
      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Green flag sat idle too long - reset by server.");
    }
    tfr.greenLastTouched = bz_getCurrentTime();
    tfr.greenFlagWasHeld = false;
  }

  if (bz_getCurrentTime() - tfr.blueLastTouched > tfr.idleTime && tfr.blueFlagWasHeld) {
    if (bz_getTeamCount(eBlueTeam) > 0) {
      resetTeamFlag("B*");
      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Blue flag sat idle too long - reset by server.");
    }
    tfr.blueLastTouched = bz_getCurrentTime();
    tfr.blueFlagWasHeld = false;
  }

  if (bz_getCurrentTime() - tfr.purpleLastTouched > tfr.idleTime && tfr.purpleFlagWasHeld) {
    if (bz_getTeamCount(ePurpleTeam) > 0) {
      resetTeamFlag("P*");
      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Purple flag sat idle too long - reset by server.");
    }
    tfr.purpleLastTouched = bz_getCurrentTime();
    tfr.purpleFlagWasHeld = false;
  }
  return;
}

bool TeamFlagResetIOHandler::handle(int playerID, bz_ApiString _command, bz_ApiString _message, bz_APIStringList * /*_param*/ )
{
  std::string command = _command.c_str();
  std::string message = _message.c_str();

  bz_BasePlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);

  if (fromPlayer) {
    if (!fromPlayer->admin) {
      bz_sendTextMessage(BZ_SERVER, playerID, "You must be admin to use the teamflagreset commands.");
      bz_freePlayerRecord(fromPlayer);
      return true;
    }
    bz_freePlayerRecord(fromPlayer);
  }

  if (command == "tfrtime") {

    double invalue = ConvertToInteger(message);

    if (invalue > 0) {

      tfr.timerOff = false;
      tfr.idleTime = invalue * 60;

      int AdjTime = (int) (tfr.idleTime / 60 + 0.5);
      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Team flag idle time has been set to %i minutes.", AdjTime);

      ResetFlagData();
      return true;
    } else {
      bz_sendTextMessagef(BZ_SERVER, playerID, "Team flag idle time invalid: must be between 1 and 120 minutes.");
      return true;
    }
    return true;
  }

  if (command == "tfroff") {

    tfr.timerOff = true;
    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Team flag reset is disabled.");
    return true;
  }

  if (command == "tfron") {

    tfr.timerOff = false;
    ResetFlagData();
    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Team flag reset is enabled.");
    return true;
  }

  if (command == "tfrstatus") {

    if (tfr.timerOff)
      bz_sendTextMessagef(BZ_SERVER, playerID, "Team flag reset is disabled.");
    else
      bz_sendTextMessagef(BZ_SERVER, playerID, "Team flag reset is enabled.");

    int AdjTime = (int) (tfr.idleTime / 60 + 0.5);
    bz_sendTextMessagef(BZ_SERVER, playerID, "Team flag idle time is: %i minutes.", AdjTime);

    return true;
  }

  return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
