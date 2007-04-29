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

/* WinWindow:
 *	OpenGL Windows window.
 */

#ifndef BZF_WINWINDOW_H
#define	BZF_WINWINDOW_H

#include "BzfWindow.h"
#include "WinDisplay.h"
#include "WinVisual.h"

class WinWindow : public BzfWindow {
  public:
			WinWindow(const WinDisplay*, WinVisual*);
			~WinWindow();

    bool		isValid() const;

    void		showWindow(bool);

    void		getPosition(int& x, int& y);
    void		getSize(int& width, int& height) const;

    void		setTitle(const char*);
    void		setPosition(int x, int y);
    void		setSize(int width, int height);
    void		setMinSize(int width, int height);
    void		setFullscreen();
    void		setFullscreen(bool on);

    void		iconify();

    void		warpMouse(int x, int y);
    void		getMouse(int& x, int& y) const;
    void		grabMouse();
    void		ungrabMouse();
    void		showMouse();
    void		hideMouse();

    void		setGamma(float);
    float		getGamma() const;
    bool		hasGammaControl() const;

    void		makeCurrent();
    void		swapBuffers();
    void		makeContext();
    void		freeContext();

    // other Windows stuff
    static HWND		getHandle();
    LONG		queryNewPalette();
    void		paletteChanged();
    bool		activate();
    bool		deactivate();
    void		onDestroy();
    static WinWindow*	lookupWindow(HWND);
    static void		deactivateAll();
    static void		reactivateAll();

  private:
    BYTE		getIntensityValue(float) const;
    static float	getComponentFromIndex(int i, UINT nbits, UINT shift);
    void		makeColormap(const PIXELFORMATDESCRIPTOR&);

    void		createChild();
    void		destroyChild();

    void		getGammaRamps(WORD*);
    void		setGammaRamps(const WORD*);

  private:
    const WinDisplay*	display;
    WinVisual		visual;
    bool		inDestroy;
    static HWND		hwnd;
    HWND		hwndChild;
    HGLRC		hRC;
    HDC			hDC;
    HDC			hDCChild;
    bool		inactiveDueToDeactivate;
    bool		inactiveDueToDeactivateAll;
    bool		useColormap;
    bool		hasGamma;
    float		gammaVal;
    WORD		origGammaRamps[6 * 256];
    PIXELFORMATDESCRIPTOR pfd;
    WinWindow*		prev;
    WinWindow*		next;
    static WinWindow*	first;
    static HPALETTE	colormap;
};

#endif // BZF_WINWINDOW_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
