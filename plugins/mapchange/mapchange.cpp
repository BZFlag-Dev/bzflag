// mapchange.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <math.h>

class Game
{
public:
  bool hasBeenPlayed;

  std::string mapFile;
  std::string publicText;
  Game()
  {
    hasBeenPlayed = false;
  }
};

typedef enum
{
  eTimedGame,
  eMaxKillScore,
  eMaxCapScore,
  eNoPlayers, 
  eManual
}EndCond;

typedef enum
{
  eLoopInf,
  eRandomInf,
  eRandomOnce,
  eNoLoop
}CycleMode; 

EndCond		    endCond;
double		    timeLimit;
int		    scoreCapLimit;
double		    startTime;
int		    currentIndex;

std::vector<Game>   gameList;
CycleMode	    cycleMode;

class MapChangeEventHandler : public bz_EventHandler
{
public:
  virtual void process ( bz_EventData *eventData );
};

class MapChangeCommandHandler : public bz_CustomSlashCommandHandler
{
public:
  virtual bool handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params );
  virtual const char* help ( bz_ApiString command  );
};

MapChangeCommandHandler	slash;
MapChangeEventHandler handler;

EndCond condFromString ( const std::string &str )
{
  if ( str == "timed")
    return eTimedGame;
  else if ( str == "maxkill")
    return eMaxKillScore;
  else if ( str == "maxcap")
    return eMaxCapScore;
  else if ( str == "empty")
    return eNoPlayers;
  else if ( str == "manual")
    return eManual;

  return eTimedGame;
}

const char* condToString ( EndCond cond )
{
  if ( cond == eTimedGame)
    return "Timed";
  else if ( cond == eMaxKillScore)
    return "MaxKill";
  else if ( cond == eMaxCapScore)
    return "MaxCap";
  else if ( cond == eNoPlayers)
    return "Empty";
  else if ( cond == eManual)
    return "Manual";

  return "Timed";
}

CycleMode cycleFromString ( const std::string &str )
{
  if ( str == "loop")
    return eLoopInf;
  else if ( str == "random")
    return eRandomInf;
  else if ( str == "onerand")
    return eRandomOnce;

  return eNoLoop;
}

const char* cycleToString ( CycleMode mode )
{
  if ( mode == eLoopInf )
    return "Loop";
  else if (mode == eRandomInf )
    return "Random";
  else if ( mode == eRandomOnce )
    return "OneRand";
  
  return "NoLoop";
}

bool loadGamesFromFile ( const char* config )
{
  FILE *fp = fopen(config,"rb");
  if(!fp)
    return false;
  fseek(fp,0,SEEK_END);

  std::string text;
  unsigned int size = ftell(fp);
  fseek(fp,0,SEEK_SET);

  char *temp = (char*)malloc(size+1);
  fread(temp,size,1,fp);
  fclose(fp);
  temp[size] = 0;
  text = temp;
  free(temp);

  endCond = eTimedGame;
  timeLimit = 30.0*60.0;
  scoreCapLimit = 10;

  startTime = -1;
  currentIndex = -1;
  cycleMode = eLoopInf;

  std::vector<std::string> lines = tokenize(text,std::string("\r\n"),0,false);

  for ( unsigned int i = 0; i < (unsigned int)lines.size(); i++ )
  {
    std::string line = lines[i];

    std::vector<std::string> params = tokenize(line,std::string(","),0,true);

    if (params.size())
    {
      if (tolower(params[0]) == "mode")
      {
	if ( params.size() > 1 )
		endCond = condFromString(tolower(params[1]));
	if ( params.size() > 2 )
	{
	  if (endCond == eTimedGame )
	    timeLimit = fabs(atof(params[2].c_str()))*60.0;
	  else
	    scoreCapLimit = atoi(params[2].c_str());

	  if (timeLimit == 0.0)
	    timeLimit = 30;

	  if ( scoreCapLimit <= 0 )
	    scoreCapLimit = 10;
	}

	if ( params.size() > 3)
	  cycleMode = cycleFromString(tolower(params[3]));
      }
      else
      {
	Game  game;
	game.mapFile = params[0];
	if ( params.size() > 1 )
	  game.publicText = params[1];

	gameList.push_back(game);
      }
    }
  }
  return gameList.size() >0;
}

