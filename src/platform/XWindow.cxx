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

#include "XWindow.h"
#include "XVisual.h"
#include "OpenGLGState.h"
#if defined(XF86VIDMODE_EXT)
#  define USE_XF86VIDMODE_EXT
#  define private c_private
#  include <X11/extensions/xf86vmode.h>
#  undef private
#endif
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

//
// XWindow
//

XWindow*		XWindow::first = NULL;

XWindow::XWindow(const XDisplay* _display, XVisual* _visual) :
				BzfWindow(_display),
				display(_display->getRep()),
				window(None),
				colormap(None),
				context(NULL),
				noWM(false),
				defaultColormap(true),
				prev(NULL),
				next(NULL),
				colormapPixels(NULL),
				xsh(XAllocSizeHints()),
				gammaVal(1.0)
{
  // get desired visual
  XVisualInfo* pvisual = _visual->get();
  if (!pvisual) return;
  visual = *pvisual;

  // make a colormap
  colormap = XCreateColormap(display->getDisplay(), display->getRootWindow(),
			visual.visual, AllocNone);

  // create the window
  XSetWindowAttributes windowAttrib;
  windowAttrib.background_pixel = 0;
  windowAttrib.border_pixel = 0;
  windowAttrib.colormap = colormap;
  windowAttrib.event_mask = ExposureMask |
			StructureNotifyMask |
			PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask |
			KeyPressMask | KeyReleaseMask;
  window = XCreateWindow(display->getDisplay(), display->getRootWindow(),
			0, 0, 1, 1, 0, visual.depth,
			InputOutput, visual.visual,
			CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,
			&windowAttrib);
  if (window == None) {
    XFreeColormap(display->getDisplay(), colormap);
    colormap = None;
    return;
  }

  // set class property
  XClassHint* classHint = XAllocClassHint();
  if (classHint) {
    classHint->res_name = "bzflag";
    classHint->res_class = "BZFlag";
    XSetClassHint(display->getDisplay(), window, classHint);
    XFree(classHint);
  }

  // set window manager protocols.  we want the window manager
  // to ask us before destroying our window when the user clicks
  // the close-window button.
  Atom a;
  a = XInternAtom(display->getDisplay(), "WM_DELETE_WINDOW", true);
  if (a != None)
    XSetWMProtocols(display->getDisplay(), window, &a, 1);

  // setup colormap if visual doesn't define it
  if (visual.c_class == DirectColor) {
    // find how many bits are in each of red, green, and blue channels
    int rBits, rOffset;
    int gBits, gOffset;
    int bBits, bOffset;
    countBits(visual.red_mask, rBits, rOffset);
    countBits(visual.green_mask, gBits, gOffset);
    countBits(visual.blue_mask, bBits, bOffset);

    // allocate colors
    unsigned long rMask, gMask, bMask, pixel;
    if (XAllocColorPlanes(display->getDisplay(),
				colormap, true, &pixel, 1,
				rBits, gBits, bBits, &rMask, &gMask, &bMask))
      defaultColormap = false;
  }

  // these shouldn't happen because we request an RGBA visual, but Mesa
  // doesn't play by the usual rules.
  else if (visual.c_class == GrayScale ||
	   (visual.c_class == PseudoColor && visual.depth == 8)) {
    // allocate colors
    unsigned long mask;
    colormapPixels = new unsigned long[visual.colormap_size];
    if (colormapPixels && XAllocColorCells(display->getDisplay(),
				colormap, true, &mask,
				0, colormapPixels, visual.colormap_size))
      defaultColormap = false;
  }
  if (!defaultColormap) {
    loadColormap();
    XSetWMColormapWindows(display->getDisplay(), window, &window, 1);
  }

  // make an OpenGL context for the window.  do *not* bind it to the
  // window yet:  to support 3Dfx correctly we have to wait until the
  // window has been sized.
  context = glXCreateContext(display->getDisplay(), &visual, NULL, true);
  if (context == NULL) {
    XDestroyWindow(display->getDisplay(), window);
    XFreeColormap(display->getDisplay(), colormap);
    window = None;
    colormap = None;
    return;
  }

  // add to list
  display->ref();
  prev = NULL;
  next = first;
  if (prev) prev->next = this;
  if (next) next->prev = this;
  first = this;
}

