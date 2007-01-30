// timedctf.cpp : Defines the entry point for the DLL application.
// Version 2.1

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

// event handler callback

class FlagCapped : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

class PlayerUpdates : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

class TickEvents : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

class Commands : public bz_CustomSlashCommandHandler
{
public:
  virtual ~Commands(){};
  virtual bool handle ( int playerID, bzApiString command, bzApiString message, bzAPIStringList *param );
};

class PlayerJoined : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

FlagCapped flagcapped;
PlayerUpdates playerupdates;
TickEvents tickevents;
Commands commands;
PlayerJoined playerjoined;

// default TimeLimit is 5 minutes:

double TimeLimit = 300; 
double TimeElapsed = 0;
double TimeRemaining = 0;
double RedLastTime = bz_getCurrentTime ();
double GreenLastTime = bz_getCurrentTime ();
double BlueLastTime = bz_getCurrentTime ();
double PurpleLastTime = bz_getCurrentTime ();
double RedLastWarn = bz_getCurrentTime ();
double GreenLastWarn = bz_getCurrentTime ();
double BlueLastWarn = bz_getCurrentTime ();
double PurpleLastWarn = bz_getCurrentTime ();
int AdjTime = 0;
bool TimerRunning = false;
bool TimedCTFEnabled = true;
bool FairCTFEnabled = true;
bool NotifiedCTFOK = false;
bool FairCTF = false;

double ConvertToInt(std::string inmessage){

	int messagelength = (int)inmessage.length();

	if (messagelength > 0 && messagelength < 4){

		double messagevalue = 0;
		double tens = 1;

		for ( int i = (messagelength - 1); i >= 0; i-- ){
		
			if (inmessage[i] < '0' || inmessage[i] > '9')  // got something other than a number
				return 0; 

			tens *= 10;
			messagevalue +=  (((double)inmessage[i] - '0') / 10) * tens;

		}

		// we're ok so far, check limits:

		if (messagevalue >= 1 && messagevalue <= 120)
			return messagevalue;

	}
	return 0;
}

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
  std::string parameter = commandLine;
  double timelimitparameter = ConvertToInt(parameter);
  
  if (timelimitparameter > 0)
	  TimeLimit = timelimitparameter * 60;
  
  bz_debugMessage(4,"timedctf plugin loaded");
  bz_registerEvent(bz_eCaptureEvent,&flagcapped);
  bz_registerEvent(bz_ePlayerJoinEvent,&playerjoined);
  bz_registerEvent(bz_ePlayerUpdateEvent,&playerupdates);
  bz_registerEvent(bz_eTickEvent,&tickevents);
  bz_registerCustomSlashCommand("ctfcaptimestatus",&commands);
  bz_registerCustomSlashCommand("ctfcaptime",&commands);
  bz_registerCustomSlashCommand("ctfcaptimeon",&commands);
  bz_registerCustomSlashCommand("ctfcaptimeoff",&commands);
  bz_registerCustomSlashCommand("fairctfon",&commands);
  bz_registerCustomSlashCommand("fairctfoff",&commands);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_debugMessage(4,"timedctf plugin unloaded");
  bz_removeEvent(bz_eCaptureEvent,&flagcapped);
  bz_removeEvent(bz_ePlayerJoinEvent,&playerjoined);
  bz_removeEvent(bz_ePlayerUpdateEvent,&playerupdates);
  bz_removeEvent(bz_eTickEvent,&tickevents);
  bz_removeCustomSlashCommand("ctfcaptimestatus");
  bz_removeCustomSlashCommand("ctfcaptime");
  bz_removeCustomSlashCommand("ctfcaptimeon");
  bz_removeCustomSlashCommand("ctfcaptimeoff");
  bz_removeCustomSlashCommand("fairctfon");
  bz_removeCustomSlashCommand("fairctfoff");
  return 0;
}

void ResetTeamData(){

	RedLastTime = bz_getCurrentTime ();
	GreenLastTime = bz_getCurrentTime ();
	BlueLastTime = bz_getCurrentTime ();
	PurpleLastTime = bz_getCurrentTime ();
	RedLastWarn = bz_getCurrentTime ();
	GreenLastWarn = bz_getCurrentTime ();
	BlueLastWarn = bz_getCurrentTime ();
	PurpleLastWarn = bz_getCurrentTime ();
	return;
}

