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

/* interface header */
#include "ShotPathSegment.h"


ShotPathSegment::ShotPathSegment()
{
  // do nothing
}

ShotPathSegment::ShotPathSegment(const double _start,
				const double _end, const Ray& _ray,
				Reason _reason) :
				start(_start),
				end(_end),
				ray(_ray),
				reason(_reason)
{
  // compute bounding box
  ray.getPoint(0.0f, bbox[0]);
  ray.getPoint(float(end - start), bbox[1]);
  if (bbox[0][0] > bbox[1][0]) {
    const float tmp = bbox[0][0];
    bbox[0][0] = bbox[1][0];
    bbox[1][0] = tmp;
  }
  if (bbox[0][1] > bbox[1][1]) {
    const float tmp = bbox[0][1];
    bbox[0][1] = bbox[1][1];
    bbox[1][1] = tmp;
  }
  if (bbox[0][2] > bbox[1][2]) {
    const float tmp = bbox[0][2];
    bbox[0][2] = bbox[1][2];
    bbox[1][2] = tmp;
  }
}

ShotPathSegment::ShotPathSegment(const ShotPathSegment& segment) :
				start(segment.start),
				end(segment.end),
				ray(segment.ray),
				reason(segment.reason)
{
  // copy bounding box
  bbox[0][0] = segment.bbox[0][0];
  bbox[0][1] = segment.bbox[0][1];
  bbox[0][2] = segment.bbox[0][2];
  bbox[1][0] = segment.bbox[1][0];
  bbox[1][1] = segment.bbox[1][1];
  bbox[1][2] = segment.bbox[1][2];
}

ShotPathSegment::~ShotPathSegment()
{
  // do nothing
}

ShotPathSegment&	ShotPathSegment::operator=(const
					ShotPathSegment& segment)
{
  if (this != &segment) {
    start = segment.start;
    end = segment.end;
    ray = segment.ray;
    reason = segment.reason;
    bbox[0][0] = segment.bbox[0][0];
    bbox[0][1] = segment.bbox[0][1];
    bbox[0][2] = segment.bbox[0][2];
    bbox[1][0] = segment.bbox[1][0];
    bbox[1][1] = segment.bbox[1][1];
    bbox[1][2] = segment.bbox[1][2];
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
