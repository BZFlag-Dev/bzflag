/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "bzUnicode.h"


// UTF8StringItr
UTF8StringItr& UTF8StringItr::operator ++()
{
  curPos = nextPos;

  // get rid of signedness before doing bit magic
  const unsigned char* np = reinterpret_cast<const unsigned char*>(nextPos);

  unsigned int ch = 0;
  unsigned int extraBytesToRead = utf8bytes[*np];
  // falls through
  switch (extraBytesToRead)
  {
    case 6: ch += *np++; ch <<= 6; /* remember, illegal UTF-8 */
    case 5: ch += *np++; ch <<= 6; /* remember, illegal UTF-8 */
    case 4: ch += *np++; ch <<= 6;
    case 3: ch += *np++; ch <<= 6;
    case 2: ch += *np++; ch <<= 6;
    case 1: ch += *np++;
  }

  // put the pointer back
  nextPos = reinterpret_cast<const char*>(np);
  ch -= offsetsFromUTF8[extraBytesToRead-1];
  curChar = ch;

  return *this;
}

/* The first character in a UTF8 sequence indicates how many bytes
 * to read (among other things) */
const char UTF8StringItr::utf8bytes[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,5,5,5,5,6,6,6,6
};

/* Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence. */
const unsigned long UTF8StringItr::offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
  0x03C82080UL, 0xFA082080UL, 0x82082080UL };


// bzUTF8Char

bzUTF8Char::bzUTF8Char(unsigned int ch)
{
  int bytesOut = 0;
  if (ch < 0x80) {
    bytesOut = 1;
  } else if (ch < 0x800) {
    bytesOut = 2;
  } else if (ch < 0x10000) {
    bytesOut = 3;
  } else {
    bytesOut = 4;
  }

  buf = new char[bytesOut+1];
  buf[bytesOut] = '\0';

  buf += bytesOut;
  switch (bytesOut) { /* note: everything falls through. */
    case 4: *--buf = ((ch | byteMark) & byteMask); ch >>= 6;
    case 3: *--buf = ((ch | byteMark) & byteMask); ch >>= 6;
    case 2: *--buf = ((ch | byteMark) & byteMask); ch >>= 6;
    case 1: *--buf =  (ch | firstByteMark[bytesOut]);
  }
}

const unsigned char bzUTF8Char::firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
const unsigned int bzUTF8Char::byteMask = 0xBF;
const unsigned int bzUTF8Char::byteMark = 0x80;

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
