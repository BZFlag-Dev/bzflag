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

// BZFlag network
//#include "network.h"

// interface header
#include "RCLink.h"
#include "Roster.h"
#include "RCRobotPlayer.h"

RCLink::RCLink() :
	            status(Disconnected),
		    listenfd(-1),
		    connfd(-1)
{
}

RCLink::~RCLink()
{
}

void RCLink::startListening()
{
  struct sockaddr_in sa;

  if (status != Disconnected) {
    return;
  }

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  //sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) return;

  if (bind(listenfd, (sockaddr*)&sa, sizeof(sa)) == -1) {
    close(listenfd);
    return;
  }

  if (listen(listenfd, 1) == -1) {
    close(listenfd);
    return;
  }

  //BzfNetwork::setNonBlocking(listenfd);
  int flags = fcntl(listenfd, F_GETFL);
  fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);

  status = Listening;
}

bool RCLink::tryAccept()
{
  if (status != Listening)
    return false;

  // O_NONBLOCK is set so we'll probably return immediately.
  connfd = accept(listenfd, NULL, 0);
  if (connfd == -1)
    return false;

  //BzfNetwork::setNonBlocking(connfd);
  int flags = fcntl(connfd, F_GETFL);
  fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

  status = Connecting;
  send_amount = 0;
  recv_amount = 0;
  input_toolong = false;

  // Now we wait for them to introduce themselves.
  return true;
}

bool RCLink::send(char* message)
{
  if (output_overflow) {
    return false;
  }

  int messagelen = strlen(message);

  if (send_amount + messagelen > RC_LINK_SENDBUFLEN) {
    fprintf(stderr, "RCLink: setting output_overflow\n");
    output_overflow = true;
    return false;
  }

  memcpy(sendbuf + send_amount, message, messagelen);
  send_amount += messagelen;

  updateWrite();
  return true;
}

/*
 * Use printf to send a message on the RCLink.  We send all or nothing
 * (if we run out of buffer space).
 */
bool RCLink::sendf(const char *format, ...)
{
  va_list ap;
  int messagelen;

  if (output_overflow) {
    return false;
  }

  va_start(ap, format);
  messagelen = vsnprintf(sendbuf + send_amount,
			RC_LINK_SENDBUFLEN - send_amount, format, ap);
  va_end(ap);

  if (send_amount + messagelen >= RC_LINK_SENDBUFLEN) {
    fprintf(stderr, "RCLink: setting output_overflow\n");
    output_overflow = true;
    return false;
  }

  send_amount += messagelen;

  updateWrite();
  return true;
}

/*
 * Send as much data as possible from our outgoing buffer.
 */
int RCLink::updateWrite(bool sendIdentify)
{
  char *bufptr = sendbuf;
  int prev_send_amount = send_amount;

  if (status != Connected && (sendIdentify || status != Connecting))
    return -1;

  if (output_overflow) {
    int errorlen = strlen(RC_LINK_OVERFLOW_MSG);
    if (send_amount + errorlen <= RC_LINK_SENDBUFLEN) {
      // The output buffer has some space--we can recover.
      memcpy(sendbuf + send_amount, RC_LINK_OVERFLOW_MSG, errorlen);
      send_amount += errorlen;
      output_overflow = false;
      fprintf(stderr, "RCLink: clearing output_overflow\n");
    } else {
      fprintf(stderr, "Couldn't fix overflow.  errorlen=%d, sendamount=%d\n",
	  errorlen, send_amount);
    }
  }

  while (true) {
    if (send_amount == 0) {
      break;
    }

    int nwritten = write(connfd, bufptr, send_amount);
    if (nwritten == -1 && errno == EAGAIN) {
      break;
    } else if (nwritten == -1) {
      perror("RCLink: Write failed.  Disconnecting.");
      status = SocketError;
      return -1;
    } else {
      bufptr += nwritten;
      send_amount -= nwritten;
    }
  }

  if (bufptr != sendbuf && send_amount > 0) {
    memmove(sendbuf, bufptr, send_amount);
  }

  return prev_send_amount - send_amount;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
