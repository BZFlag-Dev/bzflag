/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* LinuxDisplay:
 *	An X display with XFree86 vidmode extension
 */

#ifndef BZF_LINUXDISPLAY_H
#define	BZF_LINUXDISPLAY_H

#include "XDisplay.h"
#if defined(XF86VIDMODE_EXT)
#define USE_XF86VIDMODE_EXT
#define private c_private
#include <X11/extensions/xf86vmode.h>
#undef private
#endif

class LinuxDisplayMode : public XDisplayMode {
  public:
#if defined(USE_XF86VIDMODE_EXT)
			LinuxDisplayMode();
			~LinuxDisplayMode();

    ResInfo**		init(XDisplay* owner, int& num, int& current);
    boolean		set(int);
    boolean		setDefault(int);

  private:
    boolean		doSet(int, boolean position);

  private:
    XDisplay*		display;
    int			numResolutions;
    int			lastResolution;
    XF86VidModeModeInfo** resolutions;
    int			origNumResolutions;
    XF86VidModeModeInfo** origResolutions;
#endif
};

#endif // BZF_LINUXDISPLAY_H
