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

#include "RCLinkFrontend.h"
#include "RCMessageFactory.h"

#include "version.h"

RCLink::State RCLinkFrontend::getDisconnectedState()
{
    return RCLink::Disconnected;
}

bool RCLinkFrontend::sendAndProcess(const RCRequest &request, const BZAdvancedRobot *bot)
{
  if (!send(request))
    return false;
  waitForReply(request.getType());

  RCReply *reply = popReply();
  while (reply != NULL)
  {
    reply->updateBot(bot);
    reply = popReply();
  }

  return true;
}

bool RCLinkFrontend::hasReply(const std::string command) const
{
  RCReply *reply = replies;
  while (reply != NULL)
  {
    if (reply->getType() == "CommandDone" && ((CommandDoneReply *)reply)->command == command)
      return true;
    reply = reply->getNext();
  }

  return false;
}

bool RCLinkFrontend::waitForReply(const std::string command)
{
  if (status != Connected)
    return false;

  if (hasReply(command))
    return true;

  while (waitForData())
  {
    update();
    if (hasReply(command))
      return true;
  }

  // We only get here if waitForData() returns false -> an error occured.
  return false;
}

/*
 * Check for activity.  If possible, fill up the recvbuf with incoming data
 * and build up RCReply objects as appropriate.
 */
bool RCLinkFrontend::update()
{
  if (status != Connected && status != Connecting)
    return false;

  updateWrite();
  int amount = updateRead();

  if (amount == -1) {
    status = Disconnected;
    return false;
  }

  if (status == Connected) {
    updateParse();
  } else if (status == Connecting) {
    int ncommands = updateParse(1);
    if (ncommands) {
      RCReply *rep = popReply();
      if (rep && rep->getType() == "IdentifyBackend") {
        send(IdentifyFrontend(getRobotsProtocolVersion()));
        status = Connected;
      } else {
        FRONTENDLOGGER << "RCLink: Expected an 'IdentifyBackend'." << std::endl;
        close(connfd);
        status = Disconnected;
        return false;
      }
    }
  }
  return true;
}

/*
 * Parse a command, create an RCReply object, and add it to replies.
 * Return true if an RCReply was successfully created.  If it failed,
 * return false.
 */
bool RCLinkFrontend::parseCommand(char *cmdline)
{
  RCReply *rep;
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

  rep = RCREPLY.Message(argv[0]);
  if (rep == NULL) {
    FRONTENDLOGGER << "RCLinkFrontend: Unregistered reply: '" << argv[0] << "'" << std::endl;
    close(connfd);
    status = Disconnected;
    return false;
  } else {
    switch (rep->parse(argv + 1, argc - 1))
    {
      case ParseOk:
        if (replies == NULL)
          replies = rep;
        else
          replies->append(rep);
        return true;
      case InvalidArgumentCount:
        FRONTENDLOGGER << "RCLinkFrontend: Invalid number of arguments (" << argc - 1
          << ") for reply: '" << argv[0] << "'" << std::endl;
        close(connfd);
        status = Disconnected;
        return false;
      case InvalidArguments:
        FRONTENDLOGGER << "RCLinkFrontend: Invalid arguments for reply: '" << argv[0] << "'" << std::endl;
        close(connfd);
        status = Disconnected;
        return false;
      default:
        FRONTENDLOGGER << "RCLinkFrontend: Parse neither succeeded or failed with a known failcode. WTF?" << std::endl;
        close(connfd);
        status = Disconnected;
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
