/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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

class TimeKeeper {
  public:
			TimeKeeper();
			TimeKeeper(const TimeKeeper&);
			~TimeKeeper();
    TimeKeeper&		operator=(const TimeKeeper&);

    float		operator-(const TimeKeeper&) const;
    TimeKeeper&		operator+=(float);
    bool		operator<=(const TimeKeeper&) const;
    float               getSeconds() const;

    static const TimeKeeper&	getCurrent();
    static const TimeKeeper&	getTick(); // const
	static const TimeKeeper&	getSunExplodeTime();
    static void			setTick();

  private:
    double		seconds;
    static TimeKeeper	currentTime;
    static TimeKeeper	tickTime;
	static TimeKeeper	sunExplodeTime;
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

inline bool		TimeKeeper::operator<=(const TimeKeeper& t) const
{
  return seconds <= t.seconds;
}

inline float		TimeKeeper::getSeconds() const
{
  return seconds;
}

#endif // BZF_TIME_KEEPER_H
// ex: shiftwidth=2 tabstop=8
