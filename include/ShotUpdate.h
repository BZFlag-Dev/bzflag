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
 * ShotUpdate:
 *	Encapsulates info needed to update a shot on remote
 *	hosts. Can be packed for transmission on the net.
 */

#ifndef	BZF_SHOT_UPDATE_H
#define	BZF_SHOT_UPDATE_H

const int		ShotUpdatePLen = PlayerIdPLen + 30;
const int		FiringInfoPLen = ShotUpdatePLen + 6;

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
};

#endif // BZF_SHOT_UPDATE_H
// ex: shiftwidth=2 tabstop=8
