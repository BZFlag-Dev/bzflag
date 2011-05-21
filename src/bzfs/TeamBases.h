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

#ifndef __TEAMBASES_H__
#define __TEAMBASES_H__

#include "common.h"

// system headers
#include <vector>
#include <map>

// common headers
#include "global.h"
#include "vectors.h"


class Obstacle;


class TeamBase {
    // This class represents one base
  public:
    TeamBase() : obstacle(NULL) {}
    TeamBase(const Obstacle* obs) { obstacle = obs; }
    void getTopCenter(fvec3& pos) const;
    void getRandomPosition(fvec3& pos) const;
    const Obstacle* getObstacle() const { return obstacle; }
  private:
    const Obstacle* obstacle;
};


class TeamBases {
    // This class represents all the bases for one team
  public:

    TeamBases();
    TeamBases(TeamColor team) : color(team) {}

    void addBase(const Obstacle* obs);
    int size() const;
    TeamColor getTeam() const;
    const fvec3& getBasePosition(int base) const;
    float findBaseZ(const fvec3& pos) const;
    const TeamBase& getRandomBase();

  private:
    typedef std::vector<TeamBase> TeamBaseList;

    TeamColor    color;
    TeamBaseList teamBases;
};

typedef std::map<int, TeamBases> BasesList;


#endif /* __TEAMBASES_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
