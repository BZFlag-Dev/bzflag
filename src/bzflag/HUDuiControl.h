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

/*
 * HUDuiControl:
 *	User interface class and functions for the basic interactive
 *	UI control.
 */

#ifndef	__HUDUICONTROL_H__
#define	__HUDUICONTROL_H__

/* common header */
#include "common.h"

/* parent interface header */
#include "HUDuiElement.h"

/* system headers */
#include <string>

/* common headers */
#include "BzfEvent.h"
#include "OpenGLGState.h"
#include "TimeKeeper.h"

class HUDuiControl;
class HUDNavigationQueue;

typedef void		(*HUDuiCallback)(HUDuiControl*, void*);

class HUDuiControl : public HUDuiElement {
  friend class HUDui;
  public:
			HUDuiControl();
    virtual		~HUDuiControl();

    void		setCallback(HUDuiCallback, void*);
    HUDuiCallback	getCallback() const;
    void*		getUserData() const;

    bool		hasFocus() const;
    void		showFocus(bool);

    void		setNavQueue(HUDNavigationQueue*);

    void		render();

    static int  getArrow() { return arrow; }

  protected:
    virtual bool	doKeyPress(const BzfKeyEvent&);
    virtual bool	doKeyRelease(const BzfKeyEvent&);

    void		renderFocus();

    void		doCallback();

  private:
    bool		showingFocus;
    HUDNavigationQueue*	navList;
    HUDuiCallback	cb;
    void*		userData;
    static OpenGLGState* gstate;
    static int	  arrow;
    static int		arrowFrame;
    static TimeKeeper	lastTime;
    static int		totalCount;
};

#endif // __HUDUICONTROL_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
