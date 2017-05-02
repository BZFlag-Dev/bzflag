// HoldTheFlag.cpp : bzfs plugin for Hold-The-flag game mode
//

#include "bzfsAPI.h"
#include <map>
#include <stdio.h>
#include <stdarg.h>

#define HOLDTHEFLAG_VER "1.00.02"
#define DO_FLAG_RESET 1

#define MAX_PLAYERID 255

typedef struct {
  bool isValid;
  int  score;
  char callsign[22];
  int capNum;
} HtfPlayer;

HtfPlayer Players[MAX_PLAYERID+1];
bool matchActive = false;
bool htfEnabled = true;
bz_eTeamType htfTeam = eNoTeam;
int nextCapNum = 0;
int NumPlayers=0;
int Leader;

class HTFscore : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
  virtual const char* Name() {return "Hold the Flag";}
  virtual void Init ( const char* config );
  virtual void Cleanup ( void );
  virtual void Event ( bz_EventData *eventData );
  virtual bool SlashCommand ( int playerID, bz_ApiString, bz_ApiString, bz_APIStringList*);
  bz_eTeamType colorNameToDef (const char *color);
  const char *colorDefToName (bz_eTeamType team);
};

BZ_PLUGIN(HTFscore)

HTFscore *htfScore = NULL;

bz_eTeamType HTFscore::colorNameToDef (const char *color)
{
  if (!strcasecmp (color, "green"))
    return eGreenTeam;
  if (!strcasecmp (color, "red"))
    return eRedTeam;
  if (!strcasecmp (color, "purple"))
    return ePurpleTeam;
  if (!strcasecmp (color, "blue"))
    return eBlueTeam;
  if (!strcasecmp (color, "rogue"))
    return eRogueTeam;
  if (!strcasecmp (color, "observer"))
    return eObservers;
  return eNoTeam;
}

const char *HTFscore::colorDefToName (bz_eTeamType team)
{
  switch (team) {
    case eGreenTeam:
      return ("Green");
    case eBlueTeam:
      return ("Blue");
    case eRedTeam:
      return ("Red");
    case ePurpleTeam:
      return ("Purple");
    case eObservers:
      return ("Observer");
    case eRogueTeam:
      return ("Rogue");
    case eRabbitTeam:
      return ("Rabbit");
    case eHunterTeam:
      return ("Hunters");
    case eAdministrators:
      return ("Administrators");
    default:
      return ("No Team");
  }
}

bool listAdd (int playerID, const char *callsign)
{
  if (playerID>MAX_PLAYERID || playerID<0)
    return false;
  Players[playerID].score = 0;
  Players[playerID].isValid = true;
  Players[playerID].capNum = -1;
  strncpy (Players[playerID].callsign, callsign, 20);
  ++NumPlayers;
  return true;
}

bool listDel (int playerID) {
  if (playerID>MAX_PLAYERID || playerID<0 || !Players[playerID].isValid)
    return false;
  Players[playerID].isValid = false;
  --NumPlayers;
  return true;
}


int sort_compare (const void *_p1, const void *_p2) {
  const int p1 = *(const int *)_p1;
  const int p2 = *(const int *)_p2;

  if (Players[p1].score != Players[p2].score)
    return Players[p2].score - Players[p1].score;
  return Players[p2].capNum - Players[p1].capNum;
}

void dispScores (int who)
{
  int sortList[MAX_PLAYERID+1];	 // do HtfPlayer *   !!
  int playerLastCapped = -1;
  int lastCapnum = -1;
  int x = 0;

  if (!htfEnabled)
    return;
  bz_sendTextMessage(BZ_SERVER, who, "**** HTF  Scoreboard ****");
  Leader = -1;
  if (NumPlayers<1)
    return;

  for (int i=0; i<MAX_PLAYERID; i++) {
    if (Players[i].isValid) {
      if (Players[i].capNum > lastCapnum) {
	playerLastCapped = i;
	lastCapnum = Players[i].capNum;
      }
      sortList[x++] = i;
    }
  }
  qsort (sortList, NumPlayers, sizeof(int), sort_compare);
  if (x != NumPlayers)
    bz_debugMessage(1, "++++++++++++++++++++++++ HTF INTERNAL ERROR: player count mismatch!");
  for (int i=0; i<NumPlayers; i++) {
    x = sortList[i];
    bz_sendTextMessagef(BZ_SERVER, who, "%20.20s :%3d %c", Players[x].callsign, Players[x].score,
			x == playerLastCapped ? '*' : ' ');
  }
  Leader = sortList[0];
}

