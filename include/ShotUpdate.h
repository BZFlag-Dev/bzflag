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

#include "common.h"

// system headers
#include <math.h>

// common headers
#include "Address.h"
#include "Flag.h"
#include "vectors.h"


const int ShotUpdatePLen = PlayerIdPLen + 32;
const int FiringInfoPLen = ShotUpdatePLen + 10;


class BaseLocalPlayer;
class BufferedNetworkMessage;


struct ShotUpdate {
  public:
    void*		pack(void*) const;
    void		pack(BufferedNetworkMessage *msg) const;
    void*		unpack(void*);

  public:
    PlayerId		player;			// who's shot
    uint16_t		id;			// shot id unique to player
    fvec3		pos;			// shot position
    fvec3		vel;			// shot velocity
    float		dt;			// time shot has existed
    TeamColor		team;
};


struct FiringInfo {
  public:
    FiringInfo();

    void*		pack(void*) const;
    void		pack(BufferedNetworkMessage *msg) const;
    void*		unpack(void*);
    void*		unpackW(void*);

  public:
    float		timeSent;
    ShotUpdate		shot;
    ShotType		shotType;
    FlagType*		flagType;			// flag when fired
    float		lifetime;		// lifetime of shot (s)
};


#endif // BZF_SHOT_UPDATE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
