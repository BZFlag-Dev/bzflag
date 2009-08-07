/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#define _WIN32_WINDOWS 0x0500

// in this file at least, we want wide-character functionality
#define UNICODE

#include "WinDisplay.h"
#include "WinWindow.h"
#include "resource.h"
#include "BzfEvent.h"
#include <stdio.h>
#include <string.h>
#include "StateDatabase.h"
#include "TextUtils.h"

class Resolution {
  public:
    int			width;
    int			height;
    int			refresh;
    int			depth;
};

static int		resolutionCompare(const void* _a, const void* _b)
{
  const Resolution* a = (const Resolution*)_a;
  const Resolution* b = (const Resolution*)_b;

  // test the stuff we actually care about
  if (a->width < b->width) return -1;
  if (a->width > b->width) return 1;
  if (a->height < b->height) return -1;
  if (a->height > b->height) return 1;
  if (a->refresh < b->refresh) return -1;
  if (a->refresh > b->refresh) return 1;
  if (a->depth < b->depth) return -1;
  if (a->depth > b->depth) return 1;

  // other info can be ordered arbitrarily
  return 0;
}

//
// WinDisplay::Rep
//

WinDisplay::Rep::Rep(const char*) : refCount(1),
				hInstance(NULL)
{
  // register our window class
  hInstance = GetModuleHandle(NULL);

  WNDCLASSEX wc;
  wc.cbSize		= sizeof(wc);
  wc.style		= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc	= (WNDPROC)windowProc;
  wc.cbClsExtra		= 0;
  wc.cbWndExtra		= 0;
  wc.hInstance		= hInstance;
  wc.hIcon		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BZICON));
  wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
  wc.lpszMenuName	= NULL;
  wc.lpszClassName	= L"BZFLAG";
  wc.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BZICON));

  if (RegisterClassEx(&wc) == 0)
    hInstance = NULL;
}

WinDisplay::Rep::~Rep()
{
  // do nothing
}

void			WinDisplay::Rep::ref()
{
  refCount++;
}

void			WinDisplay::Rep::unref()
{
  if (--refCount <= 0) delete this;
}

