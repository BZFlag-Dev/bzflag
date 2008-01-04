/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __ARES_HANDLER_H__
#define __ARES_HANDLER_H__

// bzflag global header
#include "global.h"

/* common implementation headers */
#include "network.h"

extern "C" {
#include "ares.h"
}

class AresHandler {
 public:
  AresHandler();
  ~AresHandler();

  enum ResolutionStatus {
    None = 0,
    Failed,
    HbAPending,
    HbASucceeded,
    HbNPending,
    HbNSucceeded
  };

  void		queryHostname(struct sockaddr *clientAddr);
  void		queryHost(char *hostName);
  const char   *getHostname();
  ResolutionStatus getHostAddress(struct in_addr *clientAddr);
  void		setFd(fd_set *read_set, fd_set *write_set, int &maxFile);
  void		process(fd_set *read_set, fd_set *write_set);
  ResolutionStatus getStatus() {return status;};
 private:
  static void	staticCallback(void *arg, int statusCallback,
			     struct hostent *hostent);
  void		callback(int status, struct hostent *hostent);
  // peer's network hostname (malloc/free'd)
  char	       *hostname;
  in_addr	hostAddress;
  ares_channel	aresChannel;
  ResolutionStatus status;
  bool		aresFailed;

  struct in_addr requestedAddress;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
