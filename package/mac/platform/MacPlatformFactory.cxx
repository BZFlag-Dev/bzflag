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


#include "MacPlatformFactory.h"
#include "MacDisplay.h"
#include "MacVisual.h"
#include "MacWindow.h"
#include "MacMedia.h"


PlatformFactory*	PlatformFactory::getInstance()
{

  if (!instance)
    instance = new MacPlatformFactory ();

  return instance;
}

BzfDisplay* MacPlatformFactory::createDisplay (const char *name, const char *videoFormat) {

    if (!display)

      display = new MacDisplay (name, videoFormat);

    return display;
}

BzfVisual* MacPlatformFactory::createVisual (const BzfDisplay *_display) {

    if (!visual)

      visual = new MacVisual ((const MacDisplay*)_display);

    return visual;
}

BzfWindow* MacPlatformFactory::createWindow (const BzfDisplay *_display, BzfVisual *_visual) {


    if (!window)
      window = new MacWindow ((const MacDisplay*)_display, (MacVisual*)_visual);

    return window;
}

BzfMedia*   MacPlatformFactory::createMedia () {

    if (!media)
      media = new MacMedia ();

    return media;
}

MacPlatformFactory::MacPlatformFactory () {

  display = NULL;
  visual  = NULL;
  window  = NULL;
  media   = NULL;
}

MacPlatformFactory::~MacPlatformFactory () {

 // delete display;
 // delete visual;
}





