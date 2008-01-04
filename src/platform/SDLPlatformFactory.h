/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SDLPlatformFactory:
 *	Factory for SDL platform stuff.
 */

#ifndef BZF_SDL_PLATFORM_FACTORY_H
#define	BZF_SDL_PLATFORM_FACTORY_H

#include "PlatformFactory.h"

class SdlPlatformFactory : public PlatformFactory {
 public:
  SdlPlatformFactory();
  ~SdlPlatformFactory();

  BzfDisplay*		createDisplay(const char* name, const char*);
  BzfVisual*		createVisual(const BzfDisplay*);
  BzfWindow*		createWindow(const BzfDisplay*, BzfVisual*);
  BzfMedia*		createMedia();
  BzfJoystick*		createJoystick();

 private:
  SdlPlatformFactory(const SdlPlatformFactory&);
  SdlPlatformFactory& operator=(const SdlPlatformFactory&);
};

#endif // BZF_SDL_PLATFORM_FACTORY_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
