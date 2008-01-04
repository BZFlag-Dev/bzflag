/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BeOSVisual:
 *	Builders for BeOS visuals suitable for OpenGL contexts and windows.
 */

#ifndef BZF_BEOSVISUAL_H
#define BZF_BEOSVISUAL_H

#include "BzfVisual.h"
#include "BeOSDisplay.h"

class BeOSVisual : public BzfVisual {
public:
  BeOSVisual(const BeOSDisplay*);
//  BeOSVisual(const BeOSVisual&);
  ~BeOSVisual();

  void				setLevel(int level);
  void				setDoubleBuffer(bool);
  void				setIndex(int minDepth);
  void				setRGBA(int minRed, int minGreen,
							int minBlue, int minAlpha);
  void				setDepth(int minDepth);
  void				setStencil(int minDepth);
  void				setAccum(int minRed, int minGreen,
							int minBlue, int minAlpha);
  void				setStereo(bool);
  void				setMultisample(int minSamples);

  bool				build();

  // for other BeOS stuff

private:
  const BeOSDisplay*		display;
friend class BeOSWindow;
  uint32			color;
  uint32			depth;
  uint32			stencil;
  uint32			accum;
  uint32			flags;
  bool				doubleBuffer;
};

#endif // BZF_BEOSVISUAL_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
