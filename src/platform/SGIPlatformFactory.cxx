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

#include "SGIPlatformFactory.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syssgi.h>
#include <unistd.h>

PlatformFactory*		PlatformFactory::getInstance()
{
	if (instance == NULL)
		instance = new SGIPlatformFactory;
	return instance;
}

SGIPlatformFactory::SGIPlatformFactory() :
								secondsPerTick(0.0),
								clockZero(0),
								iotimer_addr(NULL)
{
	// prepare high resolution timer
	unsigned int cycleval;
	const ptrdiff_t addr = syssgi(SGI_QUERY_CYCLECNTR, &cycleval);
	if (addr != -1) {
		const int poffmask = getpagesize() - 1;
		const __psunsigned_t phys_addr = (__psunsigned_t)addr;
		const __psunsigned_t raddr = phys_addr & ~poffmask;
		int fd = open("/dev/mmem", O_RDONLY);
		iotimer_addr = (volatile unsigned int *)mmap(0, poffmask, PROT_READ,
								MAP_PRIVATE, fd, (off_t)raddr);
		iotimer_addr = (unsigned int *)((__psunsigned_t)iotimer_addr +
												(phys_addr & poffmask));
#ifdef SGI_CYCLECNTR_SIZE
		if ((int)syssgi(SGI_CYCLECNTR_SIZE) > 32)
			iotimer_addr++;
#endif
		secondsPerTick = 1.0e-12 * (double)cycleval;
		clockZero      = *iotimer_addr;
	}
}

SGIPlatformFactory::~SGIPlatformFactory()
{
	// do nothing
}

double					SGIPlatformFactory::getClock() const
{
	if (iotimer_addr == NULL)
		return UnixPlatformFactory::getClock();
	else
		return secondsPerTick * (double)(*iotimer_addr - clockZero);
}
// ex: shiftwidth=4 tabstop=4
