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

#include "XDisplay.h"
#include "XWindow.h"
#include "BzfEvent.h"
#include <string.h>
#include <X11/keysym.h>
#include <GL/glx.h>

typedef int		(*XErrorHandler_t)(Display*, XErrorEvent*);
static boolean		errorHappened;
static int		errorHandler(Display*, XErrorEvent*)
{
  errorHappened = True;
  return 0;
}
#include <stdlib.h>
static int		ioErrorHandler(Display*)	// FIXME
{
  abort();
  return 0;
}

//
// XDisplay::Rep
//

XDisplay::Rep::Rep(const char* displayName) :
				refCount(1),
				display(NULL),
				screen(0)
{
  // open display
  display = XOpenDisplay(displayName);
  if (!display) return;
XSetIOErrorHandler(ioErrorHandler);	// FIXME

  // other initialization
  screen = DefaultScreen(display);
}

XDisplay::Rep::~Rep()
{
  if (display) XCloseDisplay(display);
}

void			XDisplay::Rep::ref()
{
  refCount++;
}

void			XDisplay::Rep::unref()
{
  if (--refCount <= 0) delete this;
}

Window			XDisplay::Rep::getRootWindow() const
{
  return display ? RootWindow(display, screen) : None;
}


//
// XDisplay
//

XDisplay::XDisplay(const char* displayName) : rep(NULL)
{
  // initialize
  numResolutions = 0;
  numVideoChannels = 0;
  defaultChannel = -1;
  defaultVideoFormats = NULL;
  defaultVideoCombo = NULL;
  numVideoFormats = NULL;
  videoFormats = NULL;
  numVideoCombos = 0;
  videoCombos = NULL;
  resolutions = NULL;

  // open display
  rep = new Rep(displayName);

  // get resolutions
  if (rep->getDisplay()) setResolutions();
}

XDisplay::~XDisplay()
{
  setResolution(getDefaultResolution());
  freeResolution();
  rep->unref();
}

boolean			XDisplay::isValid() const
{
  return rep->getDisplay() != NULL;
}

boolean			XDisplay::isEventPending() const
{
  return (XPending(rep->getDisplay()) != 0);
}

boolean			XDisplay::getEvent(BzfEvent& event) const
{
  XEvent xevent;
  XNextEvent(rep->getDisplay(), &xevent);
  switch (xevent.type) {
    case Expose:
    case ConfigureNotify:
    case MotionNotify:
    case MapNotify:
    case UnmapNotify:
    case ButtonPress:
    case ButtonRelease:
    case KeyPress:
    case KeyRelease:
      event.window = XWindow::lookupWindow(xevent.xexpose.window);
      if (!event.window) return False;
      break;

    default:
      return False;
  }

  switch (xevent.type) {
    case Expose:
      if (xevent.xexpose.count != 0) return False;
      event.type = BzfEvent::Redraw;
      break;

    case ConfigureNotify: {
/* attempt to filter out non-size changes, but getSize() returns the
 * current size so it always matches the size in the event.
      int width, height;
      event.window->getSize(width, height);
      if (width == xevent.xconfigure.width &&
	  height == xevent.xconfigure.height)
	return False;
*/
      event.type = BzfEvent::Resize;
      event.resize.width = xevent.xconfigure.width;
      event.resize.height = xevent.xconfigure.height;
      break;
    }

    case MotionNotify:
      event.type = BzfEvent::MouseMove;
      event.mouseMove.x = xevent.xmotion.x;
      event.mouseMove.y = xevent.xmotion.y;
      break;

    case MapNotify:
      event.type = BzfEvent::Map;
      break;

    case UnmapNotify:
      event.type = BzfEvent::Unmap;
      break;

    case ButtonPress:
      event.type = BzfEvent::KeyDown;
      event.keyDown.ascii = 0;
      event.keyDown.shift = 0;
      switch (xevent.xbutton.button) {
	case Button1: event.keyDown.button = BzfKeyEvent::LeftMouse; break;
	case Button2: event.keyDown.button = BzfKeyEvent::MiddleMouse; break;
	case Button3: event.keyDown.button = BzfKeyEvent::RightMouse; break;
	default:      return False;
      }
      break;

    case ButtonRelease:
      event.type = BzfEvent::KeyUp;
      event.keyUp.ascii = 0;
      event.keyUp.shift = 0;
      switch (xevent.xbutton.button) {
	case Button1: event.keyUp.button = BzfKeyEvent::LeftMouse; break;
	case Button2: event.keyUp.button = BzfKeyEvent::MiddleMouse; break;
	case Button3: event.keyUp.button = BzfKeyEvent::RightMouse; break;
	default:      return False;
      }
      break;

    case KeyPress:
      event.type = BzfEvent::KeyDown;
      if (!getKey(xevent, event.keyDown)) return False;
      break;

    case KeyRelease:
      event.type = BzfEvent::KeyUp;
      if (!getKey(xevent, event.keyUp)) return False;
      break;
  }

  return True;
}

