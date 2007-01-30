// koth.cpp : Defines the entry point for the DLL application.
// (code modified from JeffM2501's flayStay plugin)
// King of the Hill Version 2.3

#include "bzfsAPI.h"
#include <string>
#include <vector>
#include <map>
#include <math.h>

BZ_GET_PLUGIN_VERSION

bool teamPlay = false;
double kothTTH = 60; 
double adjustedTime = 60;
double timeMult = 0.03;
double timeMultMin = 0.50;
bool kothEnabled = true;
bool toldHillOpen = false;
bool oneTeamWarn = false;
bool autoTimeOn = false;
bool soundEnabled = true;
bool notEnoughTeams = true;
int TTHminutes = 0;
int TTHseconds = 30;
int playerJustWon = -1;



class KOTHMapHandler : public bz_CustomMapObjectHandler
{
public:
	virtual bool handle ( bz_ApiString object, bz_CustomMapObjectInfo *data );
};

KOTHMapHandler	kothmaphandler;

class EventHandler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData );
};

EventHandler eventHandler;

class Commands : public bz_CustomSlashCommandHandler
{
public:
  virtual ~Commands(){};
  virtual bool handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *param );
};

Commands commands;

class PlayerPaused : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

PlayerPaused playerpaused;

class PlayerJoined : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

PlayerJoined playerjoined;

class PlayerLeft : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

PlayerLeft playerleft;

class PlayerDied : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

PlayerDied playerdied;

