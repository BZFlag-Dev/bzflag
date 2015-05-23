/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
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
#include "StateDatabase.h"

WinWindow*		WinWindow::first = NULL;
HPALETTE		WinWindow::colormap = NULL;

HWND WinWindow::hwnd = NULL;

WinWindow::WinWindow(const WinDisplay* _display, WinVisual* _visual) :
				BzfWindow(_display),
				display(_display),
				visual(*_visual),
				inDestroy(false),
				hwndChild(NULL),
				hRC(NULL),
				hDC(NULL),
				hDCChild(NULL),
				inactiveDueToDeactivate(false),
				inactiveDueToDeactivateAll(false),
				useColormap(false),
				hasGamma(false),
				gammaVal(1.0f),
				prev(NULL),
				next(NULL),
				mouseGrab(false)
{
  // make window
  hwnd = CreateWindow("BZFLAG", "bzflag",
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP |
			WS_BORDER | WS_CAPTION,
			0, 0, 640, 480, NULL, NULL,
			display->getRep()->hInstance, NULL);
  if (hwnd == NULL) return;

  // create child window.  OpenGL will bind only to the child window.
  createChild();
  if (hwndChild == NULL) return;

  // get DC
  hDC = GetDC(hwnd);

  // set colormap
  if (colormap) {
    ::SelectPalette(hDC, colormap, FALSE);
    ::RealizePalette(hDC);
  }

  // other initialization
  SetMapMode(hDC, MM_TEXT);

  // add to list
  display->getRep()->ref();
  prev = NULL;
  next = first;
  if (prev) prev->next = this;
  if (next) next->prev = this;
  first = this;
}

WinWindow::~WinWindow()
{
  destroyChild();

  if (hDC != NULL)
    ReleaseDC(hwnd, hDC);
  if (hwnd != NULL)
    DestroyWindow(hwnd);

  if (prev) prev->next = next;
  if (next) next->prev = prev;
  if (first == this) first = next;

  if (first == NULL && colormap != NULL) {
    DeleteObject(colormap);
    colormap = NULL;
  }

  display->getRep()->unref();
}

bool			WinWindow::isValid() const
{
  return hwnd != NULL;
}

void			WinWindow::showWindow(bool on)
{
  ShowWindow(hwnd, on ? SW_SHOW : SW_HIDE);
}

void			WinWindow::getPosition(int& x, int& y)
{
  RECT rect;
  GetWindowRect(hwnd, &rect);
  x = (int)rect.left;
  y = (int)rect.top;
}

void			WinWindow::getSize(int& width, int& height) const
{
  RECT rect;
  GetClientRect(hwnd, &rect);
  width = (int)rect.right - (int)rect.left;
  height = (int)rect.bottom - (int)rect.top;
}

void			WinWindow::setTitle(const char* title)
{
  SetWindowText(hwnd, title);
}

void			WinWindow::setPosition(int x, int y)
{
  RECT rect;
  GetWindowRect(hwnd, &rect);
  MoveWindow(hwnd, x, y, rect.right - rect.left, rect.bottom - rect.top, FALSE);
}

void			WinWindow::setSize(int width, int height)
{
  RECT rect;
  GetWindowRect(hwnd, &rect);
  int dx = rect.right - rect.left;
  int dy = rect.bottom - rect.top;
  RECT crect;
  GetClientRect(hwnd, &crect);
  dx -= crect.right;
  dy -= crect.bottom;
  MoveWindow(hwnd, rect.left, rect.top, width + dx, height + dy, FALSE);
  MoveWindow(hwndChild, 0, 0, width, height, FALSE);
}

void			WinWindow::setMinSize(int, int)
{
  // FIXME
}