boolean			XDisplay::getKey(const XEvent& xevent,
						BzfKeyEvent& key) const
{
  char buf[3];
  KeySym keysym;
  if (XLookupString((XKeyEvent*)&xevent.xkey, buf, 1, &keysym, NULL) == 1) {
    key.ascii = buf[0];
    key.button = BzfKeyEvent::NoButton;

    if (keysym == XK_Delete) {
      key.ascii = 0;
      key.button = BzfKeyEvent::Delete;
    }
  }
  else {
    key.ascii = 0;
    switch (keysym) {
      case XK_Pause:	key.button = BzfKeyEvent::Pause; break;
      case XK_Home:	key.button = BzfKeyEvent::Home; break;
      case XK_End:	key.button = BzfKeyEvent::End; break;
      case XK_Left:	key.button = BzfKeyEvent::Left; break;
      case XK_Right:	key.button = BzfKeyEvent::Right; break;
      case XK_Up:	key.button = BzfKeyEvent::Up; break;
      case XK_Down:	key.button = BzfKeyEvent::Down; break;
      case XK_Page_Up:	key.button = BzfKeyEvent::PageUp; break;
      case XK_Page_Down: key.button = BzfKeyEvent::PageDown; break;
      case XK_Insert:	key.button = BzfKeyEvent::Insert; break;
      case XK_Delete:	key.button = BzfKeyEvent::Delete; break;
      case XK_F1:	key.button = BzfKeyEvent::F1; break;
      case XK_F2:	key.button = BzfKeyEvent::F2; break;
      case XK_F3:	key.button = BzfKeyEvent::F3; break;
      case XK_F4:	key.button = BzfKeyEvent::F4; break;
      case XK_F5:	key.button = BzfKeyEvent::F5; break;
      case XK_F6:	key.button = BzfKeyEvent::F6; break;
      case XK_F7:	key.button = BzfKeyEvent::F7; break;
      case XK_F8:	key.button = BzfKeyEvent::F8; break;
      case XK_F9:	key.button = BzfKeyEvent::F9; break;
      case XK_F10:	key.button = BzfKeyEvent::F10; break;
      case XK_F11:	key.button = BzfKeyEvent::F11; break;
      case XK_F12:	key.button = BzfKeyEvent::F12; break;
      default:		return False;
    }
  }

  key.shift = 0;
  if (xevent.xkey.state & ShiftMask) key.shift |= BzfKeyEvent::ShiftKey;
  if (xevent.xkey.state & ControlMask) key.shift |= BzfKeyEvent::ControlKey;
  if (xevent.xkey.state & Mod1Mask) key.shift |= BzfKeyEvent::AltKey;
  return True;
}

#if !defined(__sgi) || defined(NO_XSGIVC)

boolean			XDisplay::doSetResolution(int)
{
  // no switching
  return False;
}

void			XDisplay::setResolutions()
{
  ResInfo** resInfo = new ResInfo*[1];
  resInfo[0] = new ResInfo("default",
			DisplayWidth(rep->getDisplay(), rep->getScreen()),
			DisplayHeight(rep->getDisplay(), rep->getScreen()), 0);
  initResolutions(resInfo, 1, 0);
}

void			XDisplay::freeResolution()
{
  // do nothing
}

#else // __sgi

#include <stdlib.h>
#include <X11/extensions/XSGIvc.h>

#if defined(X_SGIvcListVideoFormatCombinations)
#define XSGIvcListVideoFormatsCombinations XSGIvcListVideoFormatCombinations
#endif

//
// Resolution
//

