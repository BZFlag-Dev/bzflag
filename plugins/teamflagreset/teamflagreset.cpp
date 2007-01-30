// teamflagreset.cpp : Defines the entry point for the DLL application.
// Version 2.1

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

// event handler callback

class TeamFlagResetHandler : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

class TeamFlagResetIOHandler : public bz_CustomSlashCommandHandler
{
public:
  virtual ~TeamFlagResetIOHandler(){};
  virtual bool handle ( int playerID, bzApiString command, bzApiString message, bzAPIStringList *param );
};

TeamFlagResetHandler	teamflagresethandler;
TeamFlagResetIOHandler	teamflagresetiohandler;

double IdleTime = 300; 
double RedLastTouched = bz_getCurrentTime();
double GreenLastTouched = bz_getCurrentTime();
double BlueLastTouched = bz_getCurrentTime();
double PurpleLastTouched = bz_getCurrentTime();
int AdjustedTime = 0;
bool RedFlagWasHeld = false;
bool GreenFlagWasHeld = false;
bool BlueFlagWasHeld = false;
bool PurpleFlagWasHeld = false;
bool OKToReset = false;
bool FlagTimerOff = false;
const char* FlagTouched;

double ConvertToInteger(std::string msg){

	int msglength = (int)msg.length();

	if (msglength > 0 && msglength < 4){

		double msgvalue = 0;
		double tens = 1;

		for ( int i = (msglength - 1); i >= 0; i-- ){
		
			if (msg[i] < '0' || msg[i] > '9')  // got something other than a number
				return 0; 

			tens *= 10;
			msgvalue +=  (((double)msg[i] - '0') / 10) * tens;

		}

		// we're ok so far, check limits:

		if (msgvalue >= 1 && msgvalue <= 120)
			return msgvalue;

	}
	return 0;
}

BZF_PLUGIN_CALL int bz_Load ( const char* commandLineParameter )
{
  std::string param = commandLineParameter;
  double timelimitparam = ConvertToInteger(param);
  
  if (timelimitparam > 0)
	  IdleTime = timelimitparam * 60;

  bz_debugMessage(4,"teamflagreset plugin loaded");
  bz_registerEvent(bz_eTickEvent,&teamflagresethandler);
  bz_registerCustomSlashCommand("flagidletime",&teamflagresetiohandler);
  bz_registerCustomSlashCommand("flagidletimeoff",&teamflagresetiohandler);
  bz_registerCustomSlashCommand("flagidletimeon",&teamflagresetiohandler);
  bz_registerCustomSlashCommand("flagidletimestatus",&teamflagresetiohandler);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_debugMessage(4,"teamflagreset plugin unloaded");
  bz_removeEvent(bz_eTickEvent,&teamflagresethandler);
  bz_removeCustomSlashCommand("flagidletime");
  bz_removeCustomSlashCommand("flagidletimeoff");
  bz_removeCustomSlashCommand("flagidletimeon");
  bz_removeCustomSlashCommand("flagidletimestatus");
  return 0;
}

void ResetFlagData(){

	RedLastTouched = bz_getCurrentTime();
	GreenLastTouched = bz_getCurrentTime();
	BlueLastTouched = bz_getCurrentTime();
	PurpleLastTouched = bz_getCurrentTime();
	RedFlagWasHeld = false;
	GreenFlagWasHeld = false;
	BlueFlagWasHeld = false;
	PurpleFlagWasHeld = false;

}

void resetTeamFlag (bzApiString flagSent)
{
	for ( unsigned int i = 0; i < bz_getNumFlags(); i++ )
	{
		if (flagSent == bz_getName(i))
			bool resetOK = bz_resetFlag (i);
	}
}

