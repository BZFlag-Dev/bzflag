/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// Provide BZFS with a list server connection

#ifndef __LISTSERVERCONNECTION_H__
#define __LISTSERVERCONNECTION_H__

#include <string>
#include "Address.h"
#include "Ping.h"
#include "TimeKeeper.h"

// FIXME - references bzfs.cxx
extern int NotConnected;

class ListServerLink {
public:
    // c'tor will fill list and local server information variables and do an initial ADD
    ListServerLink(std::string listServerURL, std::string publicizedAddress, std::string publicizedTitle);
    // c'tor with no arguments called when we don't want to use a list server.
    ListServerLink();
    // d'tor will REMOVE server and close connection
    ~ListServerLink();

    enum MessageType {NONE, ADD, REMOVE} nextMessageType;
    enum Phase {CONNECTING, WRITTEN} phase;
    TimeKeeper lastAddTime;

    // connection functions
    bool isConnected();
    void queueMessage(MessageType type);
    void sendQueuedMessages();
    void read();

//  FIXME - linkSocket shouldn't have to be public.  Write functions to avoid directly accessing the socket.
    int linkSocket;

private:
    // list server information
    Address address;
    int port;
    std::string hostname;
    std::string pathname;

    // connect/disconnect
    void openLink();
    void closeLink();

    // local server information
    bool publicizeServer;
    std::string publicizeAddress;
    std::string publicizeDescription;

    // messages to send, used by sendQueuedMessages
    void addMe(PingPacket pingInfo, std::string publicizedAddress, std::string publicizedTitle);
    void removeMe(std::string publicizedAddress);
    void sendMessage(std::string message);
};

inline bool ListServerLink::isConnected()
{
  return (linkSocket != NotConnected);
}

#endif //__LISTSERVERCONNECTION_H__

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