class Resolution {
  public:
    class Config {
      public:
	int		width;
	int		height;
	int		refresh;
    };

  public:
			Resolution(XSGIvcVideoFormatInfo*);
			Resolution(const char* comboName, int numFormats,
				XSGIvcVideoFormatInfo* formats);
			~Resolution();

    boolean		isValid() const;
    const char*		getName() const;
    int			getNumChannels() const;
    const Config*	getConfigs() const;
    boolean		setFormat(Display*, int screen) const;

  private:
    void		setConfig(int index, const XSGIvcVideoFormatInfo*);

  private:
    boolean		valid;
    boolean		combination;
    char*		name;
    int			numChannels;
    void*		data;
    Config*		config;
};

Resolution::Resolution(XSGIvcVideoFormatInfo* format) :
				valid(True), data(NULL)
{
  name = new char[strlen(format->name) + 1];
  strcpy(name, format->name);

  numChannels = 1;
  config = new Config[numChannels];
  setConfig(0, format);

  combination = False;
  XSGIvcVideoFormatInfo* _data = new XSGIvcVideoFormatInfo;
  *_data = *format;
  data = _data;
}

Resolution::Resolution(const char* comboName, int numFormats,
				XSGIvcVideoFormatInfo* formats) :
				valid(True), data(NULL)
{
  name = new char[strlen(comboName) + 1];
  strcpy(name, comboName);

  numChannels = numFormats;
  config = new Config[numChannels];
  for (int i = 0; i < numChannels; i++)
    setConfig(i, formats + i);

  combination = True;
  char* _data = new char[strlen(comboName) + 1];
  strcpy(_data, comboName);
  data = _data;
}

Resolution::~Resolution()
{
  delete[] name;
  delete[] config;
  if (combination) delete[] (char*)data;
  else delete (XSGIvcVideoFormatInfo*)data;
}

void			Resolution::setConfig(int index,
				const XSGIvcVideoFormatInfo* format)
{
  config[index].width = format->width;
  config[index].height = format->height;
  config[index].refresh = (int)(format->verticalRetraceRate + 0.5f);
}

boolean			Resolution::isValid() const
{
  return valid;
}

const char*		Resolution::getName() const
{
  return name;
}

int			Resolution::getNumChannels() const
{
  return numChannels;
}

const Resolution::Config* Resolution::getConfigs() const
{
  return config;
}

boolean			Resolution::setFormat(Display* dpy, int screen) const
{
  errorHappened = False;
  XErrorHandler_t prevErrorHandler = XSetErrorHandler(&errorHandler);

  Status status;
  if (combination) {
    status = XSGIvcLoadVideoFormatCombination(dpy, screen, (const char*)data);
  }
  else {
    status = XSGIvcLoadVideoFormat(dpy, screen, 0,
			(XSGIvcVideoFormatInfo*)data,
			XSGIVC_QVFHeight |
			XSGIVC_QVFWidth |
			XSGIVC_QVFSwapbufferRate,
			False);
  }

  XSync(dpy, False);
  XSetErrorHandler(prevErrorHandler);
  if (!status || errorHappened) ((Resolution*)this)->valid = False;
  return valid;
}

static int		resolutionCompare(const void* _a, const void* _b)
{
  const Resolution* a = *((const Resolution**)_a);
  const int aNumChannels = a->getNumChannels();
  const Resolution::Config* aConfig = a->getConfigs();

  const Resolution* b = *((const Resolution**)_b);
  const int bNumChannels = b->getNumChannels();
  const Resolution::Config* bConfig = b->getConfigs();

  if (aNumChannels < bNumChannels) return -1;
  if (aNumChannels > bNumChannels) return 1;
  for (int i = 0; i < aNumChannels; i++) {
    if (aConfig[i].width < bConfig[i].width) return -1;
    if (aConfig[i].width > bConfig[i].width) return 1;
    if (aConfig[i].height < bConfig[i].height) return -1;
    if (aConfig[i].height > bConfig[i].height) return 1;
    if (aConfig[i].refresh < bConfig[i].refresh) return -1;
    if (aConfig[i].refresh > bConfig[i].refresh) return 1;
  }
  return 0;
}

//
// XDisplay
//

boolean			XDisplay::doSetResolution(int index)
{
  return resolutions[index]->setFormat(rep->getDisplay(), rep->getScreen());
}