void			WinWindow::setFullscreen(bool on)
{
  if (on) {
    // no window decorations
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~(WS_BORDER | WS_CAPTION | WS_DLGFRAME | WS_THICKFRAME);
    SetWindowLong(hwnd, GWL_STYLE, style);

    // take up the whole screen
    if (display->isFullScreenOnly())
      MoveWindow(hwnd, 0, 0,
		  display->getFullWidth(),
		  display->getFullHeight(), FALSE);
    else
      MoveWindow(hwnd, 0, 0,
		  GetDeviceCaps(hDC, HORZRES),
		  GetDeviceCaps(hDC, VERTRES), FALSE);

  } else if (!display->isFullScreenOnly()) {
    // reset the resolution if we changed it before
    display->setDefaultResolution();

    // window stuff
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    style |= (WS_BORDER | WS_CAPTION);
    SetWindowLong(hwnd, GWL_STYLE, style);

    // size it
    if (BZDB.isSet("geometry")) {
      int w, h, x, y, count;
      char xs, ys;
      count = sscanf(BZDB.get("geometry").c_str(),
		 "%dx%d%c%d%c%d", &w, &h, &xs, &x, &ys, &y);
      if (w < 256) w = 256;
      if (h < 192) h = 192;
      if (count == 6) {
	if (xs == '-') x = display->getWidth() - x - w;
	if (ys == '-') y = display->getHeight() - y - h;
	setPosition(x, y);
      }
      setSize(w, h);
    } else {
      // uh.... right
      setPosition(0,0);
      setSize(640,480);
    }

    // force windows to repaint the whole desktop
    RECT rect;
    GetWindowRect(NULL, &rect);
    InvalidateRect(NULL, &rect, TRUE);
  }

  // resize child
  int width, height;
  getSize(width, height);
  MoveWindow(hwndChild, 0, 0, width, height, FALSE);

  // reset mouse grab
  if (mouseGrab) {
    ungrabMouse();
    grabMouse();
  }
}

void			WinWindow::iconify()
{
  ShowWindow(hwnd, SW_MINIMIZE);
}

void			WinWindow::warpMouse(int x, int y)
{
  POINT point;
  point.x = x;
  point.y = y;
  ClientToScreen(hwnd, &point);
  SetCursorPos((int)point.x, (int)point.y);
}

void			WinWindow::getMouse(int& x, int& y) const
{
  POINT point;
  GetCursorPos(&point);
  ScreenToClient(hwnd, &point);
  x = (int)point.x;
  y = (int)point.y;
}

void			WinWindow::grabMouse()
{
  RECT wrect;
  GetWindowRect(hwnd, &wrect);

  int xborder = GetSystemMetrics(SM_CXDLGFRAME);
  int yborder = GetSystemMetrics(SM_CYDLGFRAME);
  int titlebar = GetSystemMetrics(SM_CYCAPTION);

  DWORD style = GetWindowLong(hwnd, GWL_STYLE);
  // don't compensate for window trimmings if they're turned off
  if (!((style & (WS_BORDER | WS_CAPTION | WS_DLGFRAME))
	== (WS_BORDER | WS_CAPTION | WS_DLGFRAME)))
    xborder = yborder = titlebar = 0;

  RECT rect;
  rect.top = wrect.top + titlebar + yborder;
  rect.left = wrect.left + xborder;
  rect.bottom = wrect.bottom - yborder;
  rect.right = wrect.right - xborder;
  ClipCursor(&rect);
}

void			WinWindow::ungrabMouse()
{
  ClipCursor(NULL);
}

void			WinWindow::enableGrabMouse(bool on)
{
  if (on) {
    mouseGrab = true;
    grabMouse();
  } else {
    mouseGrab = false;
    ungrabMouse();
  }
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
  if (!useColormap && !hasGamma)
    return;

  // save gamma
  gammaVal = newGamma;

  // build gamma ramps or adjust colormap
  if (useColormap) {
    makeColormap(pfd);
    paletteChanged();
  }
  else {
    WORD map[6][256];
    for (int i = 0; i < 256; i++) {
      BYTE v = getIntensityValue((float)i / 255.0f);
      map[0][i] = map[1][i] = map[2][i] =
	map[3][i] = map[4][i] = map[5][i] = (WORD)v + ((WORD)v << 8);
    }
    setGammaRamps((const WORD*)map);
  }
}

