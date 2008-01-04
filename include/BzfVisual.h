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

/* BzfVisual:
 *	Abstract base for visuals (pixel format) suitable
 *	for OpenGL contexts and windows.
 */

#ifndef BZF_VISUAL_H
#define	BZF_VISUAL_H

#include "common.h"

class BzfVisual {
  public:
			BzfVisual();
    virtual		~BzfVisual();

    virtual void	setLevel(int level) = 0;
    virtual void	setDoubleBuffer(bool) = 0;
    virtual void	setIndex(int minDepth) = 0;
    virtual void	setRGBA(int minRed, int minGreen,
				int minBlue, int minAlpha) = 0;
    virtual void	setDepth(int minDepth) = 0;
    virtual void	setStencil(int minDepth) = 0;
    virtual void	setAccum(int minRed, int minGreen,
				int minBlue, int minAlpha) = 0;
    virtual void	setStereo(bool) = 0;
    virtual void	setMultisample(int minSamples) = 0;

    virtual bool	build() = 0;

  private:
			BzfVisual(const BzfVisual&);
    BzfVisual&		operator=(const BzfVisual&);
};

#endif // BZF_VISUAL_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
