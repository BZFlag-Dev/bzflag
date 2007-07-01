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

#include "RCLinkBackend.h"

RCLinkBackend::RCLinkBackend(int _port) :RCLink(),
		    requests(NULL)
{
  port = _port;
  startListening();
}

/*
 * Fill up the receive buffer with any available incoming data.  Return the
 * number of bytes of data read or -1 if the connection has died.
 */
int RCLinkBackend::update_read()
{
  int prev_recv_amount = recv_amount;

  if (status != Connected && status != Connecting) {
    return -1;
  }

  // read in as much data as possible
  while (true) {
    if (recv_amount == RC_LINK_RECVBUFLEN) {
      break;
    }

    int nread = read(connfd, recvbuf+recv_amount, RC_LINK_RECVBUFLEN-recv_amount);
    if (nread == 0) {
      fprintf(stderr, "RCLink: Agent Closed Connection\n");
      status = Listening;
      return -1;
    } else if (nread == -1 && errno != EAGAIN) {
      perror("RCLink: Read failed.");
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
 * Create as many RCRequest objects as possible (via parsecommand).
 * Return the number created.
 */
int RCLinkBackend::update_parse(int maxlines)
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
	  if (parsecommand(bufptr)) {
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

/*
 * Check for activity.  If possible, fill up the recvbuf with incoming data
 * and build up RCRequest objects as appropriate.
 */
void RCLinkBackend::update()
{
  if (status != Connected && status != Connecting) {
    return;
  }

  update_write();
  int amount = update_read();

  if (amount == -1) {
    status = Listening;
    return;
  }

  if (status == Connected) {
    update_parse();
  } else if (status == Connecting) {
    int ncommands = update_parse(1);
    if (ncommands) {
      RCRequest *req = poprequest();
      if (req && req->getRequestType() == HelloRequest) {
	status = Connected;
      } else {
	fprintf(stderr, "RCLink: Expected a Hello.\n");
	write(connfd, RC_LINK_NOHELLO_MSG, strlen(RC_LINK_NOHELLO_MSG));
	close(connfd);
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
bool RCLinkBackend::parsecommand(char *cmdline)
{
  RCRequest *req;
  int argc;
  char *argv[RC_LINK_MAXARGS];
  char *s, *tkn;

  s = cmdline;
  for (argc=0; argc<RC_LINK_MAXARGS; argc++) {
    tkn = strtok(s, " \r\t");
    s = NULL;
    argv[argc] = tkn;
    if (tkn == NULL || *tkn == '\0') break;
  }

  req = new RCRequest(argc, argv);
  if (req->getRequestType() == InvalidRequest) {
    fprintf(stderr, "RCLink: Invalid request: '%s'\n", argv[0]);
    sendf("error Invalid request %s\n", argv[0]);
    delete req;
    return false;
  } else {
    if (requests == NULL) {
      requests = req;
    } else {
      requests->append(req);
    }
    return true;
  }
}

RCRequest* RCLinkBackend::poprequest()
{
  RCRequest *req = requests;
  if (req != NULL) {
    requests = req->getNext();
  }
  return req;
}
RCRequest* RCLinkBackend::peekrequest()
{
  return requests;
}

