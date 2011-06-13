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

#ifndef __CUSTOMWEAPON_H__
#define __CUSTOMWEAPON_H__

/* interface header */
#include "WorldFileLocation.h"

/* system interface headers */
#include <iostream>
#include <vector>

/* common interface headers */
#include "common/BzTime.h"
#include "game/Flag.h"

/* local interface headers */
#include "WorldInfo.h"
#include "bzfs/WorldEventManager.h"


class MeshObstacle;


class CustomWeapon : public WorldFileLocation {
  public:
    CustomWeapon(const MeshObstacle* mesh = NULL);

    bool readLine(const std::string& cmd, const std::string& line);

    virtual bool read(const char* cmd, std::istream&);
    virtual void writeToWorld(WorldInfo*) const;
    virtual bool usesGroupDef() { return false; }

    static const float minWeaponDelay;

  protected:
    FlagType* type;

    float initdelay;
    std::vector<float> delay;

    float tilt;
    TeamColor teamColor;

    int eventTeam;
    bz_eEventType triggerType;

    static BzTime sync;

    const MeshObstacle* mesh;
    int posVertex;
    int dirNormal;
};

#endif  /* __CUSTOMWEAPON_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