// are we ok to run this thing fairly?:

bool TeamsBalanced(){
	
	// if not enough team players - no need to check any further:

	if (bz_getTeamCount(eRedTeam) + bz_getTeamCount(eGreenTeam) + bz_getTeamCount(eBlueTeam) + bz_getTeamCount(ePurpleTeam) <= 1)
		return false;

	// check for fair ctf - only need 2 teams close (TeamRatioTolerance or better)
	// this is crude - can be done better I'm sure:

	float TeamRatioTolerance = 0.75; // if not same size, at least 3 versus 4 or better
	float RatioRG = 0;
	float RatioRB = 0;
	float RatioRP = 0;
	float RatioGB = 0;
	float RatioBP = 0;
	float RatioGP = 0;
	float RS = (float)bz_getTeamCount(eRedTeam);
	float GS = (float)bz_getTeamCount(eGreenTeam);
	float BS = (float)bz_getTeamCount(eBlueTeam);
	float PS = (float)bz_getTeamCount(ePurpleTeam);

	if (RS >= GS && RS !=0)
		RatioRG = (GS / RS);
	if (GS > RS && GS !=0)
		RatioRG = (RS / GS);

	if (RS >= BS && RS !=0)
		RatioRB = (BS / RS);
	if (BS > RS && BS !=0)
		RatioRB = (RS / BS);
	
	if (RS >= PS && RS !=0)
		RatioRP = (PS / RS);
	if (PS > RS && PS !=0)
		RatioRP = (RS / PS);
	
	if (GS >= BS && GS !=0)
		RatioGB = (BS / GS);
	if (BS > GS && BS !=0)
		RatioGB = (GS / BS);
	
	if (PS >= GS && PS !=0)
		RatioGP = (GS / PS);
	if (GS > PS && GS !=0)
		RatioGP = (PS / GS);
	
	if (BS >= PS && BS !=0)
		RatioBP = (PS / BS);
	if (PS > BS && PS !=0)
		RatioBP = (BS / PS);
	
	if (RatioRG >= TeamRatioTolerance || RatioRB >= TeamRatioTolerance || RatioRP >= TeamRatioTolerance || RatioGB >= TeamRatioTolerance || RatioGP >= TeamRatioTolerance || RatioBP >= TeamRatioTolerance){
		
		return true;
	}
	else {
		
		return false;
	}
}

void KillTeam(bz_eTeamType TeamToKill){

	bzAPIIntList *playerList = bz_newIntList(); 
	bz_getPlayerIndexList ( playerList ); 

	for ( unsigned int i = 0; i < playerList->size(); i++ ){ 
       
		bz_PlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player){
			
				if (player->team == TeamToKill)
					bz_killPlayer(player->playerID, true, BZ_SERVER);
			}
		
		bz_freePlayerRecord(player);
	}
	bz_deleteIntList(playerList); 

	return;
}


// team warn and kill function

int TeamCheck(bz_eTeamType Team, const char* Color, double LastWarn, double LastTime){

	if (bz_getTeamCount(Team) != 0 && TimerRunning){

		TimeElapsed = bz_getCurrentTime() - LastTime;
		TimeRemaining = TimeLimit - TimeElapsed;

		if (bz_getCurrentTime() - LastWarn > 60){
			AdjTime = (int)(TimeRemaining / 60);
			bz_sendTextMessagef (BZ_SERVER, Team, "%s Team: less than %i minute(s) left to capture a flag!", Color, AdjTime + 1);
			return 1; // 1 = reset team's LastWarn
		}
		if (bz_getCurrentTime() - LastWarn > 30 && TimeRemaining < 30){
			bz_sendTextMessagef (BZ_SERVER, Team, "%s Team: less than 30 seconds left to capture a flag!", Color);
			return 1; // 1 = reset team's LastWarn
		}
		if (bz_getCurrentTime() - LastWarn > 10 && TimeRemaining < 10){
			bz_sendTextMessagef (BZ_SERVER, Team, "%s Team: less than 10 seconds left to capture a flag!", Color);
			return 1; // 1 = reset team's LastWarn
		}

		if (TimeElapsed >= TimeLimit){

			KillTeam(Team);
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s team did not capture any other team flags in time.", Color);
			AdjTime = (int)(TimeLimit / 60 + 0.5);
			bz_sendTextMessagef (BZ_SERVER, Team, "CTF timer is reset to %i minutes for the %s team.", AdjTime, Color);
			return 2; // 2 = reset team's LastWarn and LastTime
		}
	}

	return 0; // 0 = no need to reset teams LastWarn or LastTime
}

