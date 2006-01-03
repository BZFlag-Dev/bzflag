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
 * HUDui:
 *	User interface class for the heads-up display and menu system.
 */

#ifndef	__HUDUI_H__
#define	__HUDUI_H__

#include "BzfEvent.h"
#include "HUDuiControl.h"
#include "HUDuiDefaultKey.h"

class HUDui {
  public:
    static HUDuiControl* getFocus();
    static void		setFocus(HUDuiControl*);

    static HUDuiDefaultKey* getDefaultKey();
    static void		setDefaultKey(HUDuiDefaultKey*);

    static bool	keyPress(const BzfKeyEvent&);
    static bool	keyRelease(const BzfKeyEvent&);

  private:
    static HUDuiControl *focus;
    static HUDuiDefaultKey* defaultKey;
};

#endif // __HUDUI_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
