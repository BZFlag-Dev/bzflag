// keepaway.cpp : Defines the entry point for the DLL application.

#include "bzfsAPI.h"
#include <string>
#include <vector>
#include <map>
#include <math.h>

BZ_GET_PLUGIN_VERSION

class KeepAwayMapHandler : public bz_CustomMapObjectHandler
{
public:
	virtual bool handle ( bz_ApiString object, bz_CustomMapObjectInfo *data );
};

KeepAwayMapHandler	keepawaymaphandler;

class KeepAwayEventHandler : public bz_EventHandler
{
public:
	virtual void process ( bz_EventData *eventData );
};

KeepAwayEventHandler keepawayeventhandler;

class KeepAwayCommands : public bz_CustomSlashCommandHandler
{
public:
  virtual ~KeepAwayCommands(){};
  virtual bool handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *param );
};

KeepAwayCommands keepawaycommands;

class KeepAwayPlayerPaused : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

KeepAwayPlayerPaused keepawayplayerpaused;

class KeepAwayPlayerJoined : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

KeepAwayPlayerJoined keepawayplayerjoined;

class KeepAwayPlayerLeft : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

KeepAwayPlayerLeft keepawayplayerleft;

class KeepAwayPlayerDied : public bz_EventHandler
{
public:
	virtual void	process ( bz_EventData *eventData );
};

KeepAwayPlayerDied keepawayplayerdied;

