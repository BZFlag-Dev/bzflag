/* bzflag
* Copyright (c) 1993 - 2005 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "common.h"

/* class interface header */
#include "bzfs.h"
#include "bzfsEvents.h"
#include "GameKeeper.h"

extern void sendMessage(int playerIndex, PlayerId dstPlayer, const char *message);

// ----------------- SpreeTracker-----------------

/*typedef struct 
{
	int playerID;
	std::string callsign;
	double		startTime;
	double		lastUpdateTime;
	int			spreeTotal;
}trPlayerHistoryRecord;

std::map<int, trPlayerHistoryRecord > playerList; */

PlayHistoryTracker::PlayHistoryTracker()
{

}

PlayHistoryTracker::~PlayHistoryTracker()
{

}

void PlayHistoryTracker::process ( BaseEventData *eventData )
{
	switch (eventData->eventType)
	{
		case eNullEvent:
		case eZoneEntryEvent:
		case eCaptureEvent:
		case eZoneExitEvent:
			// really WTF!!!!
			break;

		case ePlayerDieEvent:
			{
				PlayerDieEventData	*deathRecord = (PlayerDieEventData*)eventData;
				
				GameKeeper::Player *killerData = GameKeeper::Player::getPlayerByIndex(deathRecord->killerID);
				GameKeeper::Player *victimData = GameKeeper::Player::getPlayerByIndex(deathRecord->playerID);

				// clear out the dude who got shot, since he won't be having any SPREEs
				if (playerList.find(deathRecord->playerID) != playerList.end())
				{
					trPlayerHistoryRecord	&record = playerList.find(deathRecord->playerID)->second;
					std::string message;
					if ( record.spreeTotal >= 5 && record.spreeTotal < 10 )
						message = record.callsign + "'s rampage was stoped by " + killerData->player.getCallSign();
					if ( record.spreeTotal >= 10 && record.spreeTotal < 20 )
						message = record.callsign + "'s killing spree was halted by " + killerData->player.getCallSign();
					if ( record.spreeTotal >= 20 )
						message = "The unstopable reign of " + record.callsign + " was ended by " + killerData->player.getCallSign();

					if (message.size())
						sendMessage(ServerPlayer, AllPlayers, message.c_str());

					record.spreeTotal = 0;
					record.startTime = deathRecord->time;
					record.lastUpdateTime = deathRecord->time;

				}

				// chock up another win for our killer
				// if they weren't the same as the killer ( suicide ).
				if ( (deathRecord->playerID != deathRecord->killerID) && playerList.find(deathRecord->killerID) != playerList.end())
				{
					trPlayerHistoryRecord	&record = playerList.find(deathRecord->killerID)->second;
					record.spreeTotal++;
					record.lastUpdateTime = deathRecord->time;

					std::string message;
					
					if ( record.spreeTotal > 5 )
						message = record.callsign + " is on a Rampage!";
					if ( record.spreeTotal > 10 )
						message = record.callsign + " is on a Killing Spree!";
					if ( record.spreeTotal > 20 )
						message = record.callsign + " is Unstoppable!!";
					
					if (message.size())
						sendMessage(ServerPlayer, AllPlayers, message.c_str());
				}
			}
			break;

		case ePlayerSpawnEvent:
			// really WTF!!!!
			break;

		case ePlayerJoinEvent:
			{
				trPlayerHistoryRecord	playerRecord;

				playerRecord.playerID = ((PlayerJoinPartEventData*)eventData)->playerID;
				playerRecord.callsign = ((PlayerJoinPartEventData*)eventData)->callsign;
				playerRecord.spreeTotal = 0;
				playerRecord.lastUpdateTime = ((PlayerJoinPartEventData*)eventData)->time;
				playerRecord.startTime = playerRecord.lastUpdateTime;

				playerList[((PlayerJoinPartEventData*)eventData)->playerID] = playerRecord;
			}
			break;
		case ePlayerPartEvent:
			{
				std::map<int, trPlayerHistoryRecord >::iterator	itr = playerList.find( ((PlayerJoinPartEventData*)eventData)->playerID );
				if (itr != playerList.end())
					playerList.erase(itr);
			}
			break;
	}
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
