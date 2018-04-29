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

class fairCTF : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
  virtual const char* Name () {return "Fair CTF";}

  virtual void Init ( const char* config );
  virtual void Cleanup ( void );
  virtual void Event ( bz_EventData *eventData );

  virtual bool SlashCommand (int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params);

  virtual void DropTeamFlag(int playerID);
  virtual void SetDropTime();
  virtual void UpdateState(bz_eTeamType teamLeaving);
  virtual bool isEven(bz_eTeamType teamLeaving);

  bool allowCTF;
  bool autoMode;

  float max_ratio;
  int max_gap_by_1;
  int max_gap;
  int drop_delay;


  double droptime;
};

BZ_PLUGIN(fairCTF)

void fairCTF::Init ( const char* config )
{
  // Initialize defaults
  allowCTF = true;
  autoMode = true;
  max_ratio = .25;
  max_gap_by_1 = 2;
  max_gap = 3;
  drop_delay = 5;

  // Parse out args

  std::string rawparams = config;

  std::string params[4];
  params[0] = "";
  params[1] = "";
  params[2] = "";
  params[3] = "";

  unsigned int n = 0;

  for (unsigned int i = 0; i < rawparams.length(); i++)
  {
    if (rawparams.at(i) == ':')
    {
      n++;

      if (n > 3)
      {
	break;
      }
    }
    else
    {
      params[n] += rawparams.at(i);
    }
  }

  if (params[0].length() > 0)
  {
    float tempratio = (float)atof(params[0].c_str());
    if (tempratio > 0.0)
    {
      max_ratio = tempratio;
    }
  }

  if (params[1].length() > 0)
  {
    int tempmax1gap = atoi(params[1].c_str());
    if (tempmax1gap > 0)
    {
      max_gap_by_1 = tempmax1gap;
    }
  }

  if (params[2].length() > 0)
  {
    int tempmaxgap = atoi(params[2].c_str());
    if (tempmaxgap > 0)
    {
      max_gap = tempmaxgap;
    }
  }

  if (params[3].length() > 0)
  {
    int tempdelay = atoi(params[3].c_str());
    if (tempdelay > 0)
    {
      drop_delay = tempdelay;
    }
  }

  Register(bz_eAllowFlagGrab);
  Register(bz_ePlayerJoinEvent);
  Register(bz_ePlayerPartEvent);
  Register(bz_eTickEvent);

  bz_registerCustomSlashCommand ("ctf", this);

  bz_debugMessage(4,"fairCTF plugin loaded");

  UpdateState(eNoTeam);
}

void fairCTF::Cleanup()
{

  Flush();
  bz_removeCustomSlashCommand ("ctf");

  bz_debugMessage(4,"fairCTF plugin unloaded");
}

void fairCTF::Event(bz_EventData *eventData)
{
  if (eventData->eventType == bz_eAllowFlagGrab)
  {
    bz_AllowFlagGrabData_V1* grabData = (bz_AllowFlagGrabData_V1*)eventData;

    if (!allowCTF)
    {
      // Don't allow a team flag grab
      std::string flagtype = bz_getFlagName(grabData->flagID).c_str();
      if (flagtype == "R*" || flagtype == "G*" || flagtype == "B*" || flagtype == "P*")
      {
	grabData->allow = false;
	bz_sendTextMessage (BZ_SERVER, grabData->playerID, "CTF play is currently disabled.");
      }
    }
  }
  else if (eventData->eventType == bz_ePlayerJoinEvent)
  {
    UpdateState(eNoTeam);
  }
  else if (eventData->eventType == bz_ePlayerPartEvent)
  {
    bz_PlayerJoinPartEventData_V1* partData = (bz_PlayerJoinPartEventData_V1*)eventData;
    // Need to compensate for that leaving player.
    UpdateState(partData->record->team);
  }
  else if (eventData->eventType == bz_eTickEvent)
  {

    if (droptime != 0.0 && bz_getCurrentTime() >= droptime)
    {
      // Time to drop any team flags.
      bz_APIIntList* pl = bz_getPlayerIndexList();

      for (unsigned int x = 0; x < pl->size(); x++)
      {
	DropTeamFlag(pl->get(x));
      }

      droptime = 0.0;
    }
  }
  else
  {
    // Huh?
    return;
  }
}

