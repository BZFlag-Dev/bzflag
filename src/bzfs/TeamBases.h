/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef __TEAMBASES_H__
#define __TEAMBASES_H__

#include <vector>
#include "global.h"

class TeamBases;

typedef std::map<int, TeamBases> BasesList;

class TeamBases
{
public:
  TeamBases(TeamColor team = RedTeam, bool initDefault = false);
  void addBase( const float *position, const float *size, float rotation, const float *safetyZone );
  int size() const;
  TeamColor getTeam() const;
  const float *TeamBases::getBasePosition( int base ) const;
  const float *TeamBases::getBaseSize( int base ) const;
  void *pack( void *buf ) const;
  float findBaseZ( float x, float y, float z ) const;
  void getRandomPosition( float &x, float &y, float &z ) const;
  void getSafetyZone( float &x, float &y, float &z ) const;



private:
  struct TeamBase
  {
    TeamBase() {}
    TeamBase(const float *pos, const float *siz, float rot, const float *safety);
    float position[3];
    float size[3];
    float rotation;
    float safetyZone[3];
  };
  typedef std::vector<TeamBase> TeamBaseList;

  TeamBaseList teamBases;
  TeamColor    color;
};



#endif
