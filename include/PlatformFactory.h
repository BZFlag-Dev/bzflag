/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* PlatformFactory:
 *	Abstract builder for platform dependent stuff.
 */

#ifndef BZF_PLATFORM_FACTORY_H
#define	BZF_PLATFORM_FACTORY_H

class BzfDisplay;
class BzfVisual;
class BzfWindow;
class BzfMedia;

class PlatformFactory {
  public:
			PlatformFactory();
    virtual		~PlatformFactory();

    virtual BzfDisplay*	createDisplay(const char* name,
				const char* videoFormat) = 0;
    virtual BzfVisual*	createVisual(const BzfDisplay*) = 0;
    virtual BzfWindow*	createWindow(const BzfDisplay*, BzfVisual*) = 0;

    static PlatformFactory*	getInstance();
    static BzfMedia*		getMedia();

  private:
			PlatformFactory(const PlatformFactory&);
    PlatformFactory&	operator=(const PlatformFactory&);

    virtual BzfMedia*	createMedia() = 0;

  private:
    static PlatformFactory*	instance;
    static BzfMedia*		media;
};

#endif // BZF_PLATFORM_FACTORY_H
// ex: shiftwidth=2 tabstop=8
