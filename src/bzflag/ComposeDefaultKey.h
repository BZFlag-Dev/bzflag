/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
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

// Inherits from
#include "HUDuiDefaultKey.h"

/* system interface headers */
#include <string>
#include <deque>

/* common interface headers */
#include "BzfEvent.h"


typedef std::deque<std::string> MessageQueue;

extern MessageQueue messageHistory;
extern unsigned int messageHistoryIndex;

class ComposeDefaultKey final : public HUDuiDefaultKey
{
public:
    bool      keyPress(const BzfKeyEvent&) override;
    bool      keyRelease(const BzfKeyEvent&) override;
};


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
