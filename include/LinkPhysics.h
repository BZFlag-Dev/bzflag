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
#include <set>
#include <map>
#include <iostream>

// common headers
#include "vectors.h"


class LinkPhysics {
  public:
    enum BlockTest {
      ShotSpeedTest = (1 << 0),
      TankSpeedTest = (1 << 1),
      ShotAngleTest = (1 << 2),
      TankAngleTest = (1 << 3),
      ShotTeamTest  = (1 << 4),
      TankTeamTest  = (1 << 5),
      ShotFlagTest  = (1 << 6),
      TankFlagTest  = (1 << 7),
      ShotVarTest   = (1 << 8),
      TankVarTest   = (1 << 9)
    };

  public:
    LinkPhysics();
    ~LinkPhysics();

    void finalize();

    uint16_t getTestBits() const { return testBits; }

    bool operator<(const LinkPhysics&) const;
    bool operator==(const LinkPhysics&) const;

    int   packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  public:
    uint16_t testBits;

    fvec3 shotSrcPosScale;
    fvec3 shotSrcVelScale;
    fvec3 shotDstVelOffset;
    bool  shotSameSpeed;

    fvec3 tankSrcPosScale;
    fvec3 tankSrcVelScale;
    fvec3 tankDstVelOffset;
    bool  tankSameSpeed;

    // angles in radians
    bool  tankForceAngle;  // for tankAngle
    bool  tankForceAngVel; // for tankAngVel
    float tankAngle;
    float tankAngleOffset;
    float tankAngleScale;
    float tankAngVel;
    float tankAngVelOffset;
    float tankAngVelScale;

    // speed blocks
    float shotMinSpeed;
    float shotMaxSpeed;
    float tankMinSpeed;
    float tankMaxSpeed;

    // angle blocks
    float shotMinAngle;
    float shotMaxAngle;
    float tankMinAngle;
    float tankMaxAngle;

    // team blocks
    uint8_t shotBlockTeams;
    uint8_t tankBlockTeams;

    // flag blocks
    std::set<std::string> shotBlockFlags;
    std::set<std::string> tankBlockFlags;

    // BZDB blocks
    std::string shotBlockVar;
    std::string tankBlockVar;

    // Pass messages
    std::string shotPassText;
    std::string tankPassText;
};


#endif //LINK_PHYSICS_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
