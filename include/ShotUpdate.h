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

/*
 * ShotUpdate:
 *	Encapsulates info needed to update a shot on remote
 *	hosts. Can be packed for transmission on the net.
 *
 * FiringInfo:
 *	Encapsulates info needed to create RemoteShotPath.
 *	Can be packed for transmission on the net.
 */

#ifndef	BZF_SHOT_UPDATE_H
#define	BZF_SHOT_UPDATE_H

#include <math.h>
#include "common.h"
#include "Address.h"
#include "Flag.h"

const int		ShotUpdatePLen = PlayerIdPLen + 30;
const int		FiringInfoPLen = ShotUpdatePLen + 6;

class BaseLocalPlayer;

struct ShotUpdate {
  public:
    void*		pack(void*) const;
    void*		unpack(void*);

  public:
    PlayerId		player;			// who's shot
    uint16_t		id;			// shot id unique to player
    float		pos[3];			// shot position
    float		vel[3];			// shot velocity
    float		dt;			// time shot has existed
	TeamColor   team;
};

struct FiringInfo {
  public:
			FiringInfo();
			FiringInfo(const BaseLocalPlayer&, int id);

    void*		pack(void*) const;
    void*		unpack(void*);

  public:
    ShotUpdate		shot;
    FlagDesc*		flag;			// flag when fired
    float		lifetime;		// lifetime of shot (s)
};

#endif // BZF_SHOT_UPDATE_H
// ex: shiftwidth=2 tabstop=8
