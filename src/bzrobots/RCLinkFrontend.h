/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__RCLINKFRONTEND_H__
#define	__RCLINKFRONTEND_H__

#include "common.h"

/* system interface headers */
#include <string>

/* local interface headers */
#include "RCLink.h"
#include "RCReply.h"
#include "Logger.h"
#include "BZAdvancedRobot.h"
#include "RCRequest.h"


/**
 * Remote Control Link, Frontend: Encapsulates communication between
 * backend and frontend, from the frontends point of view.
 */
class RCLinkFrontend : public RCLink
{
private:
  RCReply *replies;
  bool hasReply(const std::string command) const;

public:
  RCLinkFrontend() : RCLink(FrontendLogger::pInstance()), replies(NULL) { isFrontEnd = true; }
  bool update();
  bool parseCommand(char *cmdline);
  RCReply* popReply();
  RCReply* peekReply();
  State getDisconnectedState();
  bool waitForReply(const std::string command);
  bool sendAndProcess(const RCRequest &request, const BZAdvancedRobot *bot);
};

#else
class RCLinkFrontend;
#endif /* __RCLINKFRONTEND_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
