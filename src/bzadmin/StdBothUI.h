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

#ifndef STDBOTHUI_H
#define STDBOTHUI_H

#include "BZAdminUI.h"

using namespace std;


/** This interface is a combination of StdInUI and StdOutUI. It reads commands
    from stdin and prints the output from the server to stdout. This 
    requires polling of the stdin file descriptor, which isn't defined in 
    standard C or C++, which means that this might not work well on all 
    systems. It should work on most UNIX-like systems though. */
class StdBothUI : public BZAdminUI {
public:
  
  virtual void outputMessage(const string& msg);
  virtual bool checkCommand(string& str);

};

#endif
