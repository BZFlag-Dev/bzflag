/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Ray
 *	Encapsulates a semi-infinite ray.
 */

#ifndef	BZF_RAY_H
#define	BZF_RAY_H

#include "common.h"

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
  // do nothing
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
