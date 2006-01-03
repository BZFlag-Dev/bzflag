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

#include "WinVisual.h"

WinVisual::WinVisual(const WinDisplay* _display) :
				display(_display->getRep()),
				pixelFormat(-1),
				hDC(NULL)
{
  display->ref();
  pfd.nSize		= sizeof(pfd);
  pfd.nVersion		= 1;
  pfd.dwFlags		= PFD_DRAW_TO_WINDOW |
			  PFD_SUPPORT_OPENGL |
			  PFD_DEPTH_DONTCARE;
  pfd.iPixelType	= PFD_TYPE_RGBA;
  pfd.cColorBits	= 0;
  pfd.cRedBits		= 0;
  pfd.cRedShift		= 0;
  pfd.cGreenBits	= 0;
  pfd.cGreenShift	= 0;
  pfd.cBlueBits		= 0;
  pfd.cBlueShift	= 0;
  pfd.cAlphaBits	= 0;
  pfd.cAlphaShift	= 0;
  pfd.cAccumBits	= 0;
  pfd.cAccumRedBits	= 0;
  pfd.cAccumGreenBits	= 0;
  pfd.cAccumBlueBits	= 0;
  pfd.cAccumAlphaBits	= 0;
  pfd.cDepthBits	= 0;
  pfd.cStencilBits	= 0;
  pfd.cAuxBuffers	= 0;
  pfd.iLayerType	= PFD_MAIN_PLANE;
  pfd.bReserved		= 0;
  pfd.dwLayerMask	= 0;
  pfd.dwVisibleMask	= 0;
  pfd.dwDamageMask	= 0;
}

WinVisual::WinVisual(const WinVisual& visual) : BzfVisual(),
				display(visual.display),
				pfd(visual.pfd),
				pixelFormat(visual.pixelFormat),
				hDC(NULL)
{
  display->ref();
}

WinVisual::~WinVisual()
{
  display->unref();
}

void			WinVisual::setLevel(int level)
{
  if (level < 0) pfd.iLayerType = (BYTE)PFD_UNDERLAY_PLANE;
  else if (level > 0) pfd.iLayerType = PFD_OVERLAY_PLANE;
  else pfd.iLayerType = PFD_MAIN_PLANE;
}

void			WinVisual::setDoubleBuffer(bool on)
{
  if (on) pfd.dwFlags |= PFD_DOUBLEBUFFER;
  else pfd.dwFlags &= ~PFD_DOUBLEBUFFER;
}

void			WinVisual::setIndex(int minDepth)
{
  pfd.iPixelType = PFD_TYPE_COLORINDEX;
  pfd.cColorBits = minDepth;
}

void			WinVisual::setRGBA(int minRed, int minGreen,
				int minBlue, int minAlpha)
{
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = minRed + minGreen + minBlue + minAlpha;
}

void			WinVisual::setDepth(int minDepth)
{
  pfd.cDepthBits = minDepth;
  if (pfd.cDepthBits == 0) pfd.dwFlags |= PFD_DEPTH_DONTCARE;
  else pfd.dwFlags &= ~PFD_DEPTH_DONTCARE;
}

void			WinVisual::setStencil(int minDepth)
{
  pfd.cStencilBits = minDepth;
}

void			WinVisual::setAccum(int minRed, int minGreen,
				int minBlue, int minAlpha)
{
  pfd.cAccumBits = minRed + minGreen + minBlue + minAlpha;
}

void			WinVisual::setStereo(bool on)
{
  if (on) pfd.dwFlags |= PFD_STEREO;
  else pfd.dwFlags &= ~PFD_STEREO;
}

void			WinVisual::setMultisample(int)
{
  // do nothing
}

bool			WinVisual::build()
{
  if (pixelFormat == -1) {
    if (hDC == NULL) {
      HWND hwnd = CreateWindow("BZFLAG", "bzflag",
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
			0, 0, 1, 1, NULL, NULL, display->hInstance, NULL);
      if (hwnd == NULL) return false;
      hDC = GetDC(hwnd);
      pixelFormat = ChoosePixelFormat(hDC, &pfd);
      ReleaseDC(hwnd, hDC);
      DestroyWindow(hwnd);
      hDC = NULL;
    }
    else {
      pixelFormat = ChoosePixelFormat(hDC, &pfd);
    }
  }
  return pixelFormat > 0;
}

void			WinVisual::reset()
{
  pixelFormat = -1;
}

int			WinVisual::get(HDC _hDC,
				const PIXELFORMATDESCRIPTOR** _pfd)
{
  hDC = _hDC;
  build();
  hDC = NULL;
  if (_pfd) *_pfd = &pfd;
  return pixelFormat;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

