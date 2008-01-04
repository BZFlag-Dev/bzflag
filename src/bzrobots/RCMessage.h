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
#include <sstream>

class RCLink;
class RCRobotPlayer;

typedef enum {
  ParseError,
  ParseOk,
  InvalidArgumentCount,
  InvalidArguments
} messageParseStatus;

template <class C>
class RCMessage
{
  public:

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
    virtual messageParseStatus parse(char **arguments, int count) = 0;
    virtual std::string getType() const = 0;
    virtual void getParameters(std::ostream &stream) const = 0;

    virtual std::string asString() const {
      std::stringstream ss;
      ss << getType() << " ";
      getParameters(ss);
      return ss.str();
    }

  private:
    C *next;

  protected:
    RCLink *link;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
