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
#include "RCMessage.h"

RCLink::RCLink() :
	            status(Disconnected),
		    listenfd(-1),
		    connfd(-1)
{
}

RCLink::~RCLink()
{
}

/* Waits forever for data to come down on connfd. (NOTE: doesn't check listenfd,
 * so only use this if you're ignoring connects.) */
bool RCLink::waitForData()
{
  if (status != Connected)
  {
    error = "Not connected!";
    return false;
  }

  fd_set fds;
  int socks;

  FD_ZERO(&fds);
  FD_SET(connfd, &fds);

  while (true)
  {
    socks = select(connfd + 1, &fds, NULL, NULL, NULL);
    if (socks < 0)
    {
      status = SocketError;
      error = strerror(errno);
      return false;
    }
    else if (socks > 0)
      return true;
  }
}

void RCLink::startListening(int port)
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
  if (listenfd == -1)
    return;

  int reuse_addr = 1; /* Used so we can re-bind to our port
                         while a previous connection is still
                         in TIME_WAIT state. */
  setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

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
  {
    error = "Cannot accept when not listening!";
    return false;
  }
  // O_NONBLOCK is set so we'll probably return immediately.
  connfd = accept(listenfd, NULL, 0);
  if (connfd == -1)
  {
    error = strerror(errno);
    return false;
  }

  //BzfNetwork::setNonBlocking(connfd);
  int flags = fcntl(connfd, F_GETFL);
  fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

  status = Connecting;
  send_amount = 0;
  recv_amount = 0;
  input_toolong = false;

  fprintf(stderr, "RCLink: Accepted a new frontend connection.\n");

  // Now we wait for them to introduce themselves.
  return true;
}


bool RCLink::connect(const char *host, int port)
{
  status = Disconnected;
  connfd = socket(AF_INET, SOCK_STREAM, 0);
  if (connfd < 0)
  {
    error = strerror(errno);
    return false;
  }

  sockaddr_in remote;
  remote.sin_port = htons(port);
  remote.sin_family = AF_INET;

  hostent *hostdata = gethostbyname(host);
  if (hostdata == NULL)
  {
    error = hstrerror(h_errno);
    return false;
  }
  memcpy(&remote.sin_addr.s_addr, hostdata->h_addr_list[0], hostdata->h_length);

  if (::connect(connfd, (sockaddr *)&remote, sizeof(remote)) < 0)
  {
    error = strerror(errno);
    return false;
  }

  int flags = fcntl(connfd, F_GETFL);
  fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

  status = Connecting;
  send_amount = recv_amount = 0;
  input_toolong = false;

  return true;
}

bool RCLink::send(const char* message)
{
  if (output_overflow) {
    error = "Output overflow! (more data than buffer can take)";
    return false;
  }

  int messagelen = strlen(message);

  if (send_amount + messagelen > RC_LINK_SENDBUFLEN) {
    fprintf(stderr, "RCLink: setting output_overflow\n");
    error = "Output overflow! (more data than buffer can take)";
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
    error = "Output overflow! (more data than buffer can take)";
    return false;
  }

  va_start(ap, format);
  messagelen = vsnprintf(sendbuf + send_amount,
			RC_LINK_SENDBUFLEN - send_amount, format, ap);
  va_end(ap);

  if (send_amount + messagelen >= RC_LINK_SENDBUFLEN) {
    fprintf(stderr, "RCLink: setting output_overflow\n");
    error = "Output overflow! (more data than buffer can take)";
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
int RCLink::updateWrite()
{
  char *bufptr = sendbuf;
  int prev_send_amount = send_amount;

  if (status != Connected && status != Connecting)
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

/*
 * Fill up the receive buffer with any available incoming data.  Return the
 * number of bytes of data read or -1 if the connection has died.
 */
int RCLink::updateRead()
{
  int prev_recv_amount = recv_amount;

  if (status != Connected && status != Connecting) {
    return -1;
  }

  // read in as much data as possible
  while (true) {
    if (recv_amount == RC_LINK_RECVBUFLEN)
      break;

    int nread = read(connfd, recvbuf+recv_amount, RC_LINK_RECVBUFLEN-recv_amount);
    if (nread == 0) {
      fprintf(stderr, "RCLink: Agent Closed Connection\n");
      status = getDisconnectedState(); 
      return -1;
    } else if (nread == -1 && errno != EAGAIN) {
      perror("RCLink: Read failed");
      status = SocketError;
      return -1;
    } else if (nread == -1) {
      // got no data (remember, read is set to be nonblocking)
      break;
    } else {
      recv_amount += nread;
    }
  }

  return recv_amount - prev_recv_amount;
}

/*
 * Parse as many objects as possible (via parseCommand).
 * Return the number created.
 */
int RCLink::updateParse(int maxlines)
{
  int ncommands = 0;
  char *bufptr = recvbuf;
  char *newline;

  if (recv_amount == 0) {
    return 0;
  }

  while (true) {
    // Sometimes a remote agent will add unnecessary null characters after a
    // newline.  Drop them:
    while (recv_amount >= 1 && *bufptr == '\0') {
      bufptr++;
      recv_amount--;
    }
    if (recv_amount == 0) {
      break;
    }

    newline = (char *)memchr(bufptr, '\n', recv_amount);
    if (newline == NULL) {
      if (input_toolong) {
        // We're throwing out everything up to the next newline.
        recv_amount = 0;
        break;
      } else {
        // We need to read more before we can do anything.
        break;
      }
    } else {
      // We have a full input line.
      recv_amount -= newline - bufptr + 1;

      if (input_toolong) {
        input_toolong = false;
      } else {
        if (*bufptr == '\n' || (*bufptr == '\r' && *(bufptr+1) == '\n')) {
          // empty line: ignore
        } else {
          *newline = '\0';
          if (parseCommand(bufptr)) {
            ncommands++;
          }
        }
      }

      bufptr = newline + 1;

      if (maxlines == 1) {
        break;
      }
    }
  }

  if (bufptr != recvbuf && recv_amount > 0) {
    memmove(recvbuf, bufptr, recv_amount);
  }

  if (recv_amount == RC_LINK_RECVBUFLEN) {
    input_toolong = true;
    fprintf(stderr, "RCLink: Input line too long.  Discarding.\n");
    recv_amount = 0;
  }

  return ncommands;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
