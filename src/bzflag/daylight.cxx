/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "daylight.h"

// common headers
#include "StateDatabase.h"
#include "ParseColor.h"

static const double	radPerDeg = M_PI / 180.0;
static const double	radPerHour = M_PI / 12.0;
static const double	siderealHoursPerHour = 1.002737908;
static const double	epoch = 2415020.0;

static double		getGreenwichSideral(double julianDay)
{
  // true position requires sidereal time of midnight at prime meridian.
  // get midnight of given julian day (midnight has decimal of .5)
  double jdMidnight = floor(julianDay);
  if (julianDay - jdMidnight >= 0.5) jdMidnight += 0.5;
  else jdMidnight -= 0.5;

  // get fraction of a day
  double dayFraction = (julianDay - jdMidnight) * 24.0;

  // get Greenwich midnight sidereal time (in hours)
  double T = (jdMidnight - epoch) / 36525.0;
  const double greenwichMidnight =
		fmod((0.00002581 * T + 2400.051262) * T + 6.6460656, 24.0);

  // return Greenwich sidereal time
  return radPerHour * (greenwichMidnight + dayFraction * siderealHoursPerHour);
}

static void		gettruePosition(double julianDay,
					float latitude, float longitude,
					double sx, double sy, double sz,
					float pos[3])
{
  // get local sidereal time
  const float localSidereal = (float)(getGreenwichSideral(julianDay) -
						longitude * M_PI / 180.0);

  // rotate around polar axis (y-axis) by local sidereal time
  float tx = float(sx * cosf(localSidereal) - sz * sinf(localSidereal));
  float ty = float(sy);
  float tz = float(sz * cosf(localSidereal) + sx * sinf(localSidereal));

  // rotate by latitude to local position
  pos[0] = tx;
  pos[1] = ty * cosf((float)(latitude*M_PI/180.0)) - tz * sinf((float)(latitude*M_PI/180.0));
  pos[2] = tz * cosf((float)(latitude*M_PI/180.0)) + ty * sinf((float)(latitude*M_PI/180.0));
}

void			getCelestialTransform(double julianDay,
					float latitude, float longitude,
					float xform[4][4])
{
  // get local sidereal time
  const float localSidereal = (float)(getGreenwichSideral(julianDay) -
						longitude * M_PI / 180.0);

  // localSidereal is the amount the celestial sphere should be
  // rotated from the vernal equinox about the celestial axis.
  // construct the matrix that transforms points in the celestial
  // coordinate system (x points to vernal equinox, z points to
  // celestial north pole) to the local coordinate system on earth
  // where z is up and y points in the horizontal plane towards
  // north.
  const float cls = cosf(localSidereal);
  const float sls = sinf(localSidereal);
  const float cla = cosf((float)(latitude * M_PI / 180.0));
  const float sla = sinf((float)(latitude * M_PI / 180.0));

  // constant stuff
  xform[0][3] = xform[1][3] = xform[2][3] = 0.0f;
  xform[3][0] = xform[3][1] = xform[3][2] = 0.0f;
  xform[3][3] = 1.0f;

  // fill in rotation
  xform[0][0] = -sls;
  xform[1][0] = cls;
  xform[2][0] = 0.0f;
  xform[0][1] = -cls * sla;
  xform[1][1] = -sls * sla;
  xform[2][1] = cla;
  xform[0][2] = cls * cla;
  xform[1][2] = sls * cla;
  xform[2][2] = sla;
}

