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

#ifndef __MATCHMANAGER_H__
#define __MATCHMANAGER_H__

#include "common.h"

/* system interface headers */
#include <map>
#include <string>
#include <vector>

/* common interface headers */
#include "Singleton.h"

#include "bzfsAPI.h"

class MatchManager : public Singleton<MatchManager> , bz_EventHandler , bz_CustomSlashCommandHandler
{
public:
  // API event
  virtual void process ( bz_EventData *eventData );

  // custom slash
  virtual bool handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params );

  virtual bool autoDelete ( void ) { return true; }
protected:
  friend class Singleton<MatchManager>;

  typedef enum
  {
    eOff,
    ePregame,
    eOn,
    ePostgame
  }
  MatchState;

  MatchState matchState;

  bool paused;
  double resumeTime;

  double startTime;
  double duration;
  double resetTime;

private:
  MatchManager();
  ~MatchManager();
};


#endif /* __MATCHMANAGER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
