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

/*
 * Multicast socket utility functions
 */

#ifndef BZF_MULTICAST_H
#define BZF_MULITCAST_H

#include "network.h"
#include "common.h"
#include "Address.h"
#include <sys/types.h>

/* openBroadcast:
 *	port:		port number
 *	service:	name of service (overrides port if not NULL)
 *	addr:		resulting address of socket
 * return value:
 *	file descriptor of socket if successful
 *	-1 if failed
 *
 * openMulticast:
 *	address:	multicast group address
 *	port:		port number
 *	service:	name of service (overrides port if not NULL)
 *	ttl:		time to live
 *	interface:	address of multicast interface (default if NULL)
 *	mode:		"r" for read only, "w" for write only
 *	addr:		resulting address of socket
 * return value:
 *	file descriptor of socket if successful
 *	-1 if failed
 *
 * multicast functions other than openMulticast() and setMulticastTTL()
 * also work on broadcast sockets.
 */

int						openBroadcast(int port, const char* service,
							struct sockaddr_in* addr);
int						openMulticast(const Address& address, int port,
							const char* service, int ttl,
							const char* net_interface,
							const char* mode,
							struct sockaddr_in* addr);
int						closeMulticast(int fd);
int						setMulticastTTL(int fd, int ttl);
int						sendMulticast(int fd, const void* buffer,
							int bufferLength, const struct sockaddr_in*);
int						recvMulticast(int fd, void* buffer,
							int bufferLength, struct sockaddr_in*);

#endif // BZF_MULTICAST_H
