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

#ifndef __SERVERLIST_H__
#define __SERVERLIST_H__

//Someone checked in code without a file called ServerList.h, that was needed.
//Whoever this is, please check in the real file.
//This is just a faux file so the thing stinkin' builds

#include <vector>
#include "ServerItem.h"

class ServerList
{
public:
    unsigned int size();
    int updateFromCache();
    void startServerPings();
    boolean searchActive();
    void checkEchos();
    std::vector<ServerItem> getServers();
};

inline unsigned int ServerList::size() { return 0; }
inline int ServerList::updateFromCache() { return 0; }
inline void ServerList::startServerPings() {}
inline void ServerList::checkEchos() {}
inline boolean ServerList::searchActive() {return true;}

#endif