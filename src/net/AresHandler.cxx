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

/* interface header */
#include "AresHandler.h"

/* system implementation headers */
#include <cerrno>
#include <cstring>

bool AresHandler::globallyInited = false;

AresHandler::AresHandler(int _index)
    : index(_index), status(None)
{
    // clear the host address
    memset(&hostAddress, 0, sizeof(hostAddress));

    /* ask for local "hosts" lookups too */
    static char lookups[] = "fb";
    struct ares_options opts;
    opts.lookups = lookups;

    /* start up our resolver */
    if (!globallyInited)
    {
        aresFailed = !globalInit();
        if (aresFailed)
        {
            status = Failed;
            logDebugMessage(2,"Ares failed initializing library\n");
        }
    }

    /* start up our resolver */
    int code = ares_init_options (&aresChannel, &opts, ARES_OPT_LOOKUPS);
    aresFailed = (code != ARES_SUCCESS);
    if (aresFailed)
    {
        status = Failed;
        logDebugMessage(2,"Ares Failed initializing\n");
    }
}

AresHandler::~AresHandler()
{
    if (aresFailed)
        return;
    ares_destroy(aresChannel);
}

bool AresHandler::globalInit()
{
    if (!globallyInited)
    {
#ifdef HAVE_ARES_LIBRARY_INIT
        if (ares_library_init(ARES_LIB_INIT_ALL) == ARES_SUCCESS)
#endif
            globallyInited = true;
    }
    return globallyInited;
}

void AresHandler::globalShutdown()
{
#ifdef HAVE_ARES_LIBRARY_INIT
    if (globallyInited)
        ares_library_cleanup();
#endif
}

void AresHandler::queryHostname(const struct sockaddr *clientAddr)
{
    if (aresFailed)
        return;
    const sockaddr_in6 *ip6 = (const sockaddr_in6 *)clientAddr;
    status = HbAPending;
    // launch the asynchronous query to look up this hostname
    logDebugMessage(2,"Player [%d] submitting reverse resolve query\n", index);
    if (clientAddr->sa_family == AF_INET6 &&
            ip6->sin6_addr.__in6_u.__u6_addr32[0] == 0 &&
            ip6->sin6_addr.__in6_u.__u6_addr32[1] == 0 &&
            ip6->sin6_addr.__in6_u.__u6_addr32[2] == htonl(0xffff))
    {
        // lookup wrapped v4 in v6
        ares_gethostbyaddr(aresChannel, &ip6->sin6_addr.__in6_u.__u6_addr32[3],
                           sizeof(uint32_t), AF_INET, staticHostCallback, (void *)this);
    }
    else
        ares_gethostbyaddr(aresChannel, &((const sockaddr_in *)clientAddr)->sin_addr,
                           sizeof(in_addr), clientAddr->sa_family, staticHostCallback, (void *)this);
}

#if ARES_VERSION_MAJOR >= 1 && ARES_VERSION_MINOR >= 5
void AresHandler::staticHostCallback(void *arg, int callbackStatus, int, hostent *hostent)
#else
void AresHandler::staticHostCallback(void *arg, int callbackStatus, hostent *hostent)
#endif
{
    ((AresHandler *)arg)->callback(callbackStatus, hostent);
}

void AresHandler::callback(int callbackStatus, struct hostent *hostent)
{
    if (callbackStatus == ARES_EDESTRUCTION)
        return;
    if (callbackStatus != ARES_SUCCESS)
    {
        logDebugMessage(1,"Player [%d] failed to resolve: error %d\n", index,
                        callbackStatus);
        status = Failed;
    }
    else if (status == HbAPending)
    {
        hostName = strdup(hostent->h_name);
        status = HbASucceeded;
        logDebugMessage(2,"Player [%d] resolved to %s\n", index, hostName.c_str());
    }
    else if (status == HbNPending)
    {
        memcpy(&hostAddress, hostent->h_addr_list[0], sizeof(hostAddress));
        status = HbNSucceeded;
    }
}

void AresHandler::queryHost(const char *name, const char *service)
{
    if (aresFailed)
        return;
    ares_cancel(aresChannel);

    if (!name)
    {
        status = Failed;
        return;
    }

    // launch the asynchronous query to look up this hostname
    status = HbNPending;

    struct ares_addrinfo_hints hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    ares_getaddrinfo(aresChannel, name, service, &hints, staticAddrInfoCallback, (void *)this);
}

void AresHandler::staticAddrInfoCallback(void *arg, int callbackStatus, int, ares_addrinfo *result)
{
    ((AresHandler *)arg)->callback(callbackStatus, result);
}

void AresHandler::callback(int callbackStatus, ares_addrinfo *result)
{
    if (callbackStatus == ARES_EDESTRUCTION)
        return;
    if (callbackStatus != ARES_SUCCESS)
    {
        logDebugMessage(1,"Player [%d] failed to resolve: error %d\n", index,
                        callbackStatus);
        status = Failed;
    }
    else if (status == HbNPending)
    {
        memcpy(&hostAddr, result->nodes->ai_addr, result->nodes->ai_addrlen);
        ares_freeaddrinfo(result);
        status = HbNSucceeded;
    }
}

const char *AresHandler::getHostname()
{
    return hostName.c_str();
}

AresHandler::ResolutionStatus AresHandler::getHostAddress(in_addr *clientAddr)
{
    if (status == HbNSucceeded)
        memcpy(clientAddr, &hostAddress, sizeof(hostAddress));
    return status;
}

AresHandler::ResolutionStatus AresHandler::getHostAddr(sockaddr_in6 *_hostAddr)
{
    if (status == HbNSucceeded)
        memcpy(_hostAddr, &hostAddr, sizeof(hostAddr));
    return status;
}

void AresHandler::setFd(fd_set *read_set, fd_set *write_set, int &maxFile)
{
    if (aresFailed)
        return;
    int aresMaxFile = ares_fds(aresChannel, read_set, write_set) - 1;
    if (aresMaxFile > maxFile)
        maxFile = aresMaxFile;
}

void AresHandler::process(fd_set *read_set, fd_set *write_set)
{
    if (aresFailed)
        return;
    ares_process(aresChannel, read_set, write_set);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
