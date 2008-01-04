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

#ifndef	__RCLINKBACKEND_H__
#define	__RCLINKBACKEND_H__

#include "common.h"

/* local interface headers */
#include "RCLink.h"
#include "RCRequest.h"
#include "RCEvent.h"

#define RC_LINK_NOIDENTIFY_MSG "error IdentifyFrontend expected\n"
#define RC_LINK_IDENTIFY_STR "IdentifyBackend "


/**
 * Remote Control Link, Backend: Encapsulates communication between backend and
 * frontend, from the backends point of view.
 */
class RCLinkBackend : public RCLink
{

private:
  RCRequest *requests;
  RCEvent *events;
  void sendEvent();

public:
  RCLinkBackend();
  ~RCLinkBackend();

  void update();
  bool parseCommand(char *cmdline);
  RCRequest* popRequest();
  RCRequest* peekRequest();
  void pushEvent(RCEvent *event);
  RCEvent* popEvent();
  bool tryAccept();
  State getDisconnectedState();
  void sendAck(RCRequest *req);
  void sendPacket ( const char *data, unsigned int size, bool killit = false );

  bool send(const char *message);
  bool sendf(const char *format, ...) __attribute__ ((__format__ (__printf__, 2, 3)));
};

#endif /* __RCLINKBACKEND_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
