/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LINK_DEFINITION_H
#define LINK_DEFINITION_H

#include "common.h"

// system headers
#include <string>
#include <vector>
#include <iostream>

// common headers
#include "game/LinkPhysics.h"


class LinkDef {
  public:
    LinkDef();
    LinkDef(const LinkDef& def);
    ~LinkDef();

    void addSrc(const std::string& src);
    void addDst(const std::string& src);

    LinkDef prepend(const std::string& prefix);

    const std::vector<std::string>& getSrcs() const { return srcs; }
    const std::vector<std::string>& getDsts() const { return dsts; }

    int packSize() const;
    void* pack(void* buf) const;
    void* unpack(void* buf);

    void print(std::ostream& out, const std::string& indent) const;

  public:
    std::vector<std::string> srcs;
    std::vector<std::string> dsts;
    LinkPhysics physics;
};


typedef std::vector<LinkDef> LinkDefVec;


#endif //LINK_DEFINITION_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
