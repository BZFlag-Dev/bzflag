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

/* LinuxPlatformFactory:
 *	Factory for Linux platform stuff.
 */

#ifndef BZF_LINUX_PLATFORM_FACTORY_H
#define	BZF_LINUX_PLATFORM_FACTORY_H

#include "PlatformFactory.h"

class LinuxPlatformFactory : public PlatformFactory {
  public:
			LinuxPlatformFactory();
			~LinuxPlatformFactory();

    BzfDisplay*		createDisplay(const char* name, const char*);
    BzfVisual*		createVisual(const BzfDisplay*);
    BzfWindow*		createWindow(const BzfDisplay*, BzfVisual*);
    BzfJoystick*	createJoystick();

  private:
			LinuxPlatformFactory(const LinuxPlatformFactory&);
    LinuxPlatformFactory& operator=(const LinuxPlatformFactory&);

    BzfMedia*		createMedia();
};

#endif // BZF_LINUX_PLATFORM_FACTORY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
