/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __FLAGINFO_H__
#define __FLAGINFO_H__

/* system headers */
#include <stdlib.h>

/* bzflag common headers */
#include "Flag.h"
#include "TimeKeeper.h"

/** FlagInfo describes a flag as it pertains to the world.
 */
struct FlagInfo {
  public:
    // flag info
    Flag flag;
    // player index who has flag
    int player;
    // how many grabs before removed
    int grabs;
    // true if flag must be in game
    bool required;
    // time flag will land
    TimeKeeper dropDone;
    // number of shots on this flag
    int numShots;
};

extern FlagInfo *flag;


bool setRequiredFlag(FlagInfo& flag, FlagDesc *desc);


#else
struct FlagInfo;
#endif
// ex: shiftwidth=2 tabstop=8
