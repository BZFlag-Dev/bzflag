/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "AresHandler.h"

/* system implementation headers */
#include <errno.h>
#include <string.h>

bool AresHandler::globallyInited = false;

AresHandler::AresHandler(int _index)
  : index(_index), hostname(NULL), status(None)
{
  // clear the host address
  memset(&hostAddress, 0, sizeof(hostAddress));

  /* ask for local "hosts" lookups too */
  static const char* lookups = "fb";
  struct ares_options opts;
  opts.lookups = (char*)lookups; // we cheat, libares uses strdup

  /* start up our resolver */
  if (!globallyInited) {
    aresFailed = !globalInit();
    if (aresFailed) {
      status = Failed;
      logDebugMessage(2,"Ares failed initializing library\n");
    }
  }

  /* start up our resolver */
  int code = ares_init_options (&aresChannel, &opts, ARES_OPT_LOOKUPS);
  aresFailed = (code != ARES_SUCCESS);
  if (aresFailed) {
    status = Failed;
    logDebugMessage(2,"Ares Failed initializing\n");
  }
}

AresHandler::~AresHandler()
{
  if (aresFailed)
    return;
  ares_destroy(aresChannel);
  if (hostname) {
    free(hostname);
    hostname = NULL;
  }
}

bool AresHandler::globalInit()
{
  if (!globallyInited)
  {
#ifdef HAVE_ARES_LIBRARY_INIT
    if (ares_library_init(ARES_LIB_INIT_ALL) == ARES_SUCCESS)
#endif
      globallyInited = true;
  }
  return globallyInited;
}

void AresHandler::globalShutdown()
{
#ifdef HAVE_ARES_LIBRARY_INIT
  if (globallyInited)
    ares_library_cleanup();
#endif
}


void AresHandler::queryHostname(struct sockaddr *clientAddr)
{
  if (aresFailed)
    return;
  status = HbAPending;
  // launch the asynchronous query to look up this hostname
  ares_gethostbyaddr(aresChannel, &((sockaddr_in *)clientAddr)->sin_addr,
		     sizeof(in_addr), AF_INET, staticCallback, (void *)this);
  logDebugMessage(2,"Player [%d] submitted reverse resolve query\n", index);
}

void AresHandler::queryHost(char *hostName)
{
  if (aresFailed)
    return;
  ares_cancel(aresChannel);

  if (inet_aton(hostName, &hostAddress) != 0) {
    status = HbNSucceeded;
    return;
  }

  char *queryHostName = hostName;

  char myHost[MAXHOSTNAMELEN+1];
  if (hostName == NULL || *hostName == '\0') {
    // local address
    if (gethostname(hostname, sizeof(hostname)) < 0) {
      status = Failed;
      return;
    }
    queryHostName = myHost;
  }

  // launch the asynchronous query to look up this hostname
  status = HbNPending;
  ares_gethostbyname(aresChannel, queryHostName, AF_INET, staticCallback,
		     (void *)this);
}

#if ARES_VERSION_MAJOR >= 1 && ARES_VERSION_MINOR >= 5
void AresHandler::staticCallback(void *arg, int callbackStatus,
				 int, struct hostent *hostent)
#else
void AresHandler::staticCallback(void *arg, int callbackStatus,
				 struct hostent *hostent)
#endif
{
  ((AresHandler *)arg)->callback(callbackStatus, hostent);
}

void AresHandler::callback(int callbackStatus, struct hostent *hostent)
{
  if (callbackStatus == ARES_EDESTRUCTION)
    return;
  if (callbackStatus != ARES_SUCCESS) {
      logDebugMessage(1,"Player [%d] failed to resolve: error %d\n", index,
	     callbackStatus);
      status = Failed;
  } else if (status == HbAPending) {
    if (hostname)
      free(hostname); // shouldn't happen, but just in case
    hostname = strdup(hostent->h_name);
    status = HbASucceeded;
    logDebugMessage(2,"Player [%d] resolved to %s\n", index, hostname);
  } else if (status == HbNPending) {
    memcpy(&hostAddress, hostent->h_addr_list[0], sizeof(hostAddress));
    status = HbNSucceeded;
  }
}

const char *AresHandler::getHostname()
{
  return hostname;
}

AresHandler::ResolutionStatus AresHandler::getHostAddress(struct in_addr
							  *clientAddr)
{
  if (status == HbNSucceeded)
    memcpy(clientAddr, &hostAddress, sizeof(hostAddress));
  return status;
}

void AresHandler::setFd(fd_set *read_set, fd_set *write_set, int &maxFile)
{
  if (aresFailed)
    return;
  int aresMaxFile = ares_fds(aresChannel, read_set, write_set) - 1;
  if (aresMaxFile > maxFile)
    maxFile = aresMaxFile;
}

void AresHandler::process(fd_set *read_set, fd_set *write_set)
{
  if (aresFailed)
    return;
  ares_process(aresChannel, read_set, write_set);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