void			XDisplay::setResolutions()
{
  if (!doSetResolutions()) {
    ResInfo** resInfo = new ResInfo*[1];
    resInfo[0] = new ResInfo("default",
			DisplayWidth(rep->getDisplay(), rep->getScreen()),
			DisplayHeight(rep->getDisplay(), rep->getScreen()), 0);
    initResolutions(resInfo, 1, 0);
  }
}

boolean			XDisplay::doSetResolutions()
{
  // get video screen info
  int i, major, minor;
  XSGIvcScreenInfo screenInfo;
  if (!XSGIvcQueryVersion(rep->getDisplay(), &major, &minor))
    return False;
  if (!XSGIvcQueryVideoScreenInfo(rep->getDisplay(),
				rep->getScreen(), &screenInfo))
    return False;
  if (screenInfo.numChannels < 1) return False;
  numVideoChannels = screenInfo.numChannels;

  errorHappened = False;
  XErrorHandler_t prevErrorHandler = XSetErrorHandler(&errorHandler);

  // get the current format on each channel
  Resolution** channelFormat = new Resolution*[numVideoChannels];
  for (i = 0; i < numVideoChannels; i++) {
    XSGIvcChannelInfo* channelInfo;
    channelFormat[i] = NULL;
    if (XSGIvcQueryChannelInfo(rep->getDisplay(),
				rep->getScreen(), i, &channelInfo)) {
      if (channelInfo->active)
	channelFormat[i] = new Resolution(&channelInfo->vfinfo);
      XFree(channelInfo);
    }
  }

  // get the individually loadable formats available on each channel
  if (screenInfo.flags & XSGIVC_SIFFormatPerChannel) {
    defaultVideoFormats = new int[numVideoChannels];
    numVideoFormats = new int[numVideoChannels];
    videoFormats = new Resolution**[numVideoChannels];

    XSGIvcVideoFormatInfo pattern;
    for (i = 0; i < numVideoChannels; i++) {
      errorHappened = False;
      int numFormats = 0;
      XSGIvcVideoFormatInfo* formats = XSGIvcListVideoFormats(
				rep->getDisplay(), rep->getScreen(), i,
				&pattern, 0, False, 4096, &numFormats);
      if (!formats || errorHappened) numFormats = 0;

      videoFormats[i] = new Resolution*[numFormats];
      for (int j = 0; j < numFormats; j++)
	videoFormats[i][j] = new Resolution(formats + j);
      numVideoFormats[i] = numFormats;

      if (formats) XSGIvcFreeVideoFormatInfo(formats);
    }

    // figure out the current format on each channel by comparing the
    // current format against each format available on that channel.
    // also set the default channel (first active is as good as another).
    for (i = 0; i < numVideoChannels; i++) {
      defaultVideoFormats[i] = -1;
      for (int j = 0; j < numVideoFormats[i]; j++) {
	const char* name1 = videoFormats[i][j]->getName();
	const char* name2 = channelFormat[i]->getName();
	if ((name1[0] && name2[0] && strcmp(name1, name2) == 0) ||
	    resolutionCompare(videoFormats[i] + j, channelFormat + i) == 0) {
	  defaultVideoFormats[i] = j;
	  break;
	}
      }
      if (defaultVideoFormats[i] == -1) {
	// uh, Houston, we have a problem -- can't find current format!
	// disable format changing on this channel.
	for (int j = 0; j < numVideoFormats[i]; j++)
	  delete videoFormats[i][j];
	numVideoFormats[i] = 0;
      }

      else if (defaultChannel == -1) {
	defaultChannel = i;
      }
    }
  }

  // now get the combo formats
  if (screenInfo.flags & XSGIVC_SIFFormatCombination) {
    errorHappened = False;
    int numCombos;
    char** comboNames = XSGIvcListVideoFormatsCombinations(
					rep->getDisplay(), rep->getScreen(),
					"*", 4096, &numCombos);
    if (!comboNames || errorHappened) numCombos = 0;

    videoCombos = new Resolution*[numCombos];
    for (i = 0; i < numCombos; i++) {
      errorHappened = False;
      int numFormats, bestMatch = 0;
      XSGIvcVideoFormatInfo* formats =
		XSGIvcListVideoFormatsInCombination(
					rep->getDisplay(), rep->getScreen(),
					comboNames[i], &numFormats);
      if (formats && numFormats != 0 && !errorHappened) {
	// make format for combo
	videoCombos[numVideoCombos++] =
		new Resolution(comboNames[i], numFormats, formats);

	// compare the format in each channel with the current format
	// in each channel.  if they all match then we've found a
	// suitable default video combo name.  note that inactive
	// channels have a NULL channelFormat[] and an empty
	// formats[].name.
	if (!defaultVideoCombo || bestMatch != numVideoChannels) {
	  int match = 0;
	  for (int j = 0; j < numVideoChannels; j++) {
	    const boolean hasFormat = (j < numFormats && formats[j].name[0]);
	    const boolean isActive = (channelFormat[j] != NULL);

	    // no match if channel is inactive and there's a format or
	    // channel is active and there's no format.
	    if (hasFormat != isActive) continue;

	    // match if channel is inactive and no format on that channel.
	    if (!hasFormat && !isActive) continue;

	    // compare formats for equality
	    const Resolution::Config* c = channelFormat[j]->getConfigs();
	    if (formats[j].width != c[0].width)
	      continue;
	    if (formats[j].height != c[0].height)
	      continue;
	    if ((int)(formats[j].verticalRetraceRate + 0.5f) != c[0].refresh)
	      continue;
	    match++;
	  }

	  // if all channels matched then we've got it
	  if (match > bestMatch) {
	    bestMatch = match;
	    delete[] defaultVideoCombo;
	    defaultVideoCombo = new char[strlen(comboNames[i]) + 1];
	    strcpy(defaultVideoCombo, comboNames[i]);
	  }
	}

	XSGIvcFreeVideoFormatInfo(formats);
      }
    }

    XFree(comboNames);
  }

  // done with current channel info
  for (i = 0; i < numVideoChannels; i++)
    delete channelFormat[i];
  delete[] channelFormat;

  // now combine formats and combinations for interface.  save a
  // pointer to the one that's the currently loaded format.
  int numFormats = (defaultChannel == -1) ? 0 : numVideoFormats[defaultChannel];
  numFormats += numVideoCombos;
  resolutions = new Resolution*[numFormats];
  Resolution* currentFormat = NULL;
  if (defaultChannel != -1) {
    for (i = 0; i < numVideoFormats[defaultChannel]; i++)
      resolutions[numResolutions++] = videoFormats[defaultChannel][i];

    const int defaultFormat = defaultVideoFormats[defaultChannel];
    if (defaultFormat != -1)
      currentFormat = videoFormats[defaultChannel][defaultFormat];
  }
  for (i = 0; i < numVideoCombos; i++) {
    resolutions[numResolutions++] = videoCombos[i];

    if (defaultVideoCombo &&
	strcmp(defaultVideoCombo, videoCombos[i]->getName()) == 0)
      currentFormat = videoCombos[i];
  }

  // sort resolutions
  qsort(resolutions, numResolutions, sizeof(resolutions[0]), resolutionCompare);

  XSetErrorHandler(prevErrorHandler);

  // find current format in sorted list
  int current;
  for (current = 0; current < numResolutions; current++)
    if (resolutions[current] == currentFormat)
      break;

  // if current format found then allow format switching
  if (current < numResolutions) {
    ResInfo** resInfo = new ResInfo*[numResolutions];
    for (int i = 0; i < numResolutions; i++) {
      const Resolution* r = resolutions[i];
      const Resolution::Config* c = r->getConfigs();
      resInfo[i] = new ResInfo(r->getName(), c->width, c->height, c->refresh);
    }
    initResolutions(resInfo, numResolutions, current);
    return True;
  }

  return False;
}

void			XDisplay::freeResolution()
{
  int i, j;
  for (i = 0; i < numVideoChannels; i++) {
    for (j = 0; j < numVideoFormats[i]; j++)
      delete videoFormats[i][j];
    delete[] videoFormats[i];
  }
  delete[] videoFormats;
  for (i = 0; i < numVideoCombos; i++)
    delete videoCombos[i];
  delete[] videoCombos;
  delete[] defaultVideoFormats;
  delete[] defaultVideoCombo;
  delete[] numVideoFormats;
  delete[] resolutions;
}

#endif // __sgi
