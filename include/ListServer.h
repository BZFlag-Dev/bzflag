/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __LISTSERVER_H__
#define __LISTSERVER_H__

#include "common.h"

/* system interface headers */
#include <string>

/* common interface headers */
#include "Address.h"


const int       MaxListServers = 5;
typedef struct _ListServer {
    Address		address;
    int			port;
    int			socket;
    int			phase;
    std::string	 hostname;
    std::string	 pathname;
    int			failures;
    int			bufferSize;
    char		buffer[1024];
} ListServer;


#endif /* __LISTSERVER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