BZF_PLUGIN_CALL int bz_Load (const char* /*commandLine*/)
{
	bz_debugMessage(4,"keepaway plugin loaded");
	bz_registerCustomMapObject("keepaway",&keepawaymaphandler);
	bz_registerEvent(bz_ePlayerUpdateEvent,&keepawayeventhandler);
	bz_registerEvent(bz_ePlayerPausedEvent,&keepawayplayerpaused);
	bz_registerEvent(bz_ePlayerPartEvent,&keepawayplayerleft);
	bz_registerEvent(bz_ePlayerJoinEvent,&keepawayplayerjoined);
	bz_registerEvent(bz_ePlayerDieEvent,&keepawayplayerdied);
	bz_registerCustomSlashCommand("kastatus",&keepawaycommands);
	bz_registerCustomSlashCommand("kaon",&keepawaycommands);
	bz_registerCustomSlashCommand("kaoff",&keepawaycommands);
	bz_registerCustomSlashCommand("katimemult",&keepawaycommands);
	bz_registerCustomSlashCommand("katimemultmin",&keepawaycommands);
	bz_registerCustomSlashCommand("katime",&keepawaycommands);
	bz_registerCustomSlashCommand("kaautotimeon",&keepawaycommands);
	bz_registerCustomSlashCommand("kaautotimeoff",&keepawaycommands);
	bz_registerCustomSlashCommand("kas",&keepawaycommands);
	bz_registerCustomSlashCommand("kaffon",&keepawaycommands);
	bz_registerCustomSlashCommand("kaffoff",&keepawaycommands);
	bz_registerCustomSlashCommand("kaf",&keepawaycommands);
	bz_registerCustomSlashCommand("kaf+",&keepawaycommands);
	bz_registerCustomSlashCommand("kasoundon",&keepawaycommands);
	bz_registerCustomSlashCommand("kasoundoff",&keepawaycommands);
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload (void)
{
	bz_removeEvent(bz_ePlayerUpdateEvent,&keepawayeventhandler);
	bz_removeEvent(bz_ePlayerPausedEvent,&keepawayplayerpaused);
	bz_removeEvent(bz_ePlayerPartEvent,&keepawayplayerleft);
	bz_removeEvent(bz_ePlayerJoinEvent,&keepawayplayerjoined);
	bz_removeEvent(bz_ePlayerDieEvent,&keepawayplayerdied);
	bz_debugMessage(4,"keepaway plugin unloaded");
	bz_removeCustomMapObject("keepaway");
	bz_removeCustomSlashCommand("kastatus");
	bz_removeCustomSlashCommand("kaon");
	bz_removeCustomSlashCommand("kaoff");
	bz_removeCustomSlashCommand("katimemult");
	bz_removeCustomSlashCommand("katimemultmin");
	bz_removeCustomSlashCommand("katime");
	bz_removeCustomSlashCommand("kaautotimeon");
	bz_removeCustomSlashCommand("kaautotimeoff");
	bz_removeCustomSlashCommand("kas");
	bz_removeCustomSlashCommand("kaffon");
	bz_removeCustomSlashCommand("kaffoff");
	bz_removeCustomSlashCommand("kaf");
	bz_removeCustomSlashCommand("kaf+");
	bz_removeCustomSlashCommand("kasoundon");
	bz_removeCustomSlashCommand("kasoundoff");
	return 0;
}

class KeepAway
{
public:
	KeepAway()
	{
		id = -1;
		startTime = 0;
		team = eNoTeam;
		callsign = "";
		flagsList.clear();
		TTH = 120; 
		adjustedTime = 120;
		timeMult = 0.03;
		timeMultMin = 0.50;
		lastReminder = bz_getCurrentTime();
		reminderPeriod = 60;
		enabled = true;
		toldFlagFree = false;
		oneTeamWarn = false;
		autoTimeOn = false;
		forcedFlags = false;
		notEnoughTeams = true;
		soundEnabled = true;
		teamPlay = false;
		TTHminutes = 0;
		TTHseconds = 30;
		flagToKeepIndex = 0;
		flagToKeep = "Initiate";
	}
	bz_eTeamType team;
	std::string callsign;
	std::string flagToKeep;
	std::vector <std::string> flagsList; 
	bool teamPlay;
	double TTH; 
	double adjustedTime;
	double timeMult;
	double timeMultMin;
	double lastReminder;
	double reminderPeriod;
	double startTime;
	bool enabled;
	bool toldFlagFree;
	bool oneTeamWarn;
	bool autoTimeOn;
	bool forcedFlags;
	bool notEnoughTeams;
	bool soundEnabled;
	int TTHminutes;
	int TTHseconds;
	int flagToKeepIndex;
	int id;
};

KeepAway keepaway;

std::string convertFlag(std::string flagAbbrev)
{
	if (flagAbbrev == "V")
		return "High Speed (V)";
	if (flagAbbrev == "QT")
		return "Quick Turn (QT)";
	if (flagAbbrev == "A")
		return "Agility (A)";
	if (flagAbbrev == "OO")
		return "Oscillation Overthruster (OO)";
	if (flagAbbrev == "F")
		return "Rapid Fire (F)";
	if (flagAbbrev == "MG")
		return "Machine Gun (MG)";
	if (flagAbbrev == "GM")
		return "Guided Missile (GM)";
	if (flagAbbrev == "L")
		return "Laser (L)";
	if (flagAbbrev == "R")
		return "Ricochet (R)";
	if (flagAbbrev == "SB")
		return "Super Bullet (SB)";
	if (flagAbbrev == "ST")
		return "Stealth (ST)";
	if (flagAbbrev == "CL")
		return "Cloaking (CL)";
	if (flagAbbrev == "IB")
		return "Invisible Bullet (IB)";
	if (flagAbbrev == "T")
		return "Tiny (T)";
	if (flagAbbrev == "N")
		return "Narrow (N)";
	if (flagAbbrev == "SH")
		return "Shield (SH)";
	if (flagAbbrev == "SR")
		return "Steamroller (SR)";
	if (flagAbbrev == "SW")
		return "ShockWave (SW)";
	if (flagAbbrev == "PZ")
		return "Phantom Zone (PZ)";
	if (flagAbbrev == "G")
		return "Genocide (G)";
	if (flagAbbrev == "JP")
		return "Jumping (JP)";
	if (flagAbbrev == "ID")
		return "Identify (ID)";
	if (flagAbbrev == "MQ")
		return "Masquerade (MQ)";
	if (flagAbbrev == "BU")
		return "Burrow (BU)";
	if (flagAbbrev == "SE")
		return "Seer (SE)";
	if (flagAbbrev == "TH")
		return "Thief (TH)";
	if (flagAbbrev == "US")
		return "Useless (US)";
	if (flagAbbrev == "WG")
		return "Wings (WG)";
	if (flagAbbrev == "CB")
		return "Colorblindness (CB)";
	if (flagAbbrev == "OB")
		return "Obesity (OB)";
	if (flagAbbrev == "LT")
		return "Left Turn Only (LT)";
	if (flagAbbrev == "RT")
		return "Right Turn Only (RT)";
	if (flagAbbrev == "FO")
		return "Forward Only (FO)";
	if (flagAbbrev == "RO")
		return "Reverse Only (RO)";
	if (flagAbbrev == "M")
		return "Momentum (M)";
	if (flagAbbrev == "B")
		return "Blindness (B)";
	if (flagAbbrev == "JM")
		return "Jamming (JM)";
	if (flagAbbrev == "WA")
		return "Wide Angle (WA)";
	if (flagAbbrev == "NJ")
		return "No Jumping (NJ)";
	if (flagAbbrev == "TR")
		return "Trigger Happy (TR)";
	if (flagAbbrev == "RC")
		return "Reverse Controls (RC)";
	if (flagAbbrev == "R*")
		return "Red Team (R*)";
	if (flagAbbrev == "G*")
		return "Green Team (G*)";
	if (flagAbbrev == "B*")
		return "Blue Team (B*)";
	if (flagAbbrev == "P*")
		return "Purple Team (P*)";
	
	return "";
}

bool KeepAwayMapHandler::handle ( bz_ApiString object, bz_CustomMapObjectInfo *data )
{
	if (object != "KEEPAWAY" || !data)
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

			if ( key == "TEAMPLAY" && nubs->size() > 0 )
				keepaway.teamPlay = true;

			else if ( key == "AUTOTIME" && nubs->size() > 0 )
				keepaway.autoTimeOn = true;

			else if ( key == "AUTOTIME" && nubs->size() > 2 )
			{
				double temp1 = (double)atof(nubs->get(1).c_str());
				double temp2 = (double)atof(nubs->get(2).c_str());
				if (temp1 >= 1 && temp1 <= 99) // if parameter out of range, keeps default
					keepaway.timeMult = temp1 / 100;
				if (temp2 >= 1 && temp2 <= 99) // if parameter out of range, keeps default
					keepaway.timeMultMin = temp2 / 100;
				keepaway.autoTimeOn = true;
			}
			
			else if ( key == "NOSOUND" && nubs->size() > 0 )
				keepaway.soundEnabled = false;

			else if ( key == "HOLDTIME" && nubs->size() > 1 )
			{
				double temp = (double)atof(nubs->get(1).c_str());
				if (temp >= 1 && temp <= 7200) // if parameter out of range, keeps default
					keepaway.TTH = temp;
			}

			else if ( key == "KEEPAWAYFLAGS" && nubs->size() > 1)
			{
				for (unsigned int i = 1; i < nubs->size(); i++)
				{
					std::string flag = nubs->get(i).c_str();
					if (convertFlag(flag) != "")  // must be valid flag type, reject nub otherwise
						keepaway.flagsList.push_back(flag);
				}
			}

			else if ( key == "FORCEDFLAGS" && nubs->size() > 0 )
				keepaway.forcedFlags = true;
		}
		bz_deleteStringList(nubs);
	}

	if (keepaway.flagsList.size() > 0)
		keepaway.flagToKeepIndex = -1; // this will increment 1 when we get to getFlag() function;
	else
	{
		keepaway.flagToKeep = ""; // map file didn't give us any flags
		keepaway.flagToKeepIndex = 0;
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

	return "";
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

	if (Test1 < 1 && Test2 < 2)
	{
		if (!keepaway.oneTeamWarn)
			bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Keep Away disabled: less than 2 teams.");

		keepaway.oneTeamWarn = true;
		return true;
	}
	else
	{
		if (keepaway.oneTeamWarn)
			bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Keep Away enabled: more than 1 team.");

		keepaway.oneTeamWarn = false;
		return false;
	}
}

