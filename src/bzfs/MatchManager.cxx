/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// BZFlag common header
#include "common.h"

// Interface header
#include "MatchManager.h"

#include "StateDatabase.h"
#include "TimeKeeper.h"
#include "TextUtils.h"


/** Utility Countdown class */

CountDown::CountDown(double interval, int count)
{
	_interval = interval;
	_currentTime = 0;
	_previousTime = 0;
	_startCount = count + 1;
	_counter = _startCount;
}

int CountDown::getCounter()
{
	return _counter;
}

void CountDown::setCounter(int count)
{
	_startCount = _counter = count + 1;
}

void CountDown::doReset()
{
	_currentTime = bz_getCurrentTime();
	_previousTime = _currentTime;
	_counter = _startCount;
}

bool CountDown::doCountdown()
{
	_currentTime = bz_getCurrentTime();

	if ( ( _currentTime - _previousTime ) > _interval )
	{
		_counter--;
		_previousTime = _currentTime;
		return true;
	}

	return false;
}

bool CountDown::inProgress()
{
	return ( _counter > 1 );
}

/** MatchManager class */

/** initialize the singleton */
template <>
MatchManager* Singleton<MatchManager>::_instance = (MatchManager*)0;

/**
* default constructor, protected because of singleton
*/
MatchManager::MatchManager() : Singleton<MatchManager>()
{	
	// start future BZDB vars
	_matchPregameTime = 60;
	_matchDuration = 1800;
	_matchEndCountdown = 30;
	_matchResetTime = 120;

	_matchDisallowJoins = false;
	_matchResetScoreOnEnd = true;
	_matchReportMatches = true;
	// end future BZDB vars

	matchState = eOff;
	startTime = -1;
	duration = -1;
	endTime = 0;
	resetTime = _matchResetTime;

	resumeTime = -1;
	pauseTime = -1;


	paused = false;
	report = false;

	currentTime = 0;

	// register events & commands
	bz_registerCustomSlashCommand("match",this);
	bz_registerEvent(bz_eGetAutoTeamEvent,this);
	bz_registerEvent(bz_eTickEvent,this);
	bz_registerEvent(bz_eReportFiledEvent,this);

	bz_setMaxWaitTime(0.1f,"MATCHMANAGER");
}

/**
* default destructor, protected because of singleton
*/
MatchManager::~MatchManager()
{
	bz_removeCustomSlashCommand("match");
	bz_removeEvent(bz_eGetAutoTeamEvent,this);
	bz_removeEvent(bz_eTickEvent,this);
	bz_removeEvent(bz_eReportFiledEvent,this);

	bz_clearMaxWaitTime("MATCHMANAGER");
}

void MatchManager::start ( int playerID, bz_APIStringList *params )
{
	if ( matchState == eOn || matchState == ePregame )
		bz_sendTextMessage (BZ_SERVER, playerID, "A match is currently in progress");
	else 
		{
			if ( matchState == eOff || matchState == ePostgame )
			{
				if (params->size() == 2)
					duration = atoi(params->get(1).c_str());
				else duration = _matchDuration;

				matchState = ePregame;
				preGameTimer.doReset();
				preGameTimer.setCounter( (int) _matchPregameTime);

				// disable player spawn
				disablePlayerSpawn();

				// reset flags
				bz_resetFlags(false);

				// reset player/team scores
				resetTeamScores();
				resetPlayerScores();

				bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "A match will start in %.0f seconds", _matchPregameTime);
			}
		}
}

void MatchManager::end ( int playerID, bz_APIStringList * /* params */)
{
	if ( matchState == eOn )
	{
		matchState = ePostgame;
		paused = false;
		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "The match ended after %.0f seconds", bz_getCurrentTime() - startTime);
	}
	else
		bz_sendTextMessage (BZ_SERVER, playerID, "No match in progress");
}

void MatchManager::pause ( int playerID, bz_APIStringList *params )
{

	// pause can only be used when a match is in progress
	if ( matchState == eOn )
	{
		double d = 0;

		if (params->size() == 2)
			d = atoi(params->get(1).c_str());

		// resume the match when already paused else pause
		if (paused)
		{
			resumeTime = bz_getCurrentTime() + d;
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "The match will be resumed in %.0f seconds", resumeTime - currentTime);
		}
		else 
			{
				paused=true;
				pauseTime = bz_getCurrentTime();

				if ( d > 0 )
				{
					resumeTime = bz_getCurrentTime() + d;
					bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "The match will be paused for %.0f seconds", resumeTime - currentTime);
				}
				else
					bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "The match is paused");
			}
	}
	else
		bz_sendTextMessagef (BZ_SERVER, playerID, "No match to pause");
}

void MatchManager::substitute ( int /* playerID */, bz_APIStringList * /* params */ )
{

}

void MatchManager::doPregame()
{			
	if ( preGameTimer.inProgress() )
	{
		if ( preGameTimer.doCountdown() )
			bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "%d ...", preGameTimer.getCounter());
	}
	else
		{
			matchState = eOn;	
			endTimer.setCounter((int) _matchEndCountdown);
			endTimer.doReset();
			startTime = currentTime;	

			bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "The match started");
		}
}

