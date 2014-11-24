/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
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
#if defined(BUILD_ARES)
#include "../src/other/ares/ares.h"
#else
#include <ares.h>
#endif

class AresHandler {
 public:
  AresHandler(int index);
  ~AresHandler();

  static bool	globalInit();
  static void	globalShutdown();

  enum ResolutionStatus {
    None = 0,
    Failed,
    HbAPending,
    HbASucceeded,
    HbNPending,
    HbNSucceeded
  };

  void		setIndex ( int i ) {index = i;}
  void		queryHostname(const struct sockaddr *clientAddr);
  void		queryHost(const char *hostName);
  const char   *getHostname();
  ResolutionStatus getHostAddress(struct in_addr *clientAddr);
  void		setFd(fd_set *read_set, fd_set *write_set, int &maxFile);
  void		process(fd_set *read_set, fd_set *write_set);
  ResolutionStatus getStatus() {return status;};
 private:
#if ARES_VERSION_MAJOR >= 1 && ARES_VERSION_MINOR >= 5
  static void	staticCallback(void *arg, int statusCallback, int timeouts,
			     struct hostent *hostent);
#else
  static void	staticCallback(void *arg, int statusCallback,
			     struct hostent *hostent);
#endif
  void		callback(int status, struct hostent *hostent);
  int		index;

  std::string	hostName;
  in_addr	hostAddress;
  ares_channel	aresChannel;
  ResolutionStatus status;
  bool		aresFailed;
  std::string		lookupAddy;

  static bool	globallyInited;

};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