// reset time values if zero size team function

void ResetZeroTeams(){

	if (bz_getTeamCount(eRedTeam) == 0){
		RedLastTime = bz_getCurrentTime ();
		RedLastWarn = bz_getCurrentTime ();
	}
	if (bz_getTeamCount(eGreenTeam) == 0){
		GreenLastTime = bz_getCurrentTime ();
		GreenLastWarn = bz_getCurrentTime ();
	}
	if (bz_getTeamCount(eBlueTeam) == 0){
		BlueLastTime = bz_getCurrentTime ();
		BlueLastWarn = bz_getCurrentTime ();
	}
	if (bz_getTeamCount(ePurpleTeam) == 0){
		PurpleLastTime = bz_getCurrentTime ();
		PurpleLastWarn = bz_getCurrentTime ();
	}
	return;
}

bool OnlyOneTeamPlaying(){

	int R = bz_getTeamCount(eRedTeam);
	int G = bz_getTeamCount(eGreenTeam);
	int B = bz_getTeamCount(eBlueTeam);
	int P = bz_getTeamCount(ePurpleTeam);

	if ( R == 0 && G == 0 && B == 0 && P > 0)
		return true;
	if ( R == 0 && G == 0 && P == 0 && B > 0)
		return true;
	if ( R == 0 && B == 0 && P == 0 && G > 0)
		return true;
	if ( G == 0 && B == 0 && P == 0 && R > 0)
		return true;

	return false;
}

void PlayerJoined::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerJoinEvent)
    return;

	bz_PlayerJoinPartEventData *JoinData = (bz_PlayerJoinPartEventData*)eventData;

	// if teams are not even, notify joiner no CTF.
	// this should never be true if fair ctf is disabled (see definition of FairCTF):

	if (!FairCTF){
		bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Capture The Flag disabled - teams are not evenly balanced.");
		return;
	}

	// if timed CTF turned off, but teams now even, let everyone know it's ok to cap.
	// if fair CTF is disabled, no need to notify:

	if (FairCTF && !TimedCTFEnabled && FairCTFEnabled){
		bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Capture The Flag enabled - teams are evenly balanced.");
		return;
	}

	// if timed CTF turned off, get outta here:

	if (!TimedCTFEnabled)
		return;

	// if teams even, notify joiner how much time is left to CTF for their team:

	if (JoinData->team == eRedTeam  && TimerRunning){
		TimeElapsed = bz_getCurrentTime () - RedLastTime;
		TimeRemaining = TimeLimit - TimeElapsed;
		AdjTime = (int)(TimeRemaining / 60);
		bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", AdjTime + 1);
		return;
	}

	if (JoinData->team == eGreenTeam  && TimerRunning){
		TimeElapsed = bz_getCurrentTime () - GreenLastTime;
		TimeRemaining = TimeLimit - TimeElapsed;
		AdjTime = (int)(TimeRemaining / 60);
		bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", AdjTime + 1);
		return;
	}

	if (JoinData->team == eBlueTeam  && TimerRunning){
		TimeElapsed = bz_getCurrentTime () - BlueLastTime;
		TimeRemaining = TimeLimit - TimeElapsed;
		AdjTime = (int)(TimeRemaining / 60);
		bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", AdjTime + 1);
		return;
	}

	if (JoinData->team == ePurpleTeam  && TimerRunning){
		TimeElapsed = bz_getCurrentTime () - PurpleLastTime;
		TimeRemaining = TimeLimit - TimeElapsed;
		AdjTime = (int)(TimeRemaining / 60);
		bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", AdjTime + 1);
		return;
	}

	return;
}

