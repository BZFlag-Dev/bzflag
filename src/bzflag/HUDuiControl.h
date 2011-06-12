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

/*
 * HUDuiControl:
 *  User interface class and functions for the basic interactive
 *  UI control.
 */

#ifndef __HUDUICONTROL_H__
#define __HUDUICONTROL_H__

/* common header */
#include "common.h"

/* parent interface header */
#include "HUDuiElement.h"

/* system headers */
#include <string>

/* common headers */
#include "BzfEvent.h"
#include "ogl/OpenGLGState.h"
#include "common/BzTime.h"

class HUDuiControl;
class HUDNavigationQueue;

typedef void (*HUDuiCallback)(HUDuiControl*, void*);

class HUDuiControl : public HUDuiElement {
    friend class HUDui;
  public:
    HUDuiControl();
    virtual   ~HUDuiControl();

    void    setCallback(HUDuiCallback, void*);
    HUDuiCallback getCallback() const;
    void*   getUserData() const;

    bool isNested() { return nested; }
    void isNested(bool _nested);

    virtual bool isContainer() { return false; }

    HUDuiControl* getParent() { return parent; }
    void setParent(HUDuiControl* parentControl);

    HUDNavigationQueue* getNavList() { return navList; }

    bool    hasFocus() const;
    void    showFocus(bool);

    virtual void    setNavQueue(HUDNavigationQueue*);

    void    render();

    bool isAtNavQueueIndex(size_t index);

    static int  getArrow() { return arrow; }

  protected:
    virtual bool  doKeyPress(const BzfKeyEvent&);
    virtual bool  doKeyRelease(const BzfKeyEvent&);

    void    renderFocus();

    void    doCallback();

  private:
    bool    showingFocus;
    HUDNavigationQueue* navList;
  protected:
    HUDuiCallback cb;
    void*   userData;
  private:
    static OpenGLGState* gstate;
    static int    arrow;
    static int    arrowFrame;
    static BzTime lastTime;
    static int    totalCount;

    bool nested;
    HUDuiControl* parent;
};

#endif // __HUDUICONTROL_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