BZF_PLUGIN_CALL int bz_Load (const char* /*commandLine*/){

	bz_debugMessage(4,"koth plugin loaded");
	bz_registerCustomMapObject("KOTH",&kothmaphandler);
	bz_registerEvent(bz_ePlayerUpdateEvent,&eventHandler);
	bz_registerEvent(bz_ePlayerPausedEvent,&playerpaused);
	bz_registerEvent(bz_ePlayerPartEvent,&playerleft);
	bz_registerEvent(bz_ePlayerJoinEvent,&playerjoined);
	bz_registerEvent(bz_ePlayerDieEvent,&playerdied);
	bz_registerCustomSlashCommand("kothstatus",&commands);
	bz_registerCustomSlashCommand("kothon",&commands);
	bz_registerCustomSlashCommand("kothoff",&commands);
	bz_registerCustomSlashCommand("kothtimemult",&commands);
	bz_registerCustomSlashCommand("kothtimemultmin",&commands);
	bz_registerCustomSlashCommand("kothtime",&commands);
	bz_registerCustomSlashCommand("kothautotimeon",&commands);
	bz_registerCustomSlashCommand("kothautotimeoff",&commands);
	bz_registerCustomSlashCommand("kingsay",&commands);
	bz_registerCustomSlashCommand("kothsoundon",&commands);
	bz_registerCustomSlashCommand("kothsoundoff",&commands);
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload (void){

	bz_removeEvent(bz_ePlayerUpdateEvent,&eventHandler);
	bz_removeEvent(bz_ePlayerPausedEvent,&playerpaused);
	bz_removeEvent(bz_ePlayerPartEvent,&playerleft);
	bz_removeEvent(bz_ePlayerJoinEvent,&playerjoined);
	bz_removeEvent(bz_ePlayerDieEvent,&playerdied);
	bz_debugMessage(4,"koth plugin unloaded");
	bz_removeCustomMapObject("KOTH");
	bz_removeCustomSlashCommand("kothstatus");
	bz_removeCustomSlashCommand("kothon");
	bz_removeCustomSlashCommand("kothoff");
	bz_removeCustomSlashCommand("kothtimemult");
	bz_removeCustomSlashCommand("kothtimemultmin");
	bz_removeCustomSlashCommand("kothtime");
	bz_removeCustomSlashCommand("kothautotimeon");
	bz_removeCustomSlashCommand("kothautotimeoff");
	bz_removeCustomSlashCommand("kingsay");
	bz_removeCustomSlashCommand("kothsoundon");
	bz_removeCustomSlashCommand("kothsoundoff");
	return 0;
}

class KOTH
{
public:
	KOTH()
	{
		id = -1;
		startTime = 0;
		team = eNoTeam;
		callsign = "";
	}
	bz_eTeamType team;
	int id;
	double startTime;
	std::string callsign;
};

KOTH koth;

class KOTHZone
{
public:
	KOTHZone()
	{
		box = false;
		xMax = xMin = yMax = yMin = zMax = zMin = rad = 0;
	}

	bool box;
	float xMax,xMin,yMax,yMin,zMax,zMin;
	float rad;

	bool pointIn ( float pos[3] )
	{
		if ( box )
		{
			if ( pos[0] > xMax || pos[0] < xMin )
				return false;

			if ( pos[1] > yMax || pos[1] < yMin )
				return false;

			if ( pos[2] > zMax || pos[2] < zMin )
				return false;
		}
		else
		{
			float vec[3];
			vec[0] = pos[0]-xMax;
			vec[1] = pos[1]-yMax;
			vec[2] = pos[2]-zMax;

			float dist = sqrt(vec[0]*vec[0]+vec[1]*vec[1]);
			if ( dist > rad)
				return false;

			if ( pos[2] > zMax || pos[2] < zMin )
				return false;
		}
		return true;
	}
};

KOTHZone kothzone;

bool KOTHMapHandler::handle ( bz_ApiString object, bz_CustomMapObjectInfo *data )
{
	if (object != "KOTH" || !data)
		return false;

	// parse all the chunks
	for ( unsigned int i = 0; i < data->data.size(); i++ )
	{
		std::string line = data->data.get(i).c_str();

		bz_APIStringList *nubs = bz_newStringList();
		nubs->tokenize(line.c_str()," ",0,true);
		
		if ( nubs->size() > 0)
		{
			std::string key = bz_toupper(nubs->get(0).c_str());

			if ( key == "BBOX" && nubs->size() > 6)
			{
				kothzone.box = true;
				kothzone.xMin = (float)atof(nubs->get(1).c_str());
				kothzone.xMax = (float)atof(nubs->get(2).c_str());
				kothzone.yMin = (float)atof(nubs->get(3).c_str());
				kothzone.yMax = (float)atof(nubs->get(4).c_str());
				kothzone.zMin = (float)atof(nubs->get(5).c_str());
				kothzone.zMax = (float)atof(nubs->get(6).c_str());
			}
			else if ( key == "CYLINDER" && nubs->size() > 5)
			{
				kothzone.box = false;
				kothzone.rad = (float)atof(nubs->get(5).c_str());
				kothzone.xMax =(float)atof(nubs->get(1).c_str());
				kothzone.yMax =(float)atof(nubs->get(2).c_str());
				kothzone.zMin =(float)atof(nubs->get(3).c_str());
				kothzone.zMax =(float)atof(nubs->get(4).c_str());
			}
			else if ( key == "TEAMPLAY" && nubs->size() >= 0 )
			{
				teamPlay = true;
			}
			else if ( key == "NOSOUND" && nubs->size() >= 0 )
				soundEnabled = false;

			else if ( key == "AUTOTIME" && nubs->size() == 1 )
			{
				autoTimeOn = true;
			}
			else if ( key == "AUTOTIME" && nubs->size() > 2 )
			{
				double temp1 = (double)atof(nubs->get(1).c_str());
				double temp2 = (double)atof(nubs->get(2).c_str());
				if (temp1 >= 1 && temp1 <= 99)
					timeMult = temp1 / 100;
				if (temp2 >= 1 && temp2 <= 99)
					timeMultMin = temp2 / 100;
				autoTimeOn = true;
			}
			else if ( key == "HOLDTIME" && nubs->size() > 1 )
			{
				double temp = (double)atof(nubs->get(1).c_str());
				if (temp >= 1 && temp <= 7200)
					kothTTH = temp;
			}
		}
		bz_deleteStringList(nubs);
	}
	bz_setMaxWaitTime ( 0.5 );
	return true;
}

std::string truncate(std::string cllsn, int maxStringLength)
{
	std::string fixed = "";

	for (int i = 0; i < maxStringLength; i++)
	{
		fixed.push_back(cllsn[i]);
	}

	fixed.append("~");

	return fixed;
}

const char* getTeamColor(bz_eTeamType testteam)
{
	if (testteam == eRedTeam)
		return "RED";
	if (testteam == eGreenTeam)
		return "GREEN";
	if (testteam == eBlueTeam)
		return "BLUE";
	if (testteam == ePurpleTeam)
		return "PURPLE";
	if (testteam == eRogueTeam)
		return "ROGUE";

	return " ";
}

inline bool oneTeam(bz_eTeamType leavingPlayerTeam)
{
	int RT = bz_getTeamCount(eRedTeam);
	int GT = bz_getTeamCount(eGreenTeam);
	int BT = bz_getTeamCount(eBlueTeam);
	int PT = bz_getTeamCount(ePurpleTeam);
	int RGT = bz_getTeamCount(eRogueTeam);

	if (leavingPlayerTeam == eRedTeam)
		RT--;
	if (leavingPlayerTeam == eGreenTeam)
		GT--;
	if (leavingPlayerTeam == eBlueTeam)
		BT--;
	if (leavingPlayerTeam == ePurpleTeam)
		PT--;
	if (leavingPlayerTeam == eRogueTeam)
		RGT--;

	int Test1 = (RT * GT) + (RT * BT) + (RT * PT) + (GT * BT) + (GT * PT) + (BT * PT);
	int Test2 = RT + GT + BT + PT + RGT;

	if (leavingPlayerTeam == eRedTeam)
		RT--;
	
	if (Test1 < 1 && Test2 < 2)
	{
		if (!oneTeamWarn)
			bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "King of the Hill disabled: less than 2 teams.");

		oneTeamWarn = true;
		return true;
	}
	else
	{
		if (oneTeamWarn)
			bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "King of the Hill enabled: more than 1 team.");

		oneTeamWarn = false;
		return false;
	}
}

