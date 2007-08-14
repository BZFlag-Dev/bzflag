// nagware.cpp : 'Nagware' plugin to encourage player registration
//
// $Id$

#include "bzfsAPI.h"
#include <vector>

BZ_GET_PLUGIN_VERSION

#define NAGWAREPLUG_VER "1.00.01"
#define MAX_PLAYERID    255
#define EVENT_FREQUENCY 15      // number of seconds between checks
#define TIME_FACTOR     60      // number of seconds per minute (useful to decrease for testing)


// TODO: check for msgs > 128 chars and warn ( readConfig() )


struct st_MsgEnt{
  st_MsgEnt(int t, int r, std::string m): time(t), repeat(r), msg(m) {}
  st_MsgEnt() {}
  int time;
  int repeat;
  std::string msg;
};
typedef struct st_MsgEnt MsgEnt;


typedef struct {
  char permName[31];
  bool enableObs;
  int  minPlayers;
  MsgEnt *kickMsg;
  std::vector <MsgEnt *> nagMsgs;
  std::string msgSuffix;
} NagConfig;

NagConfig Config;


typedef struct {
  bool isValid;
  char callsign[22];
  bz_eTeamType team;
  double  joinTime;
  double  nextEventTime;
  MsgEnt *nextEventMsg;
  bool    isVerified;
} NagPlayer;


NagPlayer Players[MAX_PLAYERID+1];
int       NumPlayers=0;
int       MaxUsedID=0;
bool      NagEnabled = true;
double    MatchStartTime = 0;
char      ConfigFilename[256] = "";
float     NextEventTime = 0.0f;


class Nagware : public bz_EventHandler, public bz_CustomSlashCommandHandler
{
  public:
    virtual void process ( bz_EventData *eventData );
    virtual bool handle ( int playerID, bz_ApiString, bz_ApiString, bz_APIStringList*);

  protected:

  private:
};

Nagware nagware;	// 'my' instance


bool readConfig (char *filename, NagConfig *cfg, int playerID);


double nextRepeat (double playerTime, MsgEnt *m){
  if (m->repeat == 0)
    return 0;
  int last = (int)((playerTime - m->time) / m->repeat);
  return (m->time + (m->repeat * (last+1)));
}

void updatePlayerNextEvent (int playerID, double now){
  unsigned int idx;
  double playerTime =  now - Players[playerID].joinTime;
  double repeat;

  if (!Players[playerID].isValid || Players[playerID].isVerified)
    return;

  Players[playerID].nextEventTime = -1;
  if (Config.nagMsgs.size() == 0)
    return;

  for (idx=0; idx<Config.nagMsgs.size(); idx++){
    if (Config.nagMsgs[idx]->time > playerTime){
      if (idx > 0 && (repeat = nextRepeat (playerTime, Config.nagMsgs[idx-1])) > 0
      && repeat < Config.nagMsgs[idx]->time){
	Players[playerID].nextEventTime = Players[playerID].joinTime + repeat;
	Players[playerID].nextEventMsg = Config.nagMsgs[idx-1];
      } else {
	Players[playerID].nextEventTime = Players[playerID].joinTime + Config.nagMsgs[idx]->time;
	Players[playerID].nextEventMsg = Config.nagMsgs[idx];
      }
      break;
    }
  }

  if (Players[playerID].nextEventTime < 0
  &&  (repeat = nextRepeat (playerTime, Config.nagMsgs[Config.nagMsgs.size()-1])) > 0){
    Players[playerID].nextEventTime = Players[playerID].joinTime + repeat;
    Players[playerID].nextEventMsg = Config.nagMsgs[Config.nagMsgs.size()-1];
  }
}


void sendNagMessage (int who, std::string *msg ){
  std::string fullMsg = *msg + Config.msgSuffix;
  unsigned int idx=0, x;

  while ((x = (unsigned int)fullMsg.find("\\n", idx)) != std::string::npos){
    bz_sendTextMessage(BZ_SERVER, who, fullMsg.substr(idx, x-idx).c_str());
    idx = x+2;
  }
  bz_sendTextMessage(BZ_SERVER, who, fullMsg.substr(idx).c_str());
}


