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

RCLink::RCLink(int _port) :
		      status(Disconnected),
		      listenfd(-1),
		      connfd(-1),
		      requests(NULL)
{
  port = _port;
  startListening();
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

void RCLink::tryAccept()
{
  if (status != Listening) return;

  // O_NONBLOCK is set so we'll probably return immediately.
  connfd = accept(listenfd, NULL, 0);
  if (connfd == -1) return;

  //BzfNetwork::setNonBlocking(connfd);
  int flags = fcntl(connfd, F_GETFL);
  fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

  status = Connecting;
  write(connfd, RC_LINK_HELLO_STR, strlen(RC_LINK_HELLO_STR));
  send_amount = 0;
  recv_amount = 0;
  input_toolong = false;

  // Now we wait for them to introduce themselves.
}

bool RCLink::respond(char* message)
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

  update_write();
  return true;
}

/*
 * Use printf to send a message on the RCLink.  We send all or nothing
 * (if we run out of buffer space).
 */
bool RCLink::respondf(const char *format, ...)
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

  update_write();
  return true;
}

/*
 * Send as much data as possible from our outgoing buffer.
 */
int RCLink::update_write()
{
  char *bufptr = sendbuf;
  int prev_send_amount = send_amount;

  if (status != Connected && status != Connecting) {
    return -1;
  }

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
int RCLink::update_read()
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
int RCLink::update_parse(int maxlines)
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
void RCLink::update()
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
      if (req && req->get_request_type() == HelloRequest) {
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
bool RCLink::parsecommand(char *cmdline)
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
  if (req->get_request_type() == InvalidRequest) {
    fprintf(stderr, "RCLink: Invalid request: '%s'\n", argv[0]);
    respondf("error Invalid request %s\n", argv[0]);
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

RCRequest* RCLink::poprequest()
{
  RCRequest *req = requests;
  if (req != NULL) {
    requests = req->getnext();
  }
  return req;
}
RCRequest* RCLink::peekrequest()
{
  return requests;
}

RCRequest::RCRequest() :
			    fail(false),
			    failstr(NULL),
			    request_type(InvalidRequest),
			    next(NULL)
{
  request_type = InvalidRequest;
}

RCRequest::RCRequest(agent_req_t reqtype) :
			    fail(false),
			    failstr(NULL),
			    request_type(reqtype),
			    next(NULL)
{
}

RCRequest::RCRequest(int argc, char **argv) :
			    fail(false),
			    failstr(NULL),
			    next(NULL)
{
  char *endptr;
  // TODO: give a better error message if argc is wrong.
  if (strcasecmp(argv[0], "agent") == 0 && argc == 2) {
    request_type = HelloRequest;
  } else if (strcasecmp(argv[0], "execute") == 0 && argc == 2) {
    request_type = execute;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "setSpeed") == 0 && argc == 3) {
    request_type = setSpeed;
    set_robotindex(argv[1]);

    speed = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for desired speed.";
    }

    speed = clamp(speed, 0.0f, 1.0f);
  } else if (strcasecmp(argv[0], "setTurnRate") == 0 && argc == 3) {
    request_type = setTurnRate;
    set_robotindex(argv[1]);

    turnRate = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for angular velocity.";
    }

    turnRate = clamp(turnRate, 0.0f, 1.0f);
  } else if (strcasecmp(argv[0], "setAhead") == 0 && argc == 3) {
    request_type = setAhead;
    set_robotindex(argv[1]);

    distance = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for distance.";
    }
  } else if (strcasecmp(argv[0], "setTurnLeft") == 0 && argc == 3) {
    request_type = setTurnLeft;
    set_robotindex(argv[1]);

    turn = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for turn angle.";
    }
  } else if (strcasecmp(argv[0], "setFire") == 0 && argc == 2) {
    request_type = setFire;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "getDistanceRemaining") == 0 && argc == 2) {
    request_type = getDistanceRemaining;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "getTurnRemaining") == 0 && argc == 2) {
    request_type = getTurnRemaining;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "getTickDuration") == 0 && argc == 2) {
    request_type = getTickDuration;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "setTickDuration") == 0 && argc == 3) {
    request_type = setTickDuration;
    set_robotindex(argv[1]);

    duration = strtof(argv[2], &endptr);
    if (endptr == argv[2]) {
      fail = true;
      failstr = "Invalid parameter for setTickDuration.";
    }
  } else if (strcasecmp(argv[0], "getTickRemaining") == 0 && argc == 2) {
    request_type = getTickRemaining;
    set_robotindex(argv[1]);
  } else if (strcasecmp(argv[0], "teams") == 0 && argc == 1) {
    request_type = TeamListRequest;
  } else if (strcasecmp(argv[0], "bases") == 0 && argc == 1) {
    request_type = BasesListRequest;
  } else if (strcasecmp(argv[0], "obstacles") == 0 && argc == 1) {
    request_type = ObstacleListRequest;
  } else if (strcasecmp(argv[0], "flags") == 0 && argc == 1) {
    request_type = FlagListRequest;
  } else if (strcasecmp(argv[0], "shots") == 0 && argc == 1) {
    request_type = ShotListRequest;
  } else if (strcasecmp(argv[0], "mytanks") == 0 && argc == 1) {
    request_type = MyTankListRequest;
  } else if (strcasecmp(argv[0], "othertanks") == 0 && argc == 1) {
    request_type = OtherTankListRequest;
  } else if (strcasecmp(argv[0], "constants") == 0 && argc == 1) {
    request_type = ConstListRequest;
  } else {
    request_type = InvalidRequest;
  }
}