void			getSunPosition(double julianDay, float latitude,
					float longitude, float pos[3])
{
  double T = (julianDay - epoch) / 36525.0;
  double geometricMeanLongitude = radPerDeg *
		((0.0003025 * T + 36000.76892) * T + 279.69668);
  geometricMeanLongitude = fmod(geometricMeanLongitude, 2.0 * M_PI);

  double meanAnomaly = radPerDeg * (358.47583 +
		((0.0000033 * T + 0.000150) * T + 35999.04975) * T);
  meanAnomaly = fmod(meanAnomaly, 2.0 * M_PI);

//  double eccentricity = 0.01675104 +
//		T * (-0.000000126 * T - 0.0000418);

  double C = radPerDeg *
		(sin(meanAnomaly) * (1.919460 - (0.004789 + 0.000014 * T) * T) +
		sin(2.0 * meanAnomaly) * (0.020094 - 0.0001 * T) +
		sin(3.0 * meanAnomaly) * 0.000293);
  C = fmod(C, 2.0 * M_PI);

  double trueLongitude = geometricMeanLongitude + C;
  trueLongitude = fmod(trueLongitude, 2.0 * M_PI);

  // get obliquity (earth's tilt)
  double obliquity = radPerDeg *
		(23.452294 + (-0.0130125 + (-0.00000164 +
						0.000000503 * T) * T) * T);
  obliquity = fmod(obliquity, 2.0 * M_PI);

  // position of sun if earth didn't rotate:
  double sx, sy, sz;
  sx = sin(trueLongitude) * cos(obliquity);
  sy = sin(trueLongitude) * sin(obliquity);
  sz = cos(trueLongitude);

  // get true position
  gettruePosition(julianDay, latitude, longitude, sx, sy, sz, pos);
}

void			getMoonPosition(double julianDay, float latitude,
					float longitude, float pos[3])
{
  double T = (julianDay - epoch) / 36525.0;
  double e = 1.0 + (-0.002495 - 0.00000752 * T) * T;
  double meanLongitude = radPerDeg *
		(270.434164 + (481267.8831 + (-0.001133 + 0.0000019 * T)*T)*T);
  meanLongitude = fmod(meanLongitude, 2.0 * M_PI);

  double meanAnomaly = radPerDeg *
		(296.104608 + (477198.8491 + (0.009192 + 0.0000144 * T)*T)*T);
  meanAnomaly = fmod(meanAnomaly, 2.0 * M_PI);

  double meanElongation = radPerDeg *
		(350.737486 + (445267.1142 + (-0.001436 + 0.0000019 * T)*T)*T);
  meanElongation = fmod(meanElongation, 2.0 * M_PI);
  double meanElongation2 = 2.0 * meanElongation;

  double distFromAscendingNode = radPerDeg *
		(11.250889 + (483202.0251 + (-0.003211 - 0.0000003 * T)*T)*T);
  distFromAscendingNode = fmod(distFromAscendingNode, 2.0 * M_PI);

  // get sun's meanAnomaly
  double solMeanAnomaly = radPerDeg * (358.47583 +
		((0.0000033 * T + 0.000150) * T + 35999.04975) * T);
  solMeanAnomaly = fmod(solMeanAnomaly, 2.0 * M_PI);

  // get moon's geocentric latitude and longitude
  double geocentricLongitude = meanLongitude + radPerDeg *
		(6.288750 * sin(meanAnomaly) +
		 1.274018 * sin(meanElongation2 - meanAnomaly) +
		 0.658309 * sin(meanElongation2) +
		 0.213616 * sin(2.0 * meanAnomaly) +
		-0.185596 * sin(solMeanAnomaly) * e +
		-0.114336 * sin(2.0 * distFromAscendingNode));
  double geocentricLatitude = radPerDeg *
		(5.128189 * sin(distFromAscendingNode) +
		 0.280606 * sin(meanAnomaly + distFromAscendingNode) +
		 0.277693 * sin(meanAnomaly - distFromAscendingNode) +
		 0.173238 * sin(meanElongation2 - distFromAscendingNode) +
		 0.055413 * sin(meanElongation2 + distFromAscendingNode - meanAnomaly) +
		 0.046272 * sin(meanElongation2 - distFromAscendingNode - meanAnomaly) +
		 0.032573 * sin(meanElongation2 + distFromAscendingNode) +
		 0.017198 * sin(2.0 * meanAnomaly + distFromAscendingNode));
  geocentricLongitude = fmod(geocentricLongitude, 2.0 * M_PI);
  geocentricLatitude = fmod(geocentricLatitude, 2.0 * M_PI);

  // get obliquity (earth's tilt)
  double obliquity = radPerDeg *
		(23.452294 + (-0.0130125 + (-0.00000164 +
						0.000000503 * T) * T) * T);
  obliquity = fmod(obliquity, 2.0 * M_PI);

  // position of moon if earth didn't rotate:
  double sx, sy, sz;
  sx = cos(geocentricLatitude) * sin(geocentricLongitude) * cos(obliquity) -
	sin(geocentricLatitude) * sin(obliquity);
  sy = sin(geocentricLatitude) * cos(obliquity) +
	cos(geocentricLatitude) * sin(geocentricLongitude) * sin(obliquity);
  sz = cos(geocentricLatitude) * cos(geocentricLongitude);

  // get true position
  gettruePosition(julianDay, latitude, longitude, sx, sy, sz, pos);
}