void tickEvent (float time)
{
  int x;
  if (time < NextEventTime || !NagEnabled || MatchStartTime!=0.0)
    return;

  for (x=0; x<=MaxUsedID; x++){
    if (Players[x].isValid && !Players[x].isVerified && Players[x].nextEventTime>=0 && time>Players[x].nextEventTime){
      sendNagMessage(x, &Players[x].nextEventMsg->msg);
      updatePlayerNextEvent (x, time);
    }
  }
  if (Config.kickMsg->time>0  && NumPlayers>=Config.minPlayers){  // kick someone !
    double kicktime = Config.kickMsg->time;
    for (x=0; x<=MaxUsedID; x++)
      if (Players[x].isValid && !Players[x].isVerified && time>(Players[x].joinTime+kicktime)){
	bz_kickUser (x, Config.kickMsg->msg.c_str(), true);
	break;
      }
  }

  NextEventTime = time + (float)EVENT_FREQUENCY;
}


void dispNagMsg (int who, char *label, MsgEnt *m){
  char msg[140];

  if (m->repeat)
    sprintf (msg, "%s msg: %d (%d): ", label, m->time, m->repeat);
  else
    sprintf (msg, "%s msg: %d: ", label, m->time);
  strncat (msg, m->msg.c_str(), 130);
  if (strlen (msg) > 124)   // max line len is currently 125 (not 128!)
    strcpy (&msg[122], "...");
  bz_sendTextMessage (BZ_SERVER, who, msg);
}


void nagShowConfig (int who)
{
  unsigned int x;

  bz_sendTextMessage(BZ_SERVER, who, "nagware plugin configuration .........");
  bz_sendTextMessagef(BZ_SERVER, who, "perm name: %s", Config.permName);
  bz_sendTextMessagef(BZ_SERVER, who, "min players: %d", Config.minPlayers);
   if (Config.enableObs)
    bz_sendTextMessage(BZ_SERVER, who, "Observer kick is ENABLED");
  else
    bz_sendTextMessage(BZ_SERVER, who, "Observer kick is DISABLED");
  if (Config.msgSuffix.size() > 0 )
    bz_sendTextMessagef(BZ_SERVER, who, "message suffix: %s", Config.msgSuffix.c_str());
  for (x=0; x<Config.nagMsgs.size(); x++)
    dispNagMsg (who, "nag ", Config.nagMsgs[x]);
  if (Config.kickMsg != NULL)
    dispNagMsg (who, "kick", Config.kickMsg);
  if (NagEnabled)
    bz_sendTextMessage(BZ_SERVER, who, "(plugin is currently ENabled)");
  else
    bz_sendTextMessage(BZ_SERVER, who, "(plugin is currently DISabled)");
}


void nagEnable (bool enable, int who)
{
  NagEnabled = enable;
  bz_sendTextMessage(BZ_SERVER, who, "OK.");
}


void nagList (int who)
{
  int numUnverified = 0;
  int x, timeOn;
  double now = bz_getCurrentTime();

  bz_sendTextMessage (BZ_SERVER, who, "Callsign (unverified)    Time ON");
  for (x=0; x<=MaxUsedID; x++){
    if (Players[x].isValid && !Players[x].isVerified){
      timeOn = (int)(now - Players[x].joinTime);
      bz_sendTextMessagef (BZ_SERVER, who, "%-25.25s %3d:%02d", Players[x].callsign, timeOn/60, timeOn%60);
      ++numUnverified;
    }
  }
  if (numUnverified == 0)
    bz_sendTextMessage (BZ_SERVER, who, "--- NO unverified players.");
}


void nagReload (int who)
{
  if ( readConfig (ConfigFilename, &Config, who) ){
    bz_sendTextMessage(BZ_SERVER, who, "nagware config error, plugin disabled.");
    NagEnabled = false;
  } else {
    bz_sendTextMessage(BZ_SERVER, who, "nagware config reloaded.");
    // RECALC all player nextevents ...
    double now = bz_getCurrentTime();
    int x;
    for (x=0; x<MaxUsedID; x++)
      if (Players[x].isValid && !Players[x].isVerified)
	updatePlayerNextEvent (x, now);
  }
}