void resetScores (void)
{
  for (int i=0; i<MAX_PLAYERID; i++) {
    Players[i].score = 0;
    Players[i].capNum = -1;
  }
  nextCapNum = 0;
}

void htfCapture (int who)
{
  if (!htfEnabled)
    return;

#if DO_FLAG_RESET
  bz_resetFlags ( false );
#endif

  bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "HTF FLAG CAPTURED by %s", Players[who].callsign);
  ++Players[who].score;
  Players[who].capNum = nextCapNum++;
  dispScores(BZ_ALLUSERS);
}

void htfStartGame (void)
{
  if (!htfEnabled)
    return;

  resetScores();
  matchActive = true;
  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "HTF MATCH has begun, good luck!");
}

void htfEndGame (void)
{
  if (htfEnabled && matchActive) {
    dispScores(BZ_ALLUSERS);
    bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "HTF MATCH has ended.");
    if (Leader >= 0)
      bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "%s is the WINNER !", Players[Leader].callsign);
  }

  matchActive = false;
}

void sendHelp (int who)
{
  bz_sendTextMessage(BZ_SERVER, who, "HTF commands: reset, off, on, stats");
}

/************************** (SUB)COMMAND Implementations ... **************************/

void htfStats (int who)
{
  bz_sendTextMessagef(BZ_SERVER, who, "HTF plugin version %s", HOLDTHEFLAG_VER);
  bz_sendTextMessagef(BZ_SERVER, who,  "  Team: %s", htfScore->colorDefToName(htfTeam));
  bz_sendTextMessagef(BZ_SERVER, who,  "  Flag Reset: %s", DO_FLAG_RESET ? "ENabled" : "DISabled");
}

void htfReset (int who)
{
  resetScores();
  bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "*** HTF scores reset by %s", Players[who].callsign);
}

void htfEnable (bool onoff, int who)
{
  char msg[255];
  if (onoff == htfEnabled) {
    bz_sendTextMessage(BZ_SERVER, who, "HTF mode is already that way.");
    return;
  }
  htfEnabled = onoff;
  sprintf (msg, "*** HTF mode %s by %s", onoff ? "ENabled" : "DISabled", Players[who].callsign);
  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, msg);
}

// handle events
void HTFscore::Event ( bz_EventData *eventData )
{
  // player JOIN
  if (eventData->eventType == bz_ePlayerJoinEvent) {
    bz_PlayerJoinPartEventData_V1 *joinData = (bz_PlayerJoinPartEventData_V1*)eventData;
bz_debugMessagef(3, "++++++ HTFscore: Player JOINED (ID:%d, TEAM:%d, CALLSIGN:%s)", joinData->playerID, joinData->record->team, joinData->record->callsign.c_str()); fflush (stdout);
    if (htfTeam!=eNoTeam && joinData->record->team!=htfTeam && joinData->record->team != eObservers) {
      char msg[255];
      sprintf (msg, "HTF mode enabled, you must join the %s team to play", htfScore->colorDefToName(htfTeam));
      bz_kickUser (joinData->playerID, msg, true);
      return;
    }
    if (joinData->record->team == htfTeam)
      listAdd (joinData->playerID, joinData->record->callsign.c_str());

  // player PART
  } else if (eventData->eventType == bz_ePlayerPartEvent) {
    bz_PlayerJoinPartEventData_V1 *joinData = (bz_PlayerJoinPartEventData_V1*)eventData;
bz_debugMessagef(3, "++++++ HTFscore: Player PARTED (ID:%d, TEAM:%d, CALLSIGN:%s)", joinData->playerID, joinData->record->team, joinData->record->callsign.c_str()); fflush (stdout);

    if (joinData->record->team== htfTeam)
      listDel (joinData->playerID);

  // flag CAPTURE
  } else if (eventData->eventType == bz_eCaptureEvent) {
    bz_CTFCaptureEventData_V1 *capData = (bz_CTFCaptureEventData_V1*)eventData;
    htfCapture (capData->playerCapping);

  // game START
  } else if (eventData->eventType == bz_eGameStartEvent) {
    bz_GameStartEndEventData_V1 *msgData = (bz_GameStartEndEventData_V1*)eventData;
bz_debugMessagef(2, "++++++ HTFscore: Game START (%f, %f)", msgData->eventTime, msgData->duration); fflush (stdout);
    htfStartGame ();

  // game END
  } else if (eventData->eventType == bz_eGameEndEvent) {
    bz_GameStartEndEventData_V1 *msgData = (bz_GameStartEndEventData_V1*)eventData;
bz_debugMessagef(2, "++++++ HTFscore: Game END (%f, %f)", msgData->eventTime, msgData->duration); fflush (stdout);
    htfEndGame ();
  }
}



