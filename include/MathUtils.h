/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * The fast sqrt() implementation contained herein was taken from the
 * public domained nVIDIA Fast Math Routines and adapted for BZFlag's
 * needs as well as wrapping table construction inside a class.
 */

#ifndef __MATHUTILS_H__
#define __MATHUTILS_H__

/* common header comes first */
#include "common.h"

/* system headers */
#include <math.h>
#include <iostream>

/* common headers */
#include "TimeKeeper.h"


#define FP_BITS(fp) (*(unsigned long *)&(fp))
#define FP_ABS_BITS(fp) (FP_BITS(fp)&0x7FFFFFFF)
#define FP_SIGN_BIT(fp) (FP_BITS(fp)&0x80000000)
#define FP_ONE_BITS 0x3F800000

// r = 1/p
#define FP_INV(r,p)							\
{									\
    union {                                                             \
      float fp;                                                         \
      int i;                                                            \
    } _tmp, _i;                                                         \
    _tmp.fp = p;                                                        \
    _i.i = 2 * FP_ONE_BITS - _tmp.i;				        \
    r = _i.fp;						                \
    r = r * (2.0f - (p) * r);						\
}

/////////////////////////////////////////////////
// The following comes from Vincent Van Eeckhout
// Thanks for sending us the code!
// It's the same thing in assembly but without this C-needed line:
//    r = *(float *)&_i;

float   __two = 2.0f;

#define FP_INV2(r,p)		     \
{					\
    __asm { mov     eax,0x7F000000    }; \
    __asm { sub     eax,dword ptr [p] }; \
    __asm { mov     dword ptr [r],eax }; \
    __asm { fld     dword ptr [p]     }; \
    __asm { fmul    dword ptr [r]     }; \
    __asm { fsubr   [__two]	   }; \
    __asm { fmul    dword ptr [r]     }; \
    __asm { fstp    dword ptr [r]     }; \
}

#define FP_EXP(e,p)							  \
{									    \
    int _i;								  \
    e = -1.44269504f * (float)0x00800000 * (p);			      \
    _i = (int)e + 0x3F800000;						\
    e = *(float *)&_i;						       \
}

#define FP_NORM_TO_BYTE(i,p)						 \
{									    \
    float _n = (p) + 1.0f;						   \
    i = *(int *)&_n;							 \
    if (i >= 0x40000000)     i = 0xFF;				       \
    else if (i <=0x3F800000) i = 0;					  \
    else i = ((i) >> 15) & 0xFF;					     \
}

inline unsigned long FP_NORM_TO_BYTE2(float p)
{
  union {
    float fp;
    unsigned long ul;
  } tmp;
  tmp.fp = p + 1.0f;
  return (tmp.ul >> 15) & 0xFF;
}

inline unsigned long FP_NORM_TO_BYTE3(float p)
{
  union {
    float fp;
    unsigned long ul;
  } tmp;
  tmp.fp = p + 12582912.0f;
  return (tmp.ul & 0xFF);
}


/** The math_util class contains general routines for common
 * mathematical functions that should be used with care.  The
 * precision and/or accuracy of the routine below are not guaranteed,
 * though the implementation should useful for fast estimates where
 * accuracy is not a stringent concern.
 *
 * For the square root and inverse square root routines below, they
 * are

 */
class math_util {

private:
  /** table of precomputed square root values */
  static unsigned int _fast_sqrt_table[0x10000];
  /** keep track of whether the table was precomputed yet */
  static bool _built_fast_sqrt_table;

  /** table of random floats for performance testing */
  static float _random_floats[0x10000];
  /** keep tracke of whether the random floats are set yet */
  static bool _built_random_floats;

  typedef union FastSqrtUnion
  {
    float f;
    unsigned int i;
  } FastSqrtUnion;

protected:

  static void  build_sqrt_table()
  {
    unsigned int i;
    FastSqrtUnion s;

    for (i = 0; i <= 0x7FFF; i++) {
      // Build a float with the bit pattern i as mantissa
      //  and an exponent of 0, stored as 127
      s.i = (i << 8) | (0x7F << 23);
      s.f = (float)sqrt(s.f);

      // Take the square root then strip the first 7 bits of
      //  the mantissa into the table
      _fast_sqrt_table[i + 0x8000] = (s.i & 0x7FFFFF);

      // Repeat the process, this time with an exponent of 1,
      //  stored as 128
      s.i = (i << 8) | (0x80 << 23);
      s.f = (float)sqrt(s.f);

      _fast_sqrt_table[i] = (s.i & 0x7FFFFF);
    }
  }

  static void build_random_floats()
  {
    unsigned int i;
    bzfsrand(0); // ensure consistent seed and same rand value order
    for (i=0; i < 0x10000; i++) {
      _random_floats[i] = (float)bzfrand();
    }
  }

  /* INVERSE SQUARE ROOT */

  /** system implementation of floating point square root estimate
   */
  static inline float fastinvsqrt0(float n)
  {
    float f;
    float p = sqrtf(n);
    FP_INV(f, p)
    return f;
  }

  /** software estimate of inverse square root
   */
  static inline float fastinvsqrt1(float n)
  {
    float f;
    float p = fastsqrt1(n);
    FP_INV(f, p)
    return f;
  }

