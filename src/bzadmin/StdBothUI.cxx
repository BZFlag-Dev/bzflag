/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <iostream>
#include <sys/select.h>

#include "StdBothUI.h"
#include "global.h"


// add this UI to the map
UIAdder StdBothUI::uiAdder("stdboth", &StdBothUI::creator);


void StdBothUI::outputMessage(const string& msg) {
  std::cout<<msg<<endl;
}


bool StdBothUI::checkCommand(string& str) {
  static char buffer[MessageLen + 1];
  static int pos = 0;
  fd_set rfds;
  timeval tv;
  FD_ZERO(&rfds);
  FD_SET(0, &rfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  if (select(1, &rfds, NULL, NULL, &tv) > 0) {
    read(0, &buffer[pos], 1);
    if (buffer[pos] == '\n') {
      buffer[pos] = '\0';
      str = buffer;
      if (pos != 0) {
	pos = 0;
	return true;
      }
      pos = 0;
    }
    pos++;
  }
  return false;
}


BZAdminUI* StdBothUI::creator(const map<PlayerId, string>&, PlayerId) {
  return new StdBothUI();
}
