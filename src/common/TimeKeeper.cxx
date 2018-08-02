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

/* interface header */
#include "TimeKeeper.h"

/* system implementation headers */
#include <thread>

/* common implementation headers */
#include "TextUtils.h"
#include "bzfio.h"


namespace {
    TimeKeeper currentTime;
    TimeKeeper startTime = TimeKeeper::getCurrent(); // initialize when we started
    TimeKeeper tickTime;

    TimeKeeper sunExplodeTime{TimeKeeper::Seconds_t::max()};
    TimeKeeper sunGenesisTime{TimeKeeper::Seconds_t::min()};
    TimeKeeper nullTime{TimeKeeper::Seconds_t::zero()};
}

TimeKeeper::TimeKeeper(Seconds_t secs)
    : lastTime(secs)
{
}

TimeKeeper::operator bool() const
{
    return lastTime != nullTime.lastTime;
}

void TimeKeeper::now()
{
    lastTime = std::chrono::steady_clock::now();
}

const TimeKeeper& TimeKeeper::getCurrent()
{
    currentTime.now();
    return currentTime;
}

const TimeKeeper& TimeKeeper::getStartTime()
{
    return startTime;
}

const TimeKeeper& TimeKeeper::getTick()
{
    return tickTime;
}

void TimeKeeper::setTick()
{
    tickTime = getCurrent();
}

//static
const TimeKeeper& TimeKeeper::getSunExplodeTime()
{
    return sunExplodeTime;
}

//static
const TimeKeeper& TimeKeeper::getSunGenesisTime()
{
    return sunGenesisTime;
}

//static
const TimeKeeper& TimeKeeper::getNullTime()
{
    return nullTime;
}

const char *TimeKeeper::timestamp() // const
{
    static char buffer[256]; // static, so that it doesn't vanish
    time_t tnow = time(0);
    struct tm *now = localtime(&tnow);
    now->tm_year += 1900;
    ++now->tm_mon;

    strncpy(buffer, TextUtils::format("%04d-%02d-%02d %02d:%02d:%02d",
                                      now->tm_year, now->tm_mon, now->tm_mday,
                                      now->tm_hour, now->tm_min, now->tm_sec).c_str(), 256);
    buffer[255] = '\0'; // safety

    return buffer;
}

void TimeKeeper::localTime(int *year, int *month, int* day, int* hour, int* min, int* sec, bool* dst) // const
{
    time_t tnow = time(0);
    struct tm *now = localtime(&tnow);
    now->tm_year += 1900;
    ++now->tm_mon;

    if ( year )
        *year = now->tm_year;
    if ( month )
        *month = now->tm_mon;
    if ( day )
        *day = now->tm_mday;
    if ( hour )
        *hour = now->tm_hour;
    if ( min )
        *min = now->tm_min;
    if ( sec )
        *sec = now->tm_sec;
    if ( dst )
        *dst = now->tm_isdst != 0;
}


void TimeKeeper::UTCTime(int *year, int *month, int* day, int* wday,
                         int* hour, int* min, int* sec, bool* dst) // const
{
    time_t tnow = time(0);
    struct tm *now = gmtime(&tnow);
    now->tm_year += 1900;
    ++now->tm_mon;

    if (year)
        *year  = now->tm_year;
    if (month)
        *month = now->tm_mon;
    if (day)
        *day   = now->tm_mday;
    if (wday)
        *wday  = now->tm_wday;
    if (hour)
        *hour  = now->tm_hour;
    if (min)
        *min   = now->tm_min;
    if (sec)
        *sec   = now->tm_sec;
    if (dst)
        *dst   = (now->tm_isdst != 0);
}

// function for converting a float time (e.g. difference of two TimeKeepers)
// into an array of ints
void TimeKeeper::convertTime(Seconds_t raw, long int convertedTimes[])
{
    // std::chrono::days is C++-20
    auto days = std::chrono::duration_cast<std::chrono::duration<int32_t, std::ratio<86400>>>(raw);
    raw -= days;

    auto hours = std::chrono::duration_cast<std::chrono::hours>(raw);
    raw -= hours;

    auto mins = std::chrono::duration_cast<std::chrono::minutes>(raw);
    raw -= mins;

    auto secs = std::chrono::duration_cast<std::chrono::seconds>(raw);
    raw -= secs;
  
    convertedTimes[0] = days.count();
    convertedTimes[1] = hours.count();
    convertedTimes[2] = mins.count();
    convertedTimes[3] = secs.count();

    return;
}

// function for printing an array of ints representing a time
// as a human-readable string
const std::string TimeKeeper::printTime(long int timeValue[])
{
    std::string valueNames;
    char temp[20];

    if (timeValue[0] > 0) {
        snprintf(temp, 20, "%ld day%s", timeValue[0], timeValue[0] == 1 ? "" : "s");
        valueNames.append(temp);
    }
    if (timeValue[1] > 0) {
        if (timeValue[0] > 0)
            valueNames.append(", ");
        snprintf(temp, 20, "%ld hour%s", timeValue[1], timeValue[1] == 1 ? "" : "s");
        valueNames.append(temp);
    }
    if (timeValue[2] > 0) {
        if ((timeValue[1] > 0) || (timeValue[0] > 0))
            valueNames.append(", ");
        snprintf(temp, 20, "%ld min%s", timeValue[2], timeValue[2] == 1 ? "" : "s");
        valueNames.append(temp);
    }
    if (timeValue[3] > 0) {
        if ((timeValue[2] > 0) || (timeValue[1] > 0) || (timeValue[0] > 0))
            valueNames.append(", ");
        snprintf(temp, 20, "%ld sec%s", timeValue[3], timeValue[3] == 1 ? "" : "s");
        valueNames.append(temp);
    }

    return valueNames;
}

// function for printing a float time difference as a human-readable string
const std::string TimeKeeper::printTime(double diff)
{
    long int temp[4];
    convertTime(Seconds_t(diff), temp);
    return printTime(temp);
}


void TimeKeeper::sleep(double seconds)
{
    if (seconds > 0.0) {
        std::this_thread::sleep_for(Seconds_t(seconds));
    }
    return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
