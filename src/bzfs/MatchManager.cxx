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

/** initialize the singleton */
template <>
MatchManager* Singleton<MatchManager>::_instance = (MatchManager*)0;

/**
* default constructor, protected because of singleton
*/
MatchManager::MatchManager() : Singleton<MatchManager>()
{
	matchState = eOff;
	startTime = -1;
	duration = -1;
	resetTime = 0;

	paused = false;
	resumeTime = -1;

	bz_registerCustomSlashCommand("match",this);
	bz_registerEvent(bz_eGetAutoTeamEvent,this);
}

/**
* default destructor, protected because of singleton
*/
MatchManager::~MatchManager()
{
	bz_removeCustomSlashCommand("match");
	bz_removeEvent(bz_eGetAutoTeamEvent,this);
}

void MatchManager::process ( bz_EventData *eventData )
{
 switch(eventData->eventType)
 {
	case bz_eGetAutoTeamEvent:
	 {
	 }
	 break;
	default:
	 {
	 }
	 break;
 }
}

bool MatchManager::handle ( int playerID, bz_ApiString command, bz_ApiString /* message */, bz_APIStringList *params )
{
	double now = TimeKeeper::getCurrent().getSeconds();

	if (command == "match")
	{
		if ( !params->size() )
		{
			std::string msg;
			switch(matchState)
			{
				default:
					msg = "No match is in progress";
					break;

				case ePregame:
					msg = TextUtils::format("Match will start in %f seconds",startTime-now);
					break;

				case eOn:
					if (!paused)
						msg = TextUtils::format("Match is in progress");
					else
						msg = "Match is paused";
					break;

				case ePostgame:
					{
						double endTime = startTime + duration + resetTime;
						msg = TextUtils::format("Match is over, server will resume free play in %f seconds",endTime-now);
					}
					break;
			}

			bz_sendTextMessage (BZ_SERVER, playerID, msg.c_str());
		}
	}
	return false;
}


