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
 * ShotUpdate:
 *  Encapsulates info needed to update a shot on remote
 *  hosts. Can be packed for transmission on the net.
 *
 * FiringInfo:
 *  Encapsulates info needed to create RemoteShotPath.
 *  Can be packed for transmission on the net.
 */

#ifndef BZF_SHOT_UPDATE_H
#define BZF_SHOT_UPDATE_H

#include "common.h"

// system headers
#include <math.h>

// common headers
#include "game/global.h"
#include "vectors.h"
#include "game/Flag.h"


class NetMessage;
class BaseLocalPlayer;


struct ShotUpdate {
  public:
    int     packSize() const;
    void*   pack(void*) const;
    void    pack(NetMessage& netMsg) const;
    void*   unpack(void*);

  public:
    PlayerId    player;     // who's shot
    uint16_t    id;     // shot id unique to player
    fvec3   pos;      // shot position
    fvec3   vel;      // shot velocity
    float   dt;     // time shot has existed
    TeamColor   team;
};


struct FiringInfo {
  public:
    FiringInfo();

    int     packSize() const;
    void*   pack(void*) const;
    void    pack(NetMessage& netMsg) const;
    void*   unpack(void*);

  public:
    double    timeSent;
    ShotUpdate    shot;
    ShotType    shotType;
    FlagType*   flagType;     // flag when fired
    float   lifetime;   // lifetime of shot (s)
};


#endif // BZF_SHOT_UPDATE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
