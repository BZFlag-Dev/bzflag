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

#include "PNGImageFile.h"
#include <string>
#include <iostream>
#include "pack.h"
#include "../zlib/zlib.h"

#ifdef WIN32
#include <winsock2.h>
#endif



//
// PNGImageFile
//

unsigned char		PNGImageFile::PNGHEADER[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

const unsigned char PNGImageFile::FILTER_NONE = 0;
const unsigned char PNGImageFile::FILTER_SUB = 1;
const unsigned char PNGImageFile::FILTER_UP = 2;
const unsigned char PNGImageFile::FILTER_AVERAGE = 3;
const unsigned char PNGImageFile::FILTER_PAETH = 4;

/* 
PNGImageFile::PNGImageFile(std::istream* stream)

  validates that the file is in fact a png file and initializes the size information for it
*/

PNGImageFile::PNGImageFile(std::istream* stream) : ImageFile(stream), palette(NULL)
{
	lineBuffers[0] = NULL;
	lineBuffers[1] = NULL;

	char buffer[8];
	stream->read(buffer, 8);
	if (strncmp((char*)PNGHEADER, buffer, 8) != 0) {
		return;
	}

	PNGChunk *c = PNGChunk::readChunk(stream);
	if (c->getType() != PNGChunk::IHDR) {
		delete c;
		return;
	}

	unsigned char* data = c->getData();
	int width, height;
	data = (unsigned char *)nboUnpackInt(data, width);
	data = (unsigned char *)nboUnpackInt(data, height);

	data = (unsigned char *)nboUnpackUByte(data, bitDepth);
	data = (unsigned char *)nboUnpackUByte(data, colorDepth);
	data = (unsigned char *)nboUnpackUByte(data, compressionMethod);
	data = (unsigned char *)nboUnpackUByte(data, filterMethod);
	data = (unsigned char *)nboUnpackUByte(data, interlaceMethod);

	int channels;
	switch (colorDepth) {
		case 0:
			lineBufferSize = ((width * bitDepth)/8)+1;
			channels = 1;
		break;

		case 2:
			lineBufferSize = ((3 * width * bitDepth)/8)+1;
			channels = 3;
		break;

		case 3:
			lineBufferSize = ((width * bitDepth)/8)+1;
			channels = 3;
		break;

		case 4:
			lineBufferSize = ((2 * width * bitDepth)/8)+1;
			channels = 2;
		break;

		case 6:
			lineBufferSize = ((4 * width * bitDepth)/8)+1;
			channels = 4;
		break;
	}

	realBufferSize = channels * width + 1;
	int allocSize = realBufferSize > lineBufferSize ? realBufferSize : lineBufferSize;
	lineBuffers[0] = new unsigned char[allocSize];
	lineBuffers[1] = new unsigned char[allocSize];
	memset(lineBuffers[1], 0, allocSize);
	activeBufferIndex = 0;

	//Temporary
	if (colorDepth != 2)
		return;
	if (bitDepth != 8)
		return;
	if (filterMethod != 0)
		return;


	init(channels, width, height);
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
		delete lineBuffers[0];
	if (lineBuffers[1] != NULL)
		delete lineBuffers[1];
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
	int	bufferPos = 0;
	
	c = PNGChunk::readChunk(getStream());
	while ((c->getType() != PNGChunk::IDAT) && (c->getType() != PNGChunk::IEND)) {
		if (c->getType() == PNGChunk::PLTE)
			palette = readPalette(c);
		delete c;
		c = PNGChunk::readChunk(getStream());
	}

	unsigned char *line = getLineBuffer();

	int err;
	z_stream stream;
	stream.next_out = line;
	stream.avail_out = lineBufferSize;
	stream.zalloc = (alloc_func)NULL;
	stream.zfree = (free_func)NULL;

	while (c->getType() == PNGChunk::IDAT) {

		stream.next_in = c->getData();
		stream.avail_in = c->getLength();

		err = inflateInit(&stream);
		if (err != Z_OK) {
			delete c;
			return false;
		}

		err = inflate(&stream, Z_FINISH);
		while (err == Z_BUF_ERROR) {

			expand();

			if (!filter()) {
				delete c;
				return false;
			}

			memcpy(((unsigned char *)buffer)+bufferPos, line+1, lineBufferSize-1);
			bufferPos += lineBufferSize-1;

			switchLineBuffers();
			line = getLineBuffer();
			stream.next_out = line;
			stream.avail_out = lineBufferSize;
			err = inflate(&stream, Z_FINISH);
		}

		if (err != Z_STREAM_END) {
			delete c;
			return false;
		}
		inflateEnd(&stream);

		delete c;
		c = PNGChunk::readChunk(getStream());
	}

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
	return lineBuffers[active ? activeBufferIndex : (1 - activeBufferIndex)];
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
	if ((bitDepth == 8) && (colorDepth != 3))
		return true;

	unsigned char *pData = getLineBuffer();

	int width = getWidth();
	switch (bitDepth)
	{
		case 1:
		{
			for (int i = width-1; i >= 0; i--) {
				int byteOffset = i/8 + 1;
				int bit = 7 - i%8;
				if (*(pData+byteOffset) & bit)
					*(pData+i+1) = 0xFF;
				else
					*(pData+i+1) = 0x00;
			}
		}
		break;

		case 2:
		{
			for (int i = width-1; i >= 0; i--) {
				int byteOffset = i/4 + 1;
				int bitShift = 6-2*(i%4);
				*(pData+i+1) = (((*(pData+byteOffset)) >> bitShift) & 0x03) << 6;
			}
		}
		break;

		case 4:
		{
			for (int i = width-1; i >= 0; i--) {
				int byteOffset = i/2+1;
				int bitShift = 4-4*(i%2);
				*(pData+i+1) = (((*(pData+byteOffset)) >> bitShift) & 0x0F) << 4;
			}
		}
		break;

		case 16:
		{
			for (int i = 0; i < width; i++) {
				*(pData+i+1) = (*pData + 2*i + 1);
			}
		}
		break;

		default:
			return false;
	}

	if (colorDepth == 3) {
		if (palette == NULL)
			return FALSE;

		for (int i = width-1; i >= 0; i--) {
			PNGRGB &rgb = palette->get(*(pData+i));
			*(pData + width*3 + 1) = rgb.red;
			*(pData + width*3 + 2) = rgb.green;
			*(pData + width*3 + 3) = rgb.blue;
		}
	}

	return true;
}

/* 
bool PNGImageFile::filter()

  Filter a line, based on the first byte found in line.
*/

bool PNGImageFile::filter()
{
	int	len = lineBufferSize;
	unsigned char *pData = getLineBuffer();

	switch (*pData) {
		case FILTER_NONE:
			return true;

		case FILTER_SUB:
		{
			unsigned char last[4] = {0, 0, 0, 0};
			int channels = getNumChannels();
			for (int i = 1; i < lineBufferSize; i++) {
				*(pData+i) += last[i%channels];
				last[i%channels] = *(pData+i);
			}
			return true;
			break;
		}

		case FILTER_UP:
		{
			unsigned char *up = getLineBuffer(false);
			for (int i = 1; i < lineBufferSize; i++)
				*(pData+i) += *(up+i);
			return true;
			break;
		}

		case FILTER_AVERAGE:
		{
			unsigned char last[4] = {0, 0, 0, 0};
			unsigned char *up = getLineBuffer(false);
			int channels = getNumChannels();
			for (int i = 1; i < lineBufferSize; i++) {
				*(pData+i) += (last[i%channels] + *(up+i))/2;
				last[i%channels] = *(pData+i);
			}
			return true;
			break;
		}

		case FILTER_PAETH:
		{
			unsigned char last[4] = {0, 0, 0, 0};
			unsigned char prevLast[4] = {0, 0, 0, 0};
			unsigned char *up = getLineBuffer(false);
			int channels = getNumChannels();
			for (int i = 1; i < lineBufferSize; i++) {
				int estimate = last[i%channels] + *(up+i) - prevLast[i%channels];
				int lastError = abs(estimate - last[i%channels]);
				int upError = abs(estimate - *(up+i));
				int lastUpError = abs(estimate - prevLast[i%channels]);
				int value;
				if ((lastError <= upError) && (lastError <= lastUpError))
					value = lastError;
				else if (upError <= lastUpError)
					value = upError;
				else
					value = lastUpError;

				*(pData+i) += value;
				last[i%channels] = value;
				prevLast[i%channels] = *(up+i);
			}
			return true;
			break;
		}

		default:
			return false;
	}
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

int PNGChunk::IHDR = 'IHDR';
int PNGChunk::PLTE = 'PLTE';
int PNGChunk::IDAT = 'IDAT';
int PNGChunk::IEND = 'IEND';

/* 
PNGChunk *PNGChunk::readChunk(std::istream *stream)

  Static factory function for parsing and creating an arbitrary PNG chunk
*/

PNGChunk *PNGChunk::readChunk(std::istream *stream)
{
	PNGChunk *c = new PNGChunk();
	stream->read((char *) &c->length, 4);
	c->length = ntohl(c->length);
	stream->read((char *) &c->type, 4);
	c->type = ntohl(c->type);
	if (c->length > 0) {
		c->data = new unsigned char[c->length];
		stream->read((char*) c->data, c->length);
	}
	stream->read((char *) &c->crc, 4);
	c->crc = ntohl(c->crc);
	return c;
}

/* 
PNGChunk *PNGChunk::readChunk(std::istream *stream)

  Default (private) constructor for a png chunk
*/

PNGChunk::PNGChunk()
: length(0), type(0), data(NULL), crc(0)
{
}

/* 
PNGChunk *PNGChunk::readChunk(std::istream *stream)

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
