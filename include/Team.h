/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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

#ifndef	BZF_TEAM_H
#define	BZF_TEAM_H

#include "common.h"
#include "global.h"
#include <string>

const int		TeamPLen = 10;

struct Team {
  public:
    Team();

    void*		pack(void*) const;
    const void*		unpack(const void*);

    static const std::string	getImagePrefix(TeamColor); // const
    static const char*		getName(TeamColor); // const
    static TeamColor		getTeam(const std::string &name); // const
    static const float*		getTankColor(TeamColor); // const
    static const float*		getRadarColor(TeamColor team); // const
    static const float*		getShotColor(TeamColor team); // const
    static const std::string	getAnsiCode(TeamColor team); // const
    static bool			isColorTeam(TeamColor); // const

    static void		setColors(TeamColor,
				  const float* tank,
				  const float* radar);
    static void		updateShotColors();

  public:
    unsigned short	size;			// num players on team

    short		getWins() const { return won; }
    short		getLosses() const { return lost; }
    void		setWins(short v) { won = v; }
    void		setLosses(short v) { lost = v; }

    static float	tankColor[NumTeams][3];
    static float	radarColor[NumTeams][3];
    static float	shotColor[NumTeams][3];

  private:
    unsigned short	won;			// wins by team members
    unsigned short	lost;			// losses by team members

    static float	addBrightness(const float color); // const

};


#endif // BZF_TEAM_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
