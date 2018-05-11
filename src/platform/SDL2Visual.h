/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SDL2Visual:
 *	Encapsulates an SDL2 Visual
 */

#ifndef BZF_SDL2VISUAL_H
#define	BZF_SDL2VISUAL_H

// Inherits from
#include "BzfVisual.h"

// Local include
#include "SDL2Display.h"

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
  bool build() {return true;};
};

#endif // BZF_SDL2VISUAL_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
