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

#ifndef __COMPOSEDEFAULTKEY_H__
#define __COMPOSEDEFAULTKEY_H__

#include <deque>
#include "HUDui.h"

typedef std::deque<std::string> MessageQueue;

extern MessageQueue messageHistory;
extern unsigned int	messageHistoryIndex;


void selectNextRecipient (bool forward, bool robotIn);

class ComposeDefaultKey : public HUDuiDefaultKey {
public:
  bool		keyPress(const BzfKeyEvent&);
  bool		keyRelease(const BzfKeyEvent&);
};


#endif