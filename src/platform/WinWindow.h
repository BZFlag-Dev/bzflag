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

/* WinWindow:
 *	OpenGL Windows window.
 */

#ifndef BZF_WINWINDOW_H
#define	BZF_WINWINDOW_H

#include "BzfWindow.h"
#include "WinDisplay.h"
#include <windows.h>

class WinVisual;

class WinWindow : public BzfWindow {
  public:
			WinWindow(const WinDisplay*, WinVisual*);
			~WinWindow();

    boolean		isValid() const;

    void		showWindow(boolean);

    void		getPosition(int& x, int& y);
    void		getSize(int& width, int& height) const;

    void		setTitle(const char*);
    void		setPosition(int x, int y);
    void		setSize(int width, int height);
    void		setMinSize(int width, int height);
    void		setFullscreen();

    void		warpMouse(int x, int y);
    void		getMouse(int& x, int& y) const;
    void		grabMouse();
    void		ungrabMouse();
    void		showMouse();
    void		hideMouse();

    void		makeCurrent();
    void		swapBuffers();

    // other Windows stuff
    HWND		getHandle() const;
    LONG		queryNewPalette();
    void		paletteChanged();
    static WinWindow*	lookupWindow(HWND);

  private:
    static BYTE		getComponentFromIndex(int i, UINT nbits, UINT shift);
    static void		makeColormap(const PIXELFORMATDESCRIPTOR&);

  private:
    const WinDisplay*	display;
    HWND		hwnd;
    HGLRC		hRC;
    HDC			hDC;
    WinWindow*		prev;
    WinWindow*		next;
    static WinWindow*	first;
    static HPALETTE	colormap;
};

#endif // BZF_WINWINDOW_H
