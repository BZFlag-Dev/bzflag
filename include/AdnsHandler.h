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

#ifndef __ADNS_HANDLER_H__
#define __ADNS_HANDLER_H__

// bzflag global header
#include "global.h"

#ifdef HAVE_ADNS_H

#include <adns.h>

class AdnsHandler {
 public:
  AdnsHandler(int index, struct sockaddr *clientAddr);
  ~AdnsHandler();

  // return true if host is resolved
  bool        checkDNSResolution();
  const char *getHostname();
  static void startupResolver();
 private:
  int               index;
  // peer's network hostname (malloc/free'd)
  char             *hostname;
  // adns query state for while we're looking up hostname
  adns_query        adnsQuery;
  static adns_state adnsState;
};

#endif
#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

