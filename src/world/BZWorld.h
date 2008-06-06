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

#ifndef __BZWORLD_H__
#define __BZWORLD_H__

#include "common.h"
#ifdef _WIN32
  #ifdef BZWORLD_EXPORTS
  #define BZWORLD_API __declspec(dllexport)
  #else
  #define BZWORLD_API __declspec(dllimport)
  #endif
#else
  #define BZWORLD_API
#endif

class BZWORLD_API SampleClass
{
public:
  SampleClass();
  ~SampleClass();

  void thingy ( void );
};


#endif // __BZWORLD_H__

// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