LONG WINAPI		WinDisplay::Rep::windowProc(HWND hwnd, UINT msg,
				WPARAM wparam, LPARAM lparam)
{
  switch(msg) {
    case WM_DESTROY: {
      WinWindow* window = WinWindow::lookupWindow(hwnd);
      if (window && window->getHandle() == hwnd) window->onDestroy();
      break;
    }

    case WM_PAINT:
      ValidateRect(hwnd, NULL);
      return 0;

    case WM_QUERYNEWPALETTE: {
      WinWindow* window = WinWindow::lookupWindow(hwnd);
      if (window) return window->queryNewPalette();
      return 0;
    }

    case WM_PALETTECHANGED:
      if ((HWND)wparam != hwnd) {
	WinWindow* window = WinWindow::lookupWindow(hwnd);
	if (window) window->paletteChanged();
      }
      return 0;

    case WM_ACTIVATE: {
      WinWindow* window = WinWindow::lookupWindow(hwnd);
      if (window) {
	if (LOWORD(wparam) == WA_INACTIVE)
	{
	  if (!window->isActivating())
	  {
	    if (window->deactivate())
	      PostMessage(hwnd, WM_APP + 1, (WPARAM)0, (LPARAM)0);
	  }
	}
	else {
	  if (!window->isActivating())
	  {
	    if (window->activate())
	      PostMessage(hwnd, WM_APP + 0, (WPARAM)0, (LPARAM)0);
	  }
	}
      }
      break;
    }

    case WM_DISPLAYCHANGE: {
      WinWindow* window = WinWindow::lookupWindow(hwnd);
      if (window && window->getHandle() == hwnd) {
	// recreate opengl context
	WinWindow::deactivateAll();
	WinWindow::reactivateAll();
      }
      break;
    }

    case WM_SYSCOMMAND:
      switch (wparam) {
	// disable screen saver and monitor power-saving mode
	case SC_SCREENSAVE:
	case SC_MONITORPOWER:
	  return 0;
      }
      break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

//
// WinDisplay
//

WinDisplay::WinDisplay(const char* displayName, const char*) :
				rep(NULL),
				hwnd(NULL),
				fullWidth(0),
				fullHeight(0),
				resolutions(NULL),
				translated(false)
{
  rep = new Rep(displayName);

  // get resolutions
  if (isValid()) {
    int numModes, currentMode;
    ResInfo** resInfo = getVideoFormats(numModes, currentMode);

    // if no modes then make default
    if (!resInfo) {
      HDC hDC = GetDC(GetDesktopWindow());
      resInfo = new ResInfo*[1];
      resInfo[0] = new ResInfo("default",
				GetDeviceCaps(hDC, HORZRES),
				GetDeviceCaps(hDC, VERTRES), 0);
      ReleaseDC(GetDesktopWindow(), hDC);
      numModes = 1;
      currentMode = 0;
    }

    // register modes
    initResolutions(resInfo, numModes, currentMode);
  }
}

WinDisplay::~WinDisplay()
{
  setDefaultResolution();
  delete[] resolutions;
  rep->unref();
}

int			WinDisplay::getFullWidth() const
{
  return fullWidth;
}

int			WinDisplay::getFullHeight() const
{
  return fullHeight;
}

bool			WinDisplay::isValid() const
{
  return rep->hInstance != NULL;
}

bool			WinDisplay::isEventPending() const
{
  MSG msg;
  return (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) != 0);
}

bool WinDisplay::windowsEventToBZFEvent ( MSG &msg, BzfEvent& event ) const
{
	event.window = WinWindow::lookupWindow(msg.hwnd);
	switch (msg.message) {
	case WM_CLOSE:
	case WM_QUIT:
	case WM_SYSCOMMAND:
		if (msg.wParam != SC_CLOSE)
			break;
		event.type = BzfEvent::Quit;
		break;

	case WM_PAINT:
		event.type = BzfEvent::Redraw;
		ValidateRect(msg.hwnd, NULL);		// no more WM_PAINTs, please!
		break;

	case WM_SIZE:
		event.type = BzfEvent::Resize;
		event.resize.width = LOWORD(msg.lParam);
		event.resize.height = HIWORD(msg.lParam);
		break;

	case WM_MOUSEMOVE:
		event.type = BzfEvent::MouseMove;
		event.mouseMove.x = LOWORD(msg.lParam);
		event.mouseMove.y = HIWORD(msg.lParam);
		break;

	case WM_APP + 0:
		event.type = BzfEvent::Map;
		break;

	case WM_APP + 1:
		event.type = BzfEvent::Unmap;
		break;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		event.type = BzfEvent::KeyDown;
		event.keyDown.chr = 0;
		event.keyDown.shift = 0;
		switch (msg.message) {
	case WM_LBUTTONDOWN:	event.keyDown.button = BzfKeyEvent::LeftMouse; break;
	case WM_MBUTTONDOWN:	event.keyDown.button = BzfKeyEvent::MiddleMouse; break;
	case WM_RBUTTONDOWN:	event.keyDown.button = BzfKeyEvent::RightMouse; break;
	default:		return false;
		}
		break;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		event.type = BzfEvent::KeyUp;
		event.keyUp.chr = 0;
		event.keyUp.shift = 0;
		switch (msg.message) {
	case WM_LBUTTONUP:	event.keyUp.button = BzfKeyEvent::LeftMouse; break;
	case WM_MBUTTONUP:	event.keyUp.button = BzfKeyEvent::MiddleMouse; break;
	case WM_RBUTTONUP:	event.keyUp.button = BzfKeyEvent::RightMouse; break;
	default:		return false;
		}
		break;

	case WM_MOUSEWHEEL:{

		event.type = BzfEvent::KeyDown;
		event.keyDown.chr = 0;
		event.keyDown.shift = 0;

		if (LOWORD(msg.wParam) == MK_SHIFT)
			event.keyDown.shift |= BzfKeyEvent::ShiftKey;
		if (LOWORD(msg.wParam) == MK_CONTROL)
			event.keyDown.shift |= BzfKeyEvent::ControlKey;
		if (LOWORD(msg.wParam) == MK_ALT)
			event.keyDown.shift |= BzfKeyEvent::AltKey;

		short field = HIWORD(msg.wParam);
		if (field > 0)
			event.keyDown.button  = BzfKeyEvent::WheelUp;
		else
			event.keyDown.button  = BzfKeyEvent::WheelDown;
		 }
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		translated = (bool)(TranslateMessage(&msg) != 0);
		if (isNastyKey(msg)) return false;
		DispatchMessage(&msg);
		event.type = BzfEvent::KeyDown;
		if (!getKey(msg, event.keyDown)) return false;
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP: {
		translated = (bool)(TranslateMessage(&msg) != 0);
		if (isNastyKey(msg)) return false;
		DispatchMessage(&msg);
		event.type = BzfEvent::KeyUp;
		if (!getKey(msg, event.keyUp)) return false;
		break;
	}

	default:
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		return false;
	}
	return true;
}

bool			WinDisplay::peekEvent(BzfEvent& event) const
{
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0,0) == -1) return false;
	return windowsEventToBZFEvent(msg,event);
}

