/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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
#include "AdnsHandler.h"

// bzflag common header
#include "network.h"

#ifdef HAVE_ADNS_H
adns_state AdnsHandler::adnsState;

AdnsHandler::AdnsHandler(int _index, struct sockaddr *clientAddr)
  : index(_index), hostname(NULL), adnsQuery(NULL) {
  // launch the asynchronous query to look up this hostname
  if (adns_submit_reverse
      (adnsState, clientAddr, adns_r_ptr,
       (adns_queryflags)(adns_qf_quoteok_cname|adns_qf_cname_loose), 0,
       &adnsQuery) != 0) {
    DEBUG1("Player [%d] failed to submit reverse resolve query: errno %d\n",
	   index, getErrno());
    adnsQuery = NULL;
  } else {
    DEBUG2("Player [%d] submitted reverse resolve query\n", index);
  }
}

AdnsHandler::~AdnsHandler() {
  if (adnsQuery) {
    adns_cancel(adnsQuery);
    adnsQuery = NULL;
  }
  if (hostname) {
    free(hostname);
    hostname = NULL;
  }
}

// return true if host is resolved
bool AdnsHandler::checkDNSResolution() {
  if (!adnsQuery)
    return false;

  // check to see if query has completed
  adns_answer *answer;
  if (adns_check(adnsState, &adnsQuery, &answer, 0) != 0) {
    if (getErrno() != EAGAIN) {
      DEBUG1("Player [%d] failed to resolve: errno %d\n", index, getErrno());
      adnsQuery = NULL;
    }
    return false;
  }

  // we got our reply.
  if (answer->status != adns_s_ok) {
    DEBUG1("Player [%d] got bad status from resolver: %s\n", index,
	   adns_strerror(answer->status));
    free(answer);
    adnsQuery = NULL;
    return false;
  }

  if (hostname)
    free(hostname); // shouldn't happen, but just in case
  hostname = strdup(*answer->rrs.str);
  DEBUG1("Player [%d] resolved to hostname: %s\n", index, hostname);
  free(answer);
  adnsQuery = NULL;
  return true;
}

const char *AdnsHandler::getHostname() {
  return hostname;
}

void AdnsHandler::startupResolver() {
  /* start up our resolver if we have ADNS */
  if (adns_init(&adnsState, adns_if_nosigpipe, 0) < 0) {
    perror("ADNS init failed");
    exit(1);
  }
}

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
