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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

#include "RCLinkFrontend.h"

#include "version.h"

RCLinkFrontend::RCLinkFrontend(std::string _host, int _port) :RCLink(),
                                        replies(NULL)
{
  port = _port;
  host = _host.c_str();
  connect();
}

RCLink::State RCLinkFrontend::getDisconnectedState()
{
    return RCLink::Disconnected;
}


/*
 * Check for activity.  If possible, fill up the recvbuf with incoming data
 * and build up RCReply objects as appropriate.
 */
void RCLinkFrontend::update()
{
  if (status != Connected && status != Connecting) {
    return;
  }

  updateWrite();
  int amount = updateRead();

  if (amount == -1) {
    status = Listening;
    return;
  }

  if (status == Connected) {
    updateParse();
  } else if (status == Connecting) {
    int ncommands = updateParse(1);
    if (ncommands) {
      RCReply *req = popReply();
      if (req && req->getType() == "IdentifyBackend") {
        status = Connected;
      } else {
        fprintf(stderr, "RCLink: Expected an 'IdentifyBackend'.\n");
        close(connfd);
        status = Disconnected;
      }
    }
  }
}

/*
 * Parse a command, create an RCReply object, and add it to replies.
 * Return true if an RCReply was successfully created.  If it failed,
 * return false.
 */
bool RCLinkFrontend::parseCommand(char *cmdline)
{
  RCReply *req;
  int argc;
  char *argv[RC_LINK_MAXARGS];
  char *s, *tkn;

  s = cmdline;
  for (argc = 0; argc < RC_LINK_MAXARGS; argc++) {
    tkn = strtok(s, " \r\t");
    s = NULL;
    argv[argc] = tkn;
    if (tkn == NULL || *tkn == '\0')
      break;
  }

  req = RCReply::getInstance(argv[0], this);
  if (req == NULL) {
    fprintf(stderr, "RCLink: Invalid request: '%s'\n", argv[0]);
    sendf("error Invalid request %s\n", argv[0]);
    return false;
  } else {
    switch (req->parse(argv + 1, argc - 1))
    {
      case RCReply::ParseOk:
        if (replies == NULL)
          replies = req;
        else
          replies->append(req);
        return true;
      case RCReply::InvalidArgumentCount:
        fprintf(stderr, "RCLink: Invalid number of arguments (%d) for request: '%s'\n", argc - 1, argv[0]);
        sendf("error Invalid number of arguments (%d) for request: '%s'\n", argc - 1, argv[0]);
        return false;
      case RCReply::InvalidArguments:
        fprintf(stderr, "RCLink: Invalid arguments for request: '%s'\n", argv[0]);
        sendf("error Invalid arguments for request: '%s'\n", argv[0]);
        return false;
      default:
        fprintf(stderr, "RCLink: Parse neither succeeded or failed with a known failcode. WTF?\n");
        return false;
    }
  }
}

RCReply* RCLinkFrontend::popReply()
{
  RCReply *rep = replies;
  if (rep != NULL)
    replies = rep->getNext();
  return rep;
}
RCReply* RCLinkFrontend::peekReply()
{
  return replies;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
