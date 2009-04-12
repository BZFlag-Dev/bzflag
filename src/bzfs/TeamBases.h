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

#ifndef __TEAMBASES_H__
#define __TEAMBASES_H__

#include "common.h"

// system headers
#include <vector>
#include <map>

// common headers
#include "global.h"
#include "vectors.h"


class TeamBase
{ // This class represents one base
public:
  TeamBase() {}
  TeamBase(const fvec3& pos, const fvec3& siz, float rot);
  void getRandomPosition(float &x, float &y, float &z) const;
  fvec3 position;
  fvec3 size;
  float rotation;
};


class TeamBases
{ // This class represents all the bases for one team
public:

  TeamBases();
  TeamBases(TeamColor team, bool initDefault = false);
  void addBase(const fvec3& position, const fvec3& size, float rotation);
  int size() const;
  TeamColor getTeam() const;
  const fvec3& getBasePosition(int base) const;
  float findBaseZ(float x, float y, float z) const;
  const TeamBase& getRandomBase( int id );

private:
  typedef std::vector<TeamBase> TeamBaseList;

  TeamBaseList teamBases;
  TeamColor    color;
};

typedef std::map<int, TeamBases> BasesList;


#endif /* __TEAMBASES_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
