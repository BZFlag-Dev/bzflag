/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * UPDnetwork socket utility functions
 */

#ifndef	BZF_UDPNET_H
#define	BZF_UDPNET_H

#include <sys/types.h>
#include "network.h"
#include "common.h"
#include "Address.h"

/* openUDPnetwork:
 *	port:		port number
 *	addr:		address of socket
 * return value:
 *	file descriptor of socket if successful
 *	-1 if failed
 *
 */

int			openUDPNetwork(int port,
					struct sockaddr_in* addr);
int			closeUDPNetwork(int fd);
int			sendUDPNetwork(int fd, const void* buffer,
				int bufferLength, const struct sockaddr_in*);
int			recvUDPNetwork(int fd, void* buffer,
				int bufferLength, struct sockaddr_in*);

#endif // BZF_UDPNET_H

/* ex: shiftwidth=2 tabstop=8
 * Local Variables: ***
 * mode:C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 */