void FlagCapped::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_eCaptureEvent)
    return;

	// if timed CTF turned off, get outta here:

	if (!TimedCTFEnabled || !TimerRunning)
		return;

	bz_CTFCaptureEventData *CapData = (bz_CTFCaptureEventData*)eventData;

	// if team caps, reset their timer and notify their team

	if (CapData->teamCapping == eRedTeam){
		AdjTime = (int)(TimeLimit / 60 + 0.5);
		bz_sendTextMessagef (BZ_SERVER, eRedTeam, "CTF timer is reset to %i minutes for the red team.", AdjTime);
		RedLastTime = bz_getCurrentTime ();
		RedLastWarn = bz_getCurrentTime ();
		return;
	}
	if (CapData->teamCapping == eGreenTeam){
		AdjTime = (int)(TimeLimit / 60 + 0.5);
		bz_sendTextMessagef (BZ_SERVER, eGreenTeam, "CTF timer is reset to %i minutes for the green team.", AdjTime);
		GreenLastTime = bz_getCurrentTime ();
		GreenLastWarn = bz_getCurrentTime ();
		return;
	}
	if (CapData->teamCapping == eBlueTeam){
		AdjTime = (int)(TimeLimit / 60 + 0.5);
		bz_sendTextMessagef (BZ_SERVER, eBlueTeam, "CTF timer is reset to %i minutes for the blue team.", AdjTime);
		BlueLastTime = bz_getCurrentTime ();
		BlueLastWarn = bz_getCurrentTime ();
		return;
	}
	if (CapData->teamCapping == ePurpleTeam){
		AdjTime = (int)(TimeLimit / 60 + 0.5);
		bz_sendTextMessagef (BZ_SERVER, ePurpleTeam, "CTF timer is reset to %i minutes for the purple team.", AdjTime);
		PurpleLastTime = bz_getCurrentTime ();
		PurpleLastWarn = bz_getCurrentTime ();
		return;
	}

	return;
}

// this is where most of the decisions are made - a little clunky, but seems to work:

void TickEvents::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_eTickEvent)
		return;

	// read this function once per tick event.  If fair CTF disabled, make it look fair to rest
	// of code - need to do this to be able to have timed ctf without fair ctf.

	FairCTF = (TeamsBalanced() || !FairCTFEnabled);  

	// check/notify team balance changes while timed CTF is disabled.
	// if fair ctf is disabled, no need to check/notify about team balance changes:

	if (FairCTF && !NotifiedCTFOK && !TimedCTFEnabled && FairCTFEnabled){

		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Capture The Flag enabled - teams are evenly balanced.");
		NotifiedCTFOK = true;
		return;
	}

	if (!FairCTF && NotifiedCTFOK && !TimedCTFEnabled && FairCTFEnabled){

		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Capture The Flag disabled - teams are not evenly balanced.");
		NotifiedCTFOK = false;
		return;
	}

	// if no timed CTF, we can leave:

	if (!TimedCTFEnabled)
		return;

	// if this is true, we can leave too:

	if (!FairCTF && !TimerRunning)
		return;

	// check/notify team balance changes while timed CTF and fair CTF are enabled:

	if(!FairCTF && TimerRunning && FairCTFEnabled){

		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Capture The Flag disabled - teams are not evenly balanced.");
		TimerRunning = false;
		ResetTeamData();
		return;
	}

	// no timed ctf with fair CTF disabled and only one team present:

	if (FairCTF && !FairCTFEnabled){ 
		
		if (OnlyOneTeamPlaying()){

			if (TimerRunning)
				bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF disabled - not enough teams.");

			TimerRunning = false;
			ResetTeamData();
			return;
		}
	}

	// start timing if we have made it this far:

	if (FairCTF && !TimerRunning && !OnlyOneTeamPlaying()){ 

		AdjTime = (int)(TimeLimit / 60 + 0.5);
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", AdjTime);
		TimerRunning = true;
		ResetTeamData();
		return;
	}

	// everything is a go for timed ctf checks now.
	// check each team's time left, warn and kill if necessary:

	int RedReturn = TeamCheck(eRedTeam, "RED", RedLastWarn, RedLastTime);
	int GreenReturn = TeamCheck(eGreenTeam, "GREEN", GreenLastWarn, GreenLastTime);
	int BlueReturn = TeamCheck(eBlueTeam, "BLUE", BlueLastWarn, BlueLastTime);
	int PurpleReturn = TeamCheck(ePurpleTeam, "PURPLE", PurpleLastWarn, PurpleLastTime);

	if (RedReturn == 1)
		RedLastWarn = bz_getCurrentTime();
	if (RedReturn == 2){
		RedLastWarn = bz_getCurrentTime();
		RedLastTime = bz_getCurrentTime();
	}
	if (GreenReturn == 1)
		GreenLastWarn = bz_getCurrentTime();
	if (GreenReturn == 2){
		GreenLastWarn = bz_getCurrentTime();
		GreenLastTime = bz_getCurrentTime();
	}
	if (BlueReturn == 1)
		BlueLastWarn = bz_getCurrentTime();
	if (BlueReturn == 2){
		BlueLastWarn = bz_getCurrentTime();
		BlueLastTime = bz_getCurrentTime();
	}
	if (PurpleReturn == 1)
		PurpleLastWarn = bz_getCurrentTime();
	if (PurpleReturn == 2){
		PurpleLastWarn = bz_getCurrentTime();
		PurpleLastTime = bz_getCurrentTime();
	}

	ResetZeroTeams(); // reset team data for teams with no players.

	return;
}
	
