/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "AccessControlList.h"

bool AccessControlList::convert(char *ip, in_addr &mask) {
  unsigned char b[4];
  char *pPeriod;

  for (int i = 0; i < 3; i++) {
    pPeriod = strchr(ip, '.');
    if (pPeriod) {
      *pPeriod = 0;
      if (strcmp("*", ip) == 0)
	b[i] = 255;
      else
	b[i] = atoi(ip);
      *pPeriod = '.';
      ip = pPeriod + 1;
    }
    else
      return false;
  }
  if (strcmp("*", ip) == 0)
    b[3] = 255;
  else
    b[3] = atoi(ip);

  mask.s_addr= htonl(((unsigned int)b[0] << 24) |
		     ((unsigned int)b[1] << 16) | ((unsigned int)b[2] << 8) | (unsigned int)b[3]);
  return true;
}

void AccessControlList::expire() {
  TimeKeeper now = TimeKeeper::getCurrent();
  for (banList_t::iterator it = banList.begin(); it != banList.end();) {
    if (it->banEnd <= now) {
      it = banList.erase(it);
    } else {
      ++it;
    }
  }
  for (hostBanList_t::iterator it2 = hostBanList.begin(); it2 != hostBanList.end();) {
    if (it2->banEnd <= now) {
      it2 = hostBanList.erase(it2);
    } else {
      ++it2;
    }
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