bool anyPlayers ( void )
{
  bool moreThenZero = false;
  bz_APIIntList *players = bz_getPlayerIndexList();
  if ( players->size() )
    moreThenZero = true;

  bz_deleteIntList(players);

  return moreThenZero;
}

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
  if(!commandLine || !strlen(commandLine))
  {
    bz_debugMessage(0,"mapchange plugin requires a config file as a param and will not load");
    return -1;
  }

  if(!loadGamesFromFile(commandLine))
  {
    bz_debugMessage(0,"mapchange plugin config file failure, aborting load");
    return -1;
  }

  bz_debugMessage(4,"mapchange plugin loaded");

  if ( anyPlayers() )
    startTime = bz_getCurrentTime();

  bz_registerEvent ( bz_ePlayerJoinEvent, &handler );
  bz_registerEvent ( bz_ePlayerDieEvent, &handler );
  bz_registerEvent ( bz_ePlayerPartEvent, &handler );
  bz_registerEvent ( bz_eCaptureEvent, &handler );
  bz_registerEvent ( bz_eGetWorldEvent, &handler );
  bz_registerEvent ( bz_eTickEvent, &handler );
  bz_registerEvent ( bz_eListServerUpdateEvent, &handler );

  bz_registerCustomSlashCommand ( "mapnext",&slash );
  bz_registerCustomSlashCommand ( "mapreset",&slash );
  bz_registerCustomSlashCommand ( "mapcyclemode",&slash );
  bz_registerCustomSlashCommand ( "mapendmode",&slash );
  bz_registerCustomSlashCommand ( "maplist",&slash );
  bz_registerCustomSlashCommand ( "maplimit",&slash );

 return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
  bz_removeEvent ( bz_ePlayerJoinEvent, &handler );
  bz_removeEvent ( bz_ePlayerDieEvent, &handler );
  bz_removeEvent ( bz_ePlayerPartEvent, &handler );
  bz_removeEvent ( bz_eCaptureEvent, &handler );
  bz_removeEvent ( bz_eGetWorldEvent, &handler );
  bz_removeEvent ( bz_eTickEvent, &handler );
  bz_removeEvent ( bz_eListServerUpdateEvent, &handler );

  bz_removeCustomSlashCommand ( "mapnext" );
  bz_removeCustomSlashCommand ( "mapreset" );
  bz_removeCustomSlashCommand ( "mapcyclemode" );
  bz_removeCustomSlashCommand ( "mapendmode" );
  bz_removeCustomSlashCommand ( "maplist" );
  bz_removeCustomSlashCommand ( "maplimit" );

  bz_debugMessage(4,"mapchange plugin unloaded");
  return 0;
}

void resetGames ( void )
{
  for ( int i = 0; i < (int)gameList.size(); i++)
    gameList[i].hasBeenPlayed = false;
}

int findRandomUnplayedGame ( void )
{
  int count = 1;

  int randGame = rand()%(int)gameList.size();

  while (gameList[randGame].hasBeenPlayed || count < (int)gameList.size())
  {
    count++;
    randGame = rand()%(int)gameList.size();
  }

  if (count >= (int)gameList.size())
    return -1;

  return randGame;
}

void sendMapChangeMessage ( bool end )
{
  std::string message = "Map change!\n";
  bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,message.c_str());

  if (end)
    message = "Good bye!\n";
  else
     message = "Please Rejoin!\n";
  bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,message.c_str());
}

