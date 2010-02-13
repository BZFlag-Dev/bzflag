/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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

#include "vectors.h"


class Extents {
  public:
    Extents();
    Extents(const Extents&);
    Extents(const fvec3& mins, const fvec3& maxs);

    void reset();

    Extents& operator=(const Extents&);
    void set(const fvec3& mins, const fvec3& maxs);

    void expandToBox(const Extents& box); // expand to contain the box
    void expandToPoint(const fvec3&); // expand to contain the point
    void addMargin(float margin);     // widen the extents by "margin"

    float getWidth(int axis) const;

    bool touches(const Extents& orig) const;
    bool contains(const Extents& orig) const;
    bool contains(const fvec3& point) const;

  public:
    fvec3 mins;
    fvec3 maxs;
};


inline void Extents::reset()
{
  mins.x = mins.y = mins.z = +MAXFLOAT;
  maxs.x = maxs.y = maxs.z = -MAXFLOAT;
  return;
}


inline void Extents::set(const fvec3& _mins, const fvec3& _maxs)
{
  mins = _mins;
  maxs = _maxs;
  return;
}


inline Extents::Extents()
{
  reset();
  return;
}


inline Extents::Extents(const Extents& e)
{
  mins = e.mins;
  maxs = e.maxs;
}


inline Extents::Extents(const fvec3& _mins, const fvec3& _maxs)
{
  mins = _mins;
  maxs = _maxs;
}


inline Extents& Extents::operator=(const Extents& orig)
{
  mins = orig.mins;
  maxs = orig.maxs;
  return *this;
}


inline void Extents::expandToBox(const Extents& test)
{
  // test mins
  if (test.mins.x < mins.x) { mins.x = test.mins.x; }
  if (test.mins.y < mins.y) { mins.y = test.mins.y; }
  if (test.mins.z < mins.z) { mins.z = test.mins.z; }
  // test maxs
  if (test.maxs.x > maxs.x) { maxs.x = test.maxs.x; }
  if (test.maxs.y > maxs.y) { maxs.y = test.maxs.y; }
  if (test.maxs.z > maxs.z) { maxs.z = test.maxs.z; }
  return;
}


inline void Extents::expandToPoint(const fvec3& point)
{
  // test mins
  if (point.x < mins.x) { mins.x = point.x; }
  if (point.y < mins.y) { mins.y = point.y; }
  if (point.z < mins.z) { mins.z = point.z; }
  // test maxs
  if (point.x > maxs.x) { maxs.x = point.x; }
  if (point.y > maxs.y) { maxs.y = point.y; }
  if (point.z > maxs.z) { maxs.z = point.z; }
  return;
}


inline bool Extents::touches(const Extents& test) const
{
  if ((mins.x > test.maxs.x) || (maxs.x < test.mins.x) ||
      (mins.y > test.maxs.y) || (maxs.y < test.mins.y) ||
      (mins.z > test.maxs.z) || (maxs.z < test.mins.z)) {
    return false;
  }
  return true;
}


inline bool Extents::contains(const Extents& test) const
{
  if ((mins.x < test.mins.x) && (maxs.x > test.maxs.x) &&
      (mins.y < test.mins.y) && (maxs.y > test.maxs.y) &&
      (mins.z < test.mins.z) && (maxs.z > test.maxs.z)) {
    return true;
  }
  return false;
}


inline bool Extents::contains(const fvec3& point) const
{
  if ((mins.x < point.x) && (maxs.x > point.x) &&
      (mins.y < point.y) && (maxs.y > point.y) &&
      (mins.z < point.z) && (maxs.z > point.z)) {
    return true;
  }
  return false;
}


inline void Extents::addMargin(float margin)
{
  mins -= margin;
  maxs += margin;
  return;
}


inline float Extents::getWidth(int axis) const
{
  return (maxs[axis] - mins[axis]);
}


#endif // BZF_EXTENTS_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
