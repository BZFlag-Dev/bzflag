/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
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

#ifndef BZF_TEAM_H
#define BZF_TEAM_H

#include "common.h"
#include "global.h"

class BzfString;

const int				TeamPLen = 10;

class Team {
public:
	void*				pack(void*) const;
	void*				unpack(void*);

	static void			init();
	static BzfString	getName(TeamColor);
	static TeamColor	getEnum(const BzfString& name);
	static const float*	getTankColor(TeamColor);
	static const float*	getRadarColor(TeamColor);

private:
	static void			onColorChange(const BzfString&, void*);

public:
	unsigned short		size;						// num players on team
	unsigned short		activeSize;				// num non-observers
	unsigned short		won;						// wins by team members
	unsigned short		lost;						// losses by team members

private:
	static float		tankColor[NumTeams][3];
	static float		radarColor[NumTeams][3];
};

#endif // BZF_TEAM_H
