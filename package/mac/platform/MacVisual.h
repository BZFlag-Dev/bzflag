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

/* WinVisual:
 *	Builders for Windows visuals suitable for OpenGL contexts and windows.
 */

#ifndef BZF_MACVISUAL_H
#define	BZF_MACVISUAL_H

#include "BzfVisual.h"
#include "MacVisual.h"
#include "MacDisplay.h"

#include <vector>
using std::vector;

#include <agl.h>

class MacVisual : public BzfVisual {
  public:
			MacVisual(const MacDisplay*);
			MacVisual();

    void		setLevel(int level);
    void		setDoubleBuffer(boolean);
    void		setIndex(int minDepth);
    void		setRGBA(int minRed, int minGreen,
				int minBlue, int minAlpha);
    void		setDepth(int minDepth);
    void		setStencil(int minDepth);
    void		setAccum(int minRed, int minGreen,
				int minBlue, int minAlpha);
    void		setStereo(boolean);
    void		setMultisample(int minSamples);

    boolean		     build();
    AGLPixelFormat get () const { return pixel_format; }

    void    reset () { attributes.clear(); }
    void		addAttribute1(GLint attribute);
  protected:
    int			findAttribute(GLint attribute);

    void		addAttribute2(GLint attribute, int value);
    void		removeAttribute1(GLint attribute);
    void    removeAttribute2(GLint attribute);

  private:
    const MacDisplay *display;
    // attributes we would like for our pixel format
    // last attribute must be AGL_NONE
    vector<GLint>       attributes;
    AGLPixelFormat      pixel_format;
};

#endif // BZF_MACVISUAL_H
// ex: shiftwidth=2 tabstop=8
