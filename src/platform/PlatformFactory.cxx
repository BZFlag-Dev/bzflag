/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "PlatformFactory.h"
#ifdef HAVE_SDL
#include <SDL\SDL.h>
#endif
#include "ErrorHandler.h"

PlatformFactory*	PlatformFactory::instance = 0;
BzfMedia*		PlatformFactory::media = 0;

PlatformFactory::PlatformFactory()
{
#ifdef HAVE_SDL
  Uint32 flags = 0;
#ifdef DEBUG
  flags |= SDL_INIT_NOPARACHUTE;
#endif
  if (SDL_Init(flags) == -1) {
    printFatalError("Could not initialize SDL: %s.\n", SDL_GetError());
    exit(-1);
  }; 
#endif
}

PlatformFactory::~PlatformFactory()
{
#ifdef HAVE_SDL
  SDL_Quit();
#endif
}

BzfMedia*		PlatformFactory::getMedia()
{
  if (!media) media = getInstance()->createMedia();
  return media;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