bool			WinDisplay::getEvent(BzfEvent& event) const
{
  MSG msg;
  if (GetMessage(&msg, NULL, 0, 0) == -1) return false;
  return windowsEventToBZFEvent(msg,event);
}

void			WinDisplay::getModState(bool &shift, bool &ctrl, bool &alt) {
  shift = (GetKeyState(VK_SHIFT) < 0);
  ctrl  = (GetKeyState(VK_CONTROL) < 0);
  alt   = (GetKeyState(VK_MENU) < 0);
}

#define UNICODE_IS_HIGH_SURROGATE(ch) ((ch) >= 0xD800 && (ch) <= 0xDBFF)
#define UNICODE_IS_LOW_SURROGATE(ch) ((ch) >= 0xDC00 && (ch) <= 0xDFFF)
#define UNICODE_SURROGATE_TO_UTF32(ch, cl) (((ch) - 0xD800) * 0x400 + ((cl) - 0xDC00) + 0x10000)

bool			WinDisplay::getKey(const MSG& msg,
					BzfKeyEvent& key) const
{
  key.shift = key.chr = key.button = 0;
  if (GetKeyState(VK_SHIFT) < 0)	key.shift |= BzfKeyEvent::ShiftKey;
  if (GetKeyState(VK_CONTROL) < 0)	key.shift |= BzfKeyEvent::ControlKey;
  if (GetKeyState(VK_MENU) < 0)		key.shift |= BzfKeyEvent::AltKey;

  // We recieve key messages as follows: WM_KEYDOWN, (translated) WM_CHAR, WM_KEYUP
  // This means that in order to get a char for WM_KEYUP we must save the CHAR that follows
  // WM_KEYDOWN.  Things to remember:
  //  1: not all WM_KEYDOWNs for a particular key will generate the same WM_CHAR (` + a on 
  //	 German keyboard layouts, for instance).  Therefore we cannot remember WM_CHAR
  //	 permanently on a per-key basis.
  //  2: not all WM_KEYDOWNs for a particular key will generate a WM_CHAR AT ALL (` alone
  //     on German layouts, for instance).  Therefor we cannot generate fake WM_CHARs on
  //	 WM_KEYUPs whose preceding WM_KEYDOWN did not generate a real WM_CHAR.
  //  3: if a user holds down one key while pressing another, then lets up on the first one,
  //	 we may recieve events out of order (WM_KEYDOWN(1), WM_CHAR(1), WM_KEYDOWN(2),
  //	 WM_CHAR(2), WM_KEYUP(2), WM_KEYUP(1)).
  //  4: not all WM_KEYDOWNs are followed by a WM_KEYUP.  We get multiple WM_KEYDOWNs if
  //	 a key is held down.
  // So, with this in mind, we must store either the WM_CHAR, or the lack of one, during
  // each WM_KEYDOWN, and read it during WM_KEYUP, such that the KeyUp event gets the
  // same .chr member as the immediately preceding KeyDown event on the same key.

  key.button = buttonMap[(int)msg.wParam];

  if (translated) {
    MSG cmsg;
    if (PeekMessage(&cmsg, NULL, 0, 0, PM_NOREMOVE) &&
	(cmsg.message == WM_CHAR || cmsg.message == WM_SYSCHAR)) {
      GetMessage(&cmsg, NULL, 0, 0);
      // chr is in UTF-16, so convert it to a codepoint
      key.chr = cmsg.wParam;
      if (UNICODE_IS_HIGH_SURROGATE(key.chr >> 16) &&
          UNICODE_IS_LOW_SURROGATE(key.chr & 0xFFFF)) {
	key.chr = UNICODE_SURROGATE_TO_UTF32(key.chr >> 16, key.chr & 0xFFFF);
      }
      keyUpDownMap[key.button] = key.chr;
    } else if (keyUpDownMap.find(key.button) != keyUpDownMap.end()) {
      // perhaps we already know this character from WM_KEYDOWN?
      key.chr = keyUpDownMap[key.button];
      // each WM_KEYDOWN is only good for one WM_KEYUP (necessary for deadkeys)
      keyUpDownMap[key.button] = 0;
    }
  }

  if (key.button == BzfKeyEvent::Delete) key.chr = 0;

  return (key.chr != 0 || key.button != 0);
}

