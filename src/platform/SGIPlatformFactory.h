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

/* SGIPlatformFactory:
 *	Factory for Irix platform stuff.
 */

#ifndef BZF_SGI_PLATFORM_FACTORY_H
#define BZF_SGI_PLATFORM_FACTORY_H

#include "UnixPlatformFactory.h"

class SGIPlatformFactory : public UnixPlatformFactory {
public:
	SGIPlatformFactory();
	~SGIPlatformFactory();

	double				getClock() const;

private:
	SGIPlatformFactory(const SGIPlatformFactory&);
	SGIPlatformFactory& operator=(const SGIPlatformFactory&);

private:
	double				secondsPerTick;
	unsigned int		clockZero;
	volatile unsigned int* iotimer_addr;
};

#endif // BZF_UNIX_PLATFORM_FACTORY_H