void autoTime()
{
	int numPlayers = bz_getTeamCount(eRedTeam) + bz_getTeamCount(eGreenTeam) + bz_getTeamCount(eBlueTeam) + bz_getTeamCount(ePurpleTeam) + bz_getTeamCount(eRogueTeam);

	if (!autoTimeOn || numPlayers < 3)
	{
		adjustedTime = kothTTH;
		return;
	}

	double timeDown = ( 1 - ((double)numPlayers - 2) * timeMult);

	if (timeDown < timeMultMin) 
		timeDown = timeMultMin;

	adjustedTime = (int)(kothTTH * timeDown);

	return;
}

double ConvertToNum(std::string inmessage, double minNum, double maxNum){

	int messagelength = (int)inmessage.length();

	if (messagelength > 0 && messagelength < 5)
	{
		double messagevalue = 0;
		double tens = 1;

		for ( int i = (messagelength - 1); i >= 0; i-- ){
		
			if (inmessage[i] < '0' || inmessage[i] > '9')  // got something other than a number
				return 0; 

			tens *= 10;
			messagevalue +=  (((double)inmessage[i] - '0') / 10) * tens;
		}

		if (messagevalue >= minNum && messagevalue <= maxNum)
			return messagevalue;
	}

	return 0;
}

void killTeams(bz_eTeamType safeteam, std::string kothcallsign)
{
	bz_APIIntList *playerList = bz_newIntList(); 
	bz_getPlayerIndexList ( playerList ); 

	for ( unsigned int i = 0; i < playerList->size(); i++ ){ 
       
		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player){
			
				if (player->team != safeteam)
				{
					bz_killPlayer(player->playerID, true, BZ_SERVER);
					if (soundEnabled)
						bz_sendPlayCustomLocalSound(player->playerID,"flag_lost");
				}
				else if (soundEnabled)
					bz_sendPlayCustomLocalSound(player->playerID,"flag_won");
			}

		bz_freePlayerRecord(player);	
	}
	bz_deleteIntList(playerList); 

	bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) IS KING OF THE HILL!", getTeamColor(safeteam), kothcallsign.c_str());

	return;
}

