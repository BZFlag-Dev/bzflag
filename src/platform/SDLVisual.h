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

/* SDLVisual:
 *	Encapsulates an SDL visual
 */

#ifndef __SDLVISUAL_H__
#define	__SDLVISUAL_H__

/* interface headers */
#include "BzfVisual.h"
#include "SDLDisplay.h"


class SDLVisual : public BzfVisual {
 public:
  SDLVisual(const SDLDisplay*) { ;};
  void setLevel(int) {;};
  void setDoubleBuffer(bool);
  void setIndex(int) {;};
  void setRGBA(int minRed, int minGreen,
	       int minBlue, int minAlpha);
  void setDepth(int minDepth);
  void setStencil(int minDepth);
  void setAccum(int, int, int, int) {;};
  void setStereo(bool);
  void setMultisample(int) {;};
  bool build() {return true;};
};

#endif // __SDLVISUAL_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