bool listAdd (int playerID, const char *callsign, bz_eTeamType team, bool verified, double time)
{
  if (playerID>MAX_PLAYERID || playerID<0)
    return false;
  Players[playerID].isValid = true;
  Players[playerID].team = team;
  Players[playerID].isVerified = verified;
  strncpy (Players[playerID].callsign, callsign, 20);
  Players[playerID].joinTime = time;
  if (Config.nagMsgs.size() == 0)
    Players[playerID].nextEventTime = -1;
  else {
    Players[playerID].nextEventTime = time + (Config.nagMsgs[0]->time);
    Players[playerID].nextEventMsg = Config.nagMsgs[0];
  }
  ++NumPlayers;
  if (playerID > MaxUsedID)
    MaxUsedID = playerID;
  return true;
}

bool listDel (int playerID)
{
  if (playerID>MAX_PLAYERID || playerID<0 || !Players[playerID].isValid)
    return false;
  Players[playerID].isValid = false;
  --NumPlayers;
  return true;
}

void sendHelp (int who)
{
  bz_sendTextMessage(BZ_SERVER, who, "NAG commands: off, on, config, reload, list");
}

bool checkPerms (int playerID, char *nagCmd, const char *permName)
{
  if (permName==NULL || *permName=='\0')
    permName = "NAG";
  if (bz_hasPerm (playerID, permName))
    return true;
  bz_sendTextMessagef (BZ_SERVER, playerID, "you need \"%s\" permission to do /nag %s", permName, nagCmd);
  return false;
}


/*
 *  Event handlers ....
*/

// handle events
void Nagware::process ( bz_EventData *eventData )
{
  // player JOIN
  if (eventData->eventType == bz_ePlayerJoinEvent) {
    bz_PlayerJoinPartEventData_V1 *joinData = (bz_PlayerJoinPartEventData_V1*)eventData;
    bz_debugMessagef(4, "+++ nagware: Player JOINED (ID:%d, TEAM:%d, CALLSIGN:%s)", joinData->playerID, joinData->record->team, joinData->record->callsign.c_str()); fflush (stdout);
    listAdd (joinData->playerID, joinData->record->callsign.c_str(), joinData->record->team, joinData->record->verified, joinData->eventTime);

  // player PART
  } else if (eventData->eventType == bz_ePlayerPartEvent) {
    bz_PlayerJoinPartEventData_V1 *joinData = (bz_PlayerJoinPartEventData_V1*)eventData;
    bz_debugMessagef(4, "+++ nagware: Player PARTED (ID:%d, TEAM:%d, CALLSIGN:%s)", joinData->playerID, joinData->record->team, joinData->record->callsign.c_str()); fflush (stdout);
    listDel (joinData->playerID);

  // game START
  } else if (eventData->eventType == bz_eGameStartEvent) {
    bz_GameStartEndEventData_V1 *msgData = (bz_GameStartEndEventData_V1*)eventData;
    bz_debugMessagef(4, "+++ nagware: Game START (%f, %f)", msgData->eventTime, msgData->duration); fflush (stdout);
    MatchStartTime = msgData->eventTime;

  // game END
  } else if (eventData->eventType == bz_eGameEndEvent) {
    bz_GameStartEndEventData_V1 *msgData = (bz_GameStartEndEventData_V1*)eventData;
    bz_debugMessagef(4, "+++ nagware: Game END (%f, %f)", msgData->eventTime, msgData->duration); fflush (stdout);
    MatchStartTime = 0.0f;
    // can determine length of match, and adjust event times if needed.

  // tick
  } else if (eventData->eventType == bz_eTickEvent) {
    bz_TickEventData_V1 *msgData = (bz_TickEventData_V1*)eventData;
    tickEvent ((float)msgData->eventTime);

  }
}