void autoTime()
{
	int numPlayers = bz_getTeamCount(eRedTeam) + bz_getTeamCount(eGreenTeam) + bz_getTeamCount(eBlueTeam) + bz_getTeamCount(ePurpleTeam) + bz_getTeamCount(eRogueTeam);

	if (!keepaway.autoTimeOn || numPlayers < 3)
	{
		keepaway.adjustedTime = keepaway.TTH;
		return;
	}

	double timeDown = ( 1 - ((double)numPlayers - 2) * keepaway.timeMult);

	if (timeDown < keepaway.timeMultMin) 
		timeDown = keepaway.timeMultMin;

	keepaway.adjustedTime = (int)(keepaway.TTH * timeDown);

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

void killTeams(bz_eTeamType safeteam, std::string keepawaycallsign)
{
	bz_APIIntList *playerList = bz_newIntList(); 
	bz_getPlayerIndexList ( playerList ); 

	for ( unsigned int i = 0; i < playerList->size(); i++ ){ 
       
		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player){
			
				if (player->team != safeteam)
				{
					bz_killPlayer(player->playerID, true, BZ_SERVER);
					if (keepaway.soundEnabled)
						bz_sendPlayCustomLocalSound(player->playerID,"flag_lost");
				}
				else if (keepaway.soundEnabled)
					bz_sendPlayCustomLocalSound(player->playerID,"flag_won");
			}

		bz_freePlayerRecord(player);	
	}
	bz_deleteIntList(playerList); 

	bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) Kept the Flag Away!", getTeamColor(safeteam), keepawaycallsign.c_str());

	return;
}

