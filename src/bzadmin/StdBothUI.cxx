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

#ifdef _WIN32
#pragma warning( 4: 4786)
#endif
#include <iostream>
#ifdef _WIN32
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wincon.h>
#else
#include <sys/select.h>
#endif

#include "StdBothUI.h"
#include "global.h"


// add this UI to the map
UIAdder StdBothUI::uiAdder("stdboth", &StdBothUI::creator);

StdBothUI::StdBothUI()
{
#ifdef _WIN32
  unsigned long oldMode, newMode;
  console = GetStdHandle(STD_INPUT_HANDLE);
  GetConsoleMode(console, &oldMode);
  newMode = oldMode & ~ENABLE_LINE_INPUT;
  SetConsoleMode(console, newMode);

#endif
}

void StdBothUI::outputMessage(const string& msg) {
  std::cout<<msg<<endl;
}


bool StdBothUI::checkCommand(string& str) {
  static char buffer[MessageLen + 1];
  static int pos = 0;
  bool gotChar = false;

#ifdef _WIN32
  unsigned long numRead = 0;
  INPUT_RECORD inputEvent;
  if (WaitForSingleObject(console, 1000) == WAIT_OBJECT_0) {
    PeekConsoleInput(console, &inputEvent, 1, &numRead);
    if (numRead > 0) {
      if ((inputEvent.EventType  != KEY_EVENT)
      ||  (inputEvent.Event.KeyEvent.bKeyDown == 0)
      ||  (inputEvent.Event.KeyEvent.uChar.AsciiChar == 0)) {
        ReadConsoleInput(console, &inputEvent, 1, &numRead);
      }
      else
        gotChar = 0 != ReadFile(console, &buffer[pos], 1, &numRead, NULL);
    }
  }
#else
  fd_set rfds;
  timeval tv;
  FD_ZERO(&rfds);
  FD_SET(0, &rfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  if (select(1, &rfds, NULL, NULL, &tv) > 0) {
    read(0, &buffer[pos], 1);
    gotChar = true;
  }
#endif

  if (gotChar) {
    if (buffer[pos] == '\n' || pos == MessageLen - 1) {
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

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