bool			WinDisplay::isNastyKey(const MSG& msg) const
{
  switch (msg.wParam) {
    case VK_LWIN:
    case VK_RWIN:
      // disable windows keys
      return true;

    case VK_ESCAPE:
      // disable ctrl+escape (which pops up the start menu)
      if (GetKeyState(VK_CONTROL) < 0)
	return true;
      break;
  }
  return false;
}

bool			WinDisplay::setDefaultResolution() const
{
  ChangeDisplaySettings(0, 0);
  return true;
}

bool			WinDisplay::doSetResolution(int index)
{
  // try setting the format
  Resolution& format = resolutions[index];
  DEVMODE dm;
  memset(&dm, 0, sizeof(dm));
  dm.dmSize	     = sizeof(dm);
  dm.dmPelsWidth	= format.width;
  dm.dmPelsHeight       = format.height;
  dm.dmBitsPerPel       = format.depth;
  dm.dmDisplayFrequency = format.refresh;
  dm.dmFields	   = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
  if (dm.dmDisplayFrequency != 0)
    dm.dmFields |= DM_DISPLAYFREQUENCY;

  // test the change before really doing it
  if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN | CDS_TEST) !=
						DISP_CHANGE_SUCCESSFUL)
    return false;

  // deactivate windows before resolution change.  if we don't do this
  // then the app will almost certainly crash in the OpenGL driver.
  WinWindow::deactivateAll();

  // change resolution
  const bool changed = (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) ==
						DISP_CHANGE_SUCCESSFUL);

  // reactivate previously deactivated window after change
  WinWindow::reactivateAll();

  return changed;
}

