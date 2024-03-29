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

/* OpenGLLight:
 *  Encapsulates an OpenGL (point or directional) light source.
 */

#ifndef BZF_LINK_MANAGER_H
#define BZF_LINK_MANAGER_H

// common goes first
#include "common.h"

// system headers
#include <string>
#include <vector>
#include <iostream>

class LinkManager
{

public:

    LinkManager();
    ~LinkManager();

    void clear();

    void addLink(int src, int dst);
    void addLink(const std::string& src, const std::string& dst);

    void doLinking();

    int getTeleportTarget(int source) const;
    int getTeleportTarget(int source, unsigned int seed) const;

    int packSize() const;
    void* pack(void*) const;
    const void* unpack(const void*);

    void print(std::ostream& out, const std::string& indent) const;

private:

    void makeLinkName(int number, std::string& name);
    void findTelesByName(const std::string& name,
                         std::vector<int>& list) const;

private:

    typedef struct
    {
        std::string src;
        std::string dst;
    } LinkNameSet;

    typedef struct
    {
        std::vector<int> dsts;
    } LinkNumberSet;

    std::vector<LinkNameSet>    linkNames;
    std::vector<LinkNumberSet>  linkNumbers;
};



#endif // BZF_LINK_MANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
