/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

  private:
			LinuxPlatformFactory(const LinuxPlatformFactory&);
    LinuxPlatformFactory& operator=(const LinuxPlatformFactory&);

    BzfMedia*		createMedia();
};

#endif // BZF_LINUX_PLATFORM_FACTORY_H
// ex: shiftwidth=2 tabstop=8