XWindow::~XWindow()
{
  // free up stuff
  freeContext();
  if (window != None)
    XDestroyWindow(display->getDisplay(), window);
  if (colormap != None)
    XFreeColormap(display->getDisplay(), colormap);
  delete[] colormapPixels;

  if (prev) prev->next = next;
  if (next) next->prev = prev;
  if (first == this) first = next;

  display->unref();
  XFree(xsh);
}

bool			XWindow::isValid() const
{
  return window != None;
}

void			XWindow::showWindow(bool show)
{
  if (show) {
    // show window and wait for it (window manager may make us wait)
    XEvent event;
    XMapWindow(display->getDisplay(), window);
    XWindowEvent(display->getDisplay(), window, ExposureMask, &event);

    // if no window manager control, we need to set focus and load the
    // colormap
    if (noWM) {
      XSetInputFocus(display->getDisplay(), window,
				RevertToPointerRoot, CurrentTime);
      if (!defaultColormap)
	XInstallColormap(display->getDisplay(), colormap);
    }
  }
  else {
    // hide window
    XUnmapWindow(display->getDisplay(), window);
    if (noWM && !defaultColormap)
      XUninstallColormap(display->getDisplay(), colormap);
  }
}

void			XWindow::getPosition(int& x, int& y)
{
  XWindowAttributes attr;
  XGetWindowAttributes(display->getDisplay(), window, &attr);
  x = attr.x;
  y = attr.y;
}

void			XWindow::getSize(int& width, int& height) const
{
  XWindowAttributes attr;
  XGetWindowAttributes(display->getDisplay(), window, &attr);
  width = attr.width;
  height = attr.height;
}

void			XWindow::setTitle(const char* title)
{
  XStoreName(display->getDisplay(), window, title);
  XSetIconName(display->getDisplay(), window, title);
}

void			XWindow::setPosition(int x, int y)
{
  long dummy;
  XGetWMNormalHints(display->getDisplay(), window, xsh, &dummy);
  xsh->x = x;
  xsh->y = y;
  xsh->flags |= USPosition | PPosition;
  XSetWMNormalHints(display->getDisplay(), window, xsh);
  XMoveWindow(display->getDisplay(), window, x, y);
  XSync(display->getDisplay(), false);
}

void			XWindow::setSize(int width, int height)
{
  long dummy;
  XGetWMNormalHints(display->getDisplay(), window, xsh, &dummy);
  xsh->base_width = width;
  xsh->base_height = height;
  xsh->flags |= PBaseSize;
  XSetWMNormalHints(display->getDisplay(), window, xsh);
  XResizeWindow(display->getDisplay(), window, width, height);
  XSync(display->getDisplay(), false);
}

void			XWindow::setMinSize(int width, int height)
{
  long dummy;
  XGetWMNormalHints(display->getDisplay(), window, xsh, &dummy);
  if (width < 1 || height < 1) {
    xsh->flags &= ~PMinSize;
  }
  else {
    xsh->min_width = width;
    xsh->min_height = height;
    xsh->flags |= PMinSize;
  }
  XSetWMNormalHints(display->getDisplay(), window, xsh);
}

