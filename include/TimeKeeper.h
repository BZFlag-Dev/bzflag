/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/**
 * TimeKeeper:
 *	Standard way to keep track of time in game.
 *
 * Generally, only the difference between TimeKeeper's is useful.
 * operator-() computes the difference in seconds as a float and
 * correctly handles wraparound.
 * operator+=() allows a time in seconds to be added to a TimeKeeper.
 */

#ifndef	BZF_TIME_KEEPER_H
#define	BZF_TIME_KEEPER_H

#include "common.h"

/* system interface headers */
#include <string>


/** TimeKeeper keeps time.  It's useful to determine how much time has
 * elapsed from some other point in time.  Use getCurrent() to return a
 * timekeeper object set to the current time.  You can then use subsequent
 * calls to getCurrent and subtract the second from the first to get an
 * elapsed float time value.
 */
class TimeKeeper {
public:
  TimeKeeper();
  TimeKeeper(const TimeKeeper&);
  ~TimeKeeper();
  TimeKeeper&		operator=(const TimeKeeper&);

  double		operator-(const TimeKeeper&) const;
  bool			operator<=(const TimeKeeper&) const;
  TimeKeeper&		operator+=(double);
  TimeKeeper&		operator+=(const TimeKeeper&);

  // make a TimeKeeper with seconds = NULL act like unset
  // Fixme: must this be defined here? didn't work for me outside the class
  inline operator void*()
  {
    if (seconds > 0.0)
      return this;
    else
      return NULL;
   }

  /** Returns how many seconds have elapsed since epoch, Jan 1, 1970.
    * On windows this will actually be how many seconds have elapsed 
    * since bootup, so don't rely on the value for anything but 
    * deltas.  If real times are needed use TimeKeeper::localTime */
  double       getSeconds(void) const;

  /** returns a timekeeper representing the current time */
  static const TimeKeeper&	getCurrent(void);

  /** returns a timekeeper representing the time of program execution */
  static const TimeKeeper&	getStartTime(void);

  /** sets the time to the current time (recalculates) */
  static void			setTick(void);
  /** returns a timekeeper that is updated periodically via setTick */
  static const TimeKeeper&	getTick(void); // const

  /** returns a timekeeper representing +Inf */
  static const TimeKeeper&	getSunExplodeTime(void);
  /** returns a timekeeper representing -Inf */
  static const TimeKeeper&	getSunGenesisTime(void);
  /** returns a timekeeper representing an unset timekeeper */
  static const TimeKeeper&	getNullTime(void);


  /** returns the local time */
  static void localTime(int *year = NULL, int *month = NULL, int* day = NULL, int* hour = NULL, int* min = NULL, int* sec = NULL, bool* dst = NULL);

  /** returns a string of the local time */
  static const char		*timestamp(void);

  static void localTime( int &day);

  /** converts a time difference into an array of integers
      representing days, hours, minutes, seconds */
  static void			convertTime(double raw, long int convertedTimes[]);
  /** prints an integer-array time difference in human-readable form */
  static const std::string	printTime(long int timeValue[]);
  /** prints an float time difference in human-readable form */
  static const std::string	printTime(double diff);

  /** sleep for a given number of floating point seconds */
  static void			sleep(double secs); //const

private:
  double		seconds;
};

//
// TimeKeeper
//

inline TimeKeeper::TimeKeeper() : seconds(0.0)
{
  // do nothing
}

inline TimeKeeper::TimeKeeper(const TimeKeeper& t) :
  seconds(t.seconds)
{
  // do nothing
}

inline TimeKeeper::~TimeKeeper()
{
  // do nothing
}

inline TimeKeeper&	TimeKeeper::operator=(const TimeKeeper& t)
{
  seconds = t.seconds;
  return *this;
}

inline double		TimeKeeper::operator-(const TimeKeeper& t) const
{
  return seconds - t.seconds;
}

inline TimeKeeper&	TimeKeeper::operator+=(double dt)
{
  seconds += dt;
  return *this;
}
inline TimeKeeper&	TimeKeeper::operator+=(const TimeKeeper& t)
{
  seconds += t.seconds;
  return *this;
}

inline bool		TimeKeeper::operator<=(const TimeKeeper& t) const
{
  return seconds <= t.seconds;
}

inline double		TimeKeeper::getSeconds(void) const
{
  return seconds;
}


#endif // BZF_TIME_KEEPER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
