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

/* Address:
 *	Encapsulates an Internet address
 *
 * PlayerId:
 *	Unique network id for a player.  Can be sent
 *	across the net.
 */

#ifndef	BZF_INET_ADDR_H
#define	BZF_INET_ADDR_H

// system headers
#include <sys/types.h>
#include <vector>
#include <string>

// local headers
#include "common.h"
#include "global.h"
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
    bool		operator<(Address const&) const;
    bool		isAny() const;
    bool		isPrivate() const;
    std::string		getDotNotation() const;
    uint8_t		getIPVersion() const;

    void*		pack(void*) const;
    const void*		unpack(const void*);

    static Address	getHostAddress(const std::string &hostname = std::string(""));
    static std::string	getHostByAddress(InAddr);
    static const std::string getHostName(const std::string &hostname = std::string(""));

  private:
    std::vector <InAddr> addr;
};

typedef uint8_t		PlayerId;
const int		PlayerIdPLen = sizeof(PlayerId);
const int		ServerIdPLen = 8;

// FIXME - enum maybe? put into namespace or class cage?
const PlayerId		NoPlayer = 255;
const PlayerId		AllPlayers = 254;
const PlayerId		ServerPlayer = 253;
const PlayerId		AdminPlayers = 252;
const PlayerId		FirstTeam = 251;
const PlayerId		LastRealPlayer = FirstTeam - NumTeams;

class ServerId {
  public:
    void*		pack(void*) const;
    const void*		unpack(const void*);

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

