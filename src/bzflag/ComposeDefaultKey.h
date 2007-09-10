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

#ifndef __COMPOSEDEFAULTKEY_H__
#define __COMPOSEDEFAULTKEY_H__

#include "common.h"

/* system interface headers */
#include <string>
#include <deque>

/* common interface headers */
#include "BzfEvent.h"
#include "HUDuiDefaultKey.h"


typedef std::deque<std::string> MessageQueue;

extern MessageQueue messageHistory;
extern unsigned int	messageHistoryIndex;

class ComposeDefaultKey : public HUDuiDefaultKey {
public:
  bool		keyPress(const BzfKeyEvent&);
  bool		keyRelease(const BzfKeyEvent&);
};


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
