/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

    boolean		build();

    // for other X stuff
    XVisualInfo*	get();

  protected:
    int			findAttribute(int attribute) const;
    void		appendAttribute(int attribute, int value);
    void		removeAttribute(int index);
    void		editAttribute(int index, int value);

    boolean		matchRequirements(XVisualInfo*) const;
    static boolean	visualClassIsBetter(int thisBetter, int thanThis);

  private:
    XDisplay::Rep*	display;
    boolean		multisampleExt;
    int			attributes[65];
    int			attributeCount;
    XVisualInfo*	visual;
};

#endif // BZF_XVISUAL_H
// ex: shiftwidth=2 tabstop=8
