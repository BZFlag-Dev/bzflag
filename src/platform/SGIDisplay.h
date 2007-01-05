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
    bool		set(int);

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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

