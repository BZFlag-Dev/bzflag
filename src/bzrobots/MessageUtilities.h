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

/*
 * MessageUtilities: A namespace of useful functions for messages.
 */

#ifndef	BZROBOTS_MESSAGEUTILITIES_H
#define	BZROBOTS_MESSAGEUTILITIES_H

#include "RCReply.h"

namespace MessageUtilities
{
  bool parseFloat(char *string, float &dest);
  messageParseStatus parseSingleFloat(char **arguments, int count, float &dest);

  template <typename T>
    static T clamp(T val, T min, T max)
    {
      // Mad cred to _neon_/#scene.no and runehol/#scene.no for these two sentences:
      //  * If val is nan, the result is undefined
      //  * If max < min, the result is undefined
      if (val > max)
        return max;
      if (val < min)
        return min;
      return val;
    }
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
