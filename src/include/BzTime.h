/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
 * BzTime:
 *   Standard way to keep track of time in game.
 *
 * Generally, only the difference between BzTime's is useful.
 * operator-() computes the difference in seconds as a float and
 * correctly handles wraparound.
 * operator+=() allows a time in seconds to be added to a BzTime.
 */

#ifndef BZ_TIME_H
#define BZ_TIME_H

#include "common.h"

/* system interface headers */
#include <string>


/** BzTime keeps time.  It's useful to determine how much time has
 * elapsed from some other point in time.  Use getCurrent() to return a
 * timekeeper object set to the current time.  You can then use subsequent
 * calls to getCurrent and subtract the second from the first to get an
 * elapsed float time value.
 */
class BzTime {

  private: // member data

    double seconds;

  public: // member functions

    explicit BzTime(double secs = 0.0);

    double  operator- (const BzTime&) const;
    BzTime& operator+=(double);
    BzTime& operator+=(const BzTime&);
    bool    operator< (const BzTime&) const;
    bool    operator<=(const BzTime&) const;
    bool    operator> (const BzTime&) const;
    bool    operator>=(const BzTime&) const;
    bool    operator==(const BzTime&) const;
    bool    operator!=(const BzTime&) const;

    /** returns how many seconds have elapsed since the first call to
      * getCurrent(). If real times are needed, use BzTime::localTime() */
    inline double getSeconds() const { return seconds; }

    /** returns true if seconds != 0.0 */
    inline bool active() const { return (seconds != 0.0); }

  public: // static functions

    /** returns a timekeeper representing the current time */
    static const BzTime& getCurrent();

    /** returns a timekeeper representing the time of program execution */
    static const BzTime& getStartTime();

    /** sets the time to the current time (recalculates) */
    static void setTick();

    /** returns a timekeeper that is updated periodically via setTick */
    static const BzTime& getTick(); // const

    /** returns a timekeeper representing +Inf */
    static const BzTime& getSunExplodeTime();
    /** returns a timekeeper representing -Inf */
    static const BzTime& getSunGenesisTime();
    /** returns a timekeeper representing an unset timekeeper */
    static const BzTime& getNullTime();


    /** returns the local time */
    static void localTime(int* year = NULL, int* month = NULL, int* day = NULL,
                          int* hour = NULL, int* min = NULL, int* sec = NULL,
                          bool* dst = NULL);
    static void localTimeDOW(int* year = NULL, int* month = NULL, int* day = NULL,
                             int* dayOfWeek = NULL,
                             int* hour = NULL, int* min = NULL, int* sec = NULL,
                             bool* dst = NULL);
    /** returns a string of the local time */
    static const char* timestamp();

    /** returns a short string of the local time */
    static std::string shortTimeStamp();

    static void localTime(int& day);

    /** returns the UTC time */
    static void UTCTime(int* year = NULL, int* month = NULL, int* day = NULL, int* dayOfWeek = NULL, int* hour = NULL, int* min = NULL, int* sec = NULL, bool* dst = NULL);


    /** converts a time difference into an array of integers
        representing days, hours, minutes, seconds */
    static void convertTime(double raw, long int convertedTimes[]);

    /** prints an integer-array time difference in human-readable form */
    static const std::string printTime(long int timeValue[]);

    /** prints an float time difference in human-readable form */
    static const std::string printTime(double diff);

    /** sleep for a given number of floating point seconds */
    static void sleep(double secs); //const

    /** try to lock the process to a given CPU to avoid timekeeper from
        going back in time */
    static void setProcessorAffinity(int processor = 0);
};


//
// BzTime (more inlined functions)
//

inline BzTime::BzTime(double secs) : seconds(secs) {
  // do nothing
}


inline double BzTime::operator-(const BzTime& t) const {
  return seconds - t.seconds;
}

inline BzTime& BzTime::operator+=(double dt) {
  seconds += dt;
  return *this;
}
inline BzTime& BzTime::operator+=(const BzTime& t) {
  seconds += t.seconds;
  return *this;
}

inline bool BzTime::operator< (const BzTime& t) const { return seconds <  t.seconds; }
inline bool BzTime::operator<=(const BzTime& t) const { return seconds <= t.seconds; }
inline bool BzTime::operator> (const BzTime& t) const { return seconds >  t.seconds; }
inline bool BzTime::operator>=(const BzTime& t) const { return seconds >= t.seconds; }
inline bool BzTime::operator==(const BzTime& t) const { return seconds == t.seconds; }
inline bool BzTime::operator!=(const BzTime& t) const { return seconds != t.seconds; }


#endif // BZ_TIME_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
