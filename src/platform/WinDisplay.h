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
class Resolution;

class WinDisplay : public BzfDisplay {
  public:
			WinDisplay(const char* displayName,
				const char* videoFormat);
			~WinDisplay();

    boolean		isValid() const;
    boolean		isEventPending() const;
    boolean		getEvent(BzfEvent&) const;

    boolean		setDefaultResolution();

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
    boolean		isNastyKey(const MSG&) const;

    boolean		doSetResolution(int);
    ResInfo**		getVideoFormats(int& num, int& current);
    static boolean	canChangeDepth();

  private:
    Rep*		rep;

    // resolution info
    HWND		hwnd;
    bool		using3Dfx;
    int			fullWidth;
    int			fullHeight;
    Resolution*		resolutions;

    // for key to character translations
    boolean		translated;
    int			charCode;

    // keyboard mapping
    static const int	asciiMap[];
    static const int	asciiShiftMap[];
    static const int	buttonMap[];
};

#endif // BZF_WINDISPLAY_H
