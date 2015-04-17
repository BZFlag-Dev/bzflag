// koth.cpp : Defines the entry point for the DLL application.

#include "bzfsAPI.h"
#include <map>
#include <cmath>

class KOTHMapHandler : public bz_CustomMapObjectHandler
{
public:
	virtual bool MapObject ( bz_ApiString object, bz_CustomMapObjectInfo *data );
};

KOTHMapHandler	kothmaphandler;

class KOTHCommands : public bz_CustomSlashCommandHandler
{
public:
  virtual ~KOTHCommands(){};
  virtual bool SlashCommand ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *param );
};

KOTHCommands kothcommands;

class KOTHHandler : public bz_Plugin
{
public:
	virtual const char* Name (){return "King Of The Hill";}
	virtual void Init ( const char* config);
	virtual void Cleanup ();

	virtual void Event ( bz_EventData *eventData );

};

BZ_PLUGIN(KOTHHandler)

void KOTHHandler::Init(const char* /*commandLine*/){
	MaxWaitTime = 0.5f;

	bz_registerCustomMapObject("KOTH",&kothmaphandler);
	Register(bz_ePlayerUpdateEvent);
	Register(bz_ePlayerPausedEvent);
	Register(bz_ePlayerPartEvent);
	Register(bz_ePlayerJoinEvent);
	Register(bz_ePlayerDieEvent);
	bz_registerCustomSlashCommand("kothstatus",&kothcommands);
	bz_registerCustomSlashCommand("kothon",&kothcommands);
	bz_registerCustomSlashCommand("kothoff",&kothcommands);
	bz_registerCustomSlashCommand("kothsoundon",&kothcommands);
	bz_registerCustomSlashCommand("kothsoundoff",&kothcommands);
	bz_registerCustomSlashCommand("kothtimemult",&kothcommands);
	bz_registerCustomSlashCommand("kothtimemultmin",&kothcommands);
	bz_registerCustomSlashCommand("kothtime",&kothcommands);
	bz_registerCustomSlashCommand("kothautotimeon",&kothcommands);
	bz_registerCustomSlashCommand("kothautotimeoff",&kothcommands);
	bz_registerCustomSlashCommand("kingsay",&kothcommands);
}

void KOTHHandler::Cleanup(void){

	Flush();
	bz_removeCustomMapObject("KOTH");
	bz_removeCustomSlashCommand("kothstatus");
	bz_removeCustomSlashCommand("kothon");
	bz_removeCustomSlashCommand("kothoff");
	bz_removeCustomSlashCommand("kothsoundon");
	bz_removeCustomSlashCommand("kothsoundoff");
	bz_removeCustomSlashCommand("kothtimemult");
	bz_removeCustomSlashCommand("kothtimemultmin");
	bz_removeCustomSlashCommand("kothtime");
	bz_removeCustomSlashCommand("kothautotimeon");
	bz_removeCustomSlashCommand("kothautotimeoff");
	bz_removeCustomSlashCommand("kingsay");
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
		teamPlay = false;
		TTH = 60;
		adjustedTime = 60;
		timeMult = 0.03;
		timeMultMin = 0.50;
		enabled = true;
		toldHillOpen = false;
		onePlayerWarn = false;
		autoTimeOn = false;
		TTHminutes = 0;
		TTHseconds = 30;
		playerJustWon = -1;
		soundEnabled = true;
	}
	bz_eTeamType team;
	std::string callsign;
	double TTH;
	double adjustedTime;
	double timeMult;
	double timeMultMin;
	double startTime;
	bool teamPlay;
	bool enabled;
	bool toldHillOpen;
	bool onePlayerWarn;
	bool autoTimeOn;
	bool soundEnabled;
	int TTHminutes;
	int TTHseconds;
	int playerJustWon;
	int id;
};

KOTH koth;

class KOTHZone : public bz_CustomZoneObject {
public:
	KOTHZone() : bz_CustomZoneObject() {}
};

KOTHZone kothzone;