void RCRequest::sendack(RCLink *link)
{
  float elapsed = TimeKeeper::getCurrent() - TimeKeeper::getStartTime();

  switch (request_type) {
    case execute:
      link->respondf("ack %f execute %d\n", elapsed, get_robotindex());
      break;
    case setAhead:
      link->respondf("ack %f setAhead %d %f\n", elapsed, get_robotindex(), distance);
      break;
    case setTurnLeft:
      link->respondf("ack %f setTurnLeft %d %f\n", elapsed, get_robotindex(), turn);
      break;
    case setSpeed:
      link->respondf("ack %f setSpeed %d %f\n", elapsed, get_robotindex(), speed);
      break;
    case setTurnRate:
      link->respondf("ack %f setTurnRate %d %f\n", elapsed, get_robotindex(), turnRate);
      break;
    case setFire:
      link->respondf("ack %f setFire %d\n", elapsed, get_robotindex());
      break;
    case getDistanceRemaining:
      link->respondf("ack %f getDistanceRemaining %d\n", elapsed, get_robotindex());
      break;
    case getTurnRemaining:
      link->respondf("ack %f getTurnRemaining %d\n", elapsed, get_robotindex());
      break;
    case getTickRemaining:
      link->respondf("ack %f getTickRemaining %d\n", elapsed, get_robotindex());
      break;
    case getTickDuration:
      link->respondf("ack %f getTickDuration %d\n", elapsed, get_robotindex());
      break;
    case setTickDuration:
      link->respondf("ack %f setTickDuration %d %f\n", elapsed, get_robotindex(), duration);
      break;
    case TeamListRequest:
      link->respondf("ack %f teams\n", elapsed);
      break;
    case BasesListRequest:
      link->respondf("ack %f bases\n", elapsed);
      break;
    case ObstacleListRequest:
      link->respondf("ack %f obstacles\n", elapsed);
      break;
    case FlagListRequest:
      link->respondf("ack %f flags\n", elapsed);
      break;
    case ShotListRequest:
      link->respondf("ack %f shots\n", elapsed);
      break;
    case MyTankListRequest:
      link->respondf("ack %f mytanks\n", elapsed);
      break;
    case OtherTankListRequest:
      link->respondf("ack %f othertanks\n", elapsed);
      break;
    case ConstListRequest:
      link->respondf("ack %f constants\n", elapsed);
      break;
    default:
      link->respondf("ack %f\n", elapsed);
  }
}

void RCRequest::sendfail(RCLink *link)
{
  if (fail) {
    if (failstr) {
      link->respondf("fail %s\n", failstr);
    } else {
      link->respond("fail\n");
    }
  }
}

int RCRequest::get_robotindex()
{
  return robotindex;
}

void RCRequest::set_robotindex(char *arg)
{
  char *endptr;
  robotindex = strtol(arg, &endptr, 0);
  if (endptr == arg) {
    robotindex = -1;
    fail = true;
    failstr = "Invalid parameter for tank.";
  }
  else if (robotindex >= numRobots) {
    robotindex = -1;
  }
}

// Mad cred to _neon_/#scene.no and runehol/#scene.no for these two sentences:
//  * If val is nan, the result is undefined
//  * If high < low, the result is undefined
template <class T>
T RCRequest::clamp(T val, T min, T max)
{
  if (val > max)
    return max;
  if (val < min)
    return min;
  return val;
}

agent_req_t RCRequest::get_request_type()
{
  return request_type;
}

RCRequest *RCRequest::getnext()
{
  return next;
}

void RCRequest::append(RCRequest *newreq)
{
  if (next == NULL) {
    next = newreq;
  } else {
    next->append(newreq);
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
