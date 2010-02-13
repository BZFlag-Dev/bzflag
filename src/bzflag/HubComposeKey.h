/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __HUB_COMPOSE_KEY_H__
#define __HUB_COMPOSE_KEY_H__

#include "common.h"

// common headers
#include "BzfEvent.h"

// local headers
#include "HUDuiDefaultKey.h"
#include "ComposeDefaultKey.h" // for MessageQueue

extern MessageQueue messageHistory;
extern unsigned int messageHistoryIndex;

class HubComposeKey : public HUDuiDefaultKey {
  public:
    HubComposeKey() : keepAlive(false) {}
  public:
    void init(bool keepAlive);
    bool keyPress(const BzfKeyEvent&);
    bool keyRelease(const BzfKeyEvent&);
  private:
    bool keepAlive;
};


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
