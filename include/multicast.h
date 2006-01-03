/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Broadcast socket utility functions
 */

#ifndef	__MULTICAST_H__
#define	__MULTICAST_H__

#include "common.h"

/* system headers */
#include <sys/types.h>

/* common headers */
#include "network.h"
#include "Address.h"


/** openBroadcast:
 *	port:		port number
 *	service:	name of service (overrides port if not NULL)
 *	addr:		resulting address of socket
 * return value:
 *	file descriptor of socket if successful
 *	-1 if failed
 */
int			openBroadcast(int port, const char* service,
					struct sockaddr_in* addr);
int			closeBroadcast(int fd);
int			sendBroadcast(int fd, const void* buffer,
				int bufferLength, const struct sockaddr_in*);
int			recvBroadcast(int fd, void* buffer,
				int bufferLength, struct sockaddr_in*);

#endif  // __MULTICAST_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
