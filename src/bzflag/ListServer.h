/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __LISTSERVER_H__
#define __LISTSERVER_H__

/* system interface headers */
#include <string>
#include <curl/curl.h>

/* common interface headers */
#include "Address.h"


const int       MaxListServers = 5;
class ListServer {
  public:
    Address		address;
    int			port;
    int			socket;
    int			phase;
    std::string         hostname;
    std::string         pathname;
    int			bufferSize;
#ifdef _WIN32
    char		buffer[16384 + 1];
#else
    char		buffer[CURL_MAX_WRITE_SIZE + 1];
#endif
};


#endif /* __LISTSERVER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
