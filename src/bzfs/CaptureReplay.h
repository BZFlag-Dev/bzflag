/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CAPTURE_REPLAY_H__
#define __CAPTURE_REPLAY_H__

#include "bzfs.h"

// I tried using classes, and I almost hurled.
// Here's the compromise. Please note that no
// private data is visible in the header file. 
// If someone really feels the need to use the
// dot operator, like Replay.init(), I'll make
// some global structures with a bunch of
// pointers to functions.

namespace Capture {
  extern bool init ();
  extern bool kill ();

  extern bool start ();
  extern bool stop (); 
  extern bool setSize (int Mbytes);  // set max size, in Mbytes
  extern bool setRate (int seconds); // set state update rate
  extern bool saveFile (const char *filename); // unbuffered save
  extern bool saveBuffer (const char *filename);
  extern bool sendStats (int playerIndex);
  
  extern bool enabled ();
  extern int getSize (); // returned in bytes, _not_ Mbytes
  extern int getRate ();
  extern const char * getFileName ();

  extern bool addPacket (uint16_t code, int len, const void * data,
                         bool fake = false); // fake used internally
};

namespace Replay {
  extern bool init (); // must be done before any players join
  extern bool kill ();

  extern bool sendFileList (int playerIndex);
  extern bool loadFile (const char *filename);
  extern bool play ();
  extern bool skip (int seconds); // forward or backwards
  
  extern bool enabled ();
  extern bool replaying ();

  extern int getMaxBytes ();
  extern const char * getFileName ();

  extern float nextTime ();
  extern bool sendPackets ();
};

// Some notes:
//
// - Any packets that get broadcast are buffered. Look for the 
//   Capture::addPacket() hook in broadcastMessage(). For now,
//   it will not be mainting any information with regards to the
//   state of the game during replay. It'll just be firing the
//   packets back out the way that they came.
//
// - Player and Flag states have to be saved when a capture is
//   started. If not, how will you know what the field started
//   like?
//
// - We have to watch for collisions between the PlayerID's that
//   are being sent by the replay, and the PlyaerIDs of those
//   watching. For now, I'm just going to for replay watching
//   observer ids to be above 50. We could also do a PlayerID
//   mapping, but then you'd have to dig into any packet that
//   uses a PlayerID.
//
// - To avoid having to track game state, we're simply going to
//   take snapshots of the player and flag states at a specific
//   time interval. These state packets will only be saved if
//   there have been broadcasted packets, so that idle servers
//   won't neccesarily have massive files if there saving straight
//   to a file.
//
// - Initially, I'm just going to dump everything to a big file,
//   until it hits the MaxBytes size. Shortly thereafter, I plan
//   on implementing a doubly linked list to maintain the captured
//   info in memory. You'll then be able to take a snapshot at any
//   point in a game. Working out the initial state is going to be
//   a bit of a problem...
//
// - Ideally, it would be nice to be able to set replay mode for
//   individual players. Then, admins would be able to review events
//   that happened 30 seconds ago without having to disconnect from
//   the server.
//

#endif  /* __CAPTURE_REPLAY_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
