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

#include "Random.h"
#include <gcrypt.h>

INSTANTIATE_GUARDED_SINGLETON(StrongRandom)

StrongRandom::StrongRandom()
{
  offset = buffer_size;
}

char * StrongRandom::get(size_t size)
{
  if(offset + size > buffer_size) {
    gcry_randomize((unsigned char*)buffer, buffer_size, GCRY_STRONG_RANDOM);
    offset = 0;
  }
  char * ret = buffer + offset; 
  offset += size;
  return ret;
}

bool StrongRandom::get(char *buf, size_t size)
{
  if(size <= buffer_size) {
    memcpy(buf, get(size), size); 
    return true;
  }
  return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
