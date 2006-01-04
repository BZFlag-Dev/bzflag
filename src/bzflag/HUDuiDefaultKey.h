/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * HUDuiDefaultKey:
 *	User interface class for the heads-up display.
 */

#ifndef	__HUDUIDEFAULTKEY_H__
#define	__HUDUIDEFAULTKEY_H__

#include "BzfEvent.h"

class HUDuiDefaultKey {
  public:
			HUDuiDefaultKey();
    virtual		~HUDuiDefaultKey();

    virtual bool	keyPress(const BzfKeyEvent&);
    virtual bool	keyRelease(const BzfKeyEvent&);
};

#endif // __HUDUIDEFAULTKEY_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
