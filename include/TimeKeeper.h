/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
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

    float		operator-(const TimeKeeper&) const;
    bool		operator<=(const TimeKeeper&) const;
    TimeKeeper&		operator+=(float);
    TimeKeeper&		operator+=(const TimeKeeper&) ;

    /** returns how many seconds have elapsed since epoch, Jan 1, 1970 */
    float               getSeconds(void) const;

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

private:
    double		seconds;
    static TimeKeeper	currentTime;
    static TimeKeeper	tickTime;
    static TimeKeeper	sunExplodeTime;
    static TimeKeeper	sunGenesisTime;
    static TimeKeeper	nullTime;
    static TimeKeeper	startTime;
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

inline float		TimeKeeper::operator-(const TimeKeeper& t) const
{
  return (float)(seconds - t.seconds);
}

inline TimeKeeper&	TimeKeeper::operator+=(float dt)
{
  seconds += double(dt);
  return *this;
}
inline TimeKeeper&	TimeKeeper::operator+=(const TimeKeeper& t)
{
  seconds += double(t.seconds);
  return *this;
}

inline bool		TimeKeeper::operator<=(const TimeKeeper& t) const
{
  return seconds <= t.seconds;
}

inline float		TimeKeeper::getSeconds(void) const
{
  return (float)seconds;
}

#endif // BZF_TIME_KEEPER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

