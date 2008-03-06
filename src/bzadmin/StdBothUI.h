/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef STDBOTHUI_H
#define STDBOTHUI_H

/* interface headers */
#include "BZAdminUI.h"
#include "UIMap.h"


class BZAdminClient;


/** This interface is a combination of StdInUI and StdOutUI. It reads commands
    from stdin and prints the output from the server to stdout. This
    requires polling of the stdin file descriptor, which isn't defined in
    standard C or C++, which means that this might not work well on all
    systems. It should work on most UNIX-like systems though. */
class StdBothUI : public BZAdminUI {
public:
  StdBothUI(BZAdminClient& c);
  virtual void outputMessage(const std::string& msg, ColorCode color);
  virtual bool checkCommand(std::string& str);

  /** This function returns a pointer to a dynamically allocated
      StdBothUI object. */
  static BZAdminUI* creator(BZAdminClient&);

 protected:

  static UIAdder uiAdder;

  bool atEOF;

#ifdef _WIN32
 public:
  HANDLE console;
  HANDLE readEvent, processedEvent;
  HANDLE thread;
  char buffer[MessageLen + 1];
  int pos;
#endif
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
