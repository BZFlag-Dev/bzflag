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

/* WinPlatformFactory:
 *	Factory for Windows platform stuff.
 */

#ifndef BZF_WINPLATFORM_FACTORY_H
#define	BZF_WINPLATFORM_FACTORY_H

#include "PlatformFactory.h"

#ifdef HAVE_SDL
class SDLWindow;
#endif
class WinWindow;

class WinPlatformFactory : public PlatformFactory {
  public:
			WinPlatformFactory();
			~WinPlatformFactory();

    BzfDisplay*		createDisplay(const char* name,
				const char* videoFormat);
    BzfVisual*		createVisual(const BzfDisplay*);
    BzfWindow*		createWindow(const BzfDisplay*, BzfVisual*);
    BzfJoystick*	createJoystick();

  private:
			WinPlatformFactory(const WinPlatformFactory&);
    WinPlatformFactory&	operator=(const WinPlatformFactory&);

    BzfMedia*		createMedia();

  private:
#ifdef HAVE_SDL
    static SDLWindow*	sdlWindow;
#endif
    static WinWindow*	winWindow;
};

#endif // BZF_WINPLATFORM_FACTORY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