void killPlayers(int safeid, std::string keepawaycallsign)
{
	bz_APIIntList *playerList = bz_newIntList(); 
	bz_getPlayerIndexList ( playerList ); 

	for ( unsigned int i = 0; i < playerList->size(); i++ ){ 
       
		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player){
			
				if (player->playerID != safeid)
				{
					bz_killPlayer(player->playerID, true, keepaway.id);
					if (keepaway.soundEnabled)
						bz_sendPlayCustomLocalSound(player->playerID,"flag_lost");
				}
				else if (keepaway.soundEnabled)
					bz_sendPlayCustomLocalSound(player->playerID,"flag_won");
			}

		bz_freePlayerRecord(player);	
	}

	bz_deleteIntList(playerList); 

	bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s Kept the Flag Away!", keepawaycallsign.c_str());
	
	return;
}

void sendWarnings(const char* teamcolor, std::string playercallsign, double keepawaystartedtime)
{
	double TimeElapsed = bz_getCurrentTime() - keepawaystartedtime;
	double TimeRemaining = keepaway.adjustedTime - TimeElapsed;
	int toTens = int((TimeRemaining + 5) / 10) * 10;
	
	if ((TimeRemaining/60) < keepaway.TTHminutes && keepaway.adjustedTime > 59 && TimeRemaining >= 1)
	{
		if (!keepaway.teamPlay || keepaway.team == eRogueTeam)
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s has %s flag; %i secs left!", playercallsign.c_str(), keepaway.flagToKeep.c_str(), toTens);
		else
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) has %s flag; %i secs left!", teamcolor, playercallsign.c_str(), keepaway.flagToKeep.c_str(), toTens);	
		
		keepaway.TTHminutes--;
	}
	
	if (keepaway.adjustedTime < keepaway.TTHseconds)
	{
		keepaway.TTHseconds = keepaway.TTHseconds - 10;
		return;
	}

	if (TimeRemaining < keepaway.TTHseconds && TimeRemaining >= 1)
	{
		if (!keepaway.teamPlay || keepaway.team == eRogueTeam)
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s has %s flag; %i secs left!", playercallsign.c_str(), keepaway.flagToKeep.c_str(), keepaway.TTHseconds);
		else
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) has %s flag; %i secs left!", teamcolor, playercallsign.c_str(), keepaway.flagToKeep.c_str(), keepaway.TTHseconds);

		keepaway.TTHseconds = keepaway.TTHseconds - 10;
	}
	return;
}

