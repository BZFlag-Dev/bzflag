/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
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

BZ_GET_PLUGIN_VERSION

class fairCTF : public bz_EventHandler
{
public:
  fairCTF();
  virtual bool isEven();
  virtual void updateEven();
  virtual void process (bz_EventData *eventData);

  bool is_auto;

  bool is_even;
  float max_ratio;
  int max_gap_by_1;
  int max_gap;

  double switchtime;
  int activeteams;
};


fairCTF fairctf;

class ctfCommand : public bz_CustomSlashCommandHandler
{
public:
  virtual ~ctfCommand(){};
  virtual bool handle ( int playerID, bzApiString command, bzApiString message, bzAPIStringList *params );
};

ctfCommand ctf_command;


BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{

  bz_registerEvent (bz_eTickEvent, &fairctf);
  bz_registerCustomSlashCommand ("ctf", &ctf_command);

  std::string rawparams = commandLine;

  unsigned int n = 0;

  //get ratio from param string
  float temp_ratio = 0.0;
  int digit = 0;
  float dec_digit = 0.0; 
  bool has_hit_dot = false;
  int number_of_dec_places = 0;
  while(n < rawparams.length() && rawparams[n] != ':')
  {
    if (has_hit_dot)
    {
      number_of_dec_places++;
      digit = static_cast<int> (rawparams[n] - '0');
      dec_digit = static_cast<float> (digit);
      for (int x = 0; x < number_of_dec_places; x++)
      {
	dec_digit /= 10.0;
      }

      temp_ratio += dec_digit;
    }
    else
    {
      if (rawparams[n] == '.')
      {
	has_hit_dot = true;
      }
      else
      {
	digit = static_cast<int> (rawparams[n] - '0');
	temp_ratio *= 10;
	temp_ratio += digit;
      }
    }
    n++;
  }
  if (temp_ratio != 0.0)
  {
    fairctf.max_ratio = temp_ratio;
  }

  n++;

  //get max 
  int temp_1_gap = 0;
  while(n < rawparams.length() && rawparams[n] != ':')
  {
    digit = static_cast<int> (rawparams[n] - '0');
    temp_1_gap *= 10;
    temp_1_gap += digit;
    n++;
  }
  if (temp_1_gap != 0)
  {
    fairctf.max_gap_by_1 = temp_1_gap;
  }

  n++;

  int temp_max_gap = 0;
  while(n < rawparams.length())
  {
    digit = static_cast<int> (rawparams[n] - '0');
    temp_max_gap *= 10;
    temp_max_gap += digit;
    n++;
  }
  if (temp_max_gap != 0)
  {
    fairctf.max_gap = temp_max_gap;
  }

  bz_debugMessage(4,"fairCTF plugin loaded");
  return 0;
}

BZF_PLUGIN_CALL int bz_Unload (  )
{

  bz_removeEvent (bz_eTickEvent, &fairctf);
  bz_removeCustomSlashCommand ("ctf");

  bz_debugMessage(4,"fairCTF plugin unloaded");
  return 0;
}

fairCTF::fairCTF()
{
  //initialize evenness state and constants (in case they aren't specified)
  is_even = true;
  max_ratio = .25;
  max_gap_by_1 = 2;
  max_gap = 3;

  is_auto = true;
}

bool fairCTF::isEven()
{

  int teamsizes[4]; 

  teamsizes[0] = bz_getTeamCount (eRedTeam);
  teamsizes[1] = bz_getTeamCount (eGreenTeam);
  teamsizes[2] = bz_getTeamCount (eBlueTeam);
  teamsizes[3] = bz_getTeamCount (ePurpleTeam);

  //check how many active teams there are

  int tempactiveteams = 0;

  for (int x = 0; x < 4; x++)
  {
    if (teamsizes[x] > 0)
    {
      tempactiveteams++;
    }
  }

  //reset CTF scores if only one team present

  if (tempactiveteams == 1)
  {
    bz_resetTeamScore (eRedTeam);
    bz_resetTeamScore (eGreenTeam);
    bz_resetTeamScore (eBlueTeam);
    bz_resetTeamScore (ePurpleTeam);
  }

  activeteams = tempactiveteams;


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

void fairCTF::updateEven()
{
  if (isEven())
  {
    if (!is_even)
    {
      if (bz_getCurrentTime() - switchtime >= 5.0)
      {
	if (activeteams > 1)
	{
	  bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Teams are now even enough to be fair. CTF enabled.");
	}
	is_even = true;
      }
    }
    else
    {
      switchtime = bz_getCurrentTime();
    }
  }
  else
  {
    if (is_even)
    {
      if (bz_getCurrentTime() - switchtime >= 3.0)
      {
	if (activeteams > 1)
	{
	  bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, "Teams are uneven. CTF disabled.");
	}
	is_even = false;
      }
    }
    else
    {
      switchtime = bz_getCurrentTime();
    }
  }
}

void fairCTF::process(bz_EventData *eventData)
{
  if (is_auto)
  {
    updateEven();
  }
  if (!is_even)
  {  
  
    bzAPIIntList pl;

    bz_getPlayerIndexList(&pl);
    
    for (unsigned int x = 0; x < pl.size(); x++)
    {
      bz_PlayerRecord *pr;
      pr = bz_getPlayerByIndex(pl.get(x));

      if (pr != NULL)
      {
	if (pr->currentFlag == "Red team flag" || pr->currentFlag == "Green team flag" || pr->currentFlag == "Blue team flag" || pr->currentFlag == "Purple team flag")
	{
	  bz_removePlayerFlag(pl.get(x));
	  bz_sendTextMessage (BZ_SERVER, pl.get(x), "No CTF!");
	}
      }
      bz_freePlayerRecord (pr);
    }
  }
}

bool ctfCommand::handle(int playerID, bzApiString command, bzApiString message, bzAPIStringList *params)
{
  bz_PlayerRecord *pr = bz_getPlayerByIndex (playerID);
  std::string cs = pr->callsign.c_str();
  bz_freePlayerRecord (pr);

  if (!bz_hasPerm(playerID, "FAIRCTF"))
  {
    bz_sendTextMessage (BZ_SERVER, playerID, (cs + ", you do not have permission to run the /ctf command.").c_str());
  }
  else
  {
    if (message == "on")
    {
      fairctf.is_auto = false;
      if (!fairctf.is_even)
      {
	bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, ("CTF has been manually enabled by " + cs + ".").c_str());
	fairctf.is_even = true;
      }
      bz_sendTextMessage (BZ_SERVER, eAdministrators, ("fairCTF setting has been changed to \"on\" by " + cs + ".").c_str());
    }
    else if (message == "off")
    {
      fairctf.is_auto = false;
      if (fairctf.is_even)
      {
	bz_sendTextMessage (BZ_SERVER, BZ_ALLUSERS, ("CTF has been manually disabled by " + cs + ".").c_str());
	fairctf.is_even = false;
      }
      
      bz_sendTextMessage (BZ_SERVER, eAdministrators,  ("fairCTF setting has been changed to \"off\" by " + cs + ".").c_str());
    }
    else if (message == "auto")
    {
      fairctf.is_auto = true;
      bz_sendTextMessage (BZ_SERVER, eAdministrators, ("fairCTF setting has been changed to \"auto\" by " + cs + ".").c_str());
    }
    else
    {
      bz_sendTextMessage (BZ_SERVER, playerID, "Usage: /ctf on|off|auto");
    }
  }
  return true;
}





// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

