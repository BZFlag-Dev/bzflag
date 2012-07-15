/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CAPTURE_REPLAY_H__
#define __CAPTURE_REPLAY_H__


#include "common.h"

const int ReplayObservers = 16;

enum ReplayPacketMode {
  RealPacket   = 0, // broadcasted during replay
  StatePacket  = 1, // broadcasted to those you aren't yet stateful
  UpdatePacket = 2, // never broadcasted (only for replay use)
  HiddenPacket = 3  // never broadcasted (stored for admin. purposes)
};

namespace Record {
  extern bool init ();
  extern bool kill ();

  extern bool setDirectory (const char *dirname);
  extern const char* getDirectory ();

  extern bool start (int playerIndex);
  extern bool stop (int playerIndex);
  extern bool setSize (int playerIndex, int Mbytes);  // set max size, in Mbytes
  extern bool setRate (int playerIndex, int seconds); // set state update rate
  extern bool saveFile (int playerIndex, const char *filename); // unbuffered save
  extern bool saveBuffer (int playerIndex, const char *filename, int seconds);
  extern bool sendStats (int playerIndex);

  extern bool enabled ();

  extern bool getAllowFileRecs();
  extern void setAllowFileRecs(bool value);

  extern bool addPacket (uint16_t code, int len, const void * data,
			 uint16_t mode = RealPacket);

  extern void sendHelp (int playerIndex);
}

namespace Replay {
  extern bool init (); // must be done before any players join
  extern bool kill ();

  extern bool sendFileList (int playerIndex, const char* options);
  extern bool loadFile (int playerIndex, const char *filename);
  extern bool unloadFile (int playerIndex);
  extern bool play (int playerIndex);
  extern bool loop (int playerIndex);
  extern bool sendStats (int playerIndex);
  extern bool skip (int playerIndex, int seconds); // 0 secs jumps to next packet
  extern bool pause (int playerIndex);

  extern bool enabled ();
  extern bool playing ();

  extern float nextTime ();
  extern bool sendPackets ();

  extern void sendHelp (int playerIndex);

  enum ReplayListSort {
    SortNone = 0,
    SortByName = 1,
    SortByTime = 2
  };
}

// Some notes:
//
// - Any packets that get broadcast are buffered. Look for the
//   Record::addPacket() hook in broadcastMessage(). For now,
//   it will not be mainting any information with regards to the
//   state of the game during replay. It'll just be firing the
//   packets back out the way that they came.
//
// - We have to watch for collisions between the PlayerID's that
//   are being sent by the replay, and the PlayerIDs of those
//   watching. For now, I'm just going to force replay watching
//   observer ids to be above 200. We could also do a PlayerID
//   mapping, but then you'd have to dig into every packet that
//   uses a PlayerID (ick).
//
// - To avoid having to track game state, we're simply going to
//   take snapshots of the player and flag states periodically.
//   These state packets will only be saved if there have been
//   broadcasted packets, so that idle servers won't necessarily
//   have massive files if they're saving straight to a file.
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
