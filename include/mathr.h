/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

//
// floating point type.  explicit use of float and double are allowed;
// doubles should be used when the extra precision is required and
// floats are usually used for colors, texture coordinates, and other
// rendering parameters.
//

#ifndef BZF_MATHR_H
#define BZF_MATHR_H

#include "common.h"
#include <math.h>
#include <vector>

// choose type of reals
#if !defined(REAL_DOUBLE) && !defined(REAL_FLOAT)
//#define REAL_DOUBLE
#define REAL_FLOAT
#endif

// define Real type, a macro to create constants, and macros to
// generate OpenGL names/suffixes for Real.  use R_(x) instead of x
// when including constants in code (for example, a = R_(1.0);
// instead of a = 1.0;) to avoid warnings.
#if defined(REAL_DOUBLE)
typedef double Real;
#define R_(x__)				x__
#define R_gl(x__)			x__ ## d
#define R_glv(x__)			x__ ## dv
#define glLoadMatrixr		glLoadMatrixd
#define glMultMatrixr		glMultMatrixd
#elif defined(REAL_FLOAT)
typedef float Real;
#define R_(x__)				x__ ## f
#define R_gl(x__)			x__ ## f
#define R_glv(x__)			x__ ## fv
#define glLoadMatrixr		glLoadMatrixf
#define glMultMatrixr		glMultMatrixf
#else
#error no type for Real
#endif

// a vector of Real
typedef std::vector<Real> VectorN;

// some platforms don't have float versions of the math library
#if defined(_old_linux_) || defined(sun)
#define asinf			(float)asin
#define atanf			(float)atan
#define atan2f  		(float)atan2
#define ceilf			(float)ceil
#define cosf			(float)cos
#define expf			(float)exp
#define fabsf			(float)fabs
#define floorf  		(float)floor
#define fmodf			(float)fmod
#define hypotf  		(float)hypot
#define logf			(float)log
#define powf			(float)pow
#define sinf			(float)sin
#define sqrtf			(float)sqrt
#define tanf			(float)tan
#endif
#if defined(_WIN32)
#define hypotf			(float)hypot
#endif

// fix other missing things
#ifndef MAXFLOAT
#define MAXFLOAT		3.402823466e+38f
#endif

//
// constants for Real
//

#if defined(REAL_DOUBLE)
static const Real R_MAX =		1.7976931348623157e+308;
static const Real R_PI =		3.14159265358979323846;
static const Real R_SQRT1_2 =	0.70710678118654752440;
#elif defined(REAL_FLOAT)
static const Real R_MAX =		3.402823466e+38f;
static const Real R_PI =		3.141593f;
static const Real R_SQRT1_2 =	0.7071068f;
#endif

//
// math functions for Real
//

#if defined(REAL_DOUBLE) || defined(REAL_FLOAT)
inline Real asinr(Real x) { return R_(asin)(x); }
inline Real atanr(Real x) { return R_(atan)(x); }
inline Real atan2r(Real x, Real y) { return R_(atan2)(x, y); }
inline Real ceilr(Real x) { return R_(ceil)(x); }
inline Real cosr(Real x) { return R_(cos)(x); }
inline Real expr(Real x) { return R_(exp)(x); }
inline Real fabsr(Real x) { return R_(fabs)(x); }
inline Real floorr(Real x) { return R_(floor)(x); }
inline Real fmodr(Real x, Real y) { return R_(fmod)(x, y); }
inline Real hypotr(Real x, Real y) { return R_(hypot)(x, y); }
inline Real logr(Real x) { return R_(log)(x); }
inline Real powr(Real x, Real y) { return R_(pow)(x, y); }
inline Real sinr(Real x) { return R_(sin)(x); }
inline Real sqrtr(Real x) { return R_(sqrt)(x); }
inline Real tanr(Real x) { return R_(tan)(x); }
inline Real rad2deg(Real x) { return R_(180.0) * (x / R_PI); }
inline Real deg2rad(Real x) { return R_PI * (x / R_(180.0)); }
#endif

#endif
