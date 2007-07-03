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
 * Remote Control Link, Frontend: Encapsulates communication between backend and
 * frontend, from the frontends point of view.
 */

#ifndef	BZF_RC_LINK_FRONTEND_H
#define	BZF_RC_LINK_FRONTEND_H

#include "RCLink.h"
#include "RCRequest.h"

class RCLinkFrontend : public RCLink
{
  private:
    RCRequest *requests;

  public:
    RCLinkFrontend(std::string _host, int _port);
    void update();
    bool parseCommand(char *cmdline);
    RCRequest* popRequest();
    RCRequest* peekRequest();
    bool tryAccept();
    State getDisconnectedState();
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
