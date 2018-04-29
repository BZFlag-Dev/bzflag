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

class rabbitTimer : public bz_Plugin
{
public:
  virtual const char* Name () {return "Rabit Timer";}
  virtual void Init ( const char* config );

  virtual void Event ( bz_EventData *eventData );

  float rabbitKillTimeLimit;	// the max time the rabbit has to kill someone
  bool rollOver;		// whether or not to roll over leftover seconds

  float rabbitDeathTime;

  int currentRabbit;
};

BZ_PLUGIN(rabbitTimer)

void rabbitTimer::Event(bz_EventData *eventData)
{
  if (eventData->eventType == bz_eTickEvent)
  {
    bz_TickEventData_V1* tickdata = (bz_TickEventData_V1*)eventData;

    if (currentRabbit != -1 && tickdata->eventTime >= rabbitDeathTime)
    {
      // kill the wabbit!
      bz_killPlayer(currentRabbit, false, BZ_SERVER);

      // stopgap. the kill event should do this, really...
      currentRabbit = -1;
      rabbitDeathTime = (float)tickdata->eventTime + rabbitKillTimeLimit;

      bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Time's up! Selecting new rabbit.");
    }
    else if (currentRabbit == -1 && bz_getTeamCount(eHunterTeam) >= 3) // make sure we've got enough people before reactivating the timer
    {
      // find the new rabbit
      bz_APIIntList pl;
      bz_getPlayerIndexList(&pl);

      for (unsigned int i = 0; i < pl.size() && currentRabbit == -1; i++)
      {
	bz_BasePlayerRecord* pr = bz_getPlayerByIndex(pl.get(i));

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

    bz_PlayerDieEventData_V1* killdata = (bz_PlayerDieEventData_V1*)eventData;

    if (killdata->team == eRabbitTeam)
    {
      currentRabbit = -1; // we will sort this out on the next tick

      rabbitDeathTime = (float)killdata->eventTime + rabbitKillTimeLimit;
    }
    else if (killdata->killerTeam == eRabbitTeam && currentRabbit != -1)
    {
      if (rollOver)
      {
	rabbitDeathTime += rabbitKillTimeLimit;

	int limit = (int)rabbitKillTimeLimit;
	int timeremaining = (int)(rabbitDeathTime - killdata->eventTime);

	bz_sendTextMessage(BZ_SERVER, currentRabbit, bz_format("+%d seconds: %d seconds remaining.", limit, timeremaining));
      }
      else
      {
	rabbitDeathTime = (float)killdata->eventTime + rabbitKillTimeLimit;

	int limit = (int)rabbitKillTimeLimit;
	bz_sendTextMessage(BZ_SERVER, currentRabbit, bz_format("%d seconds remaining.", limit));
      }
    }
  }
  else if (eventData->eventType == bz_ePlayerPartEvent)
  {
    bz_PlayerJoinPartEventData_V1* partdata = (bz_PlayerJoinPartEventData_V1*)eventData;

    if (partdata->record->team == eRabbitTeam) // we need to select a new rabbit if the rabbit leaves.
    {
      currentRabbit = -1; // we will sort this out on the next tick

      rabbitDeathTime = (float)partdata->eventTime + rabbitKillTimeLimit;
    }
  }
}

void rabbitTimer::Init(const char* commandLine)
{
  rabbitKillTimeLimit = 30.0;
  rollOver = false;
  currentRabbit = -1;
  rabbitDeathTime = 3600.0; // something large

  std::string param = commandLine;

  if (param.size() > 0 && param.at(0) == '+')
  {
    rollOver = true;
    param  = param.erase(0,1);
  }

  int newtime = atoi(param.c_str());

  if (newtime > 0)
    rabbitKillTimeLimit = (float)newtime;

  Register(bz_ePlayerDieEvent);
  Register(bz_ePlayerPartEvent);
  Register(bz_eTickEvent);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
