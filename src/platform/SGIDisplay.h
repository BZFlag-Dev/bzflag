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

/* SGIDisplayMode:
 *	SGI video mode switching extension
 */

#ifndef BZF_SGIDISPLAY_H
#define	BZF_SGIDISPLAY_H

#include "XDisplay.h"
#if defined(__sgi) && !defined(NO_XSGIVC)
#define USE_XSGIVC_EXT
#include <X11/extensions/XSGIvc.h>
#if defined(X_SGIvcListVideoFormatCombinations)
#define XSGIvcListVideoFormatsCombinations XSGIvcListVideoFormatCombinations
#endif
#endif

class Resolution;

class SGIDisplayMode : public XDisplayMode {
  public:
#if defined(USE_XSGIVC_EXT)
			SGIDisplayMode();
			~SGIDisplayMode();

    ResInfo**		init(XDisplay* owner, int& num, int& current);
    boolean		set(int);

  private:
    XDisplay*		display;
    int			numResolutions;
    int			lastResolution;
    Resolution**	resolutions;
    int			defaultChannel;
    int			numVideoChannels;
    int			numVideoCombos;
    int*		numVideoFormats;
    Resolution***	videoFormats;
    Resolution**	videoCombos;
    int*		defaultVideoFormats;
    char*		defaultVideoCombo;
#endif
};

#endif // BZF_SGIDISPLAY_H
// ex: shiftwidth=2 tabstop=8