std::string getFlag()
{
	if (keepaway.flagToKeepIndex < -1) // this should never happen, but save a crash if something goes nuts
		return "";

	// get next flag; if not free take it from player (if forced flags)

	for (unsigned int h = 0; h < keepaway.flagsList.size(); h++) // check all specified flags
	{
		keepaway.flagToKeepIndex++; // get next one in line

		if (keepaway.flagToKeepIndex > ((int)keepaway.flagsList.size() - 1)) // go back to start if at end
			keepaway.flagToKeepIndex = 0;

		std::string flagCandidate = keepaway.flagsList[keepaway.flagToKeepIndex];
		bool flagNotHeld = true;

		bz_APIIntList *playerList = bz_newIntList(); 
		bz_getPlayerIndexList ( playerList ); 
	
		for ( unsigned int i = 0; i < playerList->size(); i++ ) 
		{ 
       		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player)
			{
				const char* playerFlag = bz_getPlayerFlag(player->playerID);
				if (playerFlag)
				{
					if (playerFlag == flagCandidate && keepaway.forcedFlags) // take it, if forced flags
					{
						bz_removePlayerFlag (player->playerID);
						bz_sendTextMessage (BZ_SERVER, player->playerID, "Sorry, server needs your flag for Keep Away :/");
					}
					if (playerFlag == flagCandidate && !keepaway.forcedFlags) // look for next free flag in list
						flagNotHeld = false;
				}
			}
			bz_freePlayerRecord(player);
		}

		bz_deleteIntList(playerList); 

		if (flagNotHeld)
			return flagCandidate;
	}

	if (keepaway.flagsList.size() > 0)  // we should never get here, but if we do keep going
		return keepaway.flagsList[0]; 
	else
		return "";
}

void initiatekeepaway(bz_eTeamType plyrteam, bz_ApiString plyrcallsign, int plyrID)
{
	keepaway.team = plyrteam;
	keepaway.callsign = plyrcallsign.c_str();

	if (keepaway.callsign.size() > 16)
	{
		std::string tofix = truncate(keepaway.callsign, 16);
		keepaway.callsign = tofix;
	}

	keepaway.id = plyrID;
	keepaway.startTime = bz_getCurrentTime();
	keepaway.TTHminutes = (int)(keepaway.adjustedTime/60 + 0.5);
	keepaway.TTHseconds = 30;
	keepaway.toldFlagFree = false;
	bool multipleof30 = false;

	if ((int)((keepaway.adjustedTime / 30) + 0.5) != (double)(keepaway.adjustedTime / 30))
		multipleof30 = false;
	else
		multipleof30 = true;

	if (!multipleof30)
	{
		if ((!keepaway.teamPlay || keepaway.team == eRogueTeam))
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s has %s flag; %i secs left!", keepaway.callsign.c_str(), keepaway.flagToKeep.c_str(), (int)keepaway.adjustedTime);
		else
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) has %s flag; %i secs left!", getTeamColor(keepaway.team), keepaway.callsign.c_str(), keepaway.flagToKeep.c_str(), (int)keepaway.adjustedTime);
	}

	if (keepaway.soundEnabled)
	{
		bz_APIIntList *playerList = bz_newIntList(); 
		bz_getPlayerIndexList ( playerList ); 

		for ( unsigned int i = 0; i < playerList->size(); i++ ) 
		{ 
     		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player)
			{
				if ((player->team != keepaway.team || player->team == eRogueTeam) && player->playerID != keepaway.id)
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

void playAlert()
{
	bz_APIIntList *playerList = bz_newIntList(); 
	bz_getPlayerIndexList ( playerList ); 

	for ( unsigned int i = 0; i < playerList->size(); i++ ) 
	{ 
     	bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

		if (player)
			bz_sendPlayCustomLocalSound(player->playerID,"hunt_select");
	
		bz_freePlayerRecord(player);
	}
	bz_deleteIntList(playerList); 

	return;
}
	
inline bool timeForReminder()
{
	double timeLeft = bz_getCurrentTime() - keepaway.lastReminder;
	if (timeLeft >= keepaway.reminderPeriod)
	{
		keepaway.lastReminder = bz_getCurrentTime();
		return true;
	}
	return false;
}

void KeepAwayPlayerPaused::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerPausedEvent || !keepaway.enabled || keepaway.flagToKeep == "")
		return;

	bz_PlayerPausedEventData_V1 *PauseData = (bz_PlayerPausedEventData_V1*)eventData;
	
	bz_BasePlayerRecord *player = bz_getPlayerByIndex(PauseData->player);

	if (player)
	{
		const char* flagHeld = bz_getPlayerFlag(player->playerID);
	
		if (flagHeld)
		{
			if (flagHeld == keepaway.flagToKeep)
			{
				bz_removePlayerFlag (player->playerID);
				bz_sendTextMessage (BZ_SERVER, PauseData->player, "Flag removed - cannot pause while holding flag.");
				keepaway.id = -1;
				keepaway.team = eNoTeam;
				keepaway.toldFlagFree = false;
			}
		}
	}
	bz_freePlayerRecord(player);

	return;
}

