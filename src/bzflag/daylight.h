/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Compute position of sun for given julian day.  Note that
 * julian days are big numbers so doubles are a must.
 */

#ifndef BZF_DAYLIGHT_H
#define BZF_DAYLIGHT_H

#include "common.h"

// return direction of sun given julian day
void					getSunPosition(double julianDay, float latitude,
										float longitude, float pos[3]);

// return direction of moon given julian day
void					getMoonPosition(double julianDay, float latitude,
										float longitude, float pos[3]);

// transform a direction from the celestial coordinate system
void					getCelestialTransform(double julianDay,
										float latitude, float longitude,
										float xform[4][4]);

#endif // BZF_DAYLIGHT_H