bool KOTHMapHandler::MapObject ( bz_ApiString object, bz_CustomMapObjectInfo *data )
{
	if (object != "KOTH" || !data)
		return false;

	kothzone.handleDefaultOptions(data);

	// parse all the chunks
	for ( unsigned int i = 0; i < data->data.size(); i++ )
	{
		std::string line = data->data.get(i).c_str();

		bz_APIStringList *nubs = bz_newStringList();
		nubs->tokenize(line.c_str()," ",0,true);

		if ( nubs->size() > 0)
		{
			std::string key = bz_toupper(nubs->get(0).c_str());

			if ( key == "TEAMPLAY")
			{
				koth.teamPlay = true;
			}
			else if ( key == "NOSOUND")
			{
				koth.soundEnabled = false;
			}
			else if ( key == "AUTOTIME" && nubs->size() == 1 )
			{
				koth.autoTimeOn = true;
			}
			else if ( key == "AUTOTIME" && nubs->size() > 2 )
			{
				double temp1 = (double)atof(nubs->get(1).c_str());
				double temp2 = (double)atof(nubs->get(2).c_str());
				if (temp1 >= 1 && temp1 <= 99)
					koth.timeMult = temp1 / 100;
				if (temp2 >= 1 && temp2 <= 99)
					koth.timeMultMin = temp2 / 100;
				koth.autoTimeOn = true;
			}
			else if ( key == "HOLDTIME" && nubs->size() > 1 )
			{
				double temp = (double)atof(nubs->get(1).c_str());
				if (temp >= 1 && temp <= 7200)
					koth.TTH = temp;
			}
		}

		bz_deleteStringList(nubs);
	}
	return true;
}

std::string truncate(std::string cllsn)
{
	std::string fixed = "";

	for (int i = 0; i < 16; i++)
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

bool onePlayer()
{
	int numPlayers = bz_getTeamCount(eRedTeam) + bz_getTeamCount(eGreenTeam) + bz_getTeamCount(eBlueTeam) + bz_getTeamCount(ePurpleTeam) + bz_getTeamCount(eRogueTeam);

	if (numPlayers <= 1)
	{
		if (!koth.onePlayerWarn)
			bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "King of the Hill disabled: less than 2 players.");

		koth.onePlayerWarn = true;
		return true;
	}
	else
	{
		if (koth.onePlayerWarn)
			bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "King of the Hill enabled: more than 1 player.");

		koth.onePlayerWarn = false;
		return false;
	}
}

void autoTime()
{
	int numPlayers = bz_getTeamCount(eRedTeam) + bz_getTeamCount(eGreenTeam) + bz_getTeamCount(eBlueTeam) + bz_getTeamCount(ePurpleTeam) + bz_getTeamCount(eRogueTeam);

	if (!koth.autoTimeOn || numPlayers < 3)
	{
		koth.adjustedTime = koth.TTH;
		return;
	}

	double timeDown = ( 1 - ((double)numPlayers - 2) * koth.timeMult);

	if (timeDown < koth.timeMultMin)
		timeDown = koth.timeMultMin;

	koth.adjustedTime = (int)(koth.TTH * timeDown);

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
					if (koth.soundEnabled)
						bz_sendPlayCustomLocalSound(player->playerID,"flag_lost");
				}
				else if (koth.soundEnabled)
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
					if (koth.soundEnabled)
						bz_sendPlayCustomLocalSound(player->playerID,"flag_lost");
				}
				else if (koth.soundEnabled)
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
	double TimeRemaining = koth.adjustedTime - TimeElapsed;
	int toTens = int((TimeRemaining + 5) / 10) * 10;

	if ((TimeRemaining/60) < koth.TTHminutes && koth.adjustedTime > 59)
	{
		if (!koth.teamPlay || koth.team == eRogueTeam)
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s will be King in %i secs!", playercallsign.c_str(), toTens);
		else
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) will be King in %i secs!", teamcolor, playercallsign.c_str(), toTens);

		koth.TTHminutes--;
	}

	if (koth.adjustedTime < koth.TTHseconds)
	{
		koth.TTHseconds = koth.TTHseconds - 10;
		return;
	}

	if (TimeRemaining < koth.TTHseconds)
	{
		if (!koth.teamPlay || koth.team == eRogueTeam)
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s will be King in %i secs!", playercallsign.c_str(), koth.TTHseconds);
		else
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) will be King in %i secs!", teamcolor, playercallsign.c_str(), koth.TTHseconds);

		koth.TTHseconds = koth.TTHseconds - 10;
	}
	return;
}

void initiatekoth(bz_eTeamType plyrteam, bz_ApiString plyrcallsign, int plyrID)
{
	koth.team = plyrteam;
	koth.callsign = plyrcallsign.c_str();

	if (koth.callsign.size() > 16)
	{
		std::string tofix = truncate(koth.callsign);
		koth.callsign = tofix;
	}

	koth.id = plyrID;
	koth.startTime = bz_getCurrentTime();
	koth.TTHminutes = (int)(koth.adjustedTime/60 + 0.5);
	koth.TTHseconds = 30;
	koth.toldHillOpen = false;
	bool multipleof30 = false;

	if ((int)((koth.adjustedTime / 30) + 0.5) != (double)(koth.adjustedTime / 30))
		multipleof30 = false;
	else
		multipleof30 = true;

	if (!multipleof30)
	{
		if ((!koth.teamPlay || koth.team == eRogueTeam))
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s has Hill; will be King in %i secs!", koth.callsign.c_str(), (int)koth.adjustedTime);
		else
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%s (%s) has Hill; will be King in %i secs!", getTeamColor(koth.team), koth.callsign.c_str(), (int)koth.adjustedTime);
	}

	if (koth.soundEnabled)
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
	if (teamToCheck == eRogueTeam || teamToCheck == eNoTeam || !koth.teamPlay)
		return true;

	bz_APIIntList *playerList = bz_newIntList();
	bz_getPlayerIndexList ( playerList );

	bool isOut = true;

	for ( unsigned int i = 0; i < playerList->size(); i++ ){

		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerList->operator[](i));

			if (player)
			{
				if (player->team == teamToCheck && kothzone.pointInZone(player->lastKnownState.pos) && player->spawned)
					isOut = false;
			}

		bz_freePlayerRecord(player);
	}

	bz_deleteIntList(playerList);

	return isOut;
}

