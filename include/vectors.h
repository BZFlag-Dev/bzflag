/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __VECTOR_MATH_H__
#define __VECTOR_MATH_H__

#include <string.h>

typedef float fvec2[2];
typedef float fvec3[3];
typedef float fvec4[4];

class cfvec2 {
  public:
    cfvec2(){};
    cfvec2(const float values[2]) {
      memcpy (data, values, sizeof(float[2]));
    }
    cfvec2& operator=(const float values[2]) {
      memcpy (data, values, sizeof(float[2]));
      return *this;
    }
    inline float& operator[](int pos) { return data[pos]; }
    float data[2];
};

class cfvec3 {
  public:
    cfvec3(){};
    cfvec3(const float values[3]) {
      memcpy (data, values, sizeof(float[3]));
    }
    cfvec3& operator=(const float values[3]) {
      memcpy (data, values, sizeof(float[3]));
      return *this;
    }
    inline float& operator[](int pos) { return data[pos]; }
    float data[3];
};

class cfvec4 {
  public:
    cfvec4(){};
    cfvec4(const float values[4]) {
      memcpy (data, values, sizeof(float[4]));
    }
    cfvec4& operator=(const float values[4]) {
      memcpy (data, values, sizeof(float[4]));
      return *this;
    }
    inline float& operator[](int pos) { return data[pos]; }
    float data[4];
};


// FIXME - some these should go into a common file
// Some handy geometry functions

static inline void vec3sub (float *result, const float* v1, const float* v2)
{
  result[0] = v1[0] - v2[0];
  result[1] = v1[1] - v2[1];
  result[2] = v1[2] - v2[2];
  return;
}
static inline float vec3dot (const float* v1, const float* v2)
{
  return (v1[0] * v2[0]) + (v1[1] * v2[1]) + (v1[2] * v2[2]);
}
static inline void vec3cross (float* result, const float* v1, const float* v2)
{
  result[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  result[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  result[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
  return;
}

#endif // __VECTOR_MATH_H__

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
