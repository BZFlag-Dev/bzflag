/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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
class FlagInfo {
public:

  FlagInfo();

  void setRequiredFlag(FlagType *desc);
  void addFlag();
  void *pack(void *buf);
  void dropFlag(float pos[3], float landingPos[3], bool vanish);
  void resetFlag(float position[3]);

  static void setSize(int _numFlags);
  static void setAllowed(std::vector<FlagType*> allowed);
  static void setExtra(int extra);

  static FlagInfo *flagList;

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

private:
  int flagIndex;
  static std::vector<FlagType*> allowedFlags;
  static int                    numExtraFlags;
  static int                    numFlags;
};
#else
class FlagInfo;
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

