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

/* WinPlatformFactory:
 *	Factory for Windows platform stuff.
 */

#ifndef BZF_WINPLATFORM_FACTORY_H
#define	BZF_WINPLATFORM_FACTORY_H

#include "PlatformFactory.h"

class MacPlatformFactory : public PlatformFactory
{
  public:
			MacPlatformFactory();
			~MacPlatformFactory();

    BzfDisplay*		createDisplay(const char* name, const char* videoFormat);
    BzfVisual*		createVisual(const BzfDisplay*);
    BzfWindow*		createWindow(const BzfDisplay*, BzfVisual*);

  private:
		//MacPlatformFactory(const MacPlatformFactory&);
    //MacPlatformFactory&	operator=(const MacPlatformFactory&);

    BzfMedia*		createMedia();

  private:
    BzfWindow  *window;
    BzfDisplay *display;
    BzfVisual  *visual;
    BzfMedia   *media;
};

#endif // BZF_WINPLATFORM_FACTORY_H
// ex: shiftwidth=2 tabstop=8
