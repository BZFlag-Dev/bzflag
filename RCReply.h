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
 * Remote Control Reply: Encapsulates the reply to a request from a frontend.
 */

#ifndef	BZF_RC_REPLY_H
#define	BZF_RC_REPLY_H

#include "common.h"
#include "RCMessage.h"

#include <string>
#include <map>

class BZAdvancedRobot;

class RCReply :public RCMessage<RCReply> {
  public:
    virtual bool updateBot(BZAdvancedRobot *) const { return true; }
    virtual messageParseStatus parse(char **arguments, int count) = 0;
    virtual std::string getType() const = 0;
    virtual void getParameters(std::ostream &stream) const = 0;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
