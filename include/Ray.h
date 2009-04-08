/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "vectors.h"

/** Encapsulates a semi-infinite ray. */

class Ray {
  public:
			Ray();
			Ray(const fvec3& o, const fvec3& d);
			Ray(const Ray&);
			~Ray();
    Ray&		operator=(const Ray&);

    const fvec3&	getOrigin() const;
    const fvec3&	getDirection() const;
    void		getPoint(float t, fvec3& p) const;

  private:
    fvec3		o;
    fvec3		d;
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

inline const fvec3&	Ray::getOrigin() const
{
  return o;
}

inline const fvec3&	Ray::getDirection() const
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
