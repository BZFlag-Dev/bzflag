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

#ifndef	BZF_RAY_H
#define	BZF_RAY_H

#include "common.h"
#include <string.h> // for memset()

/** Encapsulates a semi-infinite ray. */

class Ray {
  public:
			Ray();
			Ray(const float* o, const float* d);
			Ray(const Ray&);
			~Ray();
    Ray&		operator=(const Ray&);

    const float*	getOrigin() const;
    const float*	getDirection() const;
    void		getPoint(float t, float p[3]) const;

  private:
    float		o[3];
    float		d[3];
};

//
// Ray
//

inline Ray::Ray()
{
  memset(o, 0, sizeof(float) * 3);
}

inline Ray::~Ray()
{
  // do nothing
}

inline const float*	Ray::getOrigin() const
{
  return o;
}

inline const float*	Ray::getDirection() const
{
  return d;
}

#endif // BZF_RAY_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