void KeepAwayPlayerJoined::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerJoinEvent || !keepaway.enabled || keepaway.flagToKeep == "")
		return;

	bz_PlayerJoinPartEventData_V1 *joinData = (bz_PlayerJoinPartEventData_V1*)eventData;

	if (keepaway.flagToKeep == "Initiate") //first time server starts, first player initiates it.
	{
		keepaway.flagToKeep = getFlag();
		keepaway.lastReminder = bz_getCurrentTime();
	}

	autoTime();

	if (oneTeam(eNoTeam)) // don't send message if not enough teams
	{
		keepaway.notEnoughTeams = true;
		return;
	}
	else
		keepaway.notEnoughTeams = false;

	if (keepaway.id == -1 && keepaway.enabled && keepaway.flagToKeep != "")
	{
		bz_sendTextMessagef (BZ_SERVER, joinData->playerID, "Keep Away flag is %s: find it and keep it for %i seconds!", convertFlag(keepaway.flagToKeep).c_str(), (int)keepaway.adjustedTime); 
		if (keepaway.soundEnabled)
			bz_sendPlayCustomLocalSound(joinData->playerID,"hunt_select");
	}

	if (keepaway.id != -1 && keepaway.enabled && keepaway.flagToKeep != "" && (joinData->team != keepaway.team || joinData->team == eRogueTeam))
	{
		bz_sendTextMessagef (BZ_SERVER, joinData->playerID, "%s has Keep Away flag %s - kill him/her before time's up!", keepaway.callsign.c_str(), convertFlag(keepaway.flagToKeep).c_str()); 
		if (keepaway.soundEnabled)
			bz_sendPlayCustomLocalSound(joinData->playerID,"flag_alert");
	}
	
	if (keepaway.id != -1 && keepaway.enabled && keepaway.flagToKeep != "" && (joinData->team == keepaway.team && joinData->team != eRogueTeam))
	{
		bz_sendTextMessagef (BZ_SERVER, joinData->playerID, "%s has Keep Away flag %s - protect him/her until time's up!", keepaway.callsign.c_str(), convertFlag(keepaway.flagToKeep).c_str()); 
		if (keepaway.soundEnabled)
			bz_sendPlayCustomLocalSound(joinData->playerID,"teamgrab");
	}

	return;
}

void KeepAwayPlayerLeft::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerPartEvent || !keepaway.enabled || keepaway.flagToKeep == "")
		return;

	autoTime();

	bz_PlayerJoinPartEventData_V1 *partData = (bz_PlayerJoinPartEventData_V1*)eventData;

	if (partData->playerID == keepaway.id)
	{
		keepaway.id = -1;
		keepaway.team = eNoTeam;
		keepaway.toldFlagFree = false;
	}
	
	if (oneTeam(partData->team)) // team count check
		keepaway.notEnoughTeams = true;
	else
		keepaway.notEnoughTeams = false;

	return;
}

void KeepAwayPlayerDied::process ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerDieEvent || !keepaway.enabled || keepaway.flagToKeep == "")
		return;

	bz_PlayerDieEventData_V1 *dieData = (bz_PlayerDieEventData_V1*)eventData;

	if (dieData->playerID == keepaway.id)
	{
		keepaway.id = -1;
		keepaway.team = eNoTeam;
		keepaway.toldFlagFree = false;
	}
	
	return;
}

