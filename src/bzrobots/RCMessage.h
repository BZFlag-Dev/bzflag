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
 * Remote Control Message: Encapsulates generic messages between
 * frontends and backends.
 */

#ifndef	BZF_RC_MESSAGE_H
#define	BZF_RC_MESSAGE_H

#include "common.h"
#include <string>
#include <map>
#include <ostream>

class RCLink;
class RCRobotPlayer;

template <class C>
class RCMessage
{
  public:
    typedef enum {
      ParseError,
      ParseOk,
      InvalidArgumentCount,
      InvalidArguments
    } parseStatus;

    RCMessage() :next(NULL), link(NULL) { }
    virtual ~RCMessage() {}

    void setLink(RCLink *_link) { link = _link; }

    /* This is for the linked-list aspect of RCMessage. */
    C *getNext() { return next; }
    void append(C *newreq)
    {
      if (next == NULL)
        next = newreq;
      else
        next->append(newreq);
    }

    /* These three are dependent on the specific packet-type, so they are
     * left for the complete implementations. :-) */
    virtual parseStatus parse(char **arguments, int count) = 0;
    virtual std::string getType() const = 0;
    virtual void getParameters(std::ostream &stream) const = 0;

    /* Utility functions. */
    static bool parseFloat(char *string, float &dest)
    {
      char *endptr;
      dest = strtof(string, &endptr);
      if (endptr == string)
          return false;

      /* We don't want NaN no matter what - it's of no use in this scenario.
       * (And strtof will allow the string "NAN" as NaN) */
      if (isnan(dest))
          dest = 0.0f;

      return true;
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

  private:
    C *next;

  protected:
    RCLink *link;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