BzfDisplay::ResInfo**	WinDisplay::getVideoFormats(
				int& numModes, int& currentMode)
{
  HDC hDC = GetDC(GetDesktopWindow());

  // get the current display depth
  const bool changeDepth = canChangeDepth();
  int currentDepth = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

  // count the resolutions
  int i, j = 0;
  typedef struct {   //On WinXP, DEVMODE buffer overruns are occuring, if the DEVMODE data structure
					 //is not up to date. You need to compile with the new header files
	  DEVMODE dm;    //Using old headers, Buffer is 148, but the data returned is 156, causing stack corruption
	  byte fudge[30];//Throw some fudge in here so that it works even with old headers
  } SlopDEVMODE;
  SlopDEVMODE dm;
  DEVMODE *pdm = &dm.dm;

  memset( pdm, 0, sizeof(DEVMODE));
  pdm->dmSize = sizeof(DEVMODE);
  while (EnumDisplaySettings(NULL, j, pdm))
    j++;

  // allocate space for resolutions
  int numResolutions = j;
  resolutions = new Resolution[numResolutions];

  // enumerate all resolutions.  note that we might throw some
  // resolutions away, so resolutions may be bigger than necessary.
  for (i = j = 0; EnumDisplaySettings(NULL, j, pdm); j++) {
    // convert frequency of 1 to 0 (meaning the default)
    if ((pdm->dmFields & DM_DISPLAYFREQUENCY) && pdm->dmDisplayFrequency == 1)
      pdm->dmDisplayFrequency = 0;

    // ignore formats of different depth if we can't change depth
    if (!changeDepth && (int)pdm->dmBitsPerPel != currentDepth)
      continue;

    // ignore formats we know won't work
    if (pdm->dmPelsWidth < 640 || pdm->dmPelsHeight < 400 || pdm->dmBitsPerPel < 8)
      continue;

    // fill in format
    Resolution r;
    r.width = pdm->dmPelsWidth;
    r.height = pdm->dmPelsHeight;
    r.refresh = pdm->dmDisplayFrequency;
    r.depth = pdm->dmBitsPerPel;

    // do we already have an equivalent format already?
    int k;
    for (k = 0; k < i; ++k)
      if (resolutionCompare(&r, resolutions + k) == 0)
	break;
    if (k < i)
      continue;

    // add format
    resolutions[i++] = r;
  }
  numResolutions = i;

  // now sort resolutions
  qsort(resolutions, numResolutions, sizeof(resolutions[0]), resolutionCompare);

  // find current format
  int current = -1;
  const int currentWidth = GetDeviceCaps(hDC, HORZRES);
  const int currentHeight = GetDeviceCaps(hDC, VERTRES);
  for (i = numResolutions - 1; i >= 0; i--) {
    const Resolution* r = resolutions + i;
    if (r->width == currentWidth &&
	r->height == currentHeight &&
	r->depth == currentDepth) {
      // FIXME -- compare display refresh rate
      current = i;
      break;
    }
  }

  // make ResInfo list if we found the current format
  ResInfo** resInfo = NULL;
  if (current != -1) {
    char name[80];
    resInfo = new ResInfo*[numResolutions];
    for (i = 0; i < numResolutions; i++) {
      const Resolution* r = resolutions + i;
      if (r->refresh == 0)
	sprintf(name, "%dx%d %d bits", r->width, r->height, r->depth);
      else
	sprintf(name, "%dx%d @%dHz %d bits", r->width, r->height,
					r->refresh, r->depth);
      resInfo[i] = new ResInfo(name, r->width, r->height, r->refresh);
    }
  }

  // done with desktop DC
  ReleaseDC(GetDesktopWindow(), hDC);

  // return modes
  numModes = numResolutions;
  currentMode = current;
  return resInfo;
}

#define OSR2_BUILD_NUMBER 1111
bool			WinDisplay::canChangeDepth()
{
  // not all versions of windows change dynamically change bit depth.
  // NT 4.0 and 95 OSR2 can and we assume anything later then them
  // will too.
  OSVERSIONINFO vinfo;
  vinfo.dwOSVersionInfoSize = sizeof(vinfo);
  if (!GetVersionEx(&vinfo))
    return false;
  if (vinfo.dwMajorVersion > 4)
    return true;
  if (vinfo.dwMajorVersion == 4) {
    if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
      return true;
    if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
	LOWORD(vinfo.dwBuildNumber) >= OSR2_BUILD_NUMBER)
      return true;
  }
  return false;
}