bool fairCTF::SlashCommand (int playerID, bz_ApiString /*command*/, bz_ApiString message, bz_APIStringList * /*params*/)
{
  std::string cs = "UNKNOWN";
  bz_BasePlayerRecord* pr = bz_getPlayerByIndex(playerID);
  if (pr != NULL)
  {
    cs = pr->callsign.c_str();
    bz_freePlayerRecord (pr);
  }

  if (!bz_hasPerm(playerID, "FAIRCTF"))
  {
    bz_sendTextMessage(BZ_SERVER, playerID, (cs + ", you do not have permission to use the /ctf command.").c_str());
  }
  else
  {
    if (message == "on")
    {
      if (!autoMode && allowCTF)
      {
	bz_sendTextMessage(BZ_SERVER, playerID, "CTF is already set to \"on\".");
      }
      else
      {
	autoMode = false;
	bz_sendTextMessage (BZ_SERVER, eAdministrators, ("CTF setting has been changed to \"on\" by " + cs + ".").c_str());
	if (!allowCTF)
	{
	  bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, ("CTF has been enabled by " + cs + ".").c_str());
	  allowCTF = true;
	  droptime = 0.0;
	}
      }
    }
    else if (message == "off")
    {
      if (!autoMode && !allowCTF)
      {
	bz_sendTextMessage(BZ_SERVER, playerID, "CTF is already set to \"off\".");
      }
      else
      {
	autoMode = false;
	bz_sendTextMessage (BZ_SERVER, eAdministrators, ("CTF setting has been changed to \"off\" by " + cs + ".").c_str());
	if (allowCTF)
	{
	  bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, ("CTF has been disabled by " + cs + ".").c_str());
	  allowCTF = false;
	  SetDropTime();
	}
      }
    }
    else if (message == "auto")
    {
      if (autoMode)
      {
	bz_sendTextMessage(BZ_SERVER, playerID, "CTF is already set to \"auto\".");
      }
      else
      {
	autoMode = true;
	bz_sendTextMessage (BZ_SERVER, eAdministrators, ("CTF setting has been changed to \"auto\" by " + cs + ".").c_str());
	UpdateState(eNoTeam);
      }
    }
    else
    {
      bz_sendTextMessage (BZ_SERVER, playerID, "Usage: /ctf on|off|auto");
    }
  }
  return true;
}

void fairCTF::DropTeamFlag(int playerID)
{
  bz_BasePlayerRecord* droppr = bz_getPlayerByIndex (playerID);

  if (droppr != NULL)
  {
    // Are they carrying a team flag?
    if (droppr->currentFlag == "Red team flag" ||
      droppr->currentFlag == "Green team flag" ||
      droppr->currentFlag == "Blue team flag" ||
      droppr->currentFlag == "Purple team flag")
    {
      bz_removePlayerFlag(playerID);
      bz_sendTextMessage (BZ_SERVER, playerID, "CTF play is currently disabled.");
    }

    bz_freePlayerRecord(droppr);
  }
}

void fairCTF::UpdateState(bz_eTeamType teamLeaving)
{
  if (autoMode)
  {
    bool fair = isEven(teamLeaving);

    if (fair && !allowCTF)
    {
      allowCTF = true;
      droptime = 0.0;
      bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Team sizes are sufficiently even. CTF play is now enabled.");
    }
    else if (!fair && allowCTF)
    {
      allowCTF = false;
      bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Team sizes are uneven. CTF play is now disabled.");

      SetDropTime();
    }
  }
}

void fairCTF::SetDropTime()
{
  bz_APIIntList	*playerList = bz_newIntList();
  bz_getPlayerIndexList(playerList);
  bool TeamFlagIsCarried = false;

  // is any tank carrying a team flag?
  for (unsigned int i = 0; i < playerList->size(); i++)
  {
    const char *FlagHeld = bz_getPlayerFlag((*playerList)[i]);

    if (FlagHeld != NULL && (strcmp(FlagHeld, "R*") == 0 || strcmp(FlagHeld, "G*") == 0 || strcmp(FlagHeld, "B*") == 0 || strcmp(FlagHeld, "P*") == 0))
    {
      TeamFlagIsCarried = true;
      break;
    }
  }

  bz_deleteIntList(playerList);

  // announce drop delay only if some tank is carrying a team flag
  if (TeamFlagIsCarried)
  {
    if (drop_delay >= 0)
    {
      droptime = bz_getCurrentTime() + (double)drop_delay;
      if (drop_delay > 1)
      {
	bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, bz_format("Currently-held team flags will be dropped in %d seconds.", drop_delay));
      }
      else
      {
	bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Currently-held team flags will be dropped in 1 second.");
      }

    }
    else
    {
      bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Currently-held team flags will not be dropped.");
    }
  }
}

bool fairCTF::isEven(bz_eTeamType teamLeaving)
{

  int teamsizes[4];

  teamsizes[0] = bz_getTeamCount (eRedTeam);
  teamsizes[1] = bz_getTeamCount (eGreenTeam);
  teamsizes[2] = bz_getTeamCount (eBlueTeam);
  teamsizes[3] = bz_getTeamCount (ePurpleTeam);

  int leavingTeamIndex = (int)teamLeaving;
  if (leavingTeamIndex >= 1 && leavingTeamIndex <= 4)
  {
    // Decrement the team count for the player that's leaving the game.
    teamsizes[leavingTeamIndex - 1]--;
  }


  //check fairness

  int smallestTeam = 10000; //impossibly high
  int largestTeam = 0;

  for (int x = 0; x < 4; x++)
  {
    if (teamsizes[x] > largestTeam)
    {
      largestTeam = teamsizes[x];
    }
    if (teamsizes[x] != 0 && teamsizes[x] < smallestTeam)
    {
      smallestTeam = teamsizes[x];
    }
  }

  //check differences and ratios

  if (smallestTeam == 10000 || largestTeam == smallestTeam) //equal, or server has no team tanks
  {
    return true;
  }
  if (smallestTeam <= max_gap_by_1) // user-defined cap on a difference of 1
  {
    return false;
  }
  if (largestTeam - smallestTeam == 1) //after UD limit
  {
    return true;
  }
  if ((static_cast<float> (largestTeam - smallestTeam)) / smallestTeam > max_ratio) //greater than specified gap
  {
    return false;
  }
  if (largestTeam - smallestTeam >= max_gap)
  {
    return false;
  }

  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
