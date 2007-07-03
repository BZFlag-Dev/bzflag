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
 * Remote Control Request: Encapsulates requests between backend and frontend
 */

#ifndef	BZF_RC_REQUEST_H
#define	BZF_RC_REQUEST_H

#include "common.h"
#include "RCMessage.h"

#include <string>
#include <map>

class RCLink;
class RCRobotPlayer;
class RCReply;

class RCReply :public RCMessage<RCReply> {
  public:
    RCReply(RCLink *_link);
    virtual ~RCReply();

    virtual parseStatus parse(char **arguments, int count) = 0;
    virtual std::string getType() = 0;
    virtual void getParameters(std::ostream &stream) = 0;

    static void initializeLookup(void);

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
