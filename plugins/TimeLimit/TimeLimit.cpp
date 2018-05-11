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

// TimeLimit.cpp : bzfs plugin to change/set the match duration
//

#include "bzfsAPI.h"
#include <sstream>
#include <stdio.h>
#include <string.h>

#define MAX_TIMES 20

class TimeLimit : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:

	virtual const char* Name () {return "Time Limit";}
	virtual void Init ( const char* config );
	virtual void Cleanup ();
	virtual void Event ( bz_EventData *eventData );
	virtual bool SlashCommand ( int playerID, bz_ApiString, bz_ApiString, bz_APIStringList*);

protected:

private:

};

BZ_PLUGIN(TimeLimit)

// variable to save the original -time value
float saveTimeLimit = 0;

// list to hold the available match durations
bz_APIStringList* timeList = bz_newStringList();

// Displays the available match durations
void showMatchDurations(int playerID)
{
   bz_sendTextMessagef (BZ_SERVER, playerID, "Not a valid match duration, valid match durations are : ");
   for (unsigned i=0; i < timeList->size(); i++)
      bz_sendTextMessagef (BZ_SERVER, playerID, "* %s minute(s)",timeList->get(i).c_str());
}


// Checks if it's a valid match duration or not
bool isValidTime ( float timelimit )
{
 if ( timeList->size() == 0 ) return true;

 for (unsigned i=0; i < timeList->size(); i++) {
    if ( atof(timeList->get(i).c_str()) == timelimit ) {
      return true;
    }
 }
 return false;
}


void TimeLimit::Event ( bz_EventData *eventData )
{
  switch (eventData->eventType)
  {
    case bz_ePlayerJoinEvent: {
	    // if it's the first player that joins, then reset the time to default
	    if ( bz_getPlayerCount() == 1 && bz_isTimeManualStart() && !bz_isCountDownActive()  && !bz_isCountDownInProgress()) {
	      bz_setTimeLimit(saveTimeLimit);
	    }
    }
     break;

	//reset the time to default at gameover
    case bz_eGameEndEvent: {
	    bz_setTimeLimit(saveTimeLimit);
    }
     break;

    default: {
		// do nothing
    }
  }

}


std::string convertIntToString(const unsigned int integer)
{
  std::ostringstream ostr;

  ostr << integer;

  return ostr.str();
}


void parseCommand ( const char* commandLine )
{
  const size_t len = strlen(commandLine);

  if (len != 0) {
    unsigned int range_begin, range_end;
    char junk;

    if (sscanf(commandLine, "%u-%u%c", &range_begin, &range_end, &junk) == 2) {
      while (range_begin <= range_end)
	timeList->push_back(convertIntToString(range_begin++));
    } else if (strspn(commandLine, ",0123456789") == len)
      timeList->tokenize(commandLine, ",", MAX_TIMES, false);
  }
}


bool TimeLimit::SlashCommand ( int playerID, bz_ApiString cmd, bz_ApiString, bz_APIStringList* cmdParams )
{

  if (strcasecmp (cmd.c_str(), "timelimit")) {
	return false;
  }

  // Check permissions
  if (! bz_hasPerm(playerID,"TIMELIMIT")) {
	bz_sendTextMessagef (BZ_SERVER, playerID, "You do not have permission to run the timelimit command");
	return true;
  }

  // If the server is not configured for manual countdown the timelimit
  // command can't be used
  if (! bz_isTimeManualStart()) {
    bz_sendTextMessagef (BZ_SERVER, playerID, "This server was not configured for manual clock countdowns");
    return true;
  }

  if (cmdParams->get(0).c_str()[0] == '\0') {
    bz_sendTextMessagef (BZ_SERVER, playerID, "Usage : /timelimit <minutes>|show|reset");
    return true;
  }

  // displaying the current timelimit
  if (strcasecmp(cmdParams->get(0).c_str(),"show") == 0 ) {
    bz_sendTextMessagef (BZ_SERVER, playerID,"Match duration is set to %.0f minute(s)",(bz_getTimeLimit() / 60));
    return true;
  }

  // check if there is already a countdown in progress or if a match is
  // already in progress
  if ( bz_isCountDownInProgress() ) {
    bz_sendTextMessagef (BZ_SERVER, playerID, "There is a countdown already in progress, match duration can't be changed now");
    return true;
  } else if ( bz_isCountDownActive() ) {
    bz_sendTextMessagef (BZ_SERVER, playerID, "A game is already in progress, match duration can't be changed now");
    return true;
    }

  bz_BasePlayerRecord *playerRecord;
  playerRecord = bz_getPlayerByIndex(playerID);

  // resets the timer to the default
  if (strcasecmp(cmdParams->get(0).c_str(),"reset") == 0 ) {
    bz_setTimeLimit(saveTimeLimit);
    bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Match duration reset to %.0f minute(s) by %s",(bz_getTimeLimit() / 60),playerRecord->callsign.c_str());
    return true;
  }

  unsigned i, nonumber=0;

  for (i=0; i < strlen(cmdParams->get(0).c_str()); i++) {
     if (isdigit(cmdParams->get(0).c_str()[i]) == 0) nonumber=1;
  }

  if (nonumber == 0 ) {
    float limit = (float)atof(cmdParams->get(0).c_str());
	// Don't allow timelimit being equal or lower then 0
	if (limit > 0 ) {

	  if (! isValidTime(limit)) {

	showMatchDurations(playerID);
		return true;
      }

      bz_setTimeLimit(limit * 60);
      bz_sendTextMessagef (BZ_SERVER, BZ_ALLUSERS, "Match duration set to %.0f minute(s) by %s",(bz_getTimeLimit() / 60),playerRecord->callsign.c_str());
    } else {
	  bz_sendTextMessagef (BZ_SERVER, playerID, "Match duration can't be equal or lower then 0");
		  return true;
      }
  } else {
	bz_sendTextMessagef (BZ_SERVER, playerID, "Not a correct value");
	    return true;
    }

  return true;

}

void TimeLimit::Init ( const char* commandLine )
{
  parseCommand(commandLine);

  saveTimeLimit = bz_getTimeLimit();

  bz_registerCustomSlashCommand ("timelimit",this);
  Register(bz_ePlayerJoinEvent);
  Register(bz_eGameEndEvent);
}


void TimeLimit::Cleanup ( void )
{

  // set default timelimit back before unloading
  //bz_setTimeLimit(saveTimeLimit);

  bz_removeCustomSlashCommand ("timelimit");
  Flush();


  // set default timelimit back before unloading
  bz_setTimeLimit(saveTimeLimit);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
