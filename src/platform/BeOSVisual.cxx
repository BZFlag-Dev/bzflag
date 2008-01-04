/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "BeOSVisual.h"

BeOSVisual::BeOSVisual(const BeOSDisplay* _display) :
								display(_display),
								color(0),
								depth(0),
								stencil(0),
								accum(0),
								flags(0),
								doubleBuffer(false)
{
}

BeOSVisual::~BeOSVisual()
{
}

void					BeOSVisual::setLevel(int level)
{
/*
  if (level < 0) pfd.iLayerType = PFD_UNDERLAY_PLANE;
  else if (level > 0) pfd.iLayerType = PFD_OVERLAY_PLANE;
  else pfd.iLayerType = PFD_MAIN_PLANE;
*/
}

void					BeOSVisual::setDoubleBuffer(bool on)
{
  doubleBuffer = on;
}

void					BeOSVisual::setIndex(int minDepth)
{
// ?
//  pfd.iPixelType = PFD_TYPE_COLORINDEX;
//  pfd.cColorBits = minDepth;
}

void					BeOSVisual::setRGBA(int minRed, int minGreen,
								int minBlue, int minAlpha)
{
// ?
//  pfd.iPixelType = PFD_TYPE_RGBA;
//  pfd.cColorBits = minRed + minGreen + minBlue + minAlpha;
}

void					BeOSVisual::setDepth(int minDepth)
{
  depth = minDepth;
}

void					BeOSVisual::setStencil(int minDepth)
{
  stencil = minDepth;
}

void					BeOSVisual::setAccum(int minRed, int minGreen,
								int minBlue, int minAlpha)
{
  accum = minRed + minGreen + minBlue + minAlpha;
}

void					BeOSVisual::setStereo(bool on)
{
  // do nothing
}

void					BeOSVisual::setMultisample(int)
{
  // do nothing
}

bool					BeOSVisual::build()
{
  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
