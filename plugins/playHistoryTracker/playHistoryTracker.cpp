/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bzfsAPI.h"
#include <map>

class PlayHistoryTracker : public bz_Plugin
{
public:
  virtual const char* Name () {return "Play History Tracker";}
  virtual void Init (const char* /* config */)
  {
    // Register our events
    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerPartEvent);
    Register(bz_ePlayerJoinEvent);
  }

  virtual void Event (bz_EventData *eventData);

protected:
  std::map<int, int> spreeCount;
};

BZ_PLUGIN(PlayHistoryTracker)

void PlayHistoryTracker::Event(bz_EventData *eventData)
{
  switch (eventData->eventType) {
  default:
    break;

  case bz_ePlayerDieEvent:
    {
      bz_PlayerDieEventData_V1 *deathRecord = (bz_PlayerDieEventData_V1*)eventData;

      // Create variables to store the callsigns
      std::string victimCallsign = "UNKNOWN";
      std::string killerCallsign = "UNKNOWN";

      // Get player records for victim and killer
      bz_BasePlayerRecord *victimData = bz_getPlayerByIndex(deathRecord->playerID);
      bz_BasePlayerRecord *killerData = bz_getPlayerByIndex(deathRecord->killerID);

      // If we have valid data, update the callsigns
      if (victimData)
	victimCallsign = victimData->callsign.c_str();
      if (killerData)
	killerCallsign = killerData->callsign.c_str();

      // Free the player records
      bz_freePlayerRecord(victimData);
      bz_freePlayerRecord(killerData);

      // Handle the victim
      if (spreeCount.find(deathRecord->playerID) != spreeCount.end()) {
	// Store a quick reference to their former spree count
	int spreeTotal = spreeCount[deathRecord->playerID];

	std::string message;

	// Generate an appropriate message, if any
	if (spreeTotal >= 5 && spreeTotal < 10)
	  message = victimCallsign + std::string("'s rampage was stopped by ") + killerCallsign;
	else if (spreeTotal >= 10 && spreeTotal < 20)
	  message = victimCallsign + std::string("'s killing spree was halted by ") + killerCallsign;
	else if (spreeTotal >= 20)
	  message = std::string("The unstoppable reign of ") + victimCallsign + std::string(" was ended by ") + killerCallsign;

	// If we have a message to send, then send it
	if (message.size())
	  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, message.c_str());

	// Since they died, release their spree counter
	spreeCount[deathRecord->playerID] = 0;
      }

      // Handle the killer (if it wasn't also the victim)
      if (deathRecord->playerID != deathRecord->killerID && spreeCount.find(deathRecord->killerID) != spreeCount.end()) {
	// Store a quick reference to their newly incremented spree count
	int spreeTotal = ++spreeCount[deathRecord->playerID];

	std::string message;

	// Generate an appropriate message, if any
	if (spreeTotal == 5)
	  message = victimCallsign + std::string(" is on a Rampage!");
	else if (spreeTotal == 10)
	  message = victimCallsign + std::string(" is on a Killing Spree!");
	else if (spreeTotal == 20)
	  message = victimCallsign + std::string(" is Unstoppable!!");
	else if (spreeTotal > 20 && spreeTotal%5 == 0)
	  message = victimCallsign + std::string(" continues to rage on");

	// If we have a message to send, then send it
	if (message.size())
	  bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, message.c_str());
      }
    }
    break;

  case  bz_ePlayerJoinEvent:
    {
      // Initialize the spree counter for the player that just joined
      spreeCount[((bz_PlayerJoinPartEventData_V1*)eventData)->playerID] = 0;
    }
    break;

  case  bz_ePlayerPartEvent:
    {
      // Erase the spree counter for the player that just left
      std::map<int, int >::iterator itr = spreeCount.find(((bz_PlayerJoinPartEventData_V1*)eventData)->playerID);
      if (itr != spreeCount.end())
	spreeCount.erase(itr);
    }
    break;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
