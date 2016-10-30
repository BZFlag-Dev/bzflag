// timedctf.cpp : Defines the entry point for the DLL application.

#include "bzfsAPI.h"

// event handler callback

class TCTF
{
public:
  TCTF()
  {
    timeLimit = 300;
    timeElapsed = 0;
    timeRemaining = 0;
    redLastTime = bz_getCurrentTime ();
    greenLastTime = bz_getCurrentTime ();
    blueLastTime = bz_getCurrentTime ();
    purpleLastTime = bz_getCurrentTime ();
    redLastWarn = bz_getCurrentTime ();
    greenLastWarn = bz_getCurrentTime ();
    blueLastWarn = bz_getCurrentTime ();
    purpleLastWarn = bz_getCurrentTime ();
    adjTime = 0;
    timerRunning = false;
    enabled = true;
    fairCTFEnabled = true;
    notifiedCTFOK = false;
    fairCTF = false;
    soundEnabled = true;
  }
  double timeLimit;
  double timeElapsed;
  double timeRemaining;
  double redLastTime;
  double greenLastTime;
  double blueLastTime;
  double purpleLastTime;
  double redLastWarn;
  double greenLastWarn;
  double blueLastWarn;
  double purpleLastWarn;
  int adjTime;
  bool timerRunning;
  bool enabled;
  bool fairCTFEnabled;
  bool notifiedCTFOK;
  bool fairCTF;
  bool soundEnabled;
};

TCTF tctf;

class TCTFCommands : public bz_CustomSlashCommandHandler
{
public:
  virtual ~TCTFCommands() {};
  virtual bool SlashCommand ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *param );
};

class TCTFHandler : public bz_Plugin
{
public:

  virtual const char* Name () {return "Timed CTF";}
  virtual void Init ( const char* config );
  virtual void Cleanup ();
  virtual void Event ( bz_EventData *eventData );

};

BZ_PLUGIN(TCTFHandler)

TCTFCommands tctfcommands;