void			XWindow::setFullscreen()
{
  // see if a motif based window manager is running.  do this by
  // getting the _MOTIF_WM_INFO property on the root window.  if
  // it exists then make sure the window it refers to also exists.
  bool isMWMRunning = false;
  Atom a = XInternAtom(display->getDisplay(), "_MOTIF_WM_INFO", true);
  if (a) {
    struct BzfPropMotifWmInfo {
      public:
	long		flags;
	Window		wmWindow;
    };

    Atom type;
    int format;
    unsigned long nitems;
    unsigned long bytes_after;
    long* mwmInfo;
    XGetWindowProperty(display->getDisplay(), display->getRootWindow(),
			a, 0, 4, false,
			a, &type, &format, &nitems, &bytes_after,
			(unsigned char**)&mwmInfo);
    if (mwmInfo) {
      // get the mwm window from the properties
      const Window mwmWindow = ((BzfPropMotifWmInfo*)mwmInfo)->wmWindow;
      XFree(mwmInfo);

      // verify that window is a child of the root window
      Window root, parent, *children;
      unsigned int numChildren;
      if (XQueryTree(display->getDisplay(), mwmWindow, &root, &parent,
		&children, &numChildren)) {
	XFree(children);
	if (parent == display->getRootWindow())
	  isMWMRunning = true;
      }
    }
  }

  // turning off decorations is window manager dependent
  if (isMWMRunning) {
    // it's a Motif based window manager
    long hints[4];
    hints[0] = 0;
    hints[1] = 0;
    hints[2] = 0;
    hints[3] = 0;
    long* xhints;
    a = XInternAtom(display->getDisplay(), "_MOTIF_WM_HINTS", false);
    {
      // get current hints
      Atom type;
      int format;
      unsigned long nitems;
      unsigned long bytes_after;
      XGetWindowProperty(display->getDisplay(), window, a, 0, 4, false,
			a, &type, &format, &nitems, &bytes_after,
			(unsigned char**)&xhints);
      if (xhints) {
	hints[0] = xhints[0];
	hints[1] = xhints[1];
	hints[2] = xhints[2];
	hints[3] = xhints[3];
	XFree(xhints);
      }
    }
    hints[0] |= 2;		// MWM_HINTS_DECORATIONS flag
    hints[2] = 0;			// no decorations
    XChangeProperty(display->getDisplay(), window, a, a, 32,
			PropModeReplace, (unsigned char*)&hints, 4);
    noWM = false;
  }

  else {
    // non-motif window manager.  use override redirect to prevent window
    // manager from messing with our appearance.  unfortunately, the user
    // can't move or iconify the window either.
    XSetWindowAttributes attr;
    attr.override_redirect = true;
    XChangeWindowAttributes(display->getDisplay(),
				window, CWOverrideRedirect, &attr);
    noWM = true;
  }

  // now set position and size
  long dummy;
  XGetWMNormalHints(display->getDisplay(), window, xsh, &dummy);
  xsh->x = 0;
  xsh->y = 0;
  xsh->base_width = getDisplay()->getWidth();
  xsh->base_height = getDisplay()->getHeight();
  xsh->flags |= USPosition | PPosition | PBaseSize;

#if defined(USE_XF86VIDMODE_EXT)
  {
    int eventbase, errorbase;
    // Check if we have the XF86 vidmode extension, for virtual roots
    if (XF86VidModeQueryExtension(display->getDisplay(), &eventbase, &errorbase)) {
      int dotclock;
      XF86VidModeModeLine modeline;

      XF86VidModeGetModeLine(display->getDisplay(), display->getScreen(), &dotclock, &modeline);
      xsh->base_width=modeline.hdisplay;
      xsh->base_height=modeline.vdisplay;
      if (modeline.c_private)
	XFree(modeline.c_private);
    }
  }
#endif
// this might want to be used on non-linux too?
#ifdef __linux__
  {
    char *env;

    env=getenv("MESA_GLX_FX");
    if (env && *env != tolower('w')) { // Full screen Mesa mode
      xsh->base_width=getDisplay()->getPassthroughWidth();
      xsh->base_height=getDisplay()->getPassthroughHeight();
    }
  }
#endif

  // set the window manager hints for the window and move and resize
  // the window (overriding the window manager).  we have to override
  // the window manager for the move and resize because the window
  // *must* be the correct size when we first bind the OpenGL context
  // for the 3Dfx driver since it cannot handle later resizes.  if we
  // don't override with window manager, our move and resize will
  // probably be ignored.
  if (!noWM) {
    XSetWindowAttributes attr;
    attr.override_redirect = true;
    XChangeWindowAttributes(display->getDisplay(),
				window, CWOverrideRedirect, &attr);
  }
  XSetWMNormalHints(display->getDisplay(), window, xsh);
  XMoveResizeWindow(display->getDisplay(), window, xsh->x, xsh->y,
			xsh->base_width, xsh->base_height);
  if (!noWM) {
    XSetWindowAttributes attr;
    attr.override_redirect = false;
    XChangeWindowAttributes(display->getDisplay(),
				window, CWOverrideRedirect, &attr);
  }
  XSync(display->getDisplay(), false);
}