void MatchManager::doOngame()
{

	// check if someone unpaused the match, if so resume it
	if ( paused && currentTime >= resumeTime && resumeTime != -1 )
	{
		duration += currentTime - pauseTime; 
 		paused = false;

		bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Match resumed after %.0f seconds", currentTime - pauseTime); 

		resumeTime = -1;
		pauseTime = -1;
	}

	// start match end countdown
	if ( !paused && currentTime >= ((startTime + duration) - _matchEndCountdown) )
	{
		if ( endTimer.inProgress() )
		{
			if ( endTimer.doCountdown() )
				bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Still %d sec to go before match ends", endTimer.getCounter());
		}
		else
			{
				matchState = ePostgame;
				endTime = startTime + duration + resetTime;
				bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "The match ended");
				if ( report )
					doReportgame();

				startTime = -1;
			}
	}
}

void MatchManager::doPostgame()
{
	if ( currentTime >= endTime )
	{
		matchState = eOff;
		bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Free play is resumed");
	}
}

void MatchManager::doReportgame()
{
	bz_fileReport("report scores to the report channel");
}

void MatchManager::disablePlayerSpawn()
{
	bz_APIIntList * players = bz_getPlayerIndexList();

	for ( unsigned int i = 0; i < players->size(); i++ ) 
	{
		if ( bz_getPlayerTeam(players->get(i)) != eObservers)
		{
			if ( bz_setPlayerSpawnable(players->get(i), false) )
				bz_debugMessagef(2, "DEBUG :: no spawn success :: player => %d ", players->get(i)); 
			else
				bz_debugMessagef(2, "DEBUG :: no spawn failed :: player => %d ", players->get(i)); 
		}
	}

}

void MatchManager::resetTeamScores()
{

}

void MatchManager::resetPlayerScores()
{

}

void MatchManager::process ( bz_EventData *eventData )
{

	if ( !eventData )
		return;

	if ( eventData->eventType == bz_eGetAutoTeamEvent )
	{

	}

	if ( eventData->eventType == bz_eTickEvent )
	{
		currentTime = ((bz_TickEventData_V1 *) eventData)->eventTime;

		if ( matchState == ePregame )
			doPregame();
		else 
			if ( matchState == eOn)
				doOngame();
		else 
			if ( matchState == ePostgame)
				doPostgame();
	}


	if ( eventData->eventType == bz_eReportFiledEvent )
	{
		bz_ReportFiledEventData_V1 * data = (bz_ReportFiledEventData_V1 *) eventData;
		bz_debugMessagef(2, "DEBUG :: version => %d reporterID => %d :: message => %s :: time => %.0f", 
						data->version, data->playerID, data->message.c_str(), data->eventTime);
	}

}

bool MatchManager::handle ( int playerID, bz_ApiString command, bz_ApiString /* message */, bz_APIStringList *params )
{

	if ( command == "match")
	{
		double now = TimeKeeper::getCurrent().getSeconds();

		if ( !params->size() )
		{
			std::string msg;
			switch (matchState)
			{

				default:
						msg = "No match is in progress";
						break;

				case ePregame:
						msg = TextUtils::format("Match will start in %.0f seconds", startTime - now);
						break;

				case eOn:
						if ( !paused )	
							msg = TextUtils::format("Match is in progress, still %.0f seconds to go", (startTime + duration) - now);
						else
							msg = "Match is paused";
						break;

				case ePostgame:
						msg = TextUtils::format("Match is over, server will resume free play in %.0f seconds",endTime - now);
						break;
			}

			bz_sendTextMessagef (BZ_SERVER, playerID, "%s", msg.c_str());
			return true;

		}
		else 
			{
				// check if player has the perms to exectue the match command
				if ( !bz_hasPerm(playerID, "MATCH") )			
				{
					bz_sendTextMessage (BZ_SERVER, playerID, "You do not have permission to run the match command");
					return true;
				}

				if ( params->size() )
				{
					bz_ApiString action = params->get(0);
					action.tolower();

					if ( action == "start")
					{
						start(playerID, params);
						return true;
					}

					if ( action == "end")
					{
						end(playerID, params);
						return true;
					}

					if ( action == "pause")
					{
						pause(playerID, params);
						return true;
					}

					if ( action == "sub")
					{
						substitute(playerID, params);
						return true;
					}

					if ( action == "noreport")
					{
						if ( report )
						{
							report = false;
							bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Reporting is disabled.");
						}
						else 
							{
								report = true;
								bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Reporting is enabled.");
							}

						return true;
					}
				}

			}
	}

	return false;
}

/** 
 * Init the MatchManager. Should be hooked into bzfs.cxx at some point,
 * but not yet as this is far from finished.
 */

void MatchManager::init()
{
	bz_debugMessage(2, "Initialize MatchManager");
}