void killPlayers(int safeid, std::string kothcallsign)
{
	bz_APIIntList *playerList = bz_newIntList(); 
	bz_getPlayerIndexList ( playerList ); 

	for ( unsigned int i = 0; i < playerList->size(); i++ ){ 
       
		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player){
			
				if (player->playerID != safeid)
				{
					bz_killPlayer(player->playerID, true, koth.id);
					if (soundEnabled)
						bz_sendPlayCustomLocalSound(player->playerID,"flag_lost");
				}
				else if (soundEnabled)
					bz_sendPlayCustomLocalSound(player->playerID,"flag_won");
			}

		bz_freePlayerRecord(player);	
	}

	bz_deleteIntList(playerList); 

	bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s IS KING OF THE HILL!", kothcallsign.c_str());
	
	return;
}

void sendWarnings(const char* teamcolor, std::string playercallsign, double kothstartedtime)
{
	double TimeElapsed = bz_getCurrentTime() - kothstartedtime;
	double TimeRemaining = adjustedTime - TimeElapsed;
	int toTens = int((TimeRemaining + 5) / 10) * 10;

	if ((TimeRemaining/60) < TTHminutes && adjustedTime > 59 && TimeRemaining >= 1)
	{
		if (!teamPlay || koth.team == eRogueTeam)
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s will be King in %i secs!", playercallsign.c_str(), toTens);
		else
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) will be King in %i secs!", teamcolor, playercallsign.c_str(), toTens);	
		
		TTHminutes--;
	}
	
	if (adjustedTime < TTHseconds)
	{
		TTHseconds = TTHseconds - 10;
		return;
	}

	if (TimeRemaining < TTHseconds && TimeRemaining >= 1)
	{
		if (!teamPlay || koth.team == eRogueTeam)
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s will be King in %i secs!", playercallsign.c_str(), TTHseconds);
		else
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) will be King in %i secs!", teamcolor, playercallsign.c_str(), TTHseconds);

		TTHseconds = TTHseconds - 10;
	}
	return;
}

void initiatekoth(bz_eTeamType plyrteam, bz_ApiString plyrcallsign, int plyrID)
{
	koth.team = plyrteam;
	koth.callsign = plyrcallsign.c_str();

	if (koth.callsign.size() > 16)
	{
		std::string tofix = truncate(koth.callsign, 16);
		koth.callsign = tofix;
	}

	koth.id = plyrID;
	koth.startTime = bz_getCurrentTime();
	TTHminutes = (int)(adjustedTime/60 + 0.5);
	TTHseconds = 30;
	toldHillOpen = false;
	bool multipleof30 = false;

	if ((int)((adjustedTime / 30) + 0.5) != (double)(adjustedTime / 30))
		multipleof30 = false;
	else
		multipleof30 = true;

	if (!multipleof30)
	{
		if ((!teamPlay || koth.team == eRogueTeam))
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s has Hill; will be King in %i secs!", koth.callsign.c_str(), (int)adjustedTime);
		else
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) has Hill; will be King in %i secs!", getTeamColor(koth.team), koth.callsign.c_str(), (int)adjustedTime);
	}

	if (soundEnabled)
	{
		bz_APIIntList *playerList = bz_newIntList(); 
		bz_getPlayerIndexList ( playerList ); 

		for ( unsigned int i = 0; i < playerList->size(); i++ ) 
		{ 
     		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player)
			{
				if (player->team != koth.team)
					bz_sendPlayCustomLocalSound(player->playerID,"flag_alert");
				else
					bz_sendPlayCustomLocalSound(player->playerID,"teamgrab");
			}
	
			bz_freePlayerRecord(player);
		}
		bz_deleteIntList(playerList); 
	}
	return;
}

bool teamClear(bz_eTeamType teamToCheck)
{
	if (teamToCheck == eRogueTeam || teamToCheck == eNoTeam || !teamPlay)
		return true;

	bz_APIIntList *playerList = bz_newIntList(); 
	bz_getPlayerIndexList ( playerList ); 

	bool isOut = true;

	for ( unsigned int i = 0; i < playerList->size(); i++ ){ 
       
		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player)
			{
				if (player->team == teamToCheck && kothzone.pointIn(player->pos) && player->spawned)
					isOut = false;
			}

		bz_freePlayerRecord(player);	
	}

	bz_deleteIntList(playerList);

	return isOut;
}

