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

/* BeOSPlatformFactory:
 *	Factory for Irix platform stuff.
 */

#ifndef BZF_BEOS_PLATFORM_FACTORY_H
#define BZF_BEOS_PLATFORM_FACTORY_H

#include "PlatformFactory.h"

class BeOSPlatformFactory : public PlatformFactory {
public:
  BeOSPlatformFactory();
  ~BeOSPlatformFactory();

  BzfDisplay*		createDisplay(const char* name, const char*);
  BzfVisual*		createVisual(const BzfDisplay*);
  BzfWindow*		createWindow(const BzfDisplay*, BzfVisual*);

private:
  BeOSPlatformFactory(const BeOSPlatformFactory&);
  BeOSPlatformFactory& operator=(const BeOSPlatformFactory&);

  BzfMedia*		createMedia();
};

#endif // BZF_UNIX_PLATFORM_FACTORY_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
