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

// Jeff Myers 10/8/97 added hooks to display setings dialog
// crs 10/26/1997 modified JM's changes

#include "WinWindow.h"
#include "OpenGLGState.h"
#include <stdio.h>
#include <math.h>

WinWindow::WinWindow(void) : BzfWindow(NULL)
{
}

WinWindow::~WinWindow()
{
}

bool			WinWindow::isValid() const
{
  return true;
}

void			WinWindow::showWindow(bool on)
{
}

void			WinWindow::getPosition(int& x, int& y)
{
}

void			WinWindow::getSize(int& width, int& height) const
{
}

void			WinWindow::setTitle(const char* title)
{
}

void			WinWindow::setPosition(int x, int y)
{
}

void			WinWindow::setSize(int width, int height)
{
}

void			WinWindow::setMinSize(int, int)
{
  // FIXME
}

void			WinWindow::setFullscreen()
{
}

void			WinWindow::iconify()
{
}

void			WinWindow::warpMouse(int x, int y)
{
}

void			WinWindow::getMouse(int& x, int& y) const
{
}

void			WinWindow::grabMouse()
{
  // FIXME
}

void			WinWindow::ungrabMouse()
{
  // FIXME
}

void			WinWindow::showMouse()
{
  // FIXME
}

void			WinWindow::hideMouse()
{
  // FIXME
}

void			WinWindow::setGamma(float newGamma)
{

}

float			WinWindow::getGamma() const
{
  return 0.0f;
}

bool			WinWindow::hasGammaControl() const
{
  return false;
}

void			WinWindow::makeCurrent()
{
}

void			WinWindow::swapBuffers()
{
}

void			WinWindow::makeContext()
{
}

void			WinWindow::freeContext()
{
}

void			WinWindow::createChild()
{
}

void			WinWindow::destroyChild()
{

}

typedef BOOL		(*GammaRamp3DFX)(HDC, LPVOID);

void			WinWindow::getGammaRamps(WORD* ramps)
{
}

void			WinWindow::setGammaRamps(const WORD* ramps)
{
}

WinWindow*		WinWindow::lookupWindow(HWND hwnd)
{
    return NULL;
}

void			WinWindow::deactivateAll()
{
}

void			WinWindow::reactivateAll()
{
}

HWND			WinWindow::getHandle()
{
  return NULL;
}

LONG			WinWindow::queryNewPalette()
{
  return TRUE;
}

void			WinWindow::paletteChanged()
{
}

bool			WinWindow::activate()
{
  return false;
}

bool			WinWindow::deactivate()
{
  return false;
}

void			WinWindow::onDestroy()
{
}

BYTE			WinWindow::getIntensityValue(float i) const
{

  return 0;
}

float			WinWindow::getComponentFromIndex(
				int i, UINT nbits, UINT shift)
{
  return 0;
}

void			WinWindow::makeColormap(
				const PIXELFORMATDESCRIPTOR& pfd)
{
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