bool checkPerms (int playerID, const char *htfCmd, const char *permName)
{
  if (bz_hasPerm (playerID, permName))
    return true;
  bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "you need \"%s\" permission to do /htf %s", permName, htfCmd);
  return false;
}



// handle /htf command
bool HTFscore::SlashCommand ( int playerID, bz_ApiString cmd, bz_ApiString, bz_APIStringList* cmdParams )
{
  char subCmd[6];
  if (strcasecmp (cmd.c_str(), "htf"))   // is it for me ?
    return false;
  if (cmdParams->get(0).c_str()[0] == '\0') {
    dispScores (playerID);
    return true;
  }

  strncpy (subCmd, cmdParams->get(0).c_str(), 5);
  subCmd[4] = '\0';
  if (strcasecmp (subCmd, "rese") == 0) {
    if (checkPerms (playerID, "reset", "COUNTDOWN"))
      htfReset (playerID);
  } else if (strcasecmp (subCmd, "off") == 0) {
    if (checkPerms (playerID, "off", "HTFONOFF"))
      htfEnable (false, playerID);
  } else if (strcasecmp (subCmd, "on") == 0) {
    if (checkPerms (playerID, "off", "HTFONOFF"))
      htfEnable (true, playerID);
  } else if (strcasecmp (subCmd, "stat") == 0)
    htfStats (playerID);
  else
    sendHelp (playerID);
  return true;
}


bool commandLineHelp (void) {
  const char *help[] = {
    "Command line args:  PLUGINNAME,team=color",
    NULL
  };
  bz_debugMessage (0, "+++ HoldTheFlag plugin command-line error");
  for (int x=0; help[x]!=NULL; x++)
    bz_debugMessage (0, help[x]);
  return true;
}


bool parseCommandLine (const char *cmdLine)
{
  if (cmdLine==NULL || *cmdLine=='\0')
    return false;
  htfTeam = eGreenTeam;
  if (strcasecmp (cmdLine, "team=") == 0) {
    if ((htfTeam = htfScore->colorNameToDef(cmdLine+5)) == eNoTeam) {
      return commandLineHelp ();
    }
  }
  return false;
}


void HTFscore::Init(const char* cmdLine)
{
  htfScore = this;

  if (parseCommandLine (cmdLine))
    return;

  // get current list of player indices ...
  bz_APIIntList *playerList = bz_newIntList();
  bz_getPlayerIndexList (playerList);
  for (unsigned int i = 0; i < playerList->size(); i++) {
    bz_BasePlayerRecord *playerRecord = bz_getPlayerByIndex (playerList->get(i));
    if (playerRecord != NULL) {
      listAdd (playerList->get(i), playerRecord->callsign.c_str());
    }
    bz_freePlayerRecord (playerRecord);
  }
  bz_deleteIntList (playerList);

  bz_registerCustomSlashCommand ("htf", this);
  Register(bz_ePlayerJoinEvent);
  Register(bz_ePlayerPartEvent);
  Register(bz_eCaptureEvent);
  Register(bz_eGameStartEvent);
  Register(bz_eGameEndEvent);
}

void HTFscore::Cleanup(void)
{
	htfScore = NULL;
	Flush();
  bz_removeCustomSlashCommand ("htf");
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
