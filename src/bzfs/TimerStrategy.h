/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Timer Strategies encapsulate Timers such as the games countown 
   mechanisms with their actions.
   TODO:
   - CountdownStrategy - multi purpose, derives from BaseTimerStrategy
   For use with future GameKeeper::CountdownManager singleton class,
   derived from GameTimerStrategy:
   - GameCountdownStrategy
   - ManualCountdownStarterStrategy
   - ManualCountdownStrategy  
   - implement all this ;)
*/

#ifndef _TIMERSTRATEGY_H
#define _TIMERSTRATEGY_H

// bzflag global header
#include "common.h"

// common interface headers
#include "TimeKeeper.h"

class BaseTimerStrategy {
public:
  BaseTimerStrategy (const float duration);
  BaseTimerStrategy (const TimeKeeper endTime);
  
  virtual ~BaseTimerStrategy() {};
 
  virtual bool pause(const float delay = 0.0f);
  virtual bool resume(const float delay = 0.0f);
  
  virtual BaseTimerStrategy* action(TimeKeeper& tm) = 0;
  
private:
  void update();
  TimeKeeper startTime;
  TimeKeeper pauseTime;
  float	pauseOffset;
  float _duration;
};

class GameTimerStrategy : public BaseTimerStrategy {
public:
  GameTimerStrategy (const float duration) : BaseTimerStrategy(duration) {};
  GameTimerStrategy (const TimeKeeper endTime) : BaseTimerStrategy(endTime) {};

  virtual ~BaseTimerStrategy() {};
	  
private:
  float getTimeLeft();

  bool expired();

  void sendGameStartEvent();
  void sendGameEndEvent();
  void sendMsgTimeUpdate(float timeLeft);
}

#endif /* _TIMERSTRATEGY_H */
