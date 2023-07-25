/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
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
 *  Encapsulates an Internet address
 *
 * PlayerId:
 *  Unique network id for a player.  Can be sent
 *  across the net.
 */

#ifndef BZF_INET_ADDR_H
#define BZF_INET_ADDR_H

// system headers
#include <sys/types.h>
#include <vector>
#include <string>

// local headers
#include "common.h"
#include "global.h"
#include "network.h"
#include "Pack.h"
#include "Protocol.h"

// helpers
char *sockaddr2iptext(const struct sockaddr *sa);
char *sockaddr2iptextport(const struct sockaddr *sa);
bool splitNamePort(std::string nameport, std::string &name, int &port);
std::string joinNamePort(std::string name, int port);

class Address
{
public:
    Address();
    Address(const std::string&);
    Address(const Address&);
    Address(const Address *);
    Address(const struct sockaddr_in6*); // input in nbo
    ~Address();
    Address&        operator=(const Address&);

    bool        operator==(const Address&) const;
    bool        operator!=(const Address&) const;
    bool        operator<(Address const&) const;
    bool        isMapped() const;
    bool        isAny() const;
    bool        isPrivate() const;
    sockaddr *getAddr();
    sockaddr_in *getAddr_in();
    sockaddr_in6 *getAddr_in6();
    /* return port in network order */
    in_port_t getNPort() const;
    std::string getIpText();
    std::string getIpTextPort();

    std::string     getDotNotation() const;
    uint8_t     getIPVersion() const;

    void*       pack(void*) const;
    const void*     unpack(const void*);

    static Address  getHostAddress(const std::string &hostname = std::string(""));
    static const std::string getHostName(const std::string &hostname = std::string(""));

private:
    struct sockaddr_in6 addr;
    std::string iptext;
    std::string iptextport;
};

// FIXME - enum maybe? put into namespace or class cage?
const PlayerId      NoPlayer = 255;
const PlayerId      AllPlayers = 254;
const PlayerId      ServerPlayer = 253;
const PlayerId      AdminPlayers = 252;
const PlayerId      FirstTeam = 251;
const PlayerId      LastRealPlayer = FirstTeam - NumTeams;

#endif // BZF_INET_ADDR_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