float			WinWindow::getGamma() const
{
  return gammaVal;
}

bool			WinWindow::hasGammaControl() const
{
  return useColormap || hasGamma;
}

void			WinWindow::makeCurrent()
{
  if (hDCChild != NULL)
    wglMakeCurrent(hDCChild, hRC);
}

void			WinWindow::swapBuffers()
{
  if (hDCChild != NULL)
    SwapBuffers(hDCChild);
}

void			WinWindow::makeContext()
{
  if (!inactiveDueToDeactivate && !inactiveDueToDeactivateAll)
    if (hDCChild == NULL)
      createChild();
  makeCurrent();
}

void			WinWindow::freeContext()
{
  if (hDCChild != NULL)
    destroyChild();
}

void			WinWindow::createChild()
{
  // get parent size
  int width, height;
  getSize(width, height);

  // make window
  hwndChild = CreateWindow("BZFLAG", "opengl",
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
			WS_CHILD | WS_VISIBLE,
			0, 0, width, height, hwnd, NULL,
			display->getRep()->hInstance, NULL);
  if (hwndChild == NULL) return;

  if (display->isFullScreenOnly())
    setFullscreen(true);

  // get DC
  hDCChild = GetDC(hwndChild);

  // force visual to recalculate the PFD
  visual.reset();

  // set pixel format
  const PIXELFORMATDESCRIPTOR* tmpPFD;
  const int pixelFormat = visual.get(hDCChild, &tmpPFD);
  if (pixelFormat == 0 || !SetPixelFormat(hDCChild, pixelFormat, tmpPFD)) {
    ReleaseDC(hwndChild, hDCChild);
    DestroyWindow(hwndChild);
    hwndChild = NULL;
    hDCChild = NULL;
    return;
  }

  // get pixel format description
  DescribePixelFormat(hDCChild, pixelFormat, sizeof(pfd), &pfd);

  // make colormap
  useColormap = ((pfd.dwFlags & PFD_NEED_PALETTE) != 0);
  if (colormap == NULL && useColormap)
    makeColormap(pfd);
  if (colormap)
    ::SelectPalette(hDCChild, colormap, FALSE);

  // if no colormap then adjust gamma ramps
  if (!useColormap) {
    getGammaRamps(origGammaRamps);
    setGamma(gammaVal);
  }

  // make OpenGL context
  hRC = wglCreateContext(hDCChild);
  if (hRC == NULL) {
    ReleaseDC(hwndChild, hDCChild);
    DestroyWindow(hwnd);
    hwndChild = NULL;
    hDCChild = NULL;
    return;
  }

  if (colormap)
    ::RealizePalette(hDCChild);

  // other initialization
  SetMapMode(hDCChild, MM_TEXT);
}

void			WinWindow::destroyChild()
{
  setGammaRamps(origGammaRamps);
  if (hRC != NULL) {
    if (wglGetCurrentContext() == hRC)
      wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    hRC = NULL;
  }
  if (hDCChild != NULL) {
    ReleaseDC(hwndChild, hDCChild);
    hDCChild = NULL;
  }
  if (hwndChild != NULL) {
    DestroyWindow(hwndChild);
    hwndChild = NULL;
  }
}


void			WinWindow::getGammaRamps(WORD* ramps)
{
  if (hDCChild == NULL)
    return;

  // get device gamma ramps
  hasGamma = GetDeviceGammaRamp(hDCChild, ramps) != FALSE;
}

void			WinWindow::setGammaRamps(const WORD* ramps)
{
  if (hDCChild == NULL)
    return;

  if (hasGamma)
    SetDeviceGammaRamp(hDCChild, (LPVOID)ramps);
}

