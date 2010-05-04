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

#ifndef	BZF_PLAYERSTATE_H
#define	BZF_PLAYERSTATE_H

#include "common.h"
#include "Address.h"
#include "BufferedNetworkMessage.h"
#include "TimeKeeper.h"
#include "vectors.h"

// 58 bytes
const int PlayerUpdatePLenMax =
  sizeof(double)	+ // timestamp
  PlayerIdPLen		+ // player id
  sizeof(int32_t)	+ // order
  sizeof(int16_t)	+ // status
  sizeof(fvec3)		+ // position			(or int16_t * 3)
  sizeof(fvec3)		+ // velocity			(or int16_t * 3)
  sizeof(float)		+ // angle			(or int16_t)
  sizeof(float)		+ // angular velocity		(or int16_t)
  sizeof(int16_t)	+ // jump jets			(conditional)
  sizeof(int32_t)	+ // physics driver		(conditional)
  sizeof(int16_t)	+ // user speed			(conditional)
  sizeof(int16_t)	+ // user angular velocity	(conditional)
  sizeof(uint8_t);	  // sounds			(conditional)


class PlayerState
{
  public:
    enum PStatus {			// bit masks
      DeadStatus	= 0,		// not alive, not paused, etc.
      Alive		= (1 << 0),	// player is alive
      Exploding       	= (1 << 1),	// currently blowing up
      Teleporting	= (1 << 2),	// teleported recently
      FlagActive	= (1 << 3),	// flag special powers active
      CrossingWall	= (1 << 4),	// tank crossing building wall
      Falling		= (1 << 5),	// tank accel'd by gravity
      OnDriver		= (1 << 6),	// tank is on a physics driver
      UserInputs	= (1 << 7),	// user speed and angvel are sent
      JumpJets		= (1 << 8),	// tank has jump jets on
      PlaySound		= (1 << 9),	// play one or more sounds
      PhantomZoned	= (1 << 10),	// tank is phantom-zoned
      InBuilding	= (1 << 11),	// tank inside a building
      BackedOff		= (1 << 12)	// tank hit building and backed off
    };

    enum PStatusSounds {
      NoSounds		= 0,
      JumpSound		= (1 << 0),
      WingsSound	= (1 << 1),
      BounceSound	= (1 << 2)
    };

  public:
    PlayerState();
    void* pack(void*, uint16_t& code, bool increment = true);
    void  pack(BufferedNetworkMessage *msg, uint16_t& code, bool increment = true);
    void* unpack(void*, uint16_t code);

  public:
    long  order;	// packet ordering
    short status;	// see PStatus enum
    fvec3 pos;		// position of tank
    fvec3 velocity;	// velocity of tank
    float azimuth;	// orientation of tank
    float angVel;	// angular velocity of tank
    int   phydrv;	// physics driver

    fvec3 apparentVelocity;	// velocity of tank as derived from
                                // its last positional update

    TimeKeeper lastUpdateTime;	// the time of the last update

    // the following are to be used only for drawing
    float userSpeed;		// user's desired angular velocity
    float userAngVel;		// angular velocity of tank
    float jumpJetsScale;	// size of the jump jets

    // used to avoid awkward remote bouncy sounds
    uint8_t sounds;		// for playing sounds
};


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
