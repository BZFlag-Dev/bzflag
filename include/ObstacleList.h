/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_OBSTACLE_LIST_H
#define	BZF_OBSTACLE_LIST_H

#include "common.h"


class Obstacle;

class ObstacleList {
  public:
    ObstacleList();
    ~ObstacleList();
    
//    void add(Obstacle* obs);
    void clear();
    void tighten();
    void push_back(Obstacle* obs);
    void erase(unsigned int index);
    
    unsigned int size() const;
    Obstacle* operator[](int index) const;

  private:
    unsigned int listSize;
    unsigned int listCount;
    Obstacle** list;
};

inline unsigned int ObstacleList::size() const
{
  return listCount;
}
inline Obstacle* ObstacleList::operator[](int index) const
{
  return list[index];
}
inline void ObstacleList::erase(unsigned int index)
{
  if ((index < listCount) && (listCount > 0)) {
    listCount--;
    list[index] = list[listCount]; // order is not preserved
  }
  return;
}

#endif // BZF_OBSTACLE_LIST_H


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

