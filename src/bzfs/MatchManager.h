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

class CountDown
{
public:
  CountDown( double interval=1, int count=10 );
  ~CountDown() {};

  int getCounter();
  void setCounter(int count);
  void doReset();
  bool doCountdown();
  bool inProgress();

private:

  double _interval;
  double _currentTime;
  double _previousTime;
  int _startCount, _counter;
  
};


class MatchManager : public Singleton<MatchManager> , bz_EventHandler , bz_CustomSlashCommandHandler
{
public:
  // API event
  virtual void process ( bz_EventData *eventData );

  // custom slash
  virtual bool handle ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params );

  virtual bool autoDelete ( void ) { return true; }

  void init();

protected:
  friend class Singleton<MatchManager>;

  typedef enum {
    eOff,
    ePregame,
    eOn,
    ePostgame
  } MatchState;

  MatchState matchState;

  bool paused;
  bool report;
  double resumeTime;
  double startTime;
  double pauseTime;
  double duration;
  double resetTime;
  double endTime;

  double currentTime;
private:

  MatchManager();
  ~MatchManager();

  // countdown timers

  CountDown preGameTimer;
  CountDown endTimer;

  // start future BZDB vars
  double _matchPregameTime;
  double _matchDuration;
  double _matchEndCountdown;
  double _matchResetTime;
  
  bool _matchDisallowJoins;
  bool _matchResetScoreOnEnd;
  bool _matchReportMatches;
  // end future BZDB vars
 
  // methods
  void start ( int playerID, bz_APIStringList *params );	
  void end ( int playerID, bz_APIStringList *params );	
  void pause ( int playerID, bz_APIStringList *params );	
  void substitute ( int playerID, bz_APIStringList *params );	
  
  void doPregame();
  void doOngame();
  void doPostgame();
  void doReportgame();
  
  void disablePlayerSpawn();
  void resetTeamScores();
  void resetPlayerScores();

};

#endif /* __MATCHMANAGER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