void nextMap ( void )
{
  bool shutdown = false;
  // compute the next map, and set the index
  switch(cycleMode)
  {
    case eLoopInf:
      currentIndex++;
      if (currentIndex >= (int)gameList.size())
      {
	resetGames();
	currentIndex = 0;
      }
      break;
    case eRandomInf:
      currentIndex = findRandomUnplayedGame();
      if ( currentIndex < 0 )
      {
	resetGames();
	currentIndex = findRandomUnplayedGame();
      }
      break;

    case eRandomOnce:
       currentIndex = findRandomUnplayedGame();
       if ( currentIndex < 0 )
       {
	  shutdown = true;
	  currentIndex = -1;
       }
       break;

    case eNoLoop:
      currentIndex++;
      if (currentIndex >= (int)gameList.size())
      {
	shutdown = true;
	currentIndex = 0;
      }
      break;
  }

  sendMapChangeMessage(shutdown);

  if (shutdown)
    bz_shutdown();
  else
    bz_restart();

  startTime = -1;
}

void MapChangeEventHandler::process ( bz_EventData *eventData )
{
  if (!eventData)
    return;

  switch(eventData->eventType)
  {
    case bz_ePlayerJoinEvent:
    case bz_ePlayerPartEvent:
    {
      bz_PlayerJoinPartEventData_V1 *joinPart = (bz_PlayerJoinPartEventData_V1*)eventData;
      
      if ( joinPart->eventType == bz_ePlayerJoinEvent )
      {
	if (startTime < 0)
	  startTime = joinPart->eventTime;
      }
      else if (endCond == eNoPlayers)
      {
	if (!anyPlayers())
	  nextMap();
      }
    }
    break;

    case bz_ePlayerDieEvent:
      {
	if (endCond != eMaxKillScore)
	  break;

	bz_PlayerDieEventData_V1 *die = (bz_PlayerDieEventData_V1*)eventData;

	if ( bz_getPlayerWins(die->killerID) >= scoreCapLimit )
	  nextMap();
      }
      break;

    case bz_eCaptureEvent:
      {
	if (endCond != eMaxCapScore)
	  break;

	bz_CTFCaptureEventData_V1 *cap = (bz_CTFCaptureEventData_V1*)eventData;

	if ( bz_getTeamWins(cap->teamCapping) >= scoreCapLimit )
	  nextMap();
      }
      break;

    case bz_eTickEvent:
      {
	if (endCond != eTimedGame && startTime >= 0)
	  break;

	bz_TickEventData_V1 *tick = (bz_TickEventData_V1*)eventData;

	double timeDelta = tick->eventTime - startTime;
	if ( timeDelta >= timeLimit )
	  nextMap();
      }
      break;

    case bz_eGetWorldEvent:
      {
	if (currentIndex < 0 )
	  break;
    
	bz_GetWorldEventData_V1 *world =(bz_GetWorldEventData_V1*)eventData;

	world->generated = false;
	world->worldFile = gameList[currentIndex].mapFile.c_str();
	gameList[currentIndex].hasBeenPlayed = true;
      }
      break;

    case bz_eListServerUpdateEvent:
      {
	if (currentIndex < 0 )
	  break;

	bz_ListServerUpdateEvent_V1 *update =(bz_ListServerUpdateEvent_V1*)eventData;

	std::string desc = update->description.c_str();
	desc = replace_all(desc,std::string("%M"),gameList[currentIndex].publicText);
	update->description = desc.c_str();
	update->handled = true;
     }
      break;
 }
}

 bool MapChangeCommandHandler::handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params )
 {
   std::string cmd = tolower(command.c_str());

   std::string param;
   if ( params && params->size() )
     param = params->get(0).c_str();

   if ( cmd == "mapnext" )
   {
     if (bz_getAdmin (playerID) || bz_hasPerm(playerID,"mapchange"))
      nextMap();
     else
      bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permision to change maps");
     return true;
   }
   else if ( cmd == "mapreset" )
   {
     if (bz_getAdmin (playerID) || bz_hasPerm(playerID,"mapchange"))
     {
       resetGames();
       currentIndex = -1;
       nextMap();
     }
     else
       bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permision to reset map rotation");
     return true;
   }
   else if ( cmd == "mapcyclemode" )
   {
     if (param.size())
     {
       if (bz_getAdmin (playerID) || bz_hasPerm(playerID,"mapchange"))
       {
	cycleMode = cycleFromString(param.c_str());
	std::string mode = "Map Rotation Mode changed to ";
	mode += param.c_str();
	bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,mode.c_str());
       }
       else
	 bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permision to set map modes");
    }
     else
     {
       std::string mode = "Current Map Rotation Mode:";
       mode += cycleToString(cycleMode);
       bz_sendTextMessage(BZ_SERVER,playerID,mode.c_str());
     }
     return true;
   }
   else if ( cmd == "mapendmode" )
   {
     if (param.size())
     {
	if (bz_getAdmin (playerID) || bz_hasPerm(playerID,"mapchange"))
	{
	  startTime = bz_getCurrentTime();
	  endCond = condFromString(param);
	  std::string mode = "Map End Condition changed to ";
	  mode += param;
	  bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,mode.c_str());
	}
	else
	  bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permision to set map modes");
     }
     else
     {
       std::string mode = "Current End Condtion:";
       mode += condToString(endCond);
       bz_sendTextMessage(BZ_SERVER,playerID,mode.c_str());
     }
     return true;
   }
   else if ( cmd == "maplist" )
   {
     bz_sendTextMessage(BZ_SERVER,playerID,"Maps In Rotation");
     for (int i = 0; i < (int)gameList.size(); i++ )
	 bz_sendTextMessage(BZ_SERVER,playerID,gameList[i].mapFile.c_str());
     return true;
   }
   else if ( cmd == "maplimit" )
   {
     if (endCond == eNoPlayers || endCond == eManual )
     {
       bz_sendTextMessage(BZ_SERVER,playerID,"The currnet mode has no numeric limit");
       return true;
     }

     if (param.size())
     {
       if (bz_getAdmin (playerID) || bz_hasPerm(playerID,"mapchange"))
       {
	 std::string mode = "Map Change limit changed to ";
	if (endCond == eTimedGame)
	{
	  startTime = bz_getCurrentTime();
	  timeLimit = atof(param.c_str())*60;
	  mode += param.c_str();
	  mode += "minutes";
	}
	else
	{
	  scoreCapLimit = atoi(param.c_str());
	  mode += param.c_str();
	  if (endCond == eMaxKillScore)
	    mode += " Kills";
	  else
	    mode += " Caps";
	}
	bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,mode.c_str());
       }
       else
       {
	 std::string mode = "Map Change limit is ";
	 if (endCond == eTimedGame)
	   mode += format("%f minutes", timeLimit/60.0);
	 else
	 {
	    mode += format("%d", scoreCapLimit);
	   if (endCond == eMaxKillScore)
	     mode += " Kills";
	   else
	     mode += " Caps";
	 }
	 bz_sendTextMessage(BZ_SERVER,playerID,"You do not have permision to set map limits");
       }
     }
     else
     {
       std::string mode = "Current End Condtion:";
       mode += condToString(endCond);
       bz_sendTextMessage(BZ_SERVER,playerID,mode.c_str());
     }
     return true;
   }
   return false;
 }

 const char* MapChangeCommandHandler::help ( bz_ApiString command )
 {
   std::string text = "No help available";
   std::string cmd = tolower(command.c_str());

   if (cmd == "mapnext")
     text = "Usage /mapNext; Changes to the next map in the rotation";
   if (cmd == "mapreset")
     text = "Usage /mapReset; Resets the map rotation back to it's inital state";
   if (cmd == "mapcyclemode")
     text = "Usage /mapCycleMode (MODE); Changes to the cycle mode for map rotation\n  Valid params are Loop, Random, OneRand, and NoLoop\n  If no paramater is given thent he current mode is listed";
   if (cmd == "mapendmode")
     text = "Usage /mapEndMode (MODE); Changes to the end condition mode for map rotation\n  Valid params are Timed, MaxKill, MaxCap, Manual, and Empty\n  If no paramater is given thent he current mode is listed";
   if (cmd == "maplist")
     text = "Usage /mapList; lists all the maps in the rotation";
   if (cmd == "maplimit")
     text = "Usage /mapLimit (LIMIT); Changes the current limit to the specified value, minuets for timed games, score for others.\n  If no paramater is given thent he current limit is listed";
  
   return text.c_str();
 }


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
