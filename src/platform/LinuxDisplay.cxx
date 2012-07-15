/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "LinuxDisplay.h"
#include "XWindow.h"

#if defined(USE_XF86VIDMODE_EXT)
#include <stdio.h>

// evaluates to the (approximate) vertical retrace speed of modeinfo _r
#define getRetrace(_r)	((int)(0.5f + (1000.0f * (_r)->dotclock) / \
					((_r)->htotal * (_r)->vtotal)))

static int		resolutionCompare(const void* _a, const void* _b)
{
  const XF86VidModeModeInfo* a = *((const XF86VidModeModeInfo**)_a);
  const XF86VidModeModeInfo* b = *((const XF86VidModeModeInfo**)_b);

  // test the stuff we actually care about
  if (a->hdisplay < b->hdisplay) return -1;
  if (a->hdisplay > b->hdisplay) return 1;
  if (a->vdisplay < b->vdisplay) return -1;
  if (a->vdisplay > b->vdisplay) return 1;
  if (getRetrace(a) < getRetrace(b)) return -1;
  if (getRetrace(a) > getRetrace(b)) return 1;

  // other info can be ordered arbitrarily
  return 0;
}

//
// LinuxDisplayMode
//

LinuxDisplayMode::LinuxDisplayMode() : display(NULL),
				numResolutions(0),
				lastResolution(-1),
				resolutions(NULL),
				origNumResolutions(0),
				origResolutions(NULL)
{
  // do nothing
}

LinuxDisplayMode::~LinuxDisplayMode()
{
  delete[] resolutions;
  if (origResolutions)
    XFree(origResolutions);
}

XDisplayMode::ResInfo**	LinuxDisplayMode::init(XDisplay* _display,
				int& numModes, int& currentMode)
{
  int i, eventbase, errorbase;

  // save display for later
  display = _display;

  // Check if we have the XF86 vidmode extension
  if (!XF86VidModeQueryExtension(display->getRep()->getDisplay(),
				&eventbase, &errorbase))
    return NULL;

  // get available resolutions
  if (!XF86VidModeGetAllModeLines(display->getRep()->getDisplay(),
				display->getRep()->getScreen(),
				&numResolutions, &origResolutions))
    return NULL;

  // get current resolution
  int dotclock;
  XF86VidModeModeLine mode;
  XF86VidModeGetModeLine(display->getRep()->getDisplay(),
				display->getRep()->getScreen(),
				&dotclock, &mode);

  // make a copy of the original data
  origNumResolutions = numResolutions;
  resolutions = new XF86VidModeModeInfo*[numResolutions];
  for (i = 0; i < numResolutions; i++)
    resolutions[i] = origResolutions[i];

  // sort resolutions
  qsort(resolutions, numResolutions, sizeof(resolutions[0]), resolutionCompare);

  // find current resolution in the available resolution list
  int current;
  for (current = 0; current < numResolutions; current++) {
    const XF86VidModeModeInfo* r = resolutions[current];
    if (dotclock	== (int)r->dotclock &&
	mode.hdisplay   == r->hdisplay &&
	mode.hsyncstart == r->hsyncstart &&
	mode.hsyncend   == r->hsyncend &&
	mode.htotal     == r->htotal &&
	mode.vdisplay   == r->vdisplay &&
	mode.vsyncstart == r->vsyncstart &&
	mode.vsyncend   == r->vsyncend &&
	mode.vtotal     == r->vtotal &&
	mode.flags      == r->flags)
      break;
  }

  // no switching if current mode not found
  if (current == numResolutions)
    return NULL;

  // compress out modes that are (effectively) duplicates.  never
  // remove the current mode.
  for (i = 0; i < numResolutions - 1; ) {
    if (resolutionCompare(resolutions + i, resolutions + i + 1) == 0) {
      // is next resolution the current mode?  if so then move it down
      // so we don't blow it away.
      if (current == i + 1)
	resolutions[i] = resolutions[i + 1];

      // move remaining resolutions down (overwriting the one after i)
      for (int j = i + 2; j < numResolutions; j++)
	resolutions[j - 1] = resolutions[j];

      // do we move the current resolution down too?
      if (current > i)
	current--;

      // now one less resolution
      numResolutions--;
    }
    else {
      i++;
    }
  }

  // make ResInfo list
  char name[80];
  ResInfo** resInfo = new ResInfo*[numResolutions];
  for (i = 0; i < numResolutions; i++) {
    const XF86VidModeModeInfo* r = resolutions[i];
    sprintf(name, "%dx%d @%d", r->hdisplay, r->vdisplay, getRetrace(r));
    resInfo[i] = new ResInfo(name, r->hdisplay, r->vdisplay, getRetrace(r));
  }

  // return modes
  numModes = numResolutions;
  currentMode = current;
  lastResolution = current;
  return resInfo;
}

bool			LinuxDisplayMode::set(int index)
{
  return doSet(index, true);
}

bool			LinuxDisplayMode::setDefault(int index)
{
  return doSet(index, false);
}

bool			LinuxDisplayMode::doSet(int index, bool position)
{
  // ignore attempts to set video format to current format.
  // normally this only happens when restoring the default
  // format, when BzfDisplay deliberately forces the change.
  // that's useful for win32 where the OS knows the right
  // format and will ignore calls to switch the current
  // format.  however, irix isn't so clever and may cause
  // the display to flicker even when the format isn't
  // really changing.
  if (index == lastResolution || numResolutions <= 1)
    return true;

  // deactivate windows before resolution change.  if we don't do this
  // then the app will almost certainly crash in the OpenGL driver.
  XWindow::deactivateAll();

  // change resolution
  if (XF86VidModeSwitchToMode(display->getRep()->getDisplay(),
				display->getRep()->getScreen(),
				resolutions[index])) {
    if (position) {
      // kludge for accelerated GLX.  when we set the view port after
      // changing the resolution just before quiting, GLX does not
      // release the display to X server control.  or something like
      // that.  the effect is that you see the game window still on
      // the screen but maybe shifted around and you can't see any of
      // the other windows.  without this code, a workaround for the
      // problem is ctrl_alt_+ or ctrl_alt_- to force a resize.
      XF86VidModeSetViewPort(display->getRep()->getDisplay(),
				display->getRep()->getScreen(), 0, 0);
    }
    XSync(display->getRep()->getDisplay(), false);
    lastResolution = index;

    // reactivate previously deactivated window after change
    XWindow::reactivateAll();
    return true;
  }

  // reactivate previously deactivated window after change
  XWindow::reactivateAll();
  return false;
}

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