void			XWindow::warpMouse(int x, int y)
{
  XWarpPointer(display->getDisplay(), None, window, 0, 0, 0, 0, x, y);
}

void			XWindow::getMouse(int& x, int& y) const
{
  Window rootWindow, childWindow;
  int rootX, rootY;
  unsigned int mask;
  int mx, my;
  XQueryPointer(display->getDisplay(), window,
		&rootWindow, &childWindow,
		&rootX, &rootY, &mx, &my, &mask);
  x = mx;
  y = my;
}

void			XWindow::grabMouse()
{
  XGrabPointer(display->getDisplay(), window,
		true, 0, GrabModeAsync, GrabModeAsync,
		window, None, CurrentTime);
}

void			XWindow::ungrabMouse()
{
  XUngrabPointer(display->getDisplay(), CurrentTime);
}

void			XWindow::showMouse()
{
  XDefineCursor(display->getDisplay(), window, None);
}

void			XWindow::hideMouse()
{
  static Cursor cursor = None;

  // FIXME -- is there an easier way to hide the cursor?
  if (cursor == None) {
    Font font = XLoadFont(display->getDisplay(), "fixed");
    if (font) {
      XColor color;
      color.pixel = 0;
      color.red = color.green = color.blue = 0;
      color.flags = DoRed | DoGreen | DoBlue;
      cursor = XCreateGlyphCursor(display->getDisplay(),
				font, font, ' ', ' ', &color, &color);
      XUnloadFont(display->getDisplay(), font);
      // note we're going to leak the cursor
    }
  }
  if (cursor != None)
    XDefineCursor(display->getDisplay(), window, cursor);
}

void			XWindow::setGamma(float newGamma)
{
  gammaVal = newGamma;
  loadColormap();
}

float			XWindow::getGamma() const
{
  return gammaVal;
}

bool			XWindow::hasGammaControl() const
{
  return !defaultColormap;
}

void			XWindow::makeCurrent()
{
  if (context)
    glXMakeCurrent(display->getDisplay(), window, context);
}

void			XWindow::swapBuffers()
{
  glXSwapBuffers(display->getDisplay(), window);
}

void			XWindow::makeContext()
{
  if (!context)
    context = glXCreateContext(display->getDisplay(), &visual, NULL, true);
  makeCurrent();
}

void			XWindow::freeContext()
{
  if (context) {
    if (glXGetCurrentContext() == context)
      glXMakeCurrent(display->getDisplay(), None, NULL);
    glXDestroyContext(display->getDisplay(), context);
    context = NULL;
  }
}