// handle /nag command
bool Nagware::handle ( int playerID, bz_ApiString cmd, bz_ApiString, bz_APIStringList* cmdParams )
{
  char subCmd[6];
  if (strcasecmp (cmd.c_str(), "nag"))   // is it for me ?
    return false;

  if (cmdParams->get(0).c_str()[0] == '\0'){
    sendHelp (playerID);
    return true;
  }

  strncpy (subCmd, cmdParams->get(0).c_str(), 5);
  subCmd[4] = '\0';
  if (strcasecmp (subCmd, "conf") == 0){
    if (checkPerms (playerID, "config", Config.permName))
      nagShowConfig (playerID);
  } else if (strcasecmp (subCmd, "off") == 0){
    if (checkPerms (playerID, "off", Config.permName))
      nagEnable (false, playerID);
  } else if (strcasecmp (subCmd, "on") == 0){
    if (checkPerms (playerID, "on", Config.permName))
      nagEnable (true, playerID);
  } else if (strcasecmp (subCmd, "relo") == 0){
    if (checkPerms (playerID, "reload", Config.permName))
      nagReload (playerID);
  } else if (strcasecmp (subCmd, "list") == 0){
    if (checkPerms (playerID, "list", Config.permName))
      nagList (playerID);
  } else
    sendHelp (playerID);
  return true;
}


/*
 * Plugin load and unload...
*/

bool commandLineHelp (void)
{
  const char *help[] = {
    "Command line args:  PLUGINNAME,configname",
    "nagware plugin NOT loaded!",
    NULL
  };
  bz_debugMessage (0, "+++ nagware plugin command-line error.");
  for (int x=0; help[x]!=NULL; x++)
    bz_debugMessage (0, help[x]);
  return true;
}


bool parseCommandLine (const char *cmdLine)
{
  if (cmdLine==NULL || *cmdLine=='\0')
    return commandLineHelp ();

  strncpy (ConfigFilename, cmdLine, 255);
  if (readConfig(ConfigFilename, &Config, -1)){
    bz_debugMessage (0, "+++ nagware plugin config file error, plugin NOT loaded");
    return true;
  }
  return false;
}

