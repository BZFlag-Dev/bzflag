/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "PNGImageFile.h"
#include <iostream>
#include "Pack.h"
#include "bzfio.h"
#include "../zlib/zconf.h"
#include "../zlib/zlib.h"

#ifndef _WIN32
#include <netinet/in.h>
#else
#include <WINSOCK2.H>
#endif



//
// PNGImageFile
//

unsigned char		PNGImageFile::PNGHEADER[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

const unsigned char	PNGImageFile::MAX_COMPONENTS = 4;

const unsigned char	PNGImageFile::FILTER_NONE = 0;
const unsigned char	PNGImageFile::FILTER_SUB = 1;
const unsigned char	PNGImageFile::FILTER_UP = 2;
const unsigned char	PNGImageFile::FILTER_AVERAGE = 3;
const unsigned char	PNGImageFile::FILTER_PAETH = 4;

/*
PNGImageFile::PNGImageFile(std::istream* stream)

  validates that the file is in fact a png file and initializes the size information for it
*/

PNGImageFile::PNGImageFile(std::istream* input) : ImageFile(input), palette(NULL)
{
  lineBuffers[0] = NULL;
  lineBuffers[1] = NULL;

  char buffer[8];
  input->read(buffer, 8);
  if (strncmp((char*)PNGHEADER, buffer, 8) != 0) {
    return;
  }

  PNGChunk *c = PNGChunk::readChunk(input);
  if (c->getType() != PNGChunk::IHDR) {
    delete c;
    return;
  }

  unsigned char* data = c->getData();
  int32_t myWidth, myHeight;
  data = (unsigned char *)nboUnpackInt(data, myWidth);
  data = (unsigned char *)nboUnpackInt(data, myHeight);

  data = (unsigned char *)nboUnpackUByte(data, bitDepth);
  data = (unsigned char *)nboUnpackUByte(data, colorDepth);
  data = (unsigned char *)nboUnpackUByte(data, compressionMethod);
  data = (unsigned char *)nboUnpackUByte(data, filterMethod);
  data = (unsigned char *)nboUnpackUByte(data, interlaceMethod);

  delete c;

  int channels;
  switch (colorDepth) {
    case 0:
      lineBufferSize = (((myWidth * bitDepth + ((bitDepth < 8) ? (bitDepth+1) : 0)))/8)+1;
      channels = 1;
    break;

    case 2:
      lineBufferSize = (((3 * myWidth * bitDepth + ((bitDepth < 8) ? (bitDepth-1) : 0)))/8)+1;
      channels = 3;
    break;

    case 3:
      lineBufferSize = (((myWidth * bitDepth + ((bitDepth < 8) ? (bitDepth-1) : 0)))/8)+1;
      channels = 3;
    break;

    case 4:
      lineBufferSize = (((2 * myWidth * bitDepth + ((bitDepth < 8) ? (bitDepth-1) : 0)))/8)+1;
      channels = 2;
    break;

    case 6:
      lineBufferSize = (((4 * myWidth * bitDepth + ((bitDepth < 8) ? (bitDepth-1) : 0)))/8)+1;
      channels = 4;
    break;

    default:
      return;
  }

  realBufferSize = channels * myWidth + 1;
  int allocSize = ((realBufferSize > lineBufferSize) ? realBufferSize : lineBufferSize) + MAX_COMPONENTS;
  lineBuffers[0] = new unsigned char[allocSize];
  lineBuffers[1] = new unsigned char[allocSize];
  memset(lineBuffers[0], 0, MAX_COMPONENTS);
  memset(lineBuffers[1], 0, allocSize);
  activeBufferIndex = 0;

  if (filterMethod != 0)
    return;
  if (interlaceMethod != 0)
    return;


  init(channels, myWidth, myHeight);

  DEBUG4("Read PNG: Width %d, Height %d, Bit depth %d, Color type %d, Filter Method %d, Interlace Method %d, Channels %d.\n", myWidth, myHeight, bitDepth, colorDepth, filterMethod, interlaceMethod, channels);
}

/*
PNGImageFile::~PNGImageFile()

  cleans up memory buffers
*/

PNGImageFile::~PNGImageFile()
{
  if (palette)
    delete palette;
  if (lineBuffers[0] != NULL)
    delete [] lineBuffers[0];
  if (lineBuffers[1] != NULL)
    delete [] lineBuffers[1];
}

/*
std::string	PNGImageFile::getExtension()

  returns the expected file extension of .png for files
*/

std::string				PNGImageFile::getExtension()
{
  return ".png";
}

/*
bool PNGImageFile::read(void* buffer)

  parses the file looking for PLTE or IDAT entries and converts to 8 bit data
  in 1, 2, 3, or 4 channels
*/

bool					PNGImageFile::read(void* buffer)
{
  PNGChunk *c;
  int	bufferPos = getWidth() * getNumChannels() * (getHeight() - 1);

  c = PNGChunk::readChunk(getStream());
  while ((c->getType() != PNGChunk::IDAT) && (c->getType() != PNGChunk::IEND)) {
    if (c->getType() == PNGChunk::PLTE)
      palette = readPalette(c);
    delete c;
    c = PNGChunk::readChunk(getStream());
  }

  unsigned char *line = getLineBuffer();

  int err;
  z_stream zStream;
  zStream.next_out = line;
  zStream.avail_out = lineBufferSize;
  zStream.zalloc = (alloc_func)NULL;
  zStream.zfree = (free_func)NULL;

  err = inflateInit(&zStream);
  if (err != Z_OK) {
    delete c;
    return false;
  }

  while (c->getType() == PNGChunk::IDAT) {
    zStream.next_in = c->getData();
    zStream.avail_in = c->getLength();

    err = inflate(&zStream, Z_SYNC_FLUSH);
    while (((err == Z_OK) || err == Z_STREAM_END)  && (zStream.avail_out == 0)) {

      expand();

      if (!filter()) {
	delete c;
	return false;
      }

      memcpy(((unsigned char *)buffer)+bufferPos, line+1, realBufferSize-1);
      bufferPos -= realBufferSize-1;

      switchLineBuffers();
      line = getLineBuffer();

      zStream.next_out = line;
      zStream.avail_out = lineBufferSize;
      err = inflate(&zStream, Z_SYNC_FLUSH);
    }

    if ((err != Z_STREAM_END) && (err != Z_OK)) {
      delete c;
      return false;
    }

    delete c;
    c = PNGChunk::readChunk(getStream());
  }

  inflateEnd(&zStream);

  delete c;
  return true;
}

/*
PNGPalette* PNGImageFile::readPalette(PNGChunk *c)

  Code to parse a PLTE chunk
*/

PNGPalette* PNGImageFile::readPalette(PNGChunk *c)
{
  int numColors = c->getLength() / sizeof(PNGRGB);
  PNGPalette *p = new PNGPalette(numColors);
  PNGRGB rgb;

  unsigned char *pData = c->getData();
  for (int i = 0; i < numColors; i++) {
    pData = (unsigned char *)nboUnpackUByte(pData, rgb.red);
    pData = (unsigned char *)nboUnpackUByte(pData, rgb.green);
    pData = (unsigned char *)nboUnpackUByte(pData, rgb.blue);
    p->add(rgb);
  }
  return p;
}

/*
unsigned char *PNGImageFile::getLineBuffer(bool active)

  This is the buffer that holds one line of data. As filters may use the previous line
  to filter with, you can ask for the non active one, as well
*/

unsigned char *PNGImageFile::getLineBuffer(bool active)
{
  return MAX_COMPONENTS + lineBuffers[active ? activeBufferIndex : (1 - activeBufferIndex)];
}

/*
void PNGImageFile::switchLineBuffers()

  Change buffers from the active to unactive. In this way you always have the last line available
  for filtering.
*/

void PNGImageFile::switchLineBuffers()
{
  activeBufferIndex = 1 - activeBufferIndex;
}

/*
bool PNGImageFile::expand()

  Expand data to 8 bits. If the original data is indexed color, convert to rgb data.
*/

bool PNGImageFile::expand()
{
  // all indexed color modes require a palette
  if (colorDepth == 3 && palette == NULL)
    return false;

  unsigned char *pData = getLineBuffer();

  int myWidth = getWidth();
  int channels = getNumChannels();
  switch (bitDepth)  {
    case 1:
    {
      for (int i = myWidth-1; i >= 0; i--) {
	int byteOffset = i/8 + 1;
	int bit = 7 - i%8;
	if (colorDepth != 3) {
	  *(pData+i+1) = ((*(pData+byteOffset) >> bit) & 0x01) ? 0xFF : 0x00;
	} else {
	  PNGRGB &rgb = palette->get(((*(pData+byteOffset) >> bit) & 0x01));
	  *(pData + i*3 + 1) = rgb.red;
	  *(pData + i*3 + 2) = rgb.green;
	  *(pData + i*3 + 3) = rgb.blue;
	}
      }
    }
    break;

    case 2:
    {
      for (int i = myWidth-1; i >= 0; i--) {
	int byteOffset = i/4 + 1;
	int bitShift = 6-2*(i%4);
	if (colorDepth != 3) {
	  *(pData+i+1) = (((*(pData+byteOffset)) >> bitShift) & 0x03) << 6;
	} else {
	  PNGRGB &rgb = palette->get(((*(pData+byteOffset)) >> bitShift) & 0x03);
	  *(pData + i*3 + 1) = rgb.red;
	  *(pData + i*3 + 2) = rgb.green;
	  *(pData + i*3 + 3) = rgb.blue;
	}
      }
    }
    break;

    case 4:
    {
      for (int i = myWidth-1; i >= 0; i--) {
	int byteOffset = i/2+1;
	int bitShift = 4-4*(i%2);
	if (colorDepth != 3) {
	  *(pData+i+1) = (((*(pData+byteOffset)) >> bitShift) & 0x0F) << 4;
	} else {
	  PNGRGB &rgb = palette->get(((*(pData+byteOffset)) >> bitShift) & 0x0F);
	  *(pData + i*3 + 1) = rgb.red;
	  *(pData + i*3 + 2) = rgb.green;
	  *(pData + i*3 + 3) = rgb.blue;
	}
      }
    }
    break;

    case 8:
    {
      if (colorDepth == 3) {
	// colormapped
	for (int i = myWidth-1; i >= 0; i--) {
	  PNGRGB &rgb = palette->get(*(pData+i+1));
	  *(pData + i*3 + 1) = rgb.red;
	  *(pData + i*3 + 2) = rgb.green;
	  *(pData + i*3 + 3) = rgb.blue;
	}
      } else {
	// already in native color
	return true;
      }
    }
    break;

    case 16:
    {
      for (int i = 0; i < myWidth*channels; i++) {
	*(pData+i+1) = (*(pData + 2*i + 1));
      }
    }
    break;

    default:
      return false;
  }

  return true;
}

/*
bool PNGImageFile::filter()

  Filter a line, based on the first byte found in line. Data must be in 8 bit format
*/

bool PNGImageFile::filter()
{
  //int	len = lineBufferSize;
  unsigned char *pData = getLineBuffer();

  unsigned char filterType = *pData;
  *(pData++) = 0;

  switch (filterType) {
    case FILTER_NONE:
      return true;

    case FILTER_SUB:
    {
      int channels = getNumChannels();
      for (int i = 1; i < lineBufferSize; i++, pData++)
	*pData += *(pData-channels);
      return true;
      break;
    }

    case FILTER_UP:
    {
      unsigned char *pUp = getLineBuffer(false)+1;
      for (int i = 1; i < lineBufferSize; i++, pData++, pUp++)
	*pData += *pUp;
      return true;
      break;
    }

    case FILTER_AVERAGE:
    {
      unsigned char *pUp = getLineBuffer(false)+1;
      int channels = getNumChannels();
      for (int i = 1; i < lineBufferSize; i++, pData++, pUp++) {
	int last = *(pData-channels);
	int up = *pUp;

	*pData += (last + up)/2;
      }
      return true;
      break;
    }

    case FILTER_PAETH:
    {
      unsigned char *pUp = getLineBuffer(false)+1;
      int channels = getNumChannels();
      for (int i = 1; i < lineBufferSize; i++, pData++, pUp++) {
	int a = *(pData-channels);
	int b = *pUp;
	int c = *(pUp-channels);

	int p = b - c;
	int pc = a - c;
	int pa = abs(p);
	int pb = abs(pc);
	pc = abs(p + pc);

	*pData += (pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c;
      }
      return true;
      break;
    }

    default:
      return false;
  }
  return false;
}


/*
PNGRGB::PNGRGB()

  Default constructor for RGB value as found in a palette
*/

PNGRGB::PNGRGB()
{
  red = 0;
  green = 0;
  blue = 0;
}

/*
PNGRGB::PNGRGB()

  Initializing constructor for RGB value as found in a palette
*/

PNGRGB::PNGRGB(unsigned char r, unsigned char g, unsigned char b)
{
  red = r;
  green = g;
  blue = b;
}




/*
PNGPalette::PNGPalette(int nc)

  Constructor for a PNG palette
*/

PNGPalette::PNGPalette(int nc)
{
  curColor = 0;
  numColors = nc;
  colors = new PNGRGB[nc];
}

/*
PNGPalette::~PNGPalette()

  Destructor for a PNG palette
*/

PNGPalette::~PNGPalette()
{
  delete [] colors;
}

/*
void PNGPalette::add(PNGRGB& color)

  Add a rgb value to end of palette
*/

void PNGPalette::add(PNGRGB& color)
{
  colors[curColor++] = color;
}

/*
PNGRGB& PNGPalette::get(int index)

  Retrieve rgb value in palette at index 'index'. Returns black if invalid entry
*/

PNGRGB& PNGPalette::get(int index)
{
  static PNGRGB unknown(0,0,0);

  if ((index >= 0) && (index < numColors))
    return colors[index];

  return unknown;
}

#define PNGTAG(t_) ((((int)t_[0]) << 24) | \
		   (((int)t_[1]) << 16) | \
		   (((int)t_[2]) <<  8) | \
		   (int)t_[3])
int PNGChunk::IHDR = PNGTAG("IHDR");
int PNGChunk::PLTE = PNGTAG("PLTE");
int PNGChunk::IDAT = PNGTAG("IDAT");
int PNGChunk::IEND = PNGTAG("IEND");

/*
PNGChunk *PNGChunk::readChunk(std::istream *stream)

  Static factory function for parsing and creating an arbitrary PNG chunk
*/

PNGChunk *PNGChunk::readChunk(std::istream *input)
{
  PNGChunk *c = new PNGChunk();
  input->read((char *) &c->length, 4);
  c->length = ntohl(c->length);
  input->read((char *) &c->type, 4);
  c->type = ntohl(c->type);
  if (c->length > 0) {
    c->data = new unsigned char[c->length];
    input->read((char*) c->data, c->length);
  }
  input->read((char *) &c->crc, 4);
  c->crc = ntohl(c->crc);
  return c;
}

/*
PNGChunk::PNGChunk()

  Default (private) constructor for a png chunk
*/

PNGChunk::PNGChunk()
: length(0), type(0), data(NULL), crc(0)
{
}

/*
PNGChunk::~PNGChunk()

  Default destructor
*/

PNGChunk::~PNGChunk()
{
  if (data != NULL)
    delete[] data;
}

/*
int PNGChunk::getLength()

  returns data length
*/

int PNGChunk::getLength()
{
  return length;
}

/*
int PNGChunk::getType()

  returns chunk type
*/

int PNGChunk::getType()
{
  return type;
}

/*
unsigned char *PNGChunk::getData()

  returns chunk data
*/

unsigned char *PNGChunk::getData()
{
  return data;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