void			XWindow::loadColormap()
{
  if (defaultColormap)
    return;

  // allocate space for colors
  XColor* colors = new XColor[visual.colormap_size];
  if (!colors)
    return;

  if (visual.c_class == DirectColor) {
    // find how many bits are in each of red, green, and blue channels
    int rBits, rOffset;
    int gBits, gOffset;
    int bBits, bOffset;
    countBits(visual.red_mask, rBits, rOffset);
    countBits(visual.green_mask, gBits, gOffset);
    countBits(visual.blue_mask, bBits, bOffset);

    // create colors
    int i;
    for (i = 0; i < visual.colormap_size; i++) {
      colors[i].pixel = 0;
      colors[i].red   = 0;
      colors[i].green = 0;
      colors[i].blue  = 0;
      colors[i].flags = 0;
    }

    int n = 1 << rBits;
    for (i = 0; i < n; i++) {
      colors[i].pixel |= i << rOffset;
      colors[i].red   = getIntensityValue((float)i / (float)(n - 1));
      colors[i].flags |= DoRed;
    }

    n = 1 << gBits;
    for (i = 0; i < n; i++) {
      colors[i].pixel |= i << gOffset;
      colors[i].green = getIntensityValue((float)i / (float)(n - 1));
      colors[i].flags |= DoGreen;
    }

    n = 1 << bBits;
    for (i = 0; i < n; i++) {
      colors[i].pixel |= i << bOffset;
      colors[i].blue  = getIntensityValue((float)i / (float)(n - 1));
      colors[i].flags |= DoBlue;
    }
  }

  else if (visual.c_class == GrayScale) {
    // create colors
    for (int i = 0; i < visual.colormap_size; i++) {
      const unsigned short v = getIntensityValue((float)i /
					(float)(visual.colormap_size - 1));
      colors[i].pixel = colormapPixels[i];
      colors[i].red   = v;
      colors[i].green = v;
      colors[i].blue  = v;
      colors[i].flags = DoRed | DoGreen | DoBlue;
    }
  }

  else if (visual.c_class == PseudoColor && visual.depth == 8) {
    // assume a 3:3:2 (RGB) colormap for an 8 bit deep PseudoColor,
    // with red in the low bits, then green, then blue.

    // create colors
    for (int i = 0; i < visual.colormap_size; i++) {
      colors[i].pixel = colormapPixels[i];
      colors[i].red   = getIntensityValue(pixelField(i, 3, 0));
      colors[i].green = getIntensityValue(pixelField(i, 3, 3));
      colors[i].blue  = getIntensityValue(pixelField(i, 2, 6));
      colors[i].flags = DoRed | DoGreen | DoBlue;
    }
  }

  else {
    assert(0 && "bad visual in loadColormap()");
  }

  // set colors
  XStoreColors(display->getDisplay(), colormap, colors, visual.colormap_size);
  delete[] colors;
}

unsigned short		XWindow::getIntensityValue(float i) const
{
  if (i <= 0.0f) return 0;
  if (i >= 1.0f) return 65535;
  i = powf(i, 1.0f / gammaVal);
  return (unsigned short)(0.5f + 65535.0f * i);
}

float			XWindow::pixelField(int i, int bits, int offset)
{
  const int mask = (1 << bits) - 1;
  return (float)((i >> offset) & mask) / (float)mask;
}

void			XWindow::countBits(
				unsigned long mask, int& num, int& offset)
{
  num = 0;
  offset = 0;

  // find the offset
  while (mask && (mask & 1) == 0) {
    offset++;
    mask >>= 1;
  }

  // count the one bits
  while (mask & 1) {
    num++;
    mask >>= 1;
  }

  // verify that there are no more set bits (non-contiguous mask)
  if (mask) num = 0;
}

XWindow*		XWindow::lookupWindow(Window w)
{
  XWindow* scan = first;
  while (scan && scan->window != w) scan = scan->next;
  return scan;
}

void			XWindow::deactivateAll()
{
  for (XWindow* scan = first; scan; scan = scan->next)
    scan->freeContext();
}

void			XWindow::reactivateAll()
{
  for (XWindow* scan = first; scan; scan = scan->next)
    scan->makeContext();

  // reload context data
  if (first)
    OpenGLGState::initContext();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