double ConvertToInt(std::string inmessage) {

  int messagelength = (int)inmessage.length();

  if (messagelength > 0 && messagelength < 4) {

    double messagevalue = 0;
    double tens = 1;

    for ( int i = (messagelength - 1); i >= 0; i-- ) {

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

void TCTFHandler::Init( const char* commandLine )
{
  std::string parameter = commandLine;
  double timelimitparameter = ConvertToInt(parameter);

  if (timelimitparameter > 0)
    tctf.timeLimit = timelimitparameter * 60;

  Register(bz_eCaptureEvent);
  Register(bz_ePlayerJoinEvent);
  Register(bz_ePlayerUpdateEvent);
  Register(bz_eTickEvent);
  bz_registerCustomSlashCommand("tctfstatus",&tctfcommands);
  bz_registerCustomSlashCommand("tctftime",&tctfcommands);
  bz_registerCustomSlashCommand("tctfon",&tctfcommands);
  bz_registerCustomSlashCommand("tctfoff",&tctfcommands);
  bz_registerCustomSlashCommand("fairctfon",&tctfcommands);
  bz_registerCustomSlashCommand("fairctfoff",&tctfcommands);
  bz_registerCustomSlashCommand("tctfsoundon",&tctfcommands);
  bz_registerCustomSlashCommand("tctfsoundoff",&tctfcommands);
}

void TCTFHandler::Cleanup ( void )
{
  Flush();
  bz_removeCustomSlashCommand("tctfstatus");
  bz_removeCustomSlashCommand("tctftime");
  bz_removeCustomSlashCommand("tctfon");
  bz_removeCustomSlashCommand("tctfoff");
  bz_removeCustomSlashCommand("fairctfon");
  bz_removeCustomSlashCommand("fairctfoff");
  bz_removeCustomSlashCommand("tctfsoundon");
  bz_removeCustomSlashCommand("tctfsoundoff");
}

void ResetTeamData() {

  tctf.redLastTime = bz_getCurrentTime ();
  tctf.greenLastTime = bz_getCurrentTime ();
  tctf.blueLastTime = bz_getCurrentTime ();
  tctf.purpleLastTime = bz_getCurrentTime ();
  tctf.redLastWarn = bz_getCurrentTime ();
  tctf.greenLastWarn = bz_getCurrentTime ();
  tctf.blueLastWarn = bz_getCurrentTime ();
  tctf.purpleLastWarn = bz_getCurrentTime ();
  return;
}

// are we ok to run this thing fairly?:

bool TeamsBalanced() {

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
  // "GS" is a macro defined in /usr/include/sys/regset.h on Solaris x86
  float _GS = (float)bz_getTeamCount(eGreenTeam);
  float BS = (float)bz_getTeamCount(eBlueTeam);
  float PS = (float)bz_getTeamCount(ePurpleTeam);

  if (RS >= _GS && RS !=0)
    RatioRG = (_GS / RS);
  if (_GS > RS && _GS !=0)
    RatioRG = (RS / _GS);

  if (RS >= BS && RS !=0)
    RatioRB = (BS / RS);
  if (BS > RS && BS !=0)
    RatioRB = (RS / BS);

  if (RS >= PS && RS !=0)
    RatioRP = (PS / RS);
  if (PS > RS && PS !=0)
    RatioRP = (RS / PS);

  if (_GS >= BS && _GS !=0)
    RatioGB = (BS / _GS);
  if (BS > _GS && BS !=0)
    RatioGB = (_GS / BS);

  if (PS >= _GS && PS !=0)
    RatioGP = (_GS / PS);
  if (_GS > PS && _GS !=0)
    RatioGP = (PS / _GS);

  if (BS >= PS && BS !=0)
    RatioBP = (PS / BS);
  if (PS > BS && PS !=0)
    RatioBP = (BS / PS);

  if (RatioRG >= TeamRatioTolerance || RatioRB >= TeamRatioTolerance || RatioRP >= TeamRatioTolerance || RatioGB >= TeamRatioTolerance || RatioGP >= TeamRatioTolerance || RatioBP >= TeamRatioTolerance) {

    return true;
  }
  else {

    return false;
  }
}

void KillTeam(bz_eTeamType TeamToKill) {

  bz_APIIntList *playerList = bz_newIntList();
  bz_getPlayerIndexList ( playerList );

  for ( unsigned int i = 0; i < playerList->size(); i++ ) {

    bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i));

    if (player) {

      if (player->team == TeamToKill)
      {
	bz_killPlayer(player->playerID, true, BZ_SERVER);
	if (tctf.soundEnabled)
	  bz_sendPlayCustomLocalSound(player->playerID,"flag_lost");
      }
      else if (tctf.soundEnabled)
	bz_sendPlayCustomLocalSound(player->playerID,"flag_won");
      bz_freePlayerRecord(player);
    }

  }
  bz_deleteIntList(playerList);

  return;
}


// team warn and kill function

int TeamCheck(bz_eTeamType Team, const char* Color, double LastWarn, double LastTime) {

  if (bz_getTeamCount(Team) != 0 && tctf.timerRunning) {

    tctf.timeElapsed = bz_getCurrentTime() - LastTime;
    tctf.timeRemaining = tctf.timeLimit - tctf.timeElapsed;

    if (bz_getCurrentTime() - LastWarn > 60) {
      tctf.adjTime = (int)(tctf.timeRemaining / 60);
      bz_sendTextMessagef (BZ_SERVER, Team, "%s Team: less than %i minute(s) left to capture a flag!", Color, tctf.adjTime + 1);
      return 1; // 1 = reset team's LastWarn
    }
    if (bz_getCurrentTime() - LastWarn > 30 && tctf.timeRemaining < 30) {
      bz_sendTextMessagef (BZ_SERVER, Team, "%s Team: less than 30 seconds left to capture a flag!", Color);
      return 1; // 1 = reset team's LastWarn
    }
    if (bz_getCurrentTime() - LastWarn > 10 && tctf.timeRemaining < 20 && tctf.timeRemaining > 10) {
      bz_sendTextMessagef (BZ_SERVER, Team, "%s Team: less than 20 seconds left to capture a flag!", Color);
      return 1; // 1 = reset team's LastWarn
    }
    if (bz_getCurrentTime() - LastWarn > 10 && tctf.timeRemaining < 10 && tctf.timeRemaining > 1) {
      bz_sendTextMessagef (BZ_SERVER, Team, "%s Team: less than 10 seconds left to capture a flag!", Color);
      return 1; // 1 = reset team's LastWarn
    }

    if (tctf.timeElapsed >= tctf.timeLimit) {

      KillTeam(Team);
      bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s team did not capture any other team flags in time.", Color);
      tctf.adjTime = (int)(tctf.timeLimit / 60 + 0.5);
      bz_sendTextMessagef (BZ_SERVER, Team, "CTF timer is reset to %i minutes for the %s team.", tctf.adjTime, Color);
      return 2; // 2 = reset team's LastWarn and LastTime
    }
  }

  return 0; // 0 = no need to reset teams LastWarn or LastTime
}

// reset time values if zero size team function

void ResetZeroTeams() {

  if (bz_getTeamCount(eRedTeam) == 0) {
    tctf.redLastTime = bz_getCurrentTime ();
    tctf.redLastWarn = bz_getCurrentTime ();
  }
  if (bz_getTeamCount(eGreenTeam) == 0) {
    tctf.greenLastTime = bz_getCurrentTime ();
    tctf.greenLastWarn = bz_getCurrentTime ();
  }
  if (bz_getTeamCount(eBlueTeam) == 0) {
    tctf.blueLastTime = bz_getCurrentTime ();
    tctf.blueLastWarn = bz_getCurrentTime ();
  }
  if (bz_getTeamCount(ePurpleTeam) == 0) {
    tctf.purpleLastTime = bz_getCurrentTime ();
    tctf.purpleLastWarn = bz_getCurrentTime ();
  }
  return;
}

bool OnlyOneTeamPlaying() {

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

void TCTFPlayerJoined ( bz_EventData *eventData )
{
  if (eventData->eventType != bz_ePlayerJoinEvent)
    return;

  bz_PlayerJoinPartEventData_V1 *JoinData = (bz_PlayerJoinPartEventData_V1*)eventData;

  // if teams are not even, notify joiner no CTF.
  // this should never be true if fair ctf is disabled (see definition of tctf.fairCTF):

  if (!tctf.fairCTF) {
    bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Capture The Flag disabled - teams are not evenly balanced.");
    return;
  }

  // if timed CTF turned off, but teams now even, let everyone know it's ok to cap.
  // if fair CTF is disabled, no need to notify:

  if (tctf.fairCTF && !tctf.enabled && tctf.fairCTFEnabled) {
    bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Capture The Flag enabled - teams are evenly balanced.");
    return;
  }

  // if timed CTF turned off, get outta here:

  if (!tctf.enabled)
    return;

  // if teams even, notify joiner how much time is left to CTF for their team:

  if (JoinData->record->team == eRedTeam  && tctf.timerRunning) {
    tctf.timeElapsed = bz_getCurrentTime () - tctf.redLastTime;
    tctf.timeRemaining = tctf.timeLimit - tctf.timeElapsed;
    tctf.adjTime = (int)(tctf.timeRemaining / 60);
    bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", tctf.adjTime + 1);
    return;
  }

  if (JoinData->record->team == eGreenTeam  && tctf.timerRunning) {
    tctf.timeElapsed = bz_getCurrentTime () - tctf.greenLastTime;
    tctf.timeRemaining = tctf.timeLimit - tctf.timeElapsed;
    tctf.adjTime = (int)(tctf.timeRemaining / 60);
    bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", tctf.adjTime + 1);
    return;
  }

  if (JoinData->record->team == eBlueTeam  && tctf.timerRunning) {
    tctf.timeElapsed = bz_getCurrentTime () - tctf.blueLastTime;
    tctf.timeRemaining = tctf.timeLimit - tctf.timeElapsed;
    tctf.adjTime = (int)(tctf.timeRemaining / 60);
    bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", tctf.adjTime + 1);
    return;
  }

  if (JoinData->record->team == ePurpleTeam  && tctf.timerRunning) {
    tctf.timeElapsed = bz_getCurrentTime () - tctf.purpleLastTime;
    tctf.timeRemaining = tctf.timeLimit - tctf.timeElapsed;
    tctf.adjTime = (int)(tctf.timeRemaining / 60);
    bz_sendTextMessagef (BZ_SERVER, JoinData->playerID, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", tctf.adjTime + 1);
    return;
  }

  return;
}

void TCTFFlagCapped ( bz_EventData *eventData )
{
  if (eventData->eventType != bz_eCaptureEvent)
    return;

  // if timed CTF turned off, get outta here:

  if (!tctf.enabled || !tctf.timerRunning)
    return;

  bz_CTFCaptureEventData_V1 *CapData = (bz_CTFCaptureEventData_V1*)eventData;

  // if team caps, reset their timer and notify their team

  if (CapData->teamCapping == eRedTeam) {
    tctf.adjTime = (int)(tctf.timeLimit / 60 + 0.5);
    bz_sendTextMessagef (BZ_SERVER, eRedTeam, "CTF timer is reset to %i minutes for the red team.", tctf.adjTime);
    tctf.redLastTime = bz_getCurrentTime ();
    tctf.redLastWarn = bz_getCurrentTime ();
    return;
  }
  if (CapData->teamCapping == eGreenTeam) {
    tctf.adjTime = (int)(tctf.timeLimit / 60 + 0.5);
    bz_sendTextMessagef (BZ_SERVER, eGreenTeam, "CTF timer is reset to %i minutes for the green team.", tctf.adjTime);
    tctf.greenLastTime = bz_getCurrentTime ();
    tctf.greenLastWarn = bz_getCurrentTime ();
    return;
  }
  if (CapData->teamCapping == eBlueTeam) {
    tctf.adjTime = (int)(tctf.timeLimit / 60 + 0.5);
    bz_sendTextMessagef (BZ_SERVER, eBlueTeam, "CTF timer is reset to %i minutes for the blue team.", tctf.adjTime);
    tctf.blueLastTime = bz_getCurrentTime ();
    tctf.blueLastWarn = bz_getCurrentTime ();
    return;
  }
  if (CapData->teamCapping == ePurpleTeam) {
    tctf.adjTime = (int)(tctf.timeLimit / 60 + 0.5);
    bz_sendTextMessagef (BZ_SERVER, ePurpleTeam, "CTF timer is reset to %i minutes for the purple team.", tctf.adjTime);
    tctf.purpleLastTime = bz_getCurrentTime ();
    tctf.purpleLastWarn = bz_getCurrentTime ();
    return;
  }

  return;
}

// this is where most of the decisions are made - a little clunky, but seems to work:

void TCTFTickEvents ( bz_EventData *eventData )
{
  if (eventData->eventType != bz_eTickEvent)
    return;

  // read this function once per tick event.  If fair CTF disabled, make it look fair to rest
  // of code - need to do this to be able to have timed ctf without fair ctf.

  tctf.fairCTF = (TeamsBalanced() || !tctf.fairCTFEnabled);

  // check/notify team balance changes while timed CTF is disabled.
  // if fair ctf is disabled, no need to check/notify about team balance changes:

  if (tctf.fairCTF && !tctf.notifiedCTFOK && !tctf.enabled && tctf.fairCTFEnabled) {

    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Capture The Flag enabled - teams are evenly balanced.");
    tctf.notifiedCTFOK = true;
    return;
  }

  if (!tctf.fairCTF && tctf.notifiedCTFOK && !tctf.enabled && tctf.fairCTFEnabled) {

    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Capture The Flag disabled - teams are not evenly balanced.");
    tctf.notifiedCTFOK = false;
    return;
  }

  // if no timed CTF, we can leave:

  if (!tctf.enabled)
    return;

  // if this is true, we can leave too:

  if (!tctf.fairCTF && !tctf.timerRunning)
    return;

  // check/notify team balance changes while timed CTF and fair CTF are enabled:

  if (!tctf.fairCTF && tctf.timerRunning && tctf.fairCTFEnabled) {

    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Capture The Flag disabled - teams are not evenly balanced.");
    tctf.timerRunning = false;
    ResetTeamData();
    return;
  }

  // no timed ctf with fair CTF disabled and only one team present:

  if (tctf.fairCTF && !tctf.fairCTFEnabled) {

    if (OnlyOneTeamPlaying()) {

      if (tctf.timerRunning)
	bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF disabled - not enough teams.");

      tctf.timerRunning = false;
      ResetTeamData();
      return;
    }
  }

  // start timing if we have made it this far:

  if (tctf.fairCTF && !tctf.timerRunning && !OnlyOneTeamPlaying()) {

    tctf.adjTime = (int)(tctf.timeLimit / 60 + 0.5);
    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF now in progress - capture a flag in less than %i minute(s)!", tctf.adjTime);
    tctf.timerRunning = true;
    ResetTeamData();
    return;
  }

  // everything is a go for timed ctf checks now.
  // check each team's time left, warn and kill if necessary:

  int RedReturn = TeamCheck(eRedTeam, "RED", tctf.redLastWarn, tctf.redLastTime);
  int GreenReturn = TeamCheck(eGreenTeam, "GREEN", tctf.greenLastWarn, tctf.greenLastTime);
  int BlueReturn = TeamCheck(eBlueTeam, "BLUE", tctf.blueLastWarn, tctf.blueLastTime);
  int PurpleReturn = TeamCheck(ePurpleTeam, "PURPLE", tctf.purpleLastWarn, tctf.purpleLastTime);

  if (RedReturn == 1)
    tctf.redLastWarn = bz_getCurrentTime();
  if (RedReturn == 2) {
    tctf.redLastWarn = bz_getCurrentTime();
    tctf.redLastTime = bz_getCurrentTime();
  }
  if (GreenReturn == 1)
    tctf.greenLastWarn = bz_getCurrentTime();
  if (GreenReturn == 2) {
    tctf.greenLastWarn = bz_getCurrentTime();
    tctf.greenLastTime = bz_getCurrentTime();
  }
  if (BlueReturn == 1)
    tctf.blueLastWarn = bz_getCurrentTime();
  if (BlueReturn == 2) {
    tctf.blueLastWarn = bz_getCurrentTime();
    tctf.blueLastTime = bz_getCurrentTime();
  }
  if (PurpleReturn == 1)
    tctf.purpleLastWarn = bz_getCurrentTime();
  if (PurpleReturn == 2) {
    tctf.purpleLastWarn = bz_getCurrentTime();
    tctf.purpleLastTime = bz_getCurrentTime();
  }

  ResetZeroTeams(); // reset team data for teams with no players.

  return;
}

void TCTFPlayerUpdates ( bz_EventData *eventData )
{
  if (eventData->eventType != bz_ePlayerUpdateEvent)
    return;

  // no CTF if teams not balanced, drop team flags asap:

  if (!tctf.fairCTF) {

    int playerID = ((bz_PlayerUpdateEventData_V1*)eventData)->playerID;
    const char* FlagHeld = bz_getPlayerFlag(playerID);

    if (FlagHeld != NULL) {

      if (strcmp(FlagHeld, "R*") == 0 || strcmp(FlagHeld, "G*") == 0 || strcmp(FlagHeld, "B*") == 0 || strcmp(FlagHeld, "P*") == 0 ) {
	bz_removePlayerFlag ( playerID );
	bz_sendTextMessagef (BZ_SERVER, playerID, "Capture The Flag disabled - teams are not evenly balanced.");
      }
    }

  }

  return;
}

bool TCTFCommands::SlashCommand ( int playerID, bz_ApiString _command, bz_ApiString _message, bz_APIStringList * /*_param*/ )
{
  std::string command = _command.c_str();
  std::string message = _message.c_str();

  bz_BasePlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);

  if (fromPlayer) {
    if (!fromPlayer->admin) {
      bz_sendTextMessage(BZ_SERVER,playerID,"You must be admin to use the ctfcaptime commands.");
      bz_freePlayerRecord(fromPlayer);
      return true;
    }

    bz_freePlayerRecord(fromPlayer);
  }

  if ( command == "tctfon") {

    tctf.enabled = true;
    if (!tctf.timerRunning)
      ResetTeamData();
    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF is enabled.");
    return true;
  }

  if ( command == "tctfoff") {

    tctf.enabled = false;
    tctf.timerRunning = false;
    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF is disabled.");
    return true;
  }

  if ( command == "fairctfon") {

    tctf.fairCTFEnabled = true;
    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Fair CTF is enabled.");
    return true;
  }

  if ( command == "fairctfoff") {

    tctf.fairCTFEnabled = false;
    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Fair CTF is disabled.");
    if (!tctf.timerRunning)
      ResetTeamData();
    return true;
  }

  if ( command == "tctfsoundon") {

    tctf.soundEnabled = true;
    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF sound is enabled.");
    return true;
  }

  if ( command == "tctfsoundoff") {

    tctf.soundEnabled = false;
    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Timed CTF sound is disabled.");
    return true;
  }

  if ( command == "tctfstatus") {

    if (tctf.enabled && !tctf.timerRunning)
      bz_sendTextMessagef (BZ_SERVER, playerID, "Timed CTF is currently enabled, but not running.");

    if (tctf.enabled && tctf.timerRunning)
      bz_sendTextMessagef (BZ_SERVER, playerID, "Timed CTF is currently enabled, and running");

    if (!tctf.enabled)
      bz_sendTextMessagef (BZ_SERVER, playerID, "Timed CTF is currently disabled.");

    if (!tctf.fairCTFEnabled)
      bz_sendTextMessagef (BZ_SERVER, playerID, "Fair CTF is currently disabled");

    if (tctf.fairCTFEnabled)
      bz_sendTextMessagef (BZ_SERVER, playerID, "Fair CTF is currently enabled");

    if (!tctf.soundEnabled)
      bz_sendTextMessagef (BZ_SERVER, playerID, "Timed CTF sounds are currently disabled");

    if (tctf.soundEnabled)
      bz_sendTextMessagef (BZ_SERVER, playerID, "Timed CTF sounds are currently enabled");

    tctf.adjTime = (int)(tctf.timeLimit/60 + 0.5);
    bz_sendTextMessagef (BZ_SERVER, playerID, "CTF capture time is currently set to: %i minutes", tctf.adjTime);
    return true;
  }

  // explicit time command handler:

  if ( command == "tctftime" ) {

    double inputvalue = ConvertToInt(message);

    if (inputvalue > 0) {

      tctf.timeLimit = inputvalue * 60;
      tctf.adjTime = (int)(tctf.timeLimit / 60 + 0.5);
      bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "CTF capture time has been set to %i minutes.", tctf.adjTime);

      if (!tctf.enabled)
	bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "(Timed CTF is still disabled)");

      ResetTeamData();
      return true;
    }
    else {
      bz_sendTextMessagef (BZ_SERVER, playerID, "CTF capture time invalid: must be between 1 and 120 minutes.");
      return true;
    }

    return true;
  }

  return false;
}

void TCTFHandler::Event( bz_EventData *eventData )
{
  if (eventData->eventType == bz_ePlayerUpdateEvent)
    TCTFPlayerUpdates(eventData);
  else if (eventData->eventType == bz_eTickEvent)
    TCTFTickEvents(eventData);
  else if (eventData->eventType == bz_eCaptureEvent)
    TCTFFlagCapped(eventData);
  else if (eventData->eventType == bz_ePlayerJoinEvent)
    TCTFPlayerJoined(eventData);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
