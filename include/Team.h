/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Team:
 *   Encapsulates information about a team
 */

#ifndef	BZF_TEAM_H
#define	BZF_TEAM_H

#include "common.h"
#include "global.h"

const int		TeamPLen = 10;

struct Team {
  public:
    void*		pack(void*) const;
    void*		unpack(void*);

    static const char*	getName(TeamColor); // const
    static const float*	getTankColor(TeamColor); // const
    static const float*	getRadarColor(TeamColor); // const

    static void		setColors(TeamColor,
				const float* tank,
				const float* radar);

  public:
    unsigned short	size;			// num players on team
    unsigned short	activeSize;		// num non-observers
    unsigned short	won;			// wins by team members
    unsigned short	lost;			// losses by team members

    static float	tankColor[NumTeams][3];
    static float	radarColor[NumTeams][3];
};

#endif // BZF_TEAM_H
