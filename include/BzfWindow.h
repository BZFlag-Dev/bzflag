/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BzfWindow:
 *	Abstract, platform independent base for OpenGL windows.
 */

#ifndef BZF_WINDOW_H
#define	BZF_WINDOW_H

#include "common.h"
#include "AList.h"

class BzfDisplay;

class BzfWindowCB {
  public:
    void		(*cb)(void*);
    void*		data;
};
BZF_DEFINE_ALIST(BzfWindowCBAList, BzfWindowCB);

class BzfWindow {
  public:
			BzfWindow(const BzfDisplay*);
    virtual		~BzfWindow();

    const BzfDisplay*	getDisplay() const { return display; }
    virtual boolean	isValid() const = 0;

    virtual void	showWindow(boolean) = 0;

    virtual void	getPosition(int& x, int& y) = 0;
    virtual void	getSize(int& width, int& height) const = 0;

    virtual void	setTitle(const char*) = 0;
    virtual void	setPosition(int x, int y) = 0;
    virtual void	setSize(int width, int height) = 0;
    virtual void	setMinSize(int width, int height) = 0;
    virtual void	setFullscreen() = 0;

    virtual void	warpMouse(int x, int y) = 0;
    virtual void	getMouse(int& x, int& y) const = 0;
    virtual void	grabMouse() = 0;
    virtual void	ungrabMouse() = 0;
    virtual void	showMouse() = 0;
    virtual void	hideMouse() = 0;

    virtual void	setGamma(float) = 0;
    virtual float	getGamma() const = 0;
    virtual boolean	hasGammaControl() const = 0;

    virtual void	makeCurrent() = 0;
    virtual void	swapBuffers() = 0;
    virtual void	makeContext() = 0;
    virtual void	freeContext() = 0;

    void		callExposeCallbacks() const;
    void		addExposeCallback(void (*cb)(void*), void* data);
    void		removeExposeCallback(void (*cb)(void*), void* data);

    void		callResizeCallbacks() const;
    void		addResizeCallback(void (*cb)(void*), void* data);
    void		removeResizeCallback(void (*cb)(void*), void* data);

  private:
    const BzfDisplay*	display;
    BzfWindowCBAList	exposeCallbacks;
    BzfWindowCBAList	resizeCallbacks;
};

#endif // BZF_WINDOW_H
