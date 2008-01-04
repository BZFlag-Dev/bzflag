/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "MacPlatformFactory.h"

/* implementation headers */
#ifdef HAVE_SDL
#  include "SDLMedia.h"
#  include "SDLVisual.h"
#  include "SDLWindow.h"
#  include "SDLDisplay.h"
#  include "SDLJoystick.h"
#else
#  include "MacDisplay.h"
#  include "MacVisual.h"
#  include "MacWindow.h"
#  include "MacMedia.h"
#endif

PlatformFactory* PlatformFactory::getInstance()
{
  if (!instance)
    instance = new MacPlatformFactory();

  return instance;
}


#ifdef HAVE_SDL
BzfDisplay* MacPlatformFactory::createDisplay(const char *, const char *) {
#else
BzfDisplay* MacPlatformFactory::createDisplay(const char *name, const char *videoFormat) {
#endif
  if (!display)
#ifdef HAVE_SDL
    display = new SDLDisplay();
#else
    display = new MacDisplay(name, videoFormat);
#endif

  return display;
}

BzfVisual* MacPlatformFactory::createVisual(const BzfDisplay *_display) {
  if (!visual)
#ifdef HAVE_SDL
    visual = new SDLVisual((const SDLDisplay*)_display);
#else
    visual = new MacVisual((const MacDisplay*)_display);
#endif

  return visual;
}

BzfWindow* MacPlatformFactory::createWindow(const BzfDisplay *_display, BzfVisual *_visual) {
  if (!window)
#ifdef HAVE_SDL
    window = new SDLWindow((const SDLDisplay*)_display, (SDLVisual*)_visual);
#else
    window = new MacWindow((const MacDisplay*)_display, (MacVisual*)_visual);
#endif

  return window;
}

BzfMedia* MacPlatformFactory::createMedia() {
  if (!media)
#ifdef HAVE_SDL
    media = new SDLMedia();
#else
    media = new MacMedia();
#endif

  return media;
}

#ifdef HAVE_SDL
BzfJoystick* MacPlatformFactory::createJoystick() {
  return new SDLJoystick;
}
#endif

MacPlatformFactory::MacPlatformFactory() {
  display = NULL;
  visual  = NULL;
  window  = NULL;
  media   = NULL;
}

MacPlatformFactory::~MacPlatformFactory() {
  //delete display;
  //delete visual;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