  /** hardware instruction based inverse square root estimate
   */
  static inline float fastinvsqrt2 (float n)
  {
#if defined(__APPLE__)
    float result;
    asm ( "frsqrte %0, %1" : /*OUT*/ "=f" (result) : /*IN*/ "f" (n) );
    return result;
#else
    float f;
    float p = sqrtf(n);
    FP_INV(f, p)
    return f;
#endif
  }


  /* SQUARE ROOT */

  /** system implementation of floating point square root
   */
  static inline float fastsqrt0(float n)
  {
    return sqrtf(n);
  }

  /** software estimate replacement -- this is nvidias fast square
   * root routine is a table-based solution that trades off memory
   * utilization for performance.  it's not necessarily a cache
   * friendly method, but gives impressive results for common use.
   */
  static inline float fastsqrt1(float n)
  {
    // make sure the table is built
    if (!_built_fast_sqrt_table) {
      build_sqrt_table();
      _built_fast_sqrt_table = true;
    }

    // check for square root of 0
    if (FP_BITS(n) == 0) {
      return 0.0;
    }

    FP_BITS(n) = _fast_sqrt_table[(FP_BITS(n) >> 8) & 0xFFFF] | ((((FP_BITS(n) - 0x3F800000) >> 1) + 0x3F800000) & 0x7F800000);

    return n;
  }

  /** hardware instruction based square root estimate
   */
  static inline float fastsqrt2(float n)
  {
#if defined(__APPLE__)
    float f;
    float p = fastinvsqrt2(n);
    FP_INV(f, p)
    return f;
#else
    return sqrtf(n);
#endif
  }


public:

  /** function pointer to the fastinvsqrt routine that performs best
   * on this system.
   */
  static float (*fastinvsqrt)(float);

  /** function pointer to the fastsqrt routine that performs best on
   * this system.
   */
  static float (*fastsqrt)(float);

  /** optimize usage of square root and inverse square root at runtime
   * to use the fastest available.  A quick timed test is performed on
   * each of the routines and the function pointers are set to the
   * fastest.  There is no consideration for tolerance being taken
   * into account -- they are all assumed to be insufficient if
   * accuracy is required.
   */
  static void optimize()
  {
    /* array of function pointers for testing */
    float (*mathTest[6])(float) = {fastsqrt0, fastsqrt1, fastsqrt0,
				   fastinvsqrt0, fastinvsqrt1, fastinvsqrt0};
    const char *label[6] = {"system square root", "nvidia square root estimate", "hardware-based square root",
			    "system inverse square root", "nvidia inverse square root estimate", "harware-based inverse square root"};

    std::cout << "Optimizing math routines..." << std::endl;

    if (!_built_random_floats) {
      std::cout << "...building random float table" << std::endl;
      build_random_floats();
      _built_random_floats = true;
    }

    /* minimize cache effect by preloading */
    double sum = 0.0;
    for (unsigned int fl = 0; fl < 0x10000; fl++) {
      sum += _random_floats[fl];
    }
    // should always be false, but compiler shouldn't know that
    if (sum < -1.0f) {
      std::cerr << "ERROR: should not have reached here in MathUtils.h" << std::endl;
      exit(1);
    }

    /* test math routine candidates (reverse order) */
    unsigned int mostIterations = 0;
    unsigned int bestIndex = 0;
    for (int i = 0; i < 6; i++) {
      unsigned int iterations = 0; // how many test iterations were performed
      sum = 0.0; // make sure optimizer doesn't optimize us away

      if (i == 0) {
	std::cout << "...testing square root" << std::flush;
      } else if (i == 3) {
	std::cout << "...testing inverse square root" << std::flush;
      }

      // test for half a second per function
      TimeKeeper t = TimeKeeper::getCurrent();
      do {
	for (unsigned int f = 0; f < 0x10000; f++) {
	  sum += mathTest[i](_random_floats[f]);
	}
	iterations++;
      } while (TimeKeeper::getCurrent() - t < 0.5f);
      std::cout << " ... " << iterations;

      // should always be false, but compiler shouldn't know that
      if (sum < -1.0f) {
	std::cerr << "ERROR: should not have reached here in MathUtils.h" << std::endl;
	exit(1);
      }

      if (iterations > mostIterations) {
	mostIterations = iterations;
	bestIndex = i;
      }

      if (i == 2) {
	fastsqrt = mathTest[bestIndex];
	std::cout << std::endl << "...using " << label[bestIndex] << std::endl;
	mostIterations = 0;
	bestIndex = 0;
      } else if (i == 5) {
	fastinvsqrt = mathTest[bestIndex];
	std::cout << std::endl << "...using " << label[bestIndex] << std::endl;
	mostIterations = 0;
	bestIndex = 0;
      }
    }
    std::cout << "...done optimizing math routines." << std::endl;
    return;
  }

};

/* static initializers */
unsigned int math_util::_fast_sqrt_table[0x10000] = {0};
bool math_util::_built_fast_sqrt_table = false;
float math_util::_random_floats[0x10000] = {0};
bool math_util::_built_random_floats = false;
float (*math_util::fastinvsqrt)(float) = math_util::fastinvsqrt0;
float (*math_util::fastsqrt)(float) = math_util::fastsqrt0;

#endif  /* __MATHUTILS_H__ */


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
