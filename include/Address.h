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

/* Address:
 *	Encapsulates an Internet address
 *
 * PlayerId:
 *	Unique network id for a player.  Can be sent
 *	across the net.
 */

#ifndef	BZF_INET_ADDR_H
#define	BZF_INET_ADDR_H

#include <sys/types.h>
#include "network.h"
#include "common.h"
#include "BzfString.h"
#include "Pack.h"

typedef struct in_addr	InAddr;			// shorthand

class Address {
  public:
			Address();
			Address(const BzfString&);
			Address(const Address&);
			Address(const InAddr&);		    // input in nbo
			Address(const struct sockaddr_in&); // input in nbo
			~Address();
    Address&		operator=(const Address&);

			operator InAddr() const;
    boolean		operator==(const Address&) const;
    boolean		operator!=(const Address&) const;
    boolean		isAny() const;
    char*		getDotNotation() const;

    void*		pack(void*) const;
    void*		unpack(void*);

    static Address	getHostAddress(const char* hostname = NULL);
    static BzfString	getHostByAddress(InAddr);
    static const char*	getHostName(const char* hostname = NULL);

  private:
    InAddr		addr;
    static Address	localAddress;
};

const int		PlayerIdPLen = 8;

class PlayerId {
  public:
    void*		pack(void*) const;
    void*		unpack(void*);

    boolean		operator==(const PlayerId&) const;
    boolean		operator!=(const PlayerId&) const;

  public:
    // host and port in network byte order
    InAddr		serverHost;		// server host
    int16_t		port;			// server port
    int16_t		number;			// local player number
};

#endif // BZF_INET_ADDR_H
// ex: shiftwidth=2 tabstop=8
