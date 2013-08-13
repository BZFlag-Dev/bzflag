/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* PlatformCommon:
 *	Defines the interface classes for platform specific code used by both client and server
 */

#ifndef	PLATFORM_COMMON_H
#define	PLATFORM_COMMON_H

#include "common.h"

class URLJob
{
	public 
};

class PlatformURLManager
{
public:
	virtual ~PlatformURLManager(){};

	virtual void SetMaxJobs(int jobs) = 0;
	virtual int GetMaxJobs() = 0;
	virtual int GetJobCount() = 0;

	virtual void KillAll() = 0;

};

extern PlatformURLManager& GetURLManager();


#endif //PLATFORM_COMMON_H