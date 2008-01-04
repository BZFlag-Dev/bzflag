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

/* SolarisPlatformFactory:
 *	Factory for Sun Sparc/Solaris platform stuff.
 */

#ifndef BZF_SOLARISPLATFORM_FACTORY_H
#define	BZF_SOLARISPLATFORM_FACTORY_H

#include "PlatformFactory.h"

class SolarisPlatformFactory : public PlatformFactory {
  public:
			SolarisPlatformFactory();
			~SolarisPlatformFactory();

    BzfDisplay*		createDisplay(const char* name, const char*);
    BzfVisual*		createVisual(const BzfDisplay*);
    BzfWindow*		createWindow(const BzfDisplay*, BzfVisual*);

  private:
			SolarisPlatformFactory(const SolarisPlatformFactory&);
    SolarisPlatformFactory&	operator=(const SolarisPlatformFactory&);

    BzfMedia*		createMedia();
};

#endif // BZF_SOLARISPLATFORM_FACTORY_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
