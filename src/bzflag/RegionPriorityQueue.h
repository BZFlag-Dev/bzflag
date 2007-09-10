/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	__REGIONPRIORITYQUEUE_H__
#define	__REGIONPRIORITYQUEUE_H__

/* local interface headers */
#include "Region.h"


class RegionPriorityQueue {
 public:
  RegionPriorityQueue();
  ~RegionPriorityQueue();

  void insert(BzfRegion* region, float priority);
  BzfRegion* remove();
  void removeAll();
  bool isEmpty() const;

 private:
  struct Node {
    public:
     Node(BzfRegion* region, float priority);
    public:
     Node* next;
     BzfRegion*	region;
     float priority;
  };

 private:
  Node* head;
};


#endif /* __REGIONPRIORITYQUEUE_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
