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

/* XDisplay:
 *	Encapsulates an X windows display
 */

#ifndef BZF_XDISPLAY_H
#define	BZF_XDISPLAY_H

#include "BzfDisplay.h"
#include <X11/Xlib.h>
#include <GL/gl.h>

class BzfString;
class BzfKeyEvent;
class Resolution;

class XDisplay : public BzfDisplay {
  public:
			XDisplay(const char* displayName);
			~XDisplay();

    boolean		isValid() const;
    boolean		isEventPending() const;
    boolean		getEvent(BzfEvent&) const;

    // for other X stuff
    class Rep {
      public:
			Rep(const char*);
			~Rep();

	void		ref();
	void		unref();

	Display*	getDisplay() const { return display; }
	int		getScreen() const { return screen; }
	Window		getRootWindow() const;

      private:
	int		refCount;
	Display*	display;
	int		screen;
    };
    Rep*		getRep() const { return rep; }

  private:
			XDisplay(const XDisplay&);
    XDisplay&		operator=(const XDisplay&);

    void		setResolutions();
    boolean		doSetResolutions();
    void		freeResolution();
    boolean		doSetResolution(int);

    boolean		getKey(const XEvent&, BzfKeyEvent&) const;

  private:
    Rep*		rep;

    // resolution stuff
    int			numResolutions;
    Resolution**	resolutions;
    int			defaultChannel;
    int			numVideoChannels;
    int			numVideoCombos;
    int*		numVideoFormats;
    Resolution***	videoFormats;
    Resolution**	videoCombos;
    int*		defaultVideoFormats;
    char*		defaultVideoCombo;
};

#endif // BZF_XDISPLAY_H
