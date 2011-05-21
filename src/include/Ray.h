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

#ifndef BZF_RAY_H
#define BZF_RAY_H

#include "common.h"
#include <string.h> // for memset()
#include "vectors.h"

/** Encapsulates a semi-infinite ray. */

class Ray {
  public:
    inline Ray() : orig(0.0f, 0.0f, 0.0f) {}
    inline Ray(const fvec3& o, const fvec3& d) : orig(o) , dir(d) {}
    inline Ray(const Ray& r) : orig(r.orig), dir(r.dir)  {}
    inline Ray& operator=(const Ray& r) {
      if (this != &r) {
        orig = r.orig;
        dir  = r.dir;
      }
      return *this;
    }

    inline const fvec3& getOrigin()    const { return orig; }
    inline const fvec3& getDirection() const { return dir;  }

    inline fvec3 getPoint(float t)           const { return orig + (t * dir); }
    inline void  getPoint(float t, fvec3& p) const    { p = orig + (t * dir); }

  private:
    fvec3 orig;
    fvec3 dir;
};


#endif // BZF_RAY_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
