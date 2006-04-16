/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* WinDisplay:
 *	Encapsulates an Windows windows display
 */

#ifndef BZF_WINDISPLAY_H
	#define BZF_WINDISPLAY_H
	#include "common.h"
	#include "BzfDisplay.h"
	#include "bzfgl.h"

class BzfKeyEvent;
class Resolution;

class WinDisplay: public BzfDisplay
{
public:
	WinDisplay( const char *displayName, const char *videoFormat );
	~WinDisplay();

	virtual bool isValid()const;
	bool isEventPending()const;
	bool getEvent( BzfEvent & )const;
	bool hasGetKeyMode()
	{
		return true;
	};
	void getModState( bool &shift, bool &control, bool &alt );

	bool setDefaultResolution();

	bool isFullScreenOnly()const;
	int getFullWidth()const;
	int getFullHeight()const;

	bool peekEvent( BzfEvent &event )const;


	// for other Windows stuff
	class Rep
	{
public:
		Rep( const char* );
		~Rep();

		void ref();
		void unref();

private:
		static LONG WINAPI windowProc( HWND, UINT, WPARAM, LPARAM );

private:
		int refCount;

public:
		HINSTANCE hInstance;
	};
	Rep *getRep()const
	{
		return rep;
	}

private:
	WinDisplay( const WinDisplay & );
	WinDisplay &operator = ( const WinDisplay & );

	bool getKey( const MSG &, BzfKeyEvent & )const;
	bool isNastyKey( const MSG & )const;

	bool doSetResolution( int );
	ResInfo **getVideoFormats( int &num, int &current );
	static bool canChangeDepth();

	bool windowsEventToBZFEvent( MSG &msg, BzfEvent &event )const;

private:
	Rep *rep;

	// resolution info
	HWND hwnd;
	bool using3Dfx;
	int fullWidth;
	int fullHeight;
	Resolution *resolutions;

	// for key to character translations
	bool translated;
	int charCode;

	// keyboard mapping
	static const int asciiMap[];
	static const int asciiShiftMap[];
	static const int buttonMap[];
};

#endif // BZF_WINDISPLAY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
