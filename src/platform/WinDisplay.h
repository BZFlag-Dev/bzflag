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

/* WinDisplay:
 *	Encapsulates an Windows windows display
 */

#ifndef BZF_WINDISPLAY_H
#define	BZF_WINDISPLAY_H

#include "common.h"
#include "BzfDisplay.h"
#include "AList.h"
#include "BzfString.h"
#include <windows.h>
#include "bzfgl.h"

class BzfKeyEvent;

class WinDisplayRes {
  public:
    int			width;
    int			height;
    int			refresh;
    int			depth;
};
BZF_DEFINE_ALIST(WinDisplayResList, WinDisplayRes);

class WinDisplay : public BzfDisplay {
  public:
			WinDisplay(const char* displayName,
				const char* videoFormat);
			~WinDisplay();

    boolean		initBeforeWindow() const;

    void		initVideoFormat(HWND);

    boolean		isValid() const;
    boolean		isEventPending() const;
    boolean		getEvent(BzfEvent&) const;

    boolean		isFullScreenOnly() const;
    int			getFullWidth() const;
    int			getFullHeight() const;

    // for other Windows stuff
    class Rep {
      public:
			Rep(const char*);
			~Rep();

	void		ref();
	void		unref();

      private:
	static LONG WINAPI windowProc(HWND, UINT, WPARAM, LPARAM);

      private:
	int		refCount;

      public:
	HINSTANCE	hInstance;
    };
    Rep*		getRep() const { return rep; }

  private:
			WinDisplay(const WinDisplay&);
    WinDisplay&		operator=(const WinDisplay&);

    boolean		getKey(const MSG&, BzfKeyEvent&) const;

    boolean		doSetResolution(int);
    void		setResolutions();

    void		setVideoFormat();
    void		enumResolution(int width, int height,
				int refresh, int depth);
    static boolean	canChangeDepth();
    static BOOL CALLBACK videoFormatDialogProc(HWND hDlog, UINT iMsg,
					WPARAM wParam, LPARAM lParam);

    boolean		canChangeFormat() const;

  private:
    Rep*		rep;

    // resolution info
    boolean		init;
    HWND		hwnd;
    BzfString		configVideoFormat;
    WinDisplayResList	formats;
    bool		using3Dfx;
    int			fullWidth;
    int			fullHeight;

    // for key to character translations
    boolean		translated;
    int			charCode;

    // keyboard mapping
    static const int	asciiMap[];
    static const int	asciiShiftMap[];
    static const int	buttonMap[];

    // video format switching
    static int		videoFormat;
    static int*		videoFormatMap;
    static int		videoFormatMapSize;
    static WinDisplay*	videoFormatDisplay;
};

#endif // BZF_WINDISPLAY_H
