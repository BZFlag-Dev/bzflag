/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Extents
 *	Encapsulates the data to record the minimum and maximum
 *      values along each axis of an axis-aligned bounding box.
 */

#ifndef	BZF_EXTENTS_H
#define	BZF_EXTENTS_H

#include "common.h"


class Extents {
  public:
    Extents();
    Extents(const float mins[3], const float maxs[3]);

    void reset();

    Extents& operator=(const Extents&);
    void set(const float mins[3], const float maxs[3]);

    void expandToBox(const Extents& box); // expand to contain the box
    void expandToPoint(const float[3]); // expand to contain the point
    void addMargin(float margin);     // widen the extents by "margin"

    float getWidth(int axis) const;

    bool touches(const Extents& orig) const;
    bool contains(const Extents& orig) const;

  private:
    // force passing by reference
    Extents(const Extents& orig);

  public:
    float mins[3];
    float maxs[3];
};


inline void Extents::reset()
{
  mins[0] = mins[1] = mins[2] = +MAXFLOAT;
  maxs[0] = maxs[1] = maxs[2] = -MAXFLOAT;
  return;
}


inline void Extents::set(const float _mins[3], const float _maxs[3])
{
  mins[0] = _mins[0];
  mins[1] = _mins[1];
  mins[2] = _mins[2];
  maxs[0] = _maxs[0];
  maxs[1] = _maxs[1];
  maxs[2] = _maxs[2];
  return;
}


inline Extents::Extents()
{
  reset();
  return;
}


inline Extents::Extents(const float _mins[3], const float _maxs[3])
{
  set(_mins, _maxs);
  return;
}


inline Extents& Extents::operator=(const Extents& orig)
{
  mins[0] = orig.mins[0];
  mins[1] = orig.mins[1];
  mins[2] = orig.mins[2];
  maxs[0] = orig.maxs[0];
  maxs[1] = orig.maxs[1];
  maxs[2] = orig.maxs[2];
  return *this;
}


inline void Extents::expandToBox(const Extents& test)
{
  // test mins
  if (test.mins[0] < mins[0]) {
    mins[0] = test.mins[0];
  }
  if (test.mins[1] < mins[1]) {
    mins[1] = test.mins[1];
  }
  if (test.mins[2] < mins[2]) {
    mins[2] = test.mins[2];
  }
  // test maxs
  if (test.maxs[0] > maxs[0]) {
    maxs[0] = test.maxs[0];
  }
  if (test.maxs[1] > maxs[1]) {
    maxs[1] = test.maxs[1];
  }
  if (test.maxs[2] > maxs[2]) {
    maxs[2] = test.maxs[2];
  }
  return;
}


inline void Extents::expandToPoint(const float point[3])
{
  // test mins
  if (point[0] < mins[0]) {
    mins[0] = point[0];
  }
  if (point[1] < mins[1]) {
    mins[1] = point[1];
  }
  if (point[2] < mins[2]) {
    mins[2] = point[2];
  }
  // test maxs
  if (point[0] > maxs[0]) {
    maxs[0] = point[0];
  }
  if (point[1] > maxs[1]) {
    maxs[1] = point[1];
  }
  if (point[2] > maxs[2]) {
    maxs[2] = point[2];
  }
  return;
}


inline bool Extents::touches(const Extents& test) const
{
  if ((mins[0] > test.maxs[0]) || (maxs[0] < test.mins[0]) ||
      (mins[1] > test.maxs[1]) || (maxs[1] < test.mins[1]) ||
      (mins[2] > test.maxs[2]) || (maxs[2] < test.mins[2])) {
    return false;
  }
  return true;
}


inline bool Extents::contains(const Extents& test) const
{
  if ((mins[0] < test.mins[0]) && (maxs[0] > test.maxs[0]) &&
      (mins[1] < test.mins[1]) && (maxs[1] > test.maxs[1]) &&
      (mins[2] < test.mins[2]) && (maxs[2] > test.maxs[2])) {
    return true;
  }
  return false;
}


inline void Extents::addMargin(float margin)
{
  // subtract from the mins
  mins[0] -= margin;
  mins[1] -= margin;
  mins[2] -= margin;
  // add to the maxs
  maxs[0] += margin;
  maxs[1] += margin;
  maxs[2] += margin;
  return;
}


inline float Extents::getWidth(int axis) const
{
  return (maxs[axis] - mins[axis]);
}


#endif // BZF_EXTENTS_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