void TeamFlagResetHandler::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_eTickEvent)
		return;

	if (FlagTimerOff == true)
		return;

	bzAPIIntList *playerList = bz_newIntList();
	bz_getPlayerIndexList ( playerList );

	// check to see if anyone has picked up a team flag & count players per team
	
	for ( unsigned int i = 0; i < playerList->size(); i++ ){

		bz_PlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i));

		if (player) {
			
			FlagTouched = bz_getPlayerFlag(player->playerID);

			if (FlagTouched){

				if (strcmp(FlagTouched, "R*") == 0){
					RedLastTouched = bz_getCurrentTime();
					RedFlagWasHeld = true;
				}
				if (strcmp(FlagTouched, "G*") == 0){
					GreenLastTouched = bz_getCurrentTime();
					GreenFlagWasHeld = true;
				}
				if (strcmp(FlagTouched, "B*") == 0){
					BlueLastTouched = bz_getCurrentTime();
					BlueFlagWasHeld = true;
				}
				if (strcmp(FlagTouched, "P*") == 0){
					PurpleLastTouched = bz_getCurrentTime();
					PurpleFlagWasHeld = true;
				}
			}
	
			bz_freePlayerRecord(player);
		}
	}

	bz_deleteIntList(playerList);

	// if no teamplay, no need to reset flags

	OKToReset = false;

	if (bz_getTeamCount(eRedTeam) * bz_getTeamCount(eGreenTeam) > 0)
		OKToReset = true;
	if (bz_getTeamCount(eRedTeam) * bz_getTeamCount(eBlueTeam) > 0)
		OKToReset = true;
	if (bz_getTeamCount(eRedTeam) * bz_getTeamCount(ePurpleTeam) > 0)
		OKToReset = true;
	if (bz_getTeamCount(eGreenTeam) * bz_getTeamCount(eBlueTeam) > 0)
		OKToReset = true;
	if (bz_getTeamCount(eGreenTeam) * bz_getTeamCount(ePurpleTeam) > 0)
		OKToReset = true;
	if (bz_getTeamCount(eBlueTeam) * bz_getTeamCount(ePurpleTeam) > 0)
		OKToReset = true;

	if (OKToReset == false){
		ResetFlagData();
		return;
	}

	// check if time's up on flags and reset (if they were held at least once after last reset)

	if (bz_getCurrentTime() - RedLastTouched > IdleTime && RedFlagWasHeld){
		if (bz_getTeamCount(eRedTeam) > 0){
			resetTeamFlag ("R*");
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Red flag sat idle too long - reset by server.");
		}
		RedFlagWasHeld = false;
		RedLastTouched = bz_getCurrentTime();
	}

	if (bz_getCurrentTime() - GreenLastTouched > IdleTime && GreenFlagWasHeld){
		if (bz_getTeamCount(eGreenTeam) > 0){
			resetTeamFlag ("G*");
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Green flag sat idle too long - reset by server.");
		}
		GreenLastTouched = bz_getCurrentTime();
		GreenFlagWasHeld = false;
	}

	if (bz_getCurrentTime() - BlueLastTouched > IdleTime && BlueFlagWasHeld){
		if (bz_getTeamCount(eBlueTeam) > 0){
			resetTeamFlag ("B*");
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Blue flag sat idle too long - reset by server.");
		}
		BlueLastTouched = bz_getCurrentTime();
		BlueFlagWasHeld = false;
	}

	if (bz_getCurrentTime() - PurpleLastTouched > IdleTime && PurpleFlagWasHeld){
		if (bz_getTeamCount(ePurpleTeam) > 0){
			resetTeamFlag ("P*");
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Purple flag sat idle too long - reset by server.");
		}
		PurpleLastTouched = bz_getCurrentTime();
		PurpleFlagWasHeld = false;
	}
	return;
}

bool TeamFlagResetIOHandler::handle ( int playerID, bzApiString _command, bzApiString _message, bzAPIStringList * /*_param*/ )
{
	std::string command = _command.c_str();
	std::string message = _message.c_str();

	bz_PlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);

	if ( !fromPlayer->admin ) {
		
		bz_sendTextMessage(BZ_SERVER,playerID,"You must be admin to use the flagidletime commands.");
		bz_freePlayerRecord(fromPlayer);
		return true;
	}
	bz_freePlayerRecord(fromPlayer);
	
	if ( command == "flagidletime") {

		double invalue = ConvertToInteger(message);
  
		if (invalue > 0){

			FlagTimerOff = false;
			IdleTime = invalue * 60;

			int AdjTime = (int)(IdleTime / 60 + 0.5);
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Team flag idle time has been set to %i minutes.", AdjTime);

			ResetFlagData();
			return true;
		}
		else{
			bz_sendTextMessagef (BZ_SERVER, playerID, "Team flag idle time invalid: must be between 1 and 120 minutes.");
			return true;
		}
		return true;
	}

	if ( command == "flagidletimeoff") {

		FlagTimerOff = true;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Team flag reset is disabled.");
		return true;
	}
	
	if ( command == "flagidletimeon") {

		FlagTimerOff = false;
		ResetFlagData();
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Team flag reset is enabled.");
		return true;
	}

	if ( command == "flagidletimestatus") {

		if (FlagTimerOff)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Team flag reset is disabled.");
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "Team flag reset is enabled.");
		
		int AdjTime = (int)(IdleTime / 60 + 0.5);
		bz_sendTextMessagef (BZ_SERVER, playerID, "Team flag idle time is: %i minutes.", AdjTime);

		return true;
	}

	return false;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

