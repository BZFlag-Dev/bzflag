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

/*
 * Compute position of sun for given julian day.  Note that
 * julian days are big numbers so doubles are a must.
 */

#ifndef	BZF_DAYLIGHT_H
#define	BZF_DAYLIGHT_H

#include "common.h"
#include "bzfgl.h"

// julian day of midnight 1/1/1970
static const double	unixEpoch = 2440587.5;

// return direction of sun given julian day
void			getSunPosition(double julianDay, float latitude,
					float longitude, float pos[3]);

// return direction of moon given julian day
void			getMoonPosition(double julianDay, float latitude,
					float longitude, float pos[3]);

// transform a direction from the celestial coordinate system
void			getCelestialTransform(double julianDay,
					float latitude, float longitude,
					GLfloat (&xform)[4][4]);

// sets color of sun.  if it's nighttime, the sun is actually the moon.
void			getSunColor(const float sunDir[3], GLfloat color[3],
				GLfloat ambient[3], GLfloat& brightness);

// make sky colors given sun direction.  sun direction should be normalized.
// sky is filled with the colors for the zenith, horizon towards sun, and
// horizon away from sun, respectively.
void			getSkyColor(const float sunDir[3], GLfloat sky[4][3]);

// true if sun is high enough to cast shadows.  sun direction should be
// normalized.
bool			areShadowsCast(const float sunDir[3]);

// true if sun is low enough to let stars be visible.  sun direction
// should be normalized.
bool			areStarsVisible(const float sunDir[3]);

// true if near sunset and sky color interpolation shouldn't be from
// zenith, but from somewhere lower to flatten out the colors.
bool			getSunsetTop(const float sunDir[3], float& topAltitude);

#endif // BZF_DAYLIGHT_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