void KOTHPlayerPaused ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerPausedEvent || !koth.enabled)
		return;

	bz_PlayerPausedEventData_V1 *PauseData = (bz_PlayerPausedEventData_V1*)eventData;
	bz_BasePlayerRecord *player = bz_getPlayerByIndex(PauseData->playerID);

	if (player)
	{
		if(kothzone.pointInZone(player->lastKnownState.pos))
		{
			bz_killPlayer (PauseData->playerID, true, BZ_SERVER);
			bz_sendTextMessage (BZ_SERVER, PauseData->playerID, "Cannot pause while on the Hill.");
		}
	}
	bz_freePlayerRecord(player);

	return;
}

void KOTHPlayerJoined ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerJoinEvent || !koth.enabled)
		return;

	autoTime();

	return;
}

void KOTHPlayerLeft ( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerPartEvent || !koth.enabled)
		return;

	autoTime();

	bz_PlayerJoinPartEventData_V1 *partData = (bz_PlayerJoinPartEventData_V1*)eventData;

	if (partData->playerID == koth.id)
	{
		koth.id = -1;
		koth.team = eNoTeam;
	}

	return;
}

void KOTHPlayerDied( bz_EventData *eventData )
{
	if (eventData->eventType != bz_ePlayerDieEvent || !koth.enabled)
		return;

	bz_PlayerDieEventData_V1 *dieData = (bz_PlayerDieEventData_V1*)eventData;

	if (dieData->playerID == koth.id)
	{
		koth.id = -1;
		koth.team = eNoTeam;
	}

	return;
}

