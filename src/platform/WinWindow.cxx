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

// Jeff Myers 10/8/97 added hooks to display setings dialog
// crs 10/26/1997 modified JM's changes

#include "WinWindow.h"
#include "WinVisual.h"
#include <stdio.h>

WinWindow*		WinWindow::first = NULL;
HPALETTE		WinWindow::colormap = NULL;

WinWindow::WinWindow(const WinDisplay* _display, WinVisual* _visual) :
				BzfWindow(_display),
				display(_display),
				hwnd(NULL),
				hRC(NULL),
				hDC(NULL),
				prev(NULL),
				next(NULL)
{
  // let user change video format
  WinDisplay* winDisplay = (WinDisplay*)display;
  if (winDisplay->initBeforeWindow())
    winDisplay->initVideoFormat(NULL);

  // make window
  hwnd = CreateWindow("BZFLAG", "bzflag",
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
			0, 0, 640, 480, NULL, NULL,
			display->getRep()->hInstance, NULL);
  if (hwnd == NULL) return;

  // let user change video format
  if (!winDisplay->initBeforeWindow())
    winDisplay->initVideoFormat(hwnd);

  if (display->isFullScreenOnly())
    setFullscreen();

  hDC = GetDC(hwnd);

  // set pixel format
  const PIXELFORMATDESCRIPTOR* pfd;
  const int pixelFormat = _visual->get(hDC, &pfd);
  if (pixelFormat == 0 || !SetPixelFormat(hDC, pixelFormat, pfd)) {
    ReleaseDC(hwnd, hDC);
    DestroyWindow(hwnd);
    hwnd = NULL;
    hDC = NULL;
    return;
  }

  // get pixel format description
  PIXELFORMATDESCRIPTOR tmpPFD;
  DescribePixelFormat(hDC, pixelFormat, sizeof(tmpPFD), &tmpPFD);

  // make colormap
  if (colormap == NULL && (tmpPFD.dwFlags & PFD_NEED_PALETTE))
    makeColormap(tmpPFD);
  if (colormap)
    ::SelectPalette(hDC, colormap, FALSE);

  // make OpenGL context
  hRC = wglCreateContext(hDC);
  if (hRC == NULL) {
    ReleaseDC(hwnd, hDC);
    DestroyWindow(hwnd);
    hwnd = NULL;
    hDC = NULL;
    return;
  }

  if (colormap)
    ::RealizePalette(hDC);

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
  if (wglGetCurrentContext() == hRC)
    wglMakeCurrent(NULL, NULL);
  if (hRC != NULL)
    wglDeleteContext(hRC);
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

boolean			WinWindow::isValid() const
{
  return hwnd != NULL;
}

void			WinWindow::showWindow(boolean on)
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
}

void			WinWindow::setMinSize(int width, int height)
{
  // FIXME
}

void			WinWindow::setFullscreen()
{
  DWORD style = GetWindowLong(hwnd, GWL_STYLE);
  style &= ~(WS_BORDER | WS_CAPTION | WS_DLGFRAME | WS_THICKFRAME);
  SetWindowLong(hwnd, GWL_STYLE, style);
  if (display->isFullScreenOnly())
    MoveWindow(hwnd, 0, 0,
		display->getFullWidth(),
		display->getFullHeight(), FALSE);
  else
    MoveWindow(hwnd, 0, 0,
		GetDeviceCaps(hDC, HORZRES),
		GetDeviceCaps(hDC, VERTRES), FALSE);
}

void			WinWindow::warpMouse(int x, int y)
{
  SetCursorPos(x, y);
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

void			WinWindow::makeCurrent()
{
  wglMakeCurrent(hDC, hRC);
}

void			WinWindow::swapBuffers()
{
  SwapBuffers(hDC);
}

WinWindow*		WinWindow::lookupWindow(HWND hwnd)
{
  WinWindow* scan = first;
  while (scan && scan->hwnd != hwnd) scan = scan->next;
  return scan;
}

HWND			WinWindow::getHandle() const
{
  return hwnd;
}

LONG			WinWindow::queryNewPalette()
{
  if (colormap == NULL) return FALSE;

  HPALETTE oldPalette = ::SelectPalette(hDC, colormap, FALSE);
  ::RealizePalette(hDC);
  ::SelectPalette(hDC, oldPalette, FALSE);
  return TRUE;
}

void			WinWindow::paletteChanged()
{
  if (colormap == NULL) return;

  HPALETTE oldPalette = ::SelectPalette(hDC, colormap, FALSE);
  ::RealizePalette(hDC);
  ::SelectPalette(hDC, oldPalette, FALSE);
}

BYTE			WinWindow::getComponentFromIndex(
				int i, UINT nbits, UINT shift)
{
  static const BYTE wlbthreeto8[] = {
      0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
  };
  static const BYTE wlbtwoto8[] = {
    0, 0x55, 0xaa, 0xff
  };
  static const BYTE wlboneto8[] = {
      0, 255
  };

  BYTE val = (BYTE)(i >> shift);
  switch (nbits) {
    case 1:
      val &= 0x1;
      return wlboneto8[val];

    case 2:
      val &= 0x3;
      return wlbtwoto8[val];

    case 3:
      val &= 0x7;
      return wlbthreeto8[val];

    default:
      return 0;
  }
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
    logicalPalette->palPalEntry[i].peRed =
		getComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
    logicalPalette->palPalEntry[i].peGreen =
		getComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
    logicalPalette->palPalEntry[i].peBlue =
		getComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
    logicalPalette->palPalEntry[i].peFlags = 0;
  }

  static const boolean matchGDI = False;
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
  colormap = ::CreatePalette(logicalPalette);

  // free the cruft
  delete[] (void*)logicalPalette;
}
