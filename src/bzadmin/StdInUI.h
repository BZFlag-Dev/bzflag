/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef STDINUI_H
#define STDINUI_H

#include "common.h"

/* system interface headers */
#include <string>

/* common interface headers */
#include "Address.h"
#include "BZAdminUI.h"
#include "global.h"
#include "UIMap.h"


/** This class is an interface for bzadmin that reads commands from stdin. */
class StdInUI : public BZAdminUI {
public:

  StdInUI(BZAdminClient& c);

  bool checkCommand(std::string& str);

  /** This function returns a pointer to a dynamically allocated
      StdInUI object. */
  static BZAdminUI* creator(BZAdminClient&);

protected:

  static UIAdder uiAdder;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
