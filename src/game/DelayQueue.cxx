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

#include "common.h"
#include "global.h"
#include "DelayQueue.h"


DelayQueue::DelayQueue ()
{
  init();
}


DelayQueue::~DelayQueue ()
{
  dequeuePackets();
}


void DelayQueue::init ()
{
  input = NULL;
  output = NULL;
}


void DelayQueue::dequeuePackets ()
{
  DelayNode *dn = output;
  DelayNode *tmp;

  while (dn != NULL) {
    free (dn->data);
    tmp = dn;
    dn = dn->next;
    delete tmp;
  }

  input = NULL;
  output = NULL;
}


bool DelayQueue::addPacket (int length, const void *data, float delay)
{
  DelayNode *dn = new DelayNode;

  dn->length = length;
  dn->data = malloc (length);
  memcpy (dn->data, data, length);

  dn->sendtime = TimeKeeper::getCurrent();
  dn->sendtime += delay;

  dn->next = NULL;

  if (output == NULL) {
    output = dn;
  }

  if (input != NULL) {
    input->next = dn;
  }

  input = dn;

  return true;
};


bool DelayQueue::getPacket (int *length, void **data)
{
  if (nextPacketTime() > 0.0f) {
    return false;
  }

  // output is not NULL (nextPacketTime() checks this)

  DelayNode *dn = output;

  *length = dn->length;
  *data = dn->data;

  if (dn->next == NULL) {
    input = NULL;
  }
  output = dn->next;

  delete dn;

  return true;
};


float DelayQueue::nextPacketTime ()
{
  if (output == NULL) {
    return +Infinity;
  }
  else {
    return output->sendtime - TimeKeeper::getCurrent();
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