void PlayerUpdates::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerUpdateEvent)
		return;

	// no CTF if teams not balanced, drop team flags asap:

	if (!FairCTF){

		int playerID = ((bz_PlayerUpdateEventData*)eventData)->playerID;
		const char* FlagHeld = bz_getPlayerFlag(playerID);
				
		if (FlagHeld != NULL){

			if (strcmp(FlagHeld, "R*") == 0 || strcmp(FlagHeld, "G*") == 0 || strcmp(FlagHeld, "B*") == 0 || strcmp(FlagHeld, "P*") == 0 ){
				bz_removePlayerFlag ( playerID );
				bz_sendTextMessagef (BZ_SERVER, playerID, "Capture The Flag disabled - teams are not evenly balanced.");
			}
		}

	}

	return;
}

bool Commands::handle ( int playerID, bzApiString _command, bzApiString _message, bzAPIStringList * /*_param*/ )
{
	std::string command = _command.c_str();
	std::string message = _message.c_str();

	bz_PlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);

	if ( !fromPlayer->admin ){
    
		bz_sendTextMessage(BZ_SERVER,playerID,"You must be admin to use the ctfcaptime commands.");
		bz_freePlayerRecord(fromPlayer);
		return true;
	}

	if ( command == "ctfcaptimeon"){
	
		TimedCTFEnabled = true;
		if (!TimerRunning)
			ResetTeamData();
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF is enabled.");
		return true;
	}

	bz_freePlayerRecord(fromPlayer);

	if ( command == "ctfcaptimeoff"){
	
		TimedCTFEnabled = false;
		TimerRunning = false;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF is disabled.");
		return true;
	}

	if ( command == "fairctfon"){
	
		FairCTFEnabled = true;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Fair CTF is enabled.");
		return true;
	}

	if ( command == "fairctfoff"){
	
		FairCTFEnabled = false;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Fair CTF is disabled.");
		if (!TimerRunning)
			ResetTeamData();
		return true;
	}

	if ( command == "ctfcaptimestatus"){

		if (TimedCTFEnabled && !TimerRunning)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Timed CTF is currently enabled, but not running.");

		if (TimedCTFEnabled && TimerRunning)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Timed CTF is currently enabled, and running");

		if (!TimedCTFEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Timed CTF is currently disabled.");

		if (!FairCTFEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Fair CTF is currently disabled");

		if (FairCTFEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Fair CTF is currently enabled");

		AdjTime = (int)(TimeLimit/60 + 0.5);
		bz_sendTextMessagef (BZ_SERVER, playerID, "CTF capture time is currently set to: %i minutes", AdjTime);
		return true;
	}
    
  // explicit time command handler:

	if ( command == "ctfcaptime" ){

		double inputvalue = ConvertToInt(message);
  
		if (inputvalue > 0){

			TimeLimit = inputvalue * 60;
			AdjTime = (int)(TimeLimit / 60 + 0.5);
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "CTF capture time has been set to %i minutes.", AdjTime);

			if (!TimedCTFEnabled)
				bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "(Timed CTF is still disabled)");

			ResetTeamData();
			return true;
		}
		else{
			bz_sendTextMessagef (BZ_SERVER, playerID, "CTF capture time invalid: must be between 1 and 120 minutes.");
			return true;
		}

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

