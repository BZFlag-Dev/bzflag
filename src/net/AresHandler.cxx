/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "AresHandler.h"

/* system implementation headers */
#include <errno.h>

/* common implementation headers */
#include "network.h"

AresHandler::AresHandler(int _index)
  : index(_index), hostname(NULL), status(None)
{
  memset(&hostAddress, 0, sizeof(hostAddress));
  /* start up our resolver */
  if (ares_init(&aresChannel) != ARES_SUCCESS) {
    perror("ARES init failed");
    exit(1);
  }
}

AresHandler::~AresHandler()
{
  ares_destroy(aresChannel);
  if (hostname) {
    free(hostname);
    hostname = NULL;
  }
}

void AresHandler::queryHostname(struct sockaddr *clientAddr)
{
  status = HbAPending;
  // launch the asynchronous query to look up this hostname
  ares_gethostbyaddr(aresChannel, &((sockaddr_in *)clientAddr)->sin_addr,
		     sizeof(in_addr), AF_INET, staticCallback, (void *)this);
  DEBUG2("Player [%d] submitted reverse resolve query\n", index);
}

void AresHandler::queryHost(char *hostName)
{
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
  ares_gethostbyname(aresChannel, queryHostName, AF_INET, staticCallback,
		     (void *)this);
  status = HbNPending;
}

void AresHandler::staticCallback(void *arg, int callbackStatus,
				 struct hostent *hostent)
{
  ((AresHandler *)arg)->callback(callbackStatus, hostent);
}

void AresHandler::callback(int callbackStatus, struct hostent *hostent)
{
  if (callbackStatus == ARES_EDESTRUCTION)
    return;
  if (callbackStatus != ARES_SUCCESS) {
      DEBUG1("Player [%d] failed to resolve: error %d\n", index,
	     callbackStatus);
      status = Failed;
  } else if (status == HbAPending) {
    if (hostname)
      free(hostname); // shouldn't happen, but just in case
    hostname = strdup(hostent->h_name);
    status = HbASucceeded;
    DEBUG2("Player [%d] resolved to %s\n", index, hostname);
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
  int aresMaxFile = ares_fds(aresChannel, read_set, write_set) - 1;
  if (aresMaxFile > maxFile)
    maxFile = aresMaxFile;
}

void AresHandler::process(fd_set *read_set, fd_set *write_set)
{
  ares_process(aresChannel, read_set, write_set);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