WinWindow*		WinWindow::lookupWindow(HWND hwnd)
{
  if (hwnd == NULL)
    return NULL;

  WinWindow* scan = first;
  while (scan && scan->hwnd != hwnd && scan->hwndChild != hwnd)
    scan = scan->next;
  return scan;
}

void			WinWindow::deactivateAll()
{
  for (WinWindow* scan = first; scan; scan = scan->next) {
    scan->freeContext();
    scan->inactiveDueToDeactivateAll = true;
  }
}

void			WinWindow::reactivateAll()
{
  bool anyNewChildren = false;
  for (WinWindow* scan = first; scan; scan = scan->next) {
    const bool hadChild = (scan->hDCChild != NULL);
    scan->inactiveDueToDeactivateAll = false;
    scan->makeContext();
    if (!hadChild && scan->hDCChild != NULL)
      anyNewChildren = true;
  }

  // reload context data
  if (anyNewChildren)
    OpenGLGState::initContext();
}

HWND			WinWindow::getHandle()
{
  return hwnd;
}

LONG			WinWindow::queryNewPalette()
{
  if (colormap == NULL) return FALSE;

  HPALETTE oldPalette = ::SelectPalette(hDC, colormap, FALSE);
  ::RealizePalette(hDC);
  ::SelectPalette(hDC, oldPalette, FALSE);

  if (hDCChild != NULL) {
    oldPalette = ::SelectPalette(hDCChild, colormap, FALSE);
    ::RealizePalette(hDCChild);
    ::SelectPalette(hDCChild, oldPalette, FALSE);
  }

  return TRUE;
}

void			WinWindow::paletteChanged()
{
  if (colormap == NULL) return;

  HPALETTE oldPalette = ::SelectPalette(hDC, colormap, FALSE);
  ::RealizePalette(hDC);
  ::SelectPalette(hDC, oldPalette, FALSE);

  if (hDCChild != NULL) {
    oldPalette = ::SelectPalette(hDCChild, colormap, FALSE);
    ::RealizePalette(hDCChild);
    ::SelectPalette(hDCChild, oldPalette, FALSE);
  }
}

bool			WinWindow::activate()
{
  inactiveDueToDeactivate = false;

  // restore window
  ShowWindow(hwnd, SW_RESTORE);
  SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

  // recreate OpenGL context
  const bool hadChild = (hDCChild != NULL);
  makeContext();

  if (mouseGrab)
    grabMouse();

  if (!hadChild && hDCChild != NULL) {
    // reload context data
    OpenGLGState::initContext();

    // force a redraw
    callExposeCallbacks();

    return true;
  }

  return false;
}

bool			WinWindow::deactivate()
{
  // minimize window while not active.  skip if being destroyed.
  if (!inDestroy)
    ShowWindow(hwnd, SW_MINIMIZE);

  // destroy OpenGL context
  const bool hadChild = (hDCChild != NULL);
  freeContext();

  if (mouseGrab)
    ungrabMouse();

  inactiveDueToDeactivate = true;
  return hadChild;
}

void			WinWindow::onDestroy()
{
  inDestroy = true;
}

BYTE			WinWindow::getIntensityValue(float i) const
{
  if (i <= 0.0f) return 0;
  if (i >= 1.0f) return 255;
  i = powf(i, 1.0f / gammaVal);
  return (BYTE)(0.5f + 255.0f * i);
}

float			WinWindow::getComponentFromIndex(
				int i, UINT nbits, UINT shift)
{
  const int mask = (1 << nbits) - 1;
  return (float)((i >> shift) & mask) / (float)mask;
}

