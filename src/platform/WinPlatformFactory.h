/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
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

class WinWindow;

class WinPlatformFactory : public PlatformFactory {
  public:
			WinPlatformFactory();
			~WinPlatformFactory();

    BzfDisplay*		createDisplay(const char* name,
				const char* videoFormat);
    BzfVisual*		createVisual(const BzfDisplay*);
    BzfWindow*		createWindow(const BzfDisplay*, BzfVisual*);

  private:
			WinPlatformFactory(const WinPlatformFactory&);
    WinPlatformFactory&	operator=(const WinPlatformFactory&);

    BzfMedia*		createMedia();

  private:
    static WinWindow*	window;
};

#endif // BZF_WINPLATFORM_FACTORY_H