BZF_PLUGIN_CALL int bz_Load (const char* cmdLine)
{
  bz_BasePlayerRecord *playerRecord;
  double now = bz_getCurrentTime();

  if (parseCommandLine (cmdLine))
    return -1;

  // get current list of player indices ...
  bz_APIIntList *playerList = bz_newIntList();
  bz_getPlayerIndexList (playerList);
  for (unsigned int i = 0; i < playerList->size(); i++){
    if ((playerRecord = bz_getPlayerByIndex (playerList->get(i))) != NULL){
      listAdd (playerList->get(i), playerRecord->callsign.c_str(), playerRecord->team, playerRecord->verified, now);
      bz_freePlayerRecord (playerRecord);
    }
  }
  bz_deleteIntList (playerList);

  bz_registerCustomSlashCommand ("nag", &nagware);
  bz_registerEvent(bz_ePlayerJoinEvent, &nagware);
  bz_registerEvent(bz_ePlayerPartEvent, &nagware);
  bz_registerEvent(bz_eGameStartEvent, &nagware);
  bz_registerEvent(bz_eGameEndEvent, &nagware);
  bz_registerEvent(bz_eTickEvent, &nagware);
  bz_setMaxWaitTime (1.0f);

  bz_debugMessagef(0, "+++ nagware plugin loaded - v%s", NAGWAREPLUG_VER);
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload (void)
{
  bz_removeCustomSlashCommand ("nag");
  bz_removeEvent (bz_ePlayerJoinEvent, &nagware);
  bz_removeEvent (bz_ePlayerPartEvent, &nagware);
  bz_removeEvent (bz_eGameStartEvent, &nagware);
  bz_removeEvent (bz_eGameEndEvent, &nagware);
  bz_removeEvent (bz_eTickEvent, &nagware);
  bz_debugMessage(0, "+++ nagware plugin unloaded");
  return 0;
}


/*
 * Read Configuration file...
*/

bool configError (char *msg, int linenum, int playerID, FILE *fp){
  char send[256];
  fclose (fp);
  sprintf (send, "+++ nagware config file error (%s) at line #%d", msg, linenum);
  bz_debugMessagef(0, send);
  if (playerID >=0)
    bz_sendTextMessage(BZ_SERVER, playerID, send);
  return true;
}


char *strtrim (char *s){
  char c;
  char *p;
  while (*s == ' ')
    ++s;
  p = strlen(s) + s -1;
  while ( ((c=*p)==' ' || c=='\n') && p>s)
    *p--='\0';
  return s;
}

MsgEnt * parseCfgMessage(const char *m){
  char *p;
  int time, repeat=0;

  if ((p = strchr (m, ' ')) == NULL)
    return NULL;
  *p = '\0';
  if (strchr (m, ',') != NULL){
    if (sscanf (m, "%d,%d", &time, &repeat) != 2)
      return NULL;
  } else {
    if (sscanf (m, "%d", &time) != 1)
      return NULL;
  }
  if (time<0 || time > 500 || repeat < 0 || repeat > 1000)
    return NULL;

// TODO: check linelen < 128

  return new MsgEnt (time*TIME_FACTOR, repeat*TIME_FACTOR, p+1);
}


int compareMsgEnt (const void *a, const void *b){
  return (*(MsgEnt**)a)->time - (*(MsgEnt**)b)->time;
}


bool readConfig (char *filename, NagConfig *cfg, int playerID){
  FILE *cfile = fopen (filename, "r");
  MsgEnt *md;
  int lineNum=0;
  char line[1026];
  char *p, *key, *val;

  if (cfile == NULL){
    sprintf (line, "+++ Error opening nagware config file (%s)", filename);
    bz_debugMessagef(0, line);
    if (playerID >=0)
      bz_sendTextMessage(BZ_SERVER, playerID, line);
    return true;
  }

  // install defaults ...
  strcpy (cfg->permName, "NAG");
  cfg->enableObs = false;
  cfg->minPlayers = 0;
  cfg->msgSuffix = "";
  cfg->nagMsgs.clear();

  while ( fgets (line, 1024, cfile) != NULL ){
    ++lineNum;
    if (line[0]=='#' || strlen(line)<2)
      continue;

    if ((p = strchr (line, '=')) == NULL)
      return configError ("no '='", lineNum, playerID, cfile);
    *p = '\0';
    key = strtrim (line);
    val = strtrim (++p);

    if (!strcasecmp (key, "permname")){
      strncpy (cfg->permName, val, 30);
    } else if (!strcasecmp (key, "kickobs")){
      if ( !strcasecmp(val, "yes") || !strcasecmp(val, "true") )
	cfg->enableObs = true;
      else
	cfg->enableObs = false;
    } else if (!strcasecmp (key, "minplayers")){
      if (sscanf (val, "%d", &cfg->minPlayers)!=1 || cfg->minPlayers<1 || cfg->minPlayers>100)
	return configError ("Invalid minplayers value", lineNum, playerID, cfile);
    } else if (!strcasecmp (key, "messagesuffix")){
      cfg->msgSuffix = std::string (val);
    } else if (!strcasecmp (key, "message")){
      if ((md = parseCfgMessage (val)) == NULL)
	return configError ("Invalid message format", lineNum, playerID, cfile);
      cfg->nagMsgs.push_back (md);
    } else if (!strcasecmp (key, "kickmessage")){
      if ((md = parseCfgMessage (val)) == NULL)
	return configError ("Invalid kick message format", lineNum, playerID, cfile);
      cfg->kickMsg = md;
    } else {
      return configError ("unknown tag", lineNum, playerID, cfile);
    }
  }

  // sort the nagmsgs vector by time
  qsort (&cfg->nagMsgs[0], cfg->nagMsgs.size(), sizeof(MsgEnt *), compareMsgEnt);

  fclose (cfile);
  return false;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
