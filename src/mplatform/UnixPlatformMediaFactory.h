/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* UnixPlatformMediaFactory:
 *	Factory for common unix platform media stuff.
 */

#ifndef BZF_UNIX_PLATFORM_MEDIA_FACTORY_H
#define BZF_UNIX_PLATFORM_MEDIA_FACTORY_H

#include "PlatformMediaFactory.h"

class UnixPlatformMediaFactory : public PlatformMediaFactory {
public:
	UnixPlatformMediaFactory();
	~UnixPlatformMediaFactory();

	BzfVisual*			createVisual(const BzfDisplay*);
	BzfWindow*			createWindow(const BzfDisplay*, BzfVisual*);

private:
	UnixPlatformMediaFactory(const UnixPlatformMediaFactory&);
	UnixPlatformMediaFactory& operator=(const UnixPlatformMediaFactory&);
};

#endif // BZF_UNIX_PLATFORM_MEDIA_FACTORY_H
