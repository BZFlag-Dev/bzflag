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

/* interface header */
#include "ShotPathSegment.h"


ShotPathSegment::ShotPathSegment()
{
  // do nothing
}


ShotPathSegment::ShotPathSegment(const double _start, const double _end,
                                 const Ray& _ray, Reason _reason)
: start(_start)
, end(_end)
, ray(_ray)
, reason(_reason)
, hitObstacle(NULL)
, linkSrcID(-1)
, linkDstID(-1)
{
  // compute bounding box
  bbox.expandToPoint(ray.getOrigin());             // start
  bbox.expandToPoint(ray.getPoint(float(end - start))); // end
}


ShotPathSegment::ShotPathSegment(const ShotPathSegment& segment)
: start(segment.start)
, end(segment.end)
, ray(segment.ray)
, reason(segment.reason)
, bbox(segment.bbox)
, hitObstacle(segment.hitObstacle)
, linkSrcID(segment.linkSrcID)
, linkDstID(segment.linkDstID)
{
}


ShotPathSegment& ShotPathSegment::operator=(const ShotPathSegment& segment)
{
  if (this != &segment) {
    start = segment.start;
    end = segment.end;
    ray = segment.ray;
    reason = segment.reason;
    bbox = segment.bbox;
    hitObstacle = segment.hitObstacle;
    linkSrcID = segment.linkSrcID;
    linkDstID = segment.linkDstID;
  }
  return *this;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
