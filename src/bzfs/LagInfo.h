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

#ifndef __PINGHANDLER_H__
#define __PINGHANDLER_H__

#include "PlayerInfo.h"

/** This class monitors the lag time for each client
*/
class LagInfo {
public:
  /** A default constructor.
      It needs a pointer to the Player basic Info,     
   */
  LagInfo(PlayerInfo *_info);

  /** Get a printable version of lag statistics
  */
  void        getLagStats(char* msg);
  /** functions to be called whenever a playerUpdate or ping message arrives
   */
  int         updatePingLag(void *buf, bool &warn, bool &kick);
  void        updateLag(float timestamp, bool ooo);
  /** get the ping seqno, if need to send one now!
   */
  int         getNextPingSeqno();
  /** update the latency
   */
  bool        updateLatency(float &waitTime);
  /** set the threshold for warning/kicking
   */
  static void setThreshold(float _threshold, float _max);
private:
  PlayerInfo *info;
  // lag measurement
  float       lagavg;
  float       jitteravg;
  float       lostavg;
  float       lagalpha;
  float       jitteralpha;
  float       lostalpha;
  int         lagcount;
  int         laglastwarn;
  int         lagwarncount;
  bool        pingpending;
  TimeKeeper  nextping;
  TimeKeeper  lastping;
  TimeKeeper  lastupdate;
  int         pingseqno;
  int         pingssent;
  // jitter measurement
  float       lasttimestamp;

  static float threshold;
  static float max;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
