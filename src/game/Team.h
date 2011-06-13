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

/*
 * Team:
 *   Encapsulates information about a team
 */

#ifndef BZF_TEAM_H
#define BZF_TEAM_H

#include "common.h"

#include <string>

#include "global.h"
#include "vectors.h"

class NetMessage;


const int TeamPLen = 10;


struct Team {
  public:
    Team();

    void* pack(void*) const;
    void  pack(NetMessage& netMsg) const;
    void* unpack(void*);

    static const std::string getImagePrefix(TeamColor);  // const
    static const char*  getName(TeamColor);              // const
    static const char*  getShortName(TeamColor);         // const
    static TeamColor  getTeam(const std::string name); // const
    static const fvec4& getTankColor(TeamColor);         // const
    static const fvec4& getRadarColor(TeamColor team);   // const
    static bool         isColorTeam(TeamColor);          // const

    static void setColors(TeamColor, const fvec4& tank, const fvec4& radar);

    static bool areFoes(TeamColor team1, TeamColor team2, GameType style); //const

  public:
    unsigned short size;  // num players on team
    unsigned short won;   // wins by team members
    unsigned short lost;  // losses by team members

    static fvec4 tankColor[NumTeams];
    static fvec4 radarColor[NumTeams];

};


#endif // BZF_TEAM_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
