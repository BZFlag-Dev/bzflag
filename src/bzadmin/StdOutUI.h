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

#ifndef STDOUTUI_H
#define STDOUTUI_H

#include <string>

#include "Address.h"
#include "BZAdminUI.h"
#include "global.h"

using namespace std;


/** This class is an interface for bzadmin that reads commands from stdin. */
class StdOutUI : public BZAdminUI {
public:

  void outputMessage(const string& msg);

};

#endif
