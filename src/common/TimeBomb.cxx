/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include "TimeBomb.h"

#if !defined(TIME_BOMB)
#define TIME_BOMB ""
#endif

boolean			timeBombBoom()
{
  const char* timeBomb = timeBombString();
  if (timeBomb) {
    int day, month, year;
    time_t t;
    time(&t);
    const struct tm* tm = localtime(&t);
    sscanf(timeBomb, "%d/%d/%d", &month, &day, &year);
    if (tm->tm_year > year - 1900) return True;
    if (tm->tm_year == year - 1900 && tm->tm_mon > month - 1) return True;
    if (tm->tm_mon == month - 1 && tm->tm_mday >= day) return True;
  }
  return False;
}

const char*		timeBombString()
{
  static const char timeBomb[] = TIME_BOMB;
  if (timeBomb[0] == '\0') return NULL;
  return timeBomb;
}
// ex: shiftwidth=2 tabstop=8
