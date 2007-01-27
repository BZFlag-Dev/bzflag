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

#include "common.h"
#include "Ray.h"

Ray::Ray(const float* _o, const float* _d)
{
  o[0] = _o[0];
  o[1] = _o[1];
  o[2] = _o[2];
  d[0] = _d[0];
  d[1] = _d[1];
  d[2] = _d[2];
}

Ray::Ray(const Ray& r)
{
  o[0] = r.o[0];
  o[1] = r.o[1];
  o[2] = r.o[2];
  d[0] = r.d[0];
  d[1] = r.d[1];
  d[2] = r.d[2];
}

Ray&			Ray::operator=(const Ray& r)
{
  if (this != &r) {
    o[0] = r.o[0];
    o[1] = r.o[1];
    o[2] = r.o[2];
    d[0] = r.d[0];
    d[1] = r.d[1];
    d[2] = r.d[2];
  }
  return *this;
}

void			Ray::getPoint(float t, float p[3]) const
{
  p[0] = o[0] + t * d[0];
  p[1] = o[1] + t * d[1];
  p[2] = o[2] + t * d[2];
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
