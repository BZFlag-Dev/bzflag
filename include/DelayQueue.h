/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED `AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _DELAY_QUEUE_
#define _DELAY_QUEUE_

#include "TimeKeeper.h"


typedef struct DelayNode {
  DelayNode *next;
  int length;
  void *data;
  TimeKeeper sendtime;
} DelayNode;


class DelayQueue {
  public:
    DelayQueue ();
    ~DelayQueue ();

    void init();
    void dequeuePackets();

    bool addPacket (int length, const void *data, float time);
    bool getPacket (int *length, void **data); // true if packet available

    float nextPacketTime ();

  private:
    DelayNode *input;
    DelayNode *output;
};

#endif // _DELAY_QUEUE_