inline void checkKeepAwayHolder()
{
	bz_APIIntList *playerList = bz_newIntList(); 
	bz_getPlayerIndexList ( playerList ); 

	for ( unsigned int i = 0; i < playerList->size(); i++ ){ 
       
		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i)); 

			if (player)
			{
				const char* flagHeld = bz_getPlayerFlag(player->playerID);
				if (flagHeld)
				{
					if (flagHeld == keepaway.flagToKeep && keepaway.id == -1) // gotta a new one; initiate
					{
						initiatekeepaway(player->team, player->callsign, player->playerID);
						bz_freePlayerRecord(player);
						bz_deleteIntList(playerList); 
						return;
					}
					if (flagHeld == keepaway.flagToKeep && keepaway.id == player->playerID) // someone still has it; leave
					{
						bz_freePlayerRecord(player);
						bz_deleteIntList(playerList); 
						return;
					}
					if (flagHeld == keepaway.flagToKeep && keepaway.id != player->playerID) // must have stolen it
					{
						initiatekeepaway(player->team, player->callsign, player->playerID);
						bz_freePlayerRecord(player);
						bz_deleteIntList(playerList); 
						return;
					}
				}
			}

		bz_freePlayerRecord(player);	
	}

	keepaway.id = -1;  // no one has flag
	keepaway.team = eNoTeam;

	bz_deleteIntList(playerList); 

	return;
}

void KeepAwayEventHandler::process ( bz_EventData *eventData )
{

	if (eventData->eventType != bz_ePlayerUpdateEvent || !keepaway.enabled || keepaway.flagToKeep == "")
		return;

	if (keepaway.notEnoughTeams) // Not enough teams - we can leave
		return;
	
	checkKeepAwayHolder(); // check for someone holding flag

	if (!keepaway.toldFlagFree && keepaway.id == -1) // Flag is free - inform players
	{
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Keep Away flag: %s is free; find it and keep it!", convertFlag(keepaway.flagToKeep).c_str()); 
		keepaway.toldFlagFree = true;
		
		if ((bz_getCurrentTime() - keepaway.lastReminder) > 2 && keepaway.soundEnabled) // do not play free flag sound alert if player just won/lost (overlapping sounds)
			playAlert();
	}

	if (timeForReminder() && keepaway.id == -1)
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Keep Away flag: %s is free; find it and keep it!", convertFlag(keepaway.flagToKeep).c_str()); 

	if (keepaway.id == -1)  // no one has it, we can leave
		return;

	sendWarnings(getTeamColor(keepaway.team), keepaway.callsign, keepaway.startTime);

	double timeStanding = bz_getCurrentTime() - keepaway.startTime;

	if (timeStanding >= keepaway.adjustedTime) // time's up - kill 'em
	{
		if (keepaway.teamPlay && keepaway.team != eRogueTeam)
		{
			killTeams(keepaway.team, keepaway.callsign);
			bz_sendTextMessage (BZ_SERVER, keepaway.team, "Your team did it!  Go find the next Keep Away flag and keep it!");
		}
		else
		{
			killPlayers(keepaway.id, keepaway.callsign);
			bz_sendTextMessage (BZ_SERVER, keepaway.id, "You did it!  Go find the next Keep Away flag and keep it!");
		}

		if (!keepaway.forcedFlags)  // this will always create an open spot for getFlag(), if it's needed
			bz_removePlayerFlag (keepaway.id);

		keepaway.id = -1;
		keepaway.team = eNoTeam;
		keepaway.toldFlagFree = false;
		keepaway.flagToKeep = getFlag();
		keepaway.lastReminder = bz_getCurrentTime();

		return;
	}
}

