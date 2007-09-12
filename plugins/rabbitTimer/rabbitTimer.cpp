// rabbitTimer.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

BZ_GET_PLUGIN_VERSION

class rabbitTimer : public bz_EventHandler
{
public:
	virtual void process(bz_EventData *eventData);

	float rabbitKillTimeLimit; //the max time the rabbit has to kill someone
	bool rollOver; //whether or not to roll over leftover seconds

	float rabbitDeathTime;

	int currentRabbit;
};

rabbitTimer rabbittimer;

void rabbitTimer::process(bz_EventData *eventData)
{
	if (eventData->eventType == bz_eTickEvent)
	{
		bz_TickEventData* tickdata = (bz_TickEventData*)eventData;

		if (currentRabbit != -1 && tickdata->time >= rabbitDeathTime)
		{
			//kill the wabbit!
			bz_killPlayer(currentRabbit, false, BZ_SERVER);

			//stopgap. the kill event should do this, really...
			currentRabbit = -1;
			rabbitDeathTime = tickdata->time + rabbitKillTimeLimit;

			bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Time's up! Selecting new rabbit.");
		}
		else if (currentRabbit == -1 && bz_getTeamCount(eHunterTeam) >= 3) //make sure we've got enough people before reactivating the timer
		{
			//find the new rabbit
			bzAPIIntList pl;
			bz_getPlayerIndexList(&pl);

			for (unsigned int i = 0; i < pl.size() && currentRabbit == -1; i++)
			{
				bz_PlayerRecord* pr = bz_getPlayerByIndex(pl.get(i));

				if (pr != NULL)
				{
					if (pr->team == eRabbitTeam)
					{
						currentRabbit = pr->playerID;
						int limit = (int)rabbitKillTimeLimit;
						bz_sendTextMessage(BZ_SERVER, currentRabbit, bz_format("You have %d seconds to make a kill!", limit));
					}
					bz_freePlayerRecord(pr);
				}
			}
		}
	}
	else if (eventData->eventType == bz_ePlayerDieEvent)
	{

		bz_PlayerDieEventData* killdata = (bz_PlayerDieEventData*)eventData;

		if (killdata->team == eRabbitTeam)
		{
			currentRabbit = -1; //we will sort this out on the next tick

			rabbitDeathTime = killdata->time + rabbitKillTimeLimit;
		}
		else if (killdata->killerTeam == eRabbitTeam && currentRabbit != -1)
		{
			if (rollOver)
			{
				rabbitDeathTime += rabbitKillTimeLimit;

				int limit = (int)rabbitKillTimeLimit;
				int timeremaining = (int)(rabbitDeathTime - killdata->time);

				bz_sendTextMessage(BZ_SERVER, currentRabbit, bz_format("+%d seconds: %d seconds remaining.", limit, timeremaining));
			}
			else
			{
				rabbitDeathTime = killdata->time + rabbitKillTimeLimit;

				int limit = (int)rabbitKillTimeLimit;
				bz_sendTextMessage(BZ_SERVER, currentRabbit, bz_format("%d seconds remaining.", limit));
			}
		}
	}
	else if (eventData->eventType == bz_ePlayerDieEvent) 
	{
		bz_PlayerJoinPartEventData* partdata = (bz_PlayerJoinPartEventData*)eventData;

		if (partdata->team == eRabbitTeam) //we need to select a new rabbit if the rabbit leaves.
		{
			currentRabbit = -1; //we will sort this out on the next tick

			rabbitDeathTime = partdata->time + rabbitKillTimeLimit;
		}
	}
}
	

BZF_PLUGIN_CALL int bz_Load(const char* commandLine)
{
	rabbittimer.rabbitKillTimeLimit = 30.0;
	rabbittimer.rollOver = false;
	rabbittimer.currentRabbit = -1;
	rabbittimer.rabbitDeathTime = 3600.0; //something large

	std::string param = commandLine;
	
	if (param.size() > 0 && param.at(0) == '+')
	{
		rabbittimer.rollOver = true;
		param  = param.erase(0,1);
	}

	int newtime = atoi(param.c_str());

	if (newtime > 0)
	{
		rabbittimer.rabbitKillTimeLimit = (float)newtime;
	}

	bz_registerEvent(bz_ePlayerDieEvent,&rabbittimer);
	bz_registerEvent(bz_ePlayerPartEvent,&rabbittimer);
	bz_registerEvent(bz_eTickEvent,&rabbittimer);

	bz_debugMessage(4, "rabbitTimer plugin loaded");
	return 0;
}

BZF_PLUGIN_CALL int bz_Unload()
{

	bz_removeEvent(bz_ePlayerDieEvent,&rabbittimer);
	bz_removeEvent(bz_ePlayerPartEvent,&rabbittimer);
	bz_removeEvent(bz_eTickEvent,&rabbittimer);

	bz_debugMessage(4, "rabbitTimer plugin unloaded");
	return 0;
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

