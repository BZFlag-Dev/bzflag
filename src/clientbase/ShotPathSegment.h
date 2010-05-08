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

#ifndef	__SHOTPATHSEGMENT_H__
#define	__SHOTPATHSEGMENT_H__

/* common interface headers */
#include "BzTime.h"
#include "Ray.h"
#include "Extents.h"


class MeshFace;
class Obstacle;


class ShotPathSegment {
  public:
    enum Reason { Initial, Through, Ricochet, Teleport, Boundary };

    ShotPathSegment();
    ShotPathSegment(const BzTime& start, const BzTime& end,
                                const Ray& r, Reason = Initial);
    ShotPathSegment(const ShotPathSegment&);
    ShotPathSegment& operator=(const ShotPathSegment&);

    static std::string getReasonString(Reason reason) {
      switch (reason) {
        case Initial:  { return "Initial"; }
        case Through:  { return "Through"; }
        case Ricochet: { return "Ricochet"; }
        case Teleport: { return "Teleport"; }
        case Boundary: { return "Boundary"; }
      }
      return "unknown";
    }

  public:
    BzTime start;
    BzTime end;
    Ray    ray;
    Reason reason;
    Extents bbox;
    const Obstacle* hitObstacle;
    int linkSrcID;
    int linkDstID;
};


#endif /* __SHOTPATHSEGMENT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