inline void KOTHEventHandler( bz_EventData *eventData )
{
	if (!koth.enabled) // King of the Hill disabled - we can leave
		return;

	if (onePlayer()) // Not enough players - we can leave
		return;

	float pos[3] = {0};

	int playerID = -1;

	switch (eventData->eventType)
	{
	case bz_ePlayerUpdateEvent:
		pos[0] = ((bz_PlayerUpdateEventData_V1*)eventData)->state.pos[0];
		pos[1] = ((bz_PlayerUpdateEventData_V1*)eventData)->state.pos[1];
		pos[2] = ((bz_PlayerUpdateEventData_V1*)eventData)->state.pos[2];
		playerID = ((bz_PlayerUpdateEventData_V1*)eventData)->playerID;
		break;

	case bz_eShotFiredEvent:
		pos[0] = ((bz_ShotFiredEventData_V1*)eventData)->pos[0];
		pos[1] = ((bz_ShotFiredEventData_V1*)eventData)->pos[1];
		pos[2] = ((bz_ShotFiredEventData_V1*)eventData)->pos[2];
		playerID = ((bz_ShotFiredEventData_V1*)eventData)->playerID;
		break;

	default:
		return;
	}

	if (!koth.toldHillOpen && koth.id == -1) // Hill is open - inform players
	{
		bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Hill is not controlled - take it!");
		koth.toldHillOpen = true;
	}

	if (kothzone.pointInZone(pos)) // player is on Hill
	{
		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);

		if (player)
		{
			if (player->playerID != koth.playerJustWon && player->spawned)
			{
				if ((koth.id == -1 && player->team != koth.team) || (koth.id == -1 && teamClear(koth.team)))
					initiatekoth(player->team, player->callsign, player->playerID);

				double timeStanding = bz_getCurrentTime() - koth.startTime;

				if (timeStanding >= koth.adjustedTime && koth.id != -1) // time's up - kill 'em
				{
					if (koth.teamPlay && koth.team != eRogueTeam)
						killTeams(koth.team, koth.callsign);
					else
						killPlayers(koth.id, koth.callsign);

					if (!koth.teamPlay || koth.team == eRogueTeam)
						bz_sendTextMessage (BZ_SERVER, koth.id, "You are King of the Hill!  You must leave hill to retake it.");
					else
						bz_sendTextMessage (BZ_SERVER, koth.team, "Your team is King of the Hill!  Entire team must leave hill to retake it.");

					koth.playerJustWon = koth.id;

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
		if (playerID == koth.playerJustWon)
			koth.playerJustWon = -1;

		if (playerID == koth.id)
		{
			koth.id = -1;
			koth.team = eNoTeam;
		}
	}
}

void KOTHHandler::Event( bz_EventData *eventData )
{
	if (eventData->eventType == bz_ePlayerUpdateEvent)
		KOTHEventHandler(eventData);
	else if (eventData->eventType == bz_eShotFiredEvent)
		KOTHEventHandler(eventData);
	else if (eventData->eventType == bz_ePlayerDieEvent)
		KOTHPlayerDied(eventData);
	else if (eventData->eventType == bz_ePlayerJoinEvent)
		KOTHPlayerJoined(eventData);
	else if (eventData->eventType == bz_ePlayerPartEvent)
		KOTHPlayerLeft(eventData);
	else if (eventData->eventType == bz_ePlayerPausedEvent)
		KOTHPlayerPaused(eventData);

}


bool KOTHCommands::SlashCommand ( int playerID, bz_ApiString _command, bz_ApiString _message, bz_APIStringList * /*_param*/ )
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

	if (fromPlayer) {
	  if ( !fromPlayer->admin ) {
	    bz_sendTextMessage(BZ_SERVER,playerID,"You must be admin to use the koth commands.");
	    bz_freePlayerRecord(fromPlayer);
	    return true;
	  }

	  bz_freePlayerRecord(fromPlayer);
	}

	if ( command == "kothon")
	{
		koth.enabled = true;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill is enabled.");
		return true;
	}

	if ( command == "kothoff")
	{
		koth.enabled = false;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill is disabled.");
		return true;
	}

	if ( command == "kothsoundon")
	{
		koth.soundEnabled = true;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill sounds are enabled.");
		return true;
	}

	if ( command == "kothsoundoff")
	{
		koth.soundEnabled = false;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill sounds are disabled.");
		return true;
	}

	if ( command == "kothtimemult")
	{
		double inputvalue = ConvertToNum(message, 1, 99);

		if (inputvalue > 0)
		{
			koth.timeMult = (inputvalue/100);
			bz_sendTextMessagef (BZ_SERVER, playerID, "Auto time multiplier set to %i percent.", (int)(koth.timeMult*100 + 0.5));
		}
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "Auto time multiplier must be between 1 and 99 percent.", (int)(koth.timeMult*100 + 0.5));

		autoTime();

		return true;
	}

	if ( command == "kothtimemultmin")
	{
		double inputvalue = ConvertToNum(message, 1, 99);

		if (inputvalue > 0)
		{
			koth.timeMultMin = (inputvalue/100);
			bz_sendTextMessagef (BZ_SERVER, playerID, "Auto time multiplier minimum set to %i percent.", (int)(koth.timeMultMin*100 + 0.5));
		}
		else
			bz_sendTextMessagef (BZ_SERVER, playerID, "Auto time multiplier minimum must be between 1 and 99 percent.");

		autoTime();

		return true;
	}

	if ( command == "kothstatus")
	{
		if (koth.enabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill is currently enabled.");

		if (!koth.enabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill is currently disabled.");

		if (koth.soundEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill sounds are currently enabled.");

		if (!koth.soundEnabled)
			bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill sounds are currently disabled.");

		if (koth.autoTimeOn)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Automatic time adjustment is currently enabled.");

		if (!koth.autoTimeOn)
			bz_sendTextMessagef (BZ_SERVER, playerID, "Automatic time adjustment is currently disabled.");

		bz_sendTextMessagef (BZ_SERVER, playerID, "Time multiplier = %i percent.", (int)(koth.timeMult*100 + 0.5));

		bz_sendTextMessagef (BZ_SERVER, playerID, "Time multiplier minimum = %i percent.", (int)(koth.timeMultMin*100 + 0.5));

		int AdjTime = (int)(koth.adjustedTime + 0.5);
		bz_sendTextMessagef (BZ_SERVER, playerID, "King of the Hill hold time is currently set to: %i seconds", AdjTime);
		return true;
	}

  // explicit time command handler:

	if ( command == "kothtime" )
	{
		double inputvalue = ConvertToNum(message, 1, 7200);

		if (inputvalue > 0 )
		{
			koth.TTH = inputvalue;
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
		koth.autoTimeOn = true;
		autoTime();
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "King of the Hill automatic time adjustment on.");
		return true;
	}

	if ( command == "kothautotimeoff")
	{
		koth.autoTimeOn = false;
		koth.adjustedTime = koth.TTH;
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