void PlayerPaused::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerPausedEvent || !kothEnabled)
		return;

	bz_PlayerPausedEventData_V1 *PauseData = (bz_PlayerPausedEventData_V1*)eventData;
	bz_BasePlayerRecord *player = bz_getPlayerByIndex(PauseData->player);

	if (player)
	{
		if(kothzone.pointIn(player->pos))
		{
			bz_killPlayer (PauseData->player, true, BZ_SERVER);
			bz_sendTextMessage (BZ_SERVER, PauseData->player, "Cannot pause while on the Hill.");
		}
	}
	bz_freePlayerRecord(player);

	return;
}

void PlayerJoined::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerJoinEvent || !kothEnabled)
		return;

	autoTime();

	if (oneTeam(eNoTeam)) // don't send message if not enough teams
	{
		notEnoughTeams = true;
		return;
	}
	else
		notEnoughTeams = false;

	return;
}

void PlayerLeft::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerPartEvent || !kothEnabled)
		return;

	autoTime();

	bz_PlayerJoinPartEventData_V1 *partData = (bz_PlayerJoinPartEventData_V1*)eventData;

	if (oneTeam(partData->team)) // team count check
		notEnoughTeams = true;
	else
		notEnoughTeams = false;

	if (partData->playerID == koth.id)
	{
		koth.id = -1;
		koth.team = eNoTeam;
	}

	return;
}

void PlayerDied::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerDieEvent || !kothEnabled)
		return;

	bz_PlayerDieEventData_V1 *dieData = (bz_PlayerDieEventData_V1*)eventData;

	if (dieData->playerID == koth.id)
	{
		koth.id = -1;
		koth.team = eNoTeam;
	}
	
	return;
}

void EventHandler::process ( bz_EventData *eventData )
{
	if (!kothEnabled) // King of the Hill disabled - we can leave
		return;

	if (notEnoughTeams) // Not enough teams - we can leave
		return;
	
	float pos[3] = {0};

	int playerID = -1;

	switch (eventData->eventType)
	{
	case bz_ePlayerUpdateEvent:
		pos[0] = ((bz_PlayerUpdateEventData_V1*)eventData)->pos[0];
		pos[1] = ((bz_PlayerUpdateEventData_V1*)eventData)->pos[1];
		pos[2] = ((bz_PlayerUpdateEventData_V1*)eventData)->pos[2];
		playerID = ((bz_PlayerUpdateEventData_V1*)eventData)->player;
		break;

	case bz_eShotFiredEvent:
		pos[0] = ((bz_ShotFiredEventData_V1*)eventData)->pos[0];
		pos[1] = ((bz_ShotFiredEventData_V1*)eventData)->pos[1];
		pos[2] = ((bz_ShotFiredEventData_V1*)eventData)->pos[2];
		playerID = ((bz_ShotFiredEventData_V1*)eventData)->player;
		break;

	default:
		return;
	}

	if (!toldHillOpen && koth.id == -1) // Hill is open - inform players
	{
		bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Hill is not controlled - take it!");
		toldHillOpen = true;
	}

	if (kothzone.pointIn(pos)) // player is on Hill
	{
		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);
		
		if (player)
		{
			if (player->playerID != playerJustWon && player->spawned)
			{
				if ((koth.id == -1 && player->team != koth.team) || (koth.id == -1 && teamClear(koth.team)))
					initiatekoth(player->team, player->callsign, player->playerID);

				double timeStanding = bz_getCurrentTime() - koth.startTime;

				if (timeStanding >= adjustedTime && koth.id != -1) // time's up - kill 'em
				{
					if (teamPlay && (koth.team != eRogueTeam))
						killTeams(koth.team, koth.callsign);
					else
						killPlayers(koth.id, koth.callsign);

					if (!teamPlay || koth.team == eRogueTeam)
						bz_sendTextMessage (BZ_SERVER, koth.id, "You are King of the Hill!  You must leave hill to retake it.");
					else
						bz_sendTextMessage (BZ_SERVER, koth.team, "Your team is King of the Hill!  Entire team must leave hill to retake it.");

					playerJustWon = koth.id;

					koth.id = -1;

					return;
				}
				if (koth.id != -1)
					sendWarnings(getTeamColor(koth.team), koth.callsign, koth.startTime);
			}
		}

		bz_freePlayerRecord(player);
	}
	else // player is off Hill
	{
		if (playerID == playerJustWon)
			playerJustWon = -1;

		if (playerID == koth.id)
		{
			koth.id = -1;
			koth.team = eNoTeam;
		}
	}
}

