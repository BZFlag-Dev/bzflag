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

#ifndef	__MESSAGEUTILITIES_H__
#define	__MESSAGEUTILITIES_H__

#include "common.h"

/* system interface headers */
#include <string>

/* local interface header */
#include "RCMessage.h"


/**
 * MessageUtilities: A namespace of useful functions for messages.
 */
namespace MessageUtilities
{
  template<typename T>
  bool parse(const char *string, T &dest);
  template<>
  bool parse(const char *string, bool &dest);
  template<>
  bool parse(const char *string, double &dest);
  template<>
  bool parse(const char *string, std::string &dest);

  template<typename T>
  messageParseStatus parseSingle(char **arguments, int count, T &dest)
  {
    if (count != 1)
      return InvalidArgumentCount;
    if (!parse(arguments[0], dest))
      return InvalidArguments;

    return ParseOk;
  }

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
  template <typename T>
  static T overflow(T val, T min, T max)
  {
    if (val > max)
      return min + (val - max);
    if (val < min)
      return max + (val - min);
    return val;
  }
}

#endif /* __MESSAGEUTILITIES_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
