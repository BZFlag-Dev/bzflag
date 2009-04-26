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

#ifndef LINK_PHYSICS_H
#define LINK_PHYSICS_H

#include "common.h"

// system headers
#include <string>
#include <vector>
#include <map>
#include <iostream>

// common headers
#include "vectors.h"


class LinkPhysics {
  public:
    LinkPhysics();
    ~LinkPhysics();

    bool operator<(const LinkPhysics&) const;
    bool operator==(const LinkPhysics&) const;

    int   packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  public:
    fvec3 shotSrcPosScale;
    fvec3 shotSrcVelScale;
    fvec3 shotDstVel;
    bool  shotSameSpeed;

    fvec3 tankSrcPosScale;
    fvec3 tankSrcVelScale;
    fvec3 tankDstVel;
    bool  tankSameSpeed;

    // angles in radians
    float tankAngleOffset;
    float tankAngVelOffset;
    bool  tankForceAngle;
    bool  tankForceAngVel;
    float tankAngle;
    float tankAngVel;
    float tankAngVelScale;
};


#endif //LINK_PHYSICS_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
