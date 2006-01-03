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

#include "SGIImageFile.h"
#include <string>
#include <iostream>

//
// SGIImageFile
//

SGIImageFile::SGIImageFile(std::istream* input) : ImageFile(input)
{
  unsigned char header[512];
  readRaw(header, sizeof(header));
  if (!isOkay()) {
    // can't read header
    return;
  }

  if (swap16BE(reinterpret_cast<uint16_t*>(header + 0)) != 474) {
    // bad magic
    return;
  }

  uint16_t dimensions = swap16BE(reinterpret_cast<uint16_t*>(header + 4));
  uint32_t data32 = swap32BE(reinterpret_cast<uint32_t*>(header + 104));
  if ((header[2] != 0 && header[2] != 1) ||
      (header[3] < 1 || header[3] > 1) ||
      (dimensions < 1 || dimensions > 3) ||
      data32 != 0) {
    // unsupported format
    return;
  }

  // get dimensions
  uint16_t myWidth, myHeight, depth;
  myWidth = swap16BE(reinterpret_cast<uint16_t*>(header + 6));
  if (dimensions < 2)
    myHeight = 1;
  else
    myHeight = swap16BE(reinterpret_cast<uint16_t*>(header + 8));
  if (dimensions < 3)
    depth = 1;
  else
    depth = swap16BE(reinterpret_cast<uint16_t*>(header + 10));
  if (depth > 4)
    depth = 4;

  // save info
  isVerbatim = (header[2] == 0);
  init(static_cast<int>(depth), static_cast<int>(myWidth),
       static_cast<int>(myHeight));
}

SGIImageFile::~SGIImageFile()
{
  // do nothing
}

std::string				SGIImageFile::getExtension()
{
  return ".rgb";
}

bool					SGIImageFile::read(void* buffer)
{
  if (isVerbatim)
    return readVerbatim(buffer);
  else
    return readRLE(buffer);
}

bool					SGIImageFile::readVerbatim(void* buffer)
{
  unsigned char* image = reinterpret_cast<unsigned char*>(buffer);
  const int dx = getWidth();
  const int dy = getHeight();
  const int dz = getNumChannels();

  // make row buffer
  unsigned char* row = new unsigned char[dx];

  // read each channel one after the other
  for (int z = 0; z < dz; ++z) {
    unsigned char* dst = image + z;
    for (int y = 0; isOkay() && y < dy; ++y) {
      // read raw data
      readRaw(row, dx);

      // swizzle into place
      for (int x = 0; x < dx; ++x) {
	*dst = row[x];
	dst += dz;
      }
    }
  }

  // clean up
  delete[] row;

  return isOkay();
}

bool					SGIImageFile::readRLE(void* buffer)
{
  unsigned char* image = reinterpret_cast<unsigned char*>(buffer);
//  const int dx = getWidth();
  const int dy = getHeight();
  const int dz = getNumChannels();

  // read offset tables
  const int tableSize = dy * dz;
  uint32_t* startTable  = new uint32_t[tableSize];
  uint32_t* lengthTable = new uint32_t[tableSize];
  readRaw(startTable, 4 * tableSize);
  readRaw(lengthTable, 4 * tableSize);
  if (!isOkay()) {
    delete[] startTable;
    delete[] lengthTable;
    return false;
  }

  // convert offset tables to proper endianness
  for (int n = 0; n < tableSize; ++n) {
    swap32BE(startTable + n);
    swap32BE(lengthTable + n);
  }

  // make row buffer
  uint32_t rowSize   = 4;
  unsigned char* row = new unsigned char[rowSize];

  // read each channel one after the other
  for (int z = 0; z < dz; ++z) {
    unsigned char* dst = image + z;
    for (int y = 0; isOkay() && y < dy; ++y) {
      // get length of row
      const uint32_t length = lengthTable[y + z * dy];

      // make row buffer bigger if necessary
      if (length > rowSize) {
	delete[] row;
	rowSize = length;
	row     = new unsigned char[rowSize];
      }

      // read raw data
      getStream()->seekg(startTable[y + z * dy], std::ios::beg);
      readRaw(row, length);
      if (!isOkay())
	break;

      // decode
      unsigned char* src = row;
      while (1) {
	// check for error in image
	if (static_cast<uint32_t>(src - row) >= length) {
	  delete[] row;
	  delete[] startTable;
	  delete[] lengthTable;
	  return false;
	}

	// get next code
	const unsigned char type = *src++;
	int count = static_cast<int>(type & 0x7f);

	// zero code means end of row
	if (count == 0)
	  break;

	if (type & 0x80) {
	  // copy count pixels
	  while (count--) {
	    *dst = *src++;
	    dst += dz;
	  }
	} else {
	  // repeat pixel count times
	  const unsigned char pixel = *src++;
	  while (count--) {
	    *dst = pixel;
	    dst += dz;
	  }
	}
      }
    }
  }

  // clean up
  delete[] row;
  delete[] startTable;
  delete[] lengthTable;

  return isOkay();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

