/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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

/* system interface headers */
#include <vector>
#include <map>

/* common interface headers */
#include "global.h"


class TeamBase
{ // This class represents one base
public:
  TeamBase() {}
  TeamBase(const float *pos, const float *siz, float rot);
  void getRandomPosition( float &x, float &y, float &z ) const;
  float position[3];
  float size[3];
  float rotation;
};


class TeamBases
{ // This class represents all the bases for one team
public:

  TeamBases();
  TeamBases(TeamColor team, bool initDefault = false);
  void addBase( const float *position, const float *size, float rotation );
  int size() const;
  TeamColor getTeam() const;
  const float *getBasePosition( int base ) const;
  float findBaseZ( float x, float y, float z ) const;
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
