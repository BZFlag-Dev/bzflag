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

#ifndef __ARES_HANDLER_H__
#define __ARES_HANDLER_H__

// bzflag global header
#include "global.h"

/* common implementation headers */
#include "network.h"
#include <ares.h>

class AresHandler
{
public:
    AresHandler(int index);
    ~AresHandler();

    static bool   globalInit();
    static void   globalShutdown();

    enum ResolutionStatus
    {
        None = 0,
        Failed,
        HbAPending,
        HbASucceeded,
        HbNPending,
        HbNSucceeded
    };

    void      setIndex ( int i )
    {
        index = i;
    }
    void      queryHostname(const sockaddr *);
    void      queryHost(const char *hostName, const char *service);
    const char   *getHostname();
    ResolutionStatus getHostAddress(in_addr *);
    ResolutionStatus getHostAddr(sockaddr_in6 *);
    void      setFd(fd_set *read_set, fd_set *write_set, int &maxFile);
    void      process(fd_set *read_set, fd_set *write_set);
    ResolutionStatus getStatus()
    {
        return status;
    };
private:
#if ARES_VERSION_MAJOR >= 1 && ARES_VERSION_MINOR >= 5
    static void   staticHostCallback(void *arg, int statusCallback, int timeouts, hostent *);
#else
    static void   staticHostCallback(void *arg, int statusCallback, hostent *);
#endif
    void      callback(int status, hostent *);

    static void staticAddrInfoCallback(void *arg, int callbackStatus, int, ares_addrinfo *result);
    void callback(int callbackStatus, ares_addrinfo *result);

    int       index;

    std::string   hostName;
    in_addr   hostAddress;
    sockaddr_in6 hostAddr;
    ares_channel  aresChannel;
    ResolutionStatus status;
    bool      aresFailed;

    static bool   globallyInited;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
