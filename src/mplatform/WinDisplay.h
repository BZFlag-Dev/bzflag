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

/* WinDisplay:
 *	Encapsulates an Windows windows display
 */

#ifndef BZF_WINDISPLAY_H
#define BZF_WINDISPLAY_H

#include "BzfDisplay.h"
#include <windows.h>

class BzfKeyEvent;
class Resolution;

class WinDisplay : public BzfDisplay {
public:
	WinDisplay(const char* displayName, const char* videoFormat);
	~WinDisplay();

	bool				isValid() const;
	bool				isEventPending() const;
	bool				getEvent(BzfEvent&) const;

	// for other Windows stuff
	class Rep {
	public:
		Rep(const char*);
		~Rep();

		void			ref();
		void			unref();

	private:
		static LONG WINAPI windowProc(HWND, UINT, WPARAM, LPARAM);

	private:
		int				refCount;

	public:
		HINSTANCE		hInstance;
	};
	Rep*				getRep() const { return rep; }

private:
	WinDisplay(const WinDisplay&);
	WinDisplay&			operator=(const WinDisplay&);

	bool				getKey(const MSG&, BzfKeyEvent&) const;
	bool				isNastyKey(const MSG&) const;

	bool				doSetResolution(int);
	bool				doSetDefaultResolution();
	ResInfo**			getVideoFormats(int& num, int& current);
	static bool			canChangeDepth();

private:
	Rep*				rep;

	// resolution info
	HWND				hwnd;
	Resolution*			resolutions;

	// for key to character translations
	bool				translated;
	int					charCode;

	// keyboard mapping
	static const int	asciiMap[];
	static const int	asciiShiftMap[];
	static const int	buttonMap[];
};

#endif // BZF_WINDISPLAY_H
// ex: shiftwidth=4 tabstop=4