static void		lerpColor(GLfloat out[3], const GLfloat t0[3],
					const GLfloat t1[3], float t)
{
  out[0] = (1.0f - t) * t0[0] + t * t1[0];
  out[1] = (1.0f - t) * t0[1] + t * t1[1];
  out[2] = (1.0f - t) * t0[2] + t * t1[2];
}

static const float	nightElevation = -0.25f;	// ~sin(-15)
static const float	duskElevation = -0.17f;		// ~sin(-10)
static const float	twilightElevation = -0.087f;	// ~sin(-5)
static const float	dawnElevation = 0.0f;		// sin(0)
static const float	dayElevation = 0.087f;		// ~sin(5)

void			getSunColor(const float sunDir[3], GLfloat color[3],
					GLfloat ambient[3], GLfloat& brightness)
{
  static const GLfloat	highSunColor[3] = { 1.75f, 1.75f, 1.4f };
  static const GLfloat	lowSunColor[3] = { 0.75f, 0.27f, 0.0f };
  static const GLfloat	moonColor[3] = { 0.4f, 0.4f, 0.4f };
  static const GLfloat	nightAmbient[3] = { 0.3f, 0.3f, 0.3f };
  static const GLfloat	dayAmbient[3] = { 0.35f, 0.5f, 0.5f };

  if (sunDir[2] <= -0.009f) {		// it's the moon
    color[0] = moonColor[0];
    color[1] = moonColor[1];
    color[2] = moonColor[2];
    brightness = 0.0f;
  }
  else if (sunDir[2] < dayElevation) {
    const GLfloat t = (sunDir[2] - dawnElevation) /
			(dayElevation - dawnElevation);
    lerpColor(color, lowSunColor, highSunColor, t);
    brightness = t;
  }
  else {
    color[0] = highSunColor[0];
    color[1] = highSunColor[1];
    color[2] = highSunColor[2];
    brightness = 1.0f;
  }

  // now ambient
  if (sunDir[2] < duskElevation) {
    ambient[0] = nightAmbient[0];
    ambient[1] = nightAmbient[1];
    ambient[2] = nightAmbient[2];
  }
  else if (sunDir[2] < dayElevation) {
    const GLfloat t = (sunDir[2] - duskElevation) /
			(dayElevation - duskElevation);
    lerpColor(ambient, nightAmbient, dayAmbient, t);
  }
  else {
    ambient[0] = dayAmbient[0];
    ambient[1] = dayAmbient[1];
    ambient[2] = dayAmbient[2];
  }
}

bool			getSunsetTop(const float sunDir[3], float& topAltitude)
{
  if (sunDir[2] > nightElevation && sunDir[2] < dayElevation) {
    topAltitude = (sunDir[2]-nightElevation) / (dayElevation-nightElevation);
    return true;
  }
  return false;
}