bool KeepAwayCommands::handle ( int playerID, bz_ApiString _command, bz_ApiString _message, bz_APIStringList * /*_param*/ )
{
	std::string command = _command.c_str();
	std::string message = _message.c_str();
	const char* keepermessage = _message.c_str();

	if ( command == "kas" )
	{
		if (keepaway.id != -1)
			bz_sendTextMessage (playerID, keepaway.id, keepermessage);
		else
			bz_sendTextMessage(BZ_SERVER, playerID, "There is no one keeping the flag right now.");

		return true;
	}

	if ( command == "kaf" )
	{
		if (keepaway.id == -1)
			bz_sendTextMessagef (BZ_SERVER, playerID, "The Keep Away flag is: %s", convertFlag(keepaway.flagToKeep).c_str());
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "%s has Keep Away flag: %s", keepaway.callsign.c_str(), convertFlag(keepaway.flagToKeep).c_str()); 

		return true;
	}

	bz_BasePlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);

	if ( !fromPlayer->admin )
	{
    	bz_sendTextMessage(BZ_SERVER, playerID, "You must be admin to use the keepaway commands.");
		bz_freePlayerRecord(fromPlayer);
		return true;
	}

	bz_freePlayerRecord(fromPlayer);

	if ( command == "kasoundoff" )
	{
		keepaway.soundEnabled = false;
		bz_sendTextMessage (BZ_SERVER, playerID, "Keep Away sounds are disabled.");
		return true;
	}
		if ( command == "kasoundon" )
	{
		keepaway.soundEnabled = true;
		bz_sendTextMessage (BZ_SERVER, playerID, "Keep Away sounds are enabled.");
		return true;
	}

	if ( command == "kaf+" )
	{
		if (!keepaway.forcedFlags)  // this will always create an open spot for getFlag(), if it's needed
			bz_removePlayerFlag (keepaway.id);

		keepaway.id = -1;
		keepaway.team = eNoTeam;
		keepaway.toldFlagFree = false;
		keepaway.flagToKeep = getFlag();
		keepaway.lastReminder = bz_getCurrentTime();

		bz_sendTextMessagef(BZ_SERVER, playerID, "Keep Away flag advanced to: %s", convertFlag(keepaway.flagToKeep).c_str());

		return true;
	}

	if ( command == "kaon")
	{
		keepaway.enabled = true;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Keep Away is enabled.");
		return true;
	}

	if ( command == "kaoff")
	{
		keepaway.enabled = false;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Keep Away is disabled.");
		return true;
	}

	if ( command == "katimemult")
	{
		double inputvalue = ConvertToNum(message, 1, 99);

		if (inputvalue > 0)
		{
			keepaway.timeMult = (inputvalue/100);
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away auto time multiplier set to %i percent.", (int)(keepaway.timeMult*100 + 0.5));
		}
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away auto time multiplier must be between 1 and 99 percent.", (int)(keepaway.timeMult*100 + 0.5));

		autoTime();

		return true;
	}

	if ( command == "katimemultmin")
	{
		double inputvalue = ConvertToNum(message, 1, 99);

		if (inputvalue > 0)
		{
			keepaway.timeMultMin = (inputvalue/100);
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away auto time multiplier minimum set to %i percent.", (int)(keepaway.timeMultMin*100 + 0.5));
		}
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away auto time multiplier minimum must be between 1 and 99 percent.");

		autoTime();

		return true;
	}

	if ( command == "kastatus")
	{
		if (keepaway.enabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away is currently enabled.");

		if (!keepaway.enabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away is currently disabled.");

		if (keepaway.autoTimeOn)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away automatic time adjustment is currently enabled.");

		if (!keepaway.autoTimeOn)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away automatic time adjustment is currently disabled.");

		bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away time multiplier = %i percent.", (int)(keepaway.timeMult*100 + 0.5));

		bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away time multiplier minimum = %i percent.", (int)(keepaway.timeMultMin*100 + 0.5));
		
		int AdjTime = (int)(keepaway.adjustedTime + 0.5);
		bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away hold time is currently set to: %i seconds", AdjTime);

		if (keepaway.forcedFlags)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away forced flags is enabled.");

		if (!keepaway.forcedFlags)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away forced flags is disabled.");

		if (keepaway.soundEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away sounds are enabled.");

		if (!keepaway.soundEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away sounds are disabled.");

		return true;
	}
    
  // explicit time command handler:

	if ( command == "katime" )
	{
		double inputvalue = ConvertToNum(message, 1, 7200);
  
		if (inputvalue > 0 )
		{
			keepaway.TTH = inputvalue;
			autoTime();
			int AdjTime = (int)(inputvalue + 0.5);
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Keep Away hold time has been set to %i seconds.", AdjTime);
		}
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "Keep Away hold time invalid: must be between 1 and 7200 seconds.");

		autoTime();

		return true;
	}

	if ( command == "kaautotimeon")
	{
		keepaway.autoTimeOn = true;
		autoTime();
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Keep Away automatic time adjustment on.");
		return true;
	}

	if ( command == "kaautotimeoff")
	{
		keepaway.autoTimeOn = false;
		keepaway.adjustedTime = keepaway.TTH;
		autoTime();
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Keep Away automatic time adjustment off.");
		return true;
	}

	if ( command == "kaffon")
	{
		keepaway.forcedFlags = true;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Forced flags on.");
		return true;
	}

	if ( command == "kaffoff")
	{
		keepaway.forcedFlags = false;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Forced flags off.");
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

