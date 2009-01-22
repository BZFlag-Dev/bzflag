/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
 *	Encapsulates an OpenGL (point or directional) light source.
 */

#ifndef	BZF_LINK_MANAGER_H
#define	BZF_LINK_MANAGER_H

// common goes first
#include "common.h"

// system headers
#include <string>
#include <vector>
#include <map>
#include <iostream>

class LinkManager {
  public:
    typedef struct {
      std::string src;
      std::string dst;
    } LinkNameSet;

    typedef struct {
      std::vector<int> dsts;
    } LinkNumberSet;

  public:
    LinkManager();
    ~LinkManager();

    void clear();

    void addLink(int src, int dst);
    void addLink(const std::string& src, const std::string& dst);

    void doLinking();

    int getTeleportTarget(int source) const;
    int getTeleportTarget(int source, unsigned int seed) const;

    int getTeleportIndex(const std::string& name) const;    

    const LinkNumberSet* getLinkDsts(int linkID) const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    void makeLinkName(int number, std::string& name);
    void findTelesByName(const std::string& name,
                         std::vector<int>& list) const;

  private:
    std::vector<LinkNameSet>   linkNames;
    std::vector<LinkNumberSet> linkNumbers;
    std::map<std::string, int> teleNameMap;
};


#endif // BZF_LINK_MANAGER_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
