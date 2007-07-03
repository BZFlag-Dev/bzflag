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
 * Remote Control Link: Encapsulates communication between backend and frontend.
 * (This is the generic base-functionality)
 */

#ifndef	BZF_RC_LINK_H
#define	BZF_RC_LINK_H

#include "common.h"

#include "global.h"
#include "Address.h"
#include "TimeKeeper.h"

#define RC_LINK_RECVBUFLEN 100000
#define RC_LINK_SENDBUFLEN 100000
#define RC_LINK_MAXARGS 50
#define RC_LINK_OVERFLOW_MSG "\nerror Connection Stalled.  RC stopped" \
  " reading data!\n"

class RCLink {
  public:
    typedef enum {
      Disconnected,
      SocketError,
      Listening,
      Connecting,
      Connected
    } State;

    RCLink();
    virtual ~RCLink();

    bool connect();
    void startListening();
    virtual bool tryAccept();
    virtual State getDisconnectedState() = 0;

    virtual bool parseCommand(char *cmdline) = 0;
    int updateParse(int maxlines = 0);
    int updateWrite();
    int updateRead();
    void detachAgents();

    bool send(char *message);
    bool sendf(const char *format, ...);

  protected:
    State status;
    int listenfd, connfd;
    int port;
    const char *host;
    char recvbuf[RC_LINK_RECVBUFLEN];
    char sendbuf[RC_LINK_SENDBUFLEN];
    int recv_amount, send_amount;
    bool input_toolong, output_overflow;
};


//
// RCLink
//

#endif // BZF_RC_LINK_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
