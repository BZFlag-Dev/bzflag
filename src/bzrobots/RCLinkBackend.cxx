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

#ifndef _WIN32
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <errno.h>
#include <stdarg.h>

#include "RCReplies.h"
#include "RCLinkBackend.h"
#include "RCMessageFactory.h"

#include "bzflag.h"
#include "Roster.h"

#include "version.h"

#include "RCLink.h"

using std::endl;

RCLink::State RCLinkBackend::getDisconnectedState()
{
  return RCLink::Listening;
}

void RCLinkBackend::pushEvent(RCEvent *event)
{
  if (events == NULL)
    events = event;
  else
  {
    /* TODO: Prevent duplicate items better? */
    for (RCEvent *ev = events; ev != NULL; ev = (RCEvent *)ev->getNext())
    {
      if (ev->asString() == event->asString())
        return;
    } 

    events->append(event);
  }
}
RCEvent *RCLinkBackend::popEvent()
{
  RCEvent *event = events;
  if (event != NULL)
    events = (RCEvent *)event->getNext();
  return event;
}

/*
 * Check for activity.  If possible, fill up the recvbuf with incoming data
 * and build up RCRequest objects as appropriate.
 */

void RCLinkBackend::sendPacket ( const char *data, unsigned int size, bool killit )
{
#ifdef _USE_FAKE_NET
	fakeNetSendToFrontEnd(size,data);
	if (killit)
		fakenetDisconect();
#else
	write(connfd, data, size);
	if (killit)
		close(connfd);
#endif
}


void RCLinkBackend::update()
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
      RCRequest *req = popRequest();
      if (req && req->getType() == "IdentifyFrontend") {
        status = Connected;
      } else {
        BACKENDLOGGER << "RCLink: Expected an 'IdentifyFrontend'." << endl;
		sendPacket(RC_LINK_NOIDENTIFY_MSG, (unsigned int)strlen(RC_LINK_NOIDENTIFY_MSG),true);
        status = Listening;
      }
    }
  }
}

/*
 * Parse a command, create an RCRequest object, and add it to requests.
 * Return true if an RCRequest was successfully created.  If it failed,
 * return false.
 */
bool RCLinkBackend::parseCommand(char *cmdline)
{
  RCRequest *req;
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

  req = RCREQUEST.Message(argv[0]);
  if (req == NULL) {
    BACKENDLOGGER << "RCLink: Invalid request: '" << argv[0] << "'" << endl;
    sendf("error Invalid request %s\n", argv[0]);
    return false;
  } else {
    switch (req->parse(argv + 1, argc - 1))
    {
      case ParseOk:
        if (requests == NULL)
          requests = req;
        else
          requests->append(req);
        return true;
      case InvalidArgumentCount:
        BACKENDLOGGER << "RCLink: Invalid number of arguments (" << argc - 1
          << ") for request '" << argv[0] << "'." << endl;
        sendf("error Invalid number of arguments (%d) for request: '%s'\n", argc - 1, argv[0]);
        return false;
      case InvalidArguments:
        BACKENDLOGGER << "RCLink: Invalid arguments for request '" << argv[0] << "'." << endl;
        sendf("error Invalid arguments for request: '%s'\n", argv[0]);
        return false;
      default:
        BACKENDLOGGER << "RCLink: Parse neither succeeded nor failed with a known failcode. WTF?" << endl;
        return false;
    }
  }
}

RCRequest* RCLinkBackend::popRequest()
{
  RCRequest *req = requests;
  if (req != NULL) {
    requests = req->getNext();
  }
  return req;
}
RCRequest* RCLinkBackend::peekRequest()
{
  return requests;
}

bool RCLinkBackend::tryAccept()
{
  if (!RCLink::tryAccept())
    return false;

  sendPacket(RC_LINK_IDENTIFY_STR, (unsigned int)strlen(RC_LINK_IDENTIFY_STR));
  sendPacket(getRobotsProtocolVersion(),(unsigned int)strlen(getRobotsProtocolVersion()));
  sendPacket("\n", 1);
  return true;
}

void RCLinkBackend::sendAck(RCRequest *req)
{
  RCLink::send(CommandDoneReply(req->getType()));
}

/* We bundle Events before normal replies by 
 * calling sendEvent() before we send any data in send/sendf. 
 * This again calls send, which calls sendEvent again,
 * so it recursively loops over all pending events. */
void RCLinkBackend::sendEvent()
{
  RCEvent *event = popEvent();
  if (event != NULL)
  {
    RCLink::send(EventReply(event));
    delete event;
  }
}

bool RCLinkBackend::send(const char *message)
{
  sendEvent();
  return RCLink::send(message);
}
bool RCLinkBackend::sendf(const char *format, ...)
{
  va_list ap;
  bool ret;

  sendEvent();

  va_start(ap, format);
  ret = RCLink::vsendf(format, ap);
  va_end(ap);

  return ret;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