bool Commands::handle ( int playerID, bz_ApiString _command, bz_ApiString _message, bz_APIStringList * /*_param*/ )
{
	std::string command = _command.c_str();
	std::string message = _message.c_str();
	const char* kingmessage = _message.c_str();

	if ( command == "kingsay" )
	{
		if (koth.id != -1)
			bz_sendTextMessage (playerID, koth.id, kingmessage);
		else
			bz_sendTextMessage(BZ_SERVER,playerID,"There is no one attempting to be king right now.");

		return true;
	}

	bz_BasePlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);

	if ( !fromPlayer->admin )
	{
    	bz_sendTextMessage(BZ_SERVER,playerID,"You must be admin to use the koth commands.");
		bz_freePlayerRecord(fromPlayer);
		return true;
	}

	bz_freePlayerRecord(fromPlayer);

	if ( command == "kothon")
	{
		kothEnabled = true;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill is enabled.");
		return true;
	}

	if ( command == "kothoff")
	{
		kothEnabled = false;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill is disabled.");
		return true;
	}

	if ( command == "kothtimemult")
	{
		double inputvalue = ConvertToNum(message, 1, 99);

		if (inputvalue > 0)
		{
			timeMult = (inputvalue/100);
			bz_sendTextMessagef (BZ_SERVER, playerID, "Auto time multiplier set to %i percent.", (int)(timeMult*100 + 0.5));
		}
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "Auto time multiplier must be between 1 and 99 percent.", (int)(timeMult*100 + 0.5));

		autoTime();

		return true;
	}

	if ( command == "kothtimemultmin")
	{
		double inputvalue = ConvertToNum(message, 1, 99);

		if (inputvalue > 0)
		{
			timeMultMin = (inputvalue/100);
			bz_sendTextMessagef (BZ_SERVER, playerID, "Auto time multiplier minimum set to %i percent.", (int)(timeMultMin*100 + 0.5));
		}
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "Auto time multiplier minimum must be between 1 and 99 percent.");

		autoTime();

		return true;
	}

	if ( command == "kothstatus")
	{
		if (kothEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill is currently enabled.");

		if (!kothEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill is currently disabled.");

		if (autoTimeOn)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Automatic time adjustment is currently enabled.");

		if (!autoTimeOn)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Automatic time adjustment is currently disabled.");

		bz_sendTextMessagef (BZ_SERVER, playerID, "Time multiplier = %i percent.", (int)(timeMult*100 + 0.5));

		bz_sendTextMessagef (BZ_SERVER, playerID, "Time multiplier minimum = %i percent.", (int)(timeMultMin*100 + 0.5));
		
		int AdjTime = (int)(adjustedTime + 0.5);
		bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill hold time is currently set to: %i seconds", AdjTime);

		if (soundEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Sound is enabled.");

		if (!soundEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Sound is disabled.");

		return true;
	}
    
  // explicit time command handler:

	if ( command == "kothtime" )
	{
		double inputvalue = ConvertToNum(message, 1, 7200);
  
		if (inputvalue > 0 )
		{
			kothTTH = inputvalue;
			autoTime();
			int AdjTime = (int)(inputvalue + 0.5);
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill hold time has been set to %i seconds.", AdjTime);
		}
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill hold time invalid: must be between 1 and 7200 seconds.");

		autoTime();

		return true;
	}

	if ( command == "kothautotimeon")
	{
		autoTimeOn = true;
		autoTime();
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill automatic time adjustment on.");
		return true;
	}

	if ( command == "kothsoundon")
	{
		soundEnabled = true;
		bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill sound enabled.");
		return true;
	}

	if ( command == "kothsoundoff")
	{
		soundEnabled = false;
		bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill sound disabled.");
		return true;
	}

	if ( command == "kothautotimeoff")
	{
		autoTimeOn = false;
		adjustedTime = kothTTH;
		autoTime();
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill automatic time adjustment off.");
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

