/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// Provide BZFS with a list server connection

#ifndef __LISTSERVERCONNECTION_H__
#define __LISTSERVERCONNECTION_H__

/* inteface header */
#include "cURLManager.h"

/* system headers */
#include <string>

/* common interface headers */
#include "Address.h"
#include "Ping.h"
#include "TimeKeeper.h"

class ListServerLink : private cURLManager {
public:
    // c'tor will fill list and local server information variables and
    // do an initial ADD
    ListServerLink(std::string listServerURL, std::string publicizedAddress,
		   std::string publicizedTitle, std::string advertiseGroups);
    // c'tor with no arguments called when we don't want to use a list server.
    ListServerLink();
    // d'tor will REMOVE server and close connection
    ~ListServerLink();

    enum MessageType {NONE, ADD, REMOVE} nextMessageType;
    TimeKeeper lastAddTime;

    // connection functions
    void queueMessage(MessageType type);

private:
    static const int NotConnected;

    // list server information
    Address address;
    int port;
    std::string hostname;
    std::string pathname;

    // local server information
    Address localAddress;
    bool publicizeServer;
    std::string publicizeAddress;
    std::string publicizeDescription;
    std::string advertiseGroups;

    virtual void finalization(char *data, unsigned int length, bool good);
    std::string verifyGroupPermissions(const std::string& groups);

    // messages to send, used by sendQueuedMessages
    void addMe(PingPacket pingInfo, std::string publicizedAddress,
	       std::string publicizedTitle, std::string advertiseGroups);
    void removeMe(std::string publicizedAddress);
    void sendQueuedMessages();

  bool queuedRequest;
};
#endif //__LISTSERVERCONNECTION_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