void			getSkyColor(const float sunDir[3], GLfloat sky[4][3])
{
  static const GLfloat	nightColor[3] = { 0.04f, 0.04f, 0.08f };
  static const GLfloat	zenithColor[3] = { 0.25f, 0.55f, 0.86f };
  static const GLfloat	horizonColor[3] = { 0.43f, 0.75f, 0.95f };
  static const GLfloat	sunrise1Color[3] = { 0.30f, 0.12f, 0.08f };
  static const GLfloat	sunrise2Color[3] = { 0.47f, 0.12f, 0.08f };

  // sky colors
  if (sunDir[2] < nightElevation) {
    // nighttime
    sky[0][0] = nightColor[0];
    sky[0][1] = nightColor[1];
    sky[0][2] = nightColor[2];
    sky[1][0] = nightColor[0];
    sky[1][1] = nightColor[1];
    sky[1][2] = nightColor[2];
    sky[2][0] = nightColor[0];
    sky[2][1] = nightColor[1];
    sky[2][2] = nightColor[2];
    sky[3][0] = nightColor[0];
    sky[3][1] = nightColor[1];
    sky[3][2] = nightColor[2];
  }
  else if (sunDir[2] < twilightElevation) {
    // twilight
    const float t = (sunDir[2] - nightElevation) /
			(twilightElevation - nightElevation);
    sky[0][0] = nightColor[0];
    sky[0][1] = nightColor[1];
    sky[0][2] = nightColor[2];
    lerpColor(sky[1], nightColor, sunrise1Color, t);
    sky[2][0] = nightColor[0];
    sky[2][1] = nightColor[1];
    sky[2][2] = nightColor[2];
    sky[3][0] = nightColor[0];
    sky[3][1] = nightColor[1];
    sky[3][2] = nightColor[2];
  }
  else if (sunDir[2] < dawnElevation) {
    // sunrise or sunset
    const float t = (sunDir[2] - twilightElevation) /
			(dawnElevation - twilightElevation);
    sky[0][0] = nightColor[0];
    sky[0][1] = nightColor[1];
    sky[0][2] = nightColor[2];
    lerpColor(sky[1], sunrise1Color, sunrise2Color, t);
    sky[2][0] = nightColor[0];
    sky[2][1] = nightColor[1];
    sky[2][2] = nightColor[2];
    sky[3][0] = nightColor[0];
    sky[3][1] = nightColor[1];
    sky[3][2] = nightColor[2];
  }
  else if (sunDir[2] < dayElevation) {
    // early morning/late evening
    const float t = (sunDir[2] - dawnElevation) /
			(dayElevation - dawnElevation);
    lerpColor(sky[0], nightColor, zenithColor, t);
    lerpColor(sky[1], sunrise2Color, horizonColor, t);
    lerpColor(sky[2], nightColor, horizonColor, t);
    lerpColor(sky[3], nightColor, horizonColor, t);
  }
  else {
    // day time
    sky[0][0] = zenithColor[0];
    sky[0][1] = zenithColor[1];
    sky[0][2] = zenithColor[2];
    sky[1][0] = horizonColor[0];
    sky[1][1] = horizonColor[1];
    sky[1][2] = horizonColor[2];
    sky[2][0] = horizonColor[0];
    sky[2][1] = horizonColor[1];
    sky[2][2] = horizonColor[2];
    sky[3][0] = horizonColor[0];
    sky[3][1] = horizonColor[1];
    sky[3][2] = horizonColor[2];
  }

  // user adjustment for the sky color
  if (BZDB.get("_skyColor") != "white") {
    float skyColor[4];
    parseColorString(BZDB.get("_skyColor"), skyColor);
    sky[0][0]  *= skyColor[0];
    sky[0][1]  *= skyColor[1];
    sky[0][2]  *= skyColor[2];
    sky[1][0]  *= skyColor[0];
    sky[1][1]  *= skyColor[1];
    sky[1][2]  *= skyColor[2];
    sky[2][0]  *= skyColor[0];
    sky[2][1]  *= skyColor[1];
    sky[2][2]  *= skyColor[2];
    sky[3][0]  *= skyColor[0];
    sky[3][1]  *= skyColor[1];
    sky[3][2]  *= skyColor[2];
  }
}

bool			areShadowsCast(const float sunDir[3])
{
  return sunDir[2] > 0.5 * dayElevation;
}

bool			areStarsVisible(const float sunDir[3])
{
  return sunDir[2] < dawnElevation;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

