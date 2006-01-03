/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "RegionPriorityQueue.h"


// FIXME -- should use a heap

RegionPriorityQueue::Node::Node(BzfRegion* _region, float _priority) :
  next(0), region(_region), priority(_priority)
{
}

RegionPriorityQueue::RegionPriorityQueue() : head(0)
{
}

RegionPriorityQueue::~RegionPriorityQueue()
{
  removeAll();
}

void RegionPriorityQueue::insert(BzfRegion* region, float priority)
{
  Node* node = new Node(region, priority);
  if (!head || priority < head->priority) {
    node->next = head;
    head = node;
  }
  else {
    Node* scan = head;
    while (scan->next && priority >= scan->next->priority)
      scan = scan->next;
    node->next = scan->next;
    scan->next = node;
  }
}

BzfRegion* RegionPriorityQueue::remove()
{
  Node* tmp = head;
  head = head->next;
  BzfRegion* region = tmp->region;
  delete tmp;
  return region;
}

void RegionPriorityQueue::removeAll()
{
  while (head) {
    Node* next = head->next;
    delete head;
    head = next;
  }
  head = 0;
}

bool RegionPriorityQueue::isEmpty() const
{
  return (head == 0);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
