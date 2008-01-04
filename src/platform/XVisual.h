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

/* XVisual:
 *	Builders for X visuals suitable for OpenGL contexts and windows.
 */

#ifndef BZF_XVISUAL_H
#define	BZF_XVISUAL_H

#include "BzfVisual.h"
#include "XDisplay.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class XVisual : public BzfVisual {
  public:
			XVisual(const XDisplay*);
			~XVisual();

    void		setLevel(int level);
    void		setDoubleBuffer(bool);
    void		setIndex(int minDepth);
    void		setRGBA(int minRed, int minGreen,
				int minBlue, int minAlpha);
    void		setDepth(int minDepth);
    void		setStencil(int minDepth);
    void		setAccum(int minRed, int minGreen,
				int minBlue, int minAlpha);
    void		setStereo(bool);
    void		setMultisample(int minSamples);

    bool		build();

    // for other X stuff
    XVisualInfo*	get();

  protected:
    int			findAttribute(int attribute) const;
    void		appendAttribute(int attribute, int value);
    void		removeAttribute(int index);
    void		editAttribute(int index, int value);

    bool		matchRequirements(XVisualInfo*) const;
    static bool		visualClassIsBetter(int thisBetter, int thanThis);

  private:
    XDisplay::Rep*	display;
    bool		multisampleExt;
    int			attributes[65];
    int			attributeCount;
    XVisualInfo*	visual;
};

#endif // BZF_XVISUAL_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
