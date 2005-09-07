/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// implementation header
#include "ObstacleList.h"

// system headers
#include <string.h>
#include <stdlib.h>


static const unsigned int minListSize = 1;


ObstacleList::ObstacleList()
{
  list = NULL;
  clear();
  return;
}


ObstacleList::~ObstacleList()
{
  delete[] list;
  return;
}


void ObstacleList::clear()
{
  delete[] list;
  listCount = 0;
  listSize = minListSize;
  list = new Obstacle*[listSize];
  return;
}


void ObstacleList::push_back(Obstacle* obs)
{
  listCount++;
  if (listCount > listSize) {
    listSize = 2 * listSize;
    Obstacle** tmpList = new Obstacle*[listSize];
    memcpy (tmpList, list, (listCount - 1) * sizeof(Obstacle*));
    delete[] list;
    list = tmpList;
  }
  list[listCount - 1] = obs;
  return;
}


void ObstacleList::tighten()
{
  if ((listSize == listCount) || (listCount < minListSize)) {
    return;
  }
  listSize = listCount;
  Obstacle** tmpList = new Obstacle*[listSize];
  memcpy (tmpList, list, listCount * sizeof(Obstacle*));
  delete[] list;
  list = tmpList;
  return;
}


void ObstacleList::sort(int (*compare)(const void* a, const void* b))
{
  qsort(list, listCount, sizeof(Obstacle*), compare);
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8



