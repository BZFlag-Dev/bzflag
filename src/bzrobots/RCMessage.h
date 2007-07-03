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

class RCLink;
class RCRobotPlayer;

template <class C>
class RCMessage {
  public:
    typedef enum {
      ParseError,
      ParseOk,
      InvalidArgumentCount,
      InvalidArguments
    } parseStatus;
    typedef std::map<std::string, C *(*)(RCLink *)> lookupTable;

    /* These are static functions to allow for instantiation
     * of classes based on a string (the request command name) */
    static C *getInstance(std::string message, RCLink *_link)
    {
        if (messageLookup.find(message) != messageLookup.end())
            return messageLookup[message](_link);
        return NULL;
    }


    RCMessage(RCLink *_link) :next(NULL), link(_link) { }
    virtual ~RCMessage() {}

    /* This is for the linked-list aspect of RCMessage. */
    C *getNext() { return next; }
    void append(C *newreq)
    {
      if (next == NULL)
        next = newreq;
      else
        next->append(newreq);
    }

    virtual parseStatus parse(char **arguments, int count) = 0;
    virtual std::string getType() = 0;

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
    /* These are static data and functions to allow for instantiation 
     * of classes based on a string (the request command name) */
    static lookupTable messageLookup;
    template <typename T>
     static C* instantiate(RCLink *_link) { return new T(_link); }
    RCLink *link;
};

template <class C>
typename RCMessage<C>::lookupTable RCMessage<C>::messageLookup;

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