void			WinWindow::makeColormap(
				const PIXELFORMATDESCRIPTOR& pfd)
{
  // compute number of colors
  const int n = 1 << pfd.cColorBits;

  // make and initialize a logical palette with room for all colors
  LOGPALETTE* logicalPalette = (LOGPALETTE*)(void*)new char[
			sizeof(LOGPALETTE) + n * sizeof(PALETTEENTRY)];
  logicalPalette->palVersion = 0x300;
  logicalPalette->palNumEntries = (WORD)n;

  // set the colors in the logical palette
  int i, j;
  for (i = 0; i < n; i++) {
    logicalPalette->palPalEntry[i].peRed = getIntensityValue(
		getComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift));
    logicalPalette->palPalEntry[i].peGreen = getIntensityValue(
		getComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift));
    logicalPalette->palPalEntry[i].peBlue = getIntensityValue(
		getComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift));
    logicalPalette->palPalEntry[i].peFlags = 0;
  }

  static const bool matchGDI = false;
  if (matchGDI) {
    // default colors reserved by system
    static const int NUM_GDI_COLORS = 20;
    static const PALETTEENTRY gdi[NUM_GDI_COLORS] = {
      { 0,   0,   0,    0 },
      { 0x80,0,   0,    0 },
      { 0,   0x80,0,    0 },
      { 0x80,0x80,0,    0 },
      { 0,   0,   0x80, 0 },
      { 0x80,0,   0x80, 0 },
      { 0,   0x80,0x80, 0 },
      { 0xC0,0xC0,0xC0, 0 },
      { 192, 220, 192,  0 },
      { 166, 202, 240,  0 },
      { 255, 251, 240,  0 },
      { 160, 160, 164,  0 },
      { 0x80,0x80,0x80, 0 },
      { 0xFF,0,   0,    0 },
      { 0,   0xFF,0,    0 },
      { 0xFF,0xFF,0,    0 },
      { 0,   0,   0xFF, 0 },
      { 0xFF,0,   0xFF, 0 },
      { 0,   0xFF,0xFF, 0 },
      { 0xFF,0xFF,0xFF, 0 }
    };

    // check for exact matches to GDI palette.
    BOOL exactMatch[NUM_GDI_COLORS];
    for (j = 0; j < NUM_GDI_COLORS; j++)
      exactMatch[j] = FALSE;
    for (i = 0; i < n; i++) {
      PALETTEENTRY* p = logicalPalette->palPalEntry + i;
      for (j = 0; j < NUM_GDI_COLORS; j++)
	if (p->peRed   == gdi[j].peRed &&
	    p->peGreen == gdi[j].peGreen &&
	    p->peBlue  == gdi[j].peBlue) {
	  exactMatch[j] = TRUE;
	  p->peFlags = 1;
	  break;
	}
    }

    // match default GDI palette by least-squares.  we'll use the peFlags
    // member of the palette entries temporarily to track which entries
    // have been matched.
    for (j = 0; j < NUM_GDI_COLORS; j++) {
      if (exactMatch[j]) continue;

      int minError = 4 * 256 * 256;		// bigger than largest error
      int bestMatch = -1;

      for (i = 0; i < n; i++) {
	const PALETTEENTRY* p = logicalPalette->palPalEntry + i;
	if (p->peFlags != 0) continue;

	int error = 0;
	error += (p->peRed   - gdi[j].peRed)   * (p->peRed   - gdi[j].peRed);
	error += (p->peGreen - gdi[j].peGreen) * (p->peGreen - gdi[j].peGreen);
	error += (p->peBlue  - gdi[j].peBlue)  * (p->peBlue  - gdi[j].peBlue);
	if (error < minError) {
	  bestMatch = i;
	  minError = error;
	}
      }

      // adjust my palette to match GDI palette and note the change
      if (bestMatch != -1) {
	logicalPalette->palPalEntry[bestMatch] = gdi[j];
	logicalPalette->palPalEntry[bestMatch].peFlags = 1;
      }
    }

    // set the peFlags the way we want
    for (i = 0; i < n; i++)
      logicalPalette->palPalEntry[i].peFlags = 0;
  }

  // create the palette
  if (colormap == NULL)
    colormap = ::CreatePalette(logicalPalette);
  else
    ::SetPaletteEntries(colormap, 0, n, logicalPalette->palPalEntry + 0);

  // free the cruft
  delete [] logicalPalette;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

