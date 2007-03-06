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

WinVisual::WinVisual(const WinDisplay* _display) 
{
  
}

WinVisual::WinVisual(const WinVisual& visual) : BzfVisual()
{
}

WinVisual::~WinVisual()
{
 
}

void			WinVisual::setLevel(int level)
{
}

void			WinVisual::setDoubleBuffer(bool on)
{
}

void			WinVisual::setIndex(int minDepth)
{
}

void			WinVisual::setRGBA(int minRed, int minGreen,
				int minBlue, int minAlpha)
{
}

void			WinVisual::setDepth(int minDepth)
{
}

void			WinVisual::setStencil(int minDepth)
{
}

void			WinVisual::setAccum(int minRed, int minGreen,
				int minBlue, int minAlpha)
{
}

void			WinVisual::setStereo(bool on)
{
}

void			WinVisual::setMultisample(int)
{
  // do nothing
}

bool			WinVisual::build()
{
  return true;
}

void			WinVisual::reset()
{
}

int			WinVisual::get(HDC _hDC,
				const PIXELFORMATDESCRIPTOR** _pfd)
{
  return 24;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

