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
 * main game loop stuff
 */

#ifndef BZF_PLAYING_H
#define BZF_PLAYING_H

#include "common.h"
#include "global.h"

class BzfDisplay;
class BzfWindow;
class PlayerId;
class Player;

Player*					lookupPlayer(const PlayerId& id);
void					startPlaying(BzfDisplay* display, BzfWindow*);

bool					addExplosion(const float* pos,
							float size, float duration);
void					addTankExplosion(const float* pos);
void					addShotExplosion(const float* pos);
void					addShotPuff(const float* pos);

typedef void			(*PlayingCallback)(void*);
void					addPlayingCallback(PlayingCallback, void* data);
void					removePlayingCallback(PlayingCallback, void* data);

#endif // BZF_PLAYING_H
