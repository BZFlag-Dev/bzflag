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

class autoFlagReset : public bz_Plugin
{
  public:
    autoFlagReset();
    virtual const char* Name () {return "Automatic Flag Reset";}
    virtual void Init ( const char* config );

    virtual void Event ( bz_EventData* eventData );

    virtual bool ResetUnusedSuperflag(unsigned int flagID);

    bool incremental;
    double freq;
    double nextRunTime;

    unsigned int nextFlag;
};

BZ_PLUGIN(autoFlagReset)

autoFlagReset::autoFlagReset() : bz_Plugin(), incremental(false), freq(900), nextRunTime(bz_getCurrentTime()), nextFlag(0)
{
}

void autoFlagReset::Init (const char* commandLine)
{
  std::string cl = commandLine;

  if (nextRunTime < 0.0)
  {
    nextRunTime = 0.0;
  }

  if (cl.length() > 0)
  {
    if (cl.at(cl.length() - 1) == 'i' || cl.at(cl.length() - 1) == 'I')
    {
      // Incremental mode.
      incremental = true;

      cl = cl.substr(0, cl.length() - 1);
    }

    double newfreq = atof(cl.c_str());
    if (newfreq > 0.0)
    {
      freq = newfreq * 60.0;
    }

  }

  Register(bz_eTickEvent);

  bz_debugMessage(4,"autoFlagReset plugin loaded");
}

void autoFlagReset::Event(bz_EventData* eventData)
{
  unsigned int nflags = bz_getNumFlags();

  if (bz_getCurrentTime() < nextRunTime || nflags == 0 || eventData->eventType != bz_eTickEvent)
  {
    // Nothing to see here.
    return;
  }

  if (incremental)
  {
    // Reset one flag.

    // Limit iteration to one "cycle" of all flags.
    // Otherwise, this is an infinite loop if all flags are in use.
    for (unsigned int i = 0; i < nflags; i++)
    {
      bool worked = ResetUnusedSuperflag(nextFlag);

      nextFlag++;
      if (nextFlag >= nflags)
      {
	nextFlag = 0;
      }

      if (worked)
      {
	break;
      }
    }
    nextRunTime += freq / (double)nflags;
  }
  else
  {
    // Reset all flags.

    for (unsigned int i = 0; i < nflags; i++)
    {
      // Don't care whether it works or not.
      ResetUnusedSuperflag(i);
    }
    nextRunTime += freq;
  }
}

bool autoFlagReset::ResetUnusedSuperflag(unsigned int flagID)
{
  // Sanity check.
  if (flagID >= bz_getNumFlags())
  {
    return false;
  }

  // Make sure the flag isn't held.
  if (bz_flagPlayer(flagID) != -1)
  {
    return false;
  }

  // Make sure it's not a teamflag.
  bz_ApiString flagType = bz_getFlagName(flagID);
  if (flagType == "R*" || flagType == "G*" || flagType == "B*" || flagType == "P*" ||  flagType == "")
  {
    return false;
  }

  // Looks ok, reset it.
  return bz_resetFlag(flagID);
}

//BZF_API unsigned int bz_getNumFlags( void );
//BZF_API const bz_ApiString bz_getFlagName( int flag );
//BZF_API bool bz_resetFlag ( int flag );
//BZF_API bool bz_moveFlag ( int flag, float pos[3] );
//BZF_API int bz_flagPlayer ( int flag );
//BZF_API bool bz_getFlagPosition ( int flag, float* pos );



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
