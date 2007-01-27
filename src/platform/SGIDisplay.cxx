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

#include "SGIDisplay.h"

#if defined(USE_XSGIVC_EXT)

#include <stdlib.h>

typedef int		(*XErrorHandler_t)(Display*, XErrorEvent*);
static bool		errorHappened;
static int		errorHandler(Display*, XErrorEvent*)
{
  errorHappened = true;
  return 0;
}

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

    bool		isValid() const;
    const char*		getName() const;
    int			getNumChannels() const;
    const Config*	getConfigs() const;
    bool		setFormat(Display*, int screen) const;

  private:
    void		setConfig(int index, const XSGIvcVideoFormatInfo*);

  private:
    bool		valid;
    bool		combination;
    char*		name;
    int			numChannels;
    void*		data;
    Config*		config;
};

Resolution::Resolution(XSGIvcVideoFormatInfo* format) :
				valid(true), data(NULL)
{
  name = new char[strlen(format->name) + 1];
  strcpy(name, format->name);

  numChannels = 1;
  config = new Config[numChannels];
  setConfig(0, format);

  combination = false;
  XSGIvcVideoFormatInfo* _data = new XSGIvcVideoFormatInfo;
  *_data = *format;
  data = _data;
}

Resolution::Resolution(const char* comboName, int numFormats,
				XSGIvcVideoFormatInfo* formats) :
				valid(true), data(NULL)
{
  name = new char[strlen(comboName) + 1];
  strcpy(name, comboName);

  numChannels = numFormats;
  config = new Config[numChannels];
  for (int i = 0; i < numChannels; i++)
    setConfig(i, formats + i);

  combination = true;
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

bool			Resolution::isValid() const
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

bool			Resolution::setFormat(Display* dpy, int screen) const
{
  errorHappened = false;
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
			false);
  }

  XSync(dpy, false);
  XSetErrorHandler(prevErrorHandler);
  if (!status || errorHappened) ((Resolution*)this)->valid = false;
  return valid;
}

//
// comparision of resolutions function for qsort
//

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
// SGIDisplay
//

SGIDisplayMode::SGIDisplayMode() : display(NULL)
{
  numResolutions = 0;
  lastResolution = -1;
  numVideoChannels = 0;
  defaultChannel = -1;
  defaultVideoFormats = NULL;
  defaultVideoCombo = NULL;
  numVideoFormats = NULL;
  videoFormats = NULL;
  numVideoCombos = 0;
  videoCombos = NULL;
  resolutions = NULL;
}

SGIDisplayMode::~SGIDisplayMode()
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

XDisplayMode::ResInfo**	SGIDisplayMode::init(XDisplay* _display,
				int& numModes, int& currentMode)
{
  // save display for later
  display = _display;
  XDisplay::Rep* rep = display->getRep();

  // get video screen info
  int i, major, minor;
  XSGIvcScreenInfo screenInfo;
  if (!XSGIvcQueryVersion(rep->getDisplay(), &major, &minor))
    return NULL;
  if (!XSGIvcQueryVideoScreenInfo(rep->getDisplay(),
				rep->getScreen(), &screenInfo))
    return NULL;
  if (screenInfo.numChannels < 1) return NULL;
  numVideoChannels = screenInfo.numChannels;

  errorHappened = false;
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
      errorHappened = false;
      int numFormats = 0;
      XSGIvcVideoFormatInfo* formats = XSGIvcListVideoFormats(
				rep->getDisplay(), rep->getScreen(), i,
				&pattern, 0, false, 4096, &numFormats);
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
    errorHappened = false;
    int numCombos;
    char** comboNames = XSGIvcListVideoFormatsCombinations(
					rep->getDisplay(), rep->getScreen(),
					"*", 4096, &numCombos);
    if (!comboNames || errorHappened) numCombos = 0;

    videoCombos = new Resolution*[numCombos];
    for (i = 0; i < numCombos; i++) {
      errorHappened = false;
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
	    const bool hasFormat = (j < numFormats && formats[j].name[0]);
	    const bool isActive = (channelFormat[j] != NULL);

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

    numModes = numResolutions;
    currentMode = current;
    lastResolution = current;
    return resInfo;
  }

  return NULL;
}

bool			SGIDisplayMode::set(int index)
{
  // ignore attempts to set video format to current format.
  // normally this only happens when restoring the default
  // format, when BzfDisplay deliberately forces the change.
  // that's useful for win32 where the OS knows the right
  // format and will ignore calls to switch the current
  // format.  however, irix isn't so clever and may cause
  // the display to flicker even when the format isn't
  // really changing.
  if (index == lastResolution)
    return true;

  if (resolutions[index]->setFormat(display->getRep()->getDisplay(),
				display->getRep()->getScreen())) {
    lastResolution = index;
    return true;
  }
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
