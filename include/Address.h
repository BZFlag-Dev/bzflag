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
#include <string>
#include "common.h"
#include "network.h"
#include "Pack.h"

typedef struct in_addr	InAddr;			// shorthand

class Address {
  public:
			Address();
			Address(const std::string&);
			Address(const Address&);
			Address(const InAddr&);		    // input in nbo
			Address(const struct sockaddr_in&); // input in nbo
			~Address();
    Address&		operator=(const Address&);

			operator InAddr() const;
    bool		operator==(const Address&) const;
    bool		operator!=(const Address&) const;
    bool		isAny() const;
    std::string		getDotNotation() const;

    void*		pack(void*) const;
    void*		unpack(void*);

    static Address	getHostAddress(const char* hostname = NULL);
    static std::string	getHostByAddress(InAddr);
    static const char*	getHostName(const char* hostname = NULL);

  private:
    InAddr		addr;
    static Address	localAddress;
};

typedef uint8_t		PlayerId;
const int		PlayerIdPLen = sizeof(PlayerId);
const int		ServerIdPLen = 8;

// FIXME - enum maybe? put into namespace or class cage?
const PlayerId		NoPlayer = 255;
const PlayerId		AllPlayers = 254;
const PlayerId		ServerPlayer = 253;
const PlayerId		UnusedSpecialPlayer1 = 252; // These two ids are unused at present
const PlayerId		UnusedSpecialPlayer2 = 251; // Available for special needs in the future
const PlayerId		LastRealPlayer = 243;

class ServerId {
  public:
    void*		pack(void*) const;
    void*		unpack(void*);

    bool		operator==(const ServerId&) const;
    bool		operator!=(const ServerId&) const;

  public:
    // host and port in network byte order
    InAddr		serverHost;		// server host
    int16_t		port;			// server port
    int16_t		number;			// local player number
};

#endif // BZF_INET_ADDR_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

