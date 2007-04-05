/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "MediaFile.h"

/* system implementation headers */
#include <iostream>
#include <string>
#include <algorithm>

//
// MediaFile
//

MediaFile::MediaFile(std::istream* _stream) : stream(_stream)
{
  // do nothing
}

MediaFile::~MediaFile()
{
  // do nothing
}

bool		MediaFile::isOkay() const
{
  return (stream != NULL && stream->good());
}

void		MediaFile::readRaw(void* vbuffer, uint32_t bytes)
{
  char* buffer = reinterpret_cast<char*>(vbuffer);
  stream->read(buffer, bytes);
}

void		MediaFile::skip(uint32_t bytes)
{
  stream->ignore(bytes);
}

uint16_t	MediaFile::read16LE()
{
  uint16_t b;
  stream->read(reinterpret_cast<char*>(&b), sizeof(b));
  return swap16LE(&b);
}

uint16_t	MediaFile::read16BE()
{
  uint16_t b;
  stream->read(reinterpret_cast<char*>(&b), sizeof(b));
  return swap16BE(&b);
}

uint32_t	MediaFile::read32LE()
{
  uint32_t b;
  stream->read(reinterpret_cast<char*>(&b), sizeof(b));
  return swap32LE(&b);
}

uint32_t	MediaFile::read32BE()
{
  uint32_t b;
  stream->read(reinterpret_cast<char*>(&b), sizeof(b));
  return swap32BE(&b);
}

uint16_t	MediaFile::swap16LE(uint16_t* d)
{
  unsigned char* b = reinterpret_cast<unsigned char*>(d);
  *d = static_cast<uint16_t>(b[0]) + (static_cast<uint16_t>(b[1]) << 8);
  return *d;
}

uint16_t	MediaFile::swap16BE(uint16_t* d)
{
  unsigned char* b = reinterpret_cast<unsigned char*>(d);
  *d = static_cast<uint16_t>(b[1]) + (static_cast<uint16_t>(b[0]) << 8);
  return *d;
}

uint32_t	MediaFile::swap32LE(uint32_t* d)
{
  unsigned char* b = reinterpret_cast<unsigned char*>(d);
  *d = static_cast<uint32_t>(b[0]) + (static_cast<uint32_t>(b[1]) << 8) +
       (static_cast<uint32_t>(b[2]) << 16) +
       (static_cast<uint32_t>(b[3]) << 24);
  return *d;
}

uint32_t	MediaFile::swap32BE(uint32_t* d)
{
  unsigned char* b = reinterpret_cast<unsigned char*>(d);
  *d = static_cast<uint32_t>(b[3]) + (static_cast<uint32_t>(b[2]) << 8) +
       (static_cast<uint32_t>(b[1]) << 16) +
       (static_cast<uint32_t>(b[0]) << 24);
  return *d;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

