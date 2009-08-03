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

#ifndef __BZAUTHD_RANDOM_H__
#define __BZAUTHD_RANDOM_H__

#include "Thread.h"

class StrongRandom : public GuardedSingleton<StrongRandom>
{
public:
  StrongRandom();

  uint8_t   getU8()   { return *(uint8_t*)  get(sizeof(uint8_t)   ); }
  int8_t    get8()    { return *(int8_t*)   get(sizeof(int8_t)    ); }
  uint16_t  getU16()  { return *(uint16_t*) get(sizeof(uint16_t)  ); }
  int16_t   get16()   { return *(int16_t*)  get(sizeof(int16_t)   ); }
  uint32_t  getU32()  { return *(uint32_t*) get(sizeof(uint32_t)  ); }
  int32_t   get32()   { return *(int32_t*)  get(sizeof(int32_t)   ); }
  uint64_t  getU64()  { return *(uint64_t*) get(sizeof(uint64_t)  ); }
  int64_t   get64()   { return *(int64_t*)  get(sizeof(int64_t)   ); }

  bool get(char *buf, size_t size);
private:
  char *get(size_t size);

  static const size_t buffer_size = 64 * 1024;
  char buffer[buffer_size];
  size_t offset;
};

#define sRandom StrongRandom::guard().instance()

#endif // __BZAUTHD_RANDOM_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