const int		WinDisplay::buttonMap[] = {
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_LBUTTON
	BzfKeyEvent::NoButton,	// VK_RBUTTON
	BzfKeyEvent::NoButton,	// VK_CANCEL
	BzfKeyEvent::NoButton,	// VK_MBUTTON
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_BACK
	BzfKeyEvent::NoButton,	// VK_TAB
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_CLEAR
	BzfKeyEvent::NoButton,	// VK_RETURN
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_SHIFT
	BzfKeyEvent::NoButton,	// VK_CONTROL
	BzfKeyEvent::NoButton,	// VK_MENU
	BzfKeyEvent::Pause,	// VK_PAUSE
	BzfKeyEvent::NoButton,	// VK_CAPITAL
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_ESCAPE
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_SPACE
	BzfKeyEvent::PageUp,	// VK_PRIOR
	BzfKeyEvent::PageDown,	// VK_NEXT
	BzfKeyEvent::End,	// VK_END
	BzfKeyEvent::Home,	// VK_HOME
	BzfKeyEvent::Left,	// VK_LEFT
	BzfKeyEvent::Up,	// VK_UP
	BzfKeyEvent::Right,	// VK_RIGHT
	BzfKeyEvent::Down,	// VK_DOWN
	BzfKeyEvent::NoButton,	// VK_SELECT
	BzfKeyEvent::NoButton,	// VK_PRINT
	BzfKeyEvent::NoButton,	// VK_EXECUTE
	BzfKeyEvent::NoButton,	// VK_SNAPSHOT
	BzfKeyEvent::Insert,	// VK_INSERT
	BzfKeyEvent::Delete,	// VK_DELETE
	BzfKeyEvent::NoButton,	// VK_HELP
	BzfKeyEvent::NoButton,	// VK_0
	BzfKeyEvent::NoButton,	// VK_1
	BzfKeyEvent::NoButton,	// VK_2
	BzfKeyEvent::NoButton,	// VK_3
	BzfKeyEvent::NoButton,	// VK_4
	BzfKeyEvent::NoButton,	// VK_5
	BzfKeyEvent::NoButton,	// VK_6
	BzfKeyEvent::NoButton,	// VK_7
	BzfKeyEvent::NoButton,	// VK_8
	BzfKeyEvent::NoButton,	// VK_9
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_A
	BzfKeyEvent::NoButton,	// VK_B
	BzfKeyEvent::NoButton,	// VK_C
	BzfKeyEvent::NoButton,	// VK_D
	BzfKeyEvent::NoButton,	// VK_E
	BzfKeyEvent::NoButton,	// VK_F
	BzfKeyEvent::NoButton,	// VK_G
	BzfKeyEvent::NoButton,	// VK_H
	BzfKeyEvent::NoButton,	// VK_I
	BzfKeyEvent::NoButton,	// VK_J
	BzfKeyEvent::NoButton,	// VK_K
	BzfKeyEvent::NoButton,	// VK_L
	BzfKeyEvent::NoButton,	// VK_M
	BzfKeyEvent::NoButton,	// VK_N
	BzfKeyEvent::NoButton,	// VK_O
	BzfKeyEvent::NoButton,	// VK_P
	BzfKeyEvent::NoButton,	// VK_Q
	BzfKeyEvent::NoButton,	// VK_R
	BzfKeyEvent::NoButton,	// VK_S
	BzfKeyEvent::NoButton,	// VK_T
	BzfKeyEvent::NoButton,	// VK_U
	BzfKeyEvent::NoButton,	// VK_V
	BzfKeyEvent::NoButton,	// VK_W
	BzfKeyEvent::NoButton,	// VK_X
	BzfKeyEvent::NoButton,	// VK_Y
	BzfKeyEvent::NoButton,	// VK_Z
	BzfKeyEvent::NoButton,	// VK_LWIN
	BzfKeyEvent::NoButton,	// VK_RWIN
	BzfKeyEvent::NoButton,	// VK_APPS
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_NUMPAD0
	BzfKeyEvent::NoButton,	// VK_NUMPAD1
	BzfKeyEvent::NoButton,	// VK_NUMPAD2
	BzfKeyEvent::NoButton,	// VK_NUMPAD3
	BzfKeyEvent::NoButton,	// VK_NUMPAD4
	BzfKeyEvent::NoButton,	// VK_NUMPAD5
	BzfKeyEvent::NoButton,	// VK_NUMPAD6
	BzfKeyEvent::NoButton,	// VK_NUMPAD7
	BzfKeyEvent::NoButton,	// VK_NUMPAD8
	BzfKeyEvent::NoButton,	// VK_NUMPAD9
	BzfKeyEvent::NoButton,	// VK_MULTIPLY
	BzfKeyEvent::NoButton,	// VK_ADD
	BzfKeyEvent::NoButton,	// VK_SEPARATOR
	BzfKeyEvent::NoButton,	// VK_SUBTRACT
	BzfKeyEvent::NoButton,	// VK_DECIMAL
	BzfKeyEvent::NoButton,	// VK_DIVIDE
	BzfKeyEvent::F1,	// VK_F1
	BzfKeyEvent::F2,	// VK_F2
	BzfKeyEvent::F3,	// VK_F3
	BzfKeyEvent::F4,	// VK_F4
	BzfKeyEvent::F5,	// VK_F5
	BzfKeyEvent::F6,	// VK_F6
	BzfKeyEvent::F7,	// VK_F7
	BzfKeyEvent::F8,	// VK_F8
	BzfKeyEvent::F9,	// VK_F9
	BzfKeyEvent::F10,	// VK_F10
	BzfKeyEvent::F11,	// VK_F11
	BzfKeyEvent::F12,	// VK_F12
	BzfKeyEvent::NoButton,	// VK_F13
	BzfKeyEvent::NoButton,	// VK_F14
	BzfKeyEvent::NoButton,	// VK_F15
	BzfKeyEvent::NoButton,	// VK_F16
	BzfKeyEvent::NoButton,	// VK_F17
	BzfKeyEvent::NoButton,	// VK_F18
	BzfKeyEvent::NoButton,	// VK_F19
	BzfKeyEvent::NoButton,	// VK_F20
	BzfKeyEvent::NoButton,	// VK_F21
	BzfKeyEvent::NoButton,	// VK_F22
	BzfKeyEvent::NoButton,	// VK_F23
	BzfKeyEvent::NoButton,	// VK_F24
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_NUMLOCK
	BzfKeyEvent::NoButton,	// VK_SCROLL
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_LSHIFT
	BzfKeyEvent::NoButton,	// VK_RSHIFT
	BzfKeyEvent::NoButton,	// VK_LCONTROL
	BzfKeyEvent::NoButton,	// VK_RCONTROL
	BzfKeyEvent::NoButton,	// VK_LMENU
	BzfKeyEvent::NoButton,	// VK_RMENU
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_PROCESSKEY
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// no VK_ code
	BzfKeyEvent::NoButton,	// VK_ATTN
	BzfKeyEvent::NoButton,	// VK_CRSEL
	BzfKeyEvent::NoButton,	// VK_EXSEL
	BzfKeyEvent::NoButton,	// VK_EREOF
	BzfKeyEvent::NoButton,	// VK_PLAY
	BzfKeyEvent::NoButton,	// VK_ZOOM
	BzfKeyEvent::NoButton,	// VK_NONAME
	BzfKeyEvent::NoButton,	// VK_PA1
	BzfKeyEvent::NoButton,	// VK_OEM_CLEAR
	BzfKeyEvent::NoButton	// no VK_ code
};

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
