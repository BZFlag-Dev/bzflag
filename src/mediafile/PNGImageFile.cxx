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

PNGImageFile::PNGImageFile(std::istream* stream) : ImageFile(stream)
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
			lineBufferSize = (width * (8/bitDepth))+1;
			channels = 1;
		break;

		case 2:
			lineBufferSize = (width * 3 * (8/bitDepth))+1;
			channels = 3;
		break;

		case 3:
			lineBufferSize = (width * (8/bitDepth))+1;
			channels = 3;
		break;

		case 4:
			lineBufferSize = (width * 2 * (8/bitDepth))+1;
			channels = 2;
		break;

		case 6:
			lineBufferSize = (width * 4 * (8/bitDepth))+1;
			channels = 4;
		break;
	}

	lineBuffers[0] = new unsigned char[lineBufferSize];
	lineBuffers[1] = new unsigned char[lineBufferSize];
	memset(lineBuffers[1], 0, lineBufferSize);
	activeBufferIndex = 0;

	//Temporary
	if (colorDepth != 2)
		return;
	if (bitDepth != 8)
		return;


	init(channels, width, height);
}

PNGImageFile::~PNGImageFile()
{
	if (lineBuffers[0] != NULL)
		delete lineBuffers[0];
	if (lineBuffers[1] != NULL)
		delete lineBuffers[1];
}

std::string				PNGImageFile::getExtension()
{
	return ".png";
}

bool					PNGImageFile::read(void* buffer)
{
	PNGChunk *c;
	int	bufferPos = 0;
	
	c = PNGChunk::readChunk(getStream());
	while ((c->getType() != PNGChunk::IDAT) && (c->getType() != PNGChunk::IEND)) {
		if (c->getType() == PNGChunk::PLTE)
			readPalette(c);
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

unsigned char *PNGImageFile::getLineBuffer(bool active)
{
	return lineBuffers[active ? activeBufferIndex : (1 - activeBufferIndex)];
}

void PNGImageFile::switchLineBuffers()
{
	activeBufferIndex = 1 - activeBufferIndex;
}

bool PNGImageFile::filter()
{
	int	len = lineBufferSize;
	unsigned char *pData = getLineBuffer();

	switch (*pData) {
		case FILTER_NONE:
			return true;

		case FILTER_SUB:
		{
			unsigned char last = 0;
			for (int i = 1; i < lineBufferSize; i++) {
				*(pData+i) += last;
				last = *(pData+i);
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

		default:
			return false;
	}
}




PNGPalette::PNGPalette(int nc)
{
	curColor = 0;
	numColors = nc;
	colors = new PNGRGB[nc];
}

PNGPalette::~PNGPalette()
{
	delete [] colors;
}

void PNGPalette::add(PNGRGB& color)
{
	colors[curColor++] = color;
}

int PNGChunk::IHDR = 'IHDR';
int PNGChunk::sRGB = 'sRGB';
int PNGChunk::PLTE = 'PLTE';
int PNGChunk::IDAT = 'IDAT';
int PNGChunk::IEND = 'IEND';

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

PNGChunk::PNGChunk()
: length(0), type(0), data(NULL), crc(0)
{
}

PNGChunk::~PNGChunk()
{
	if (data != NULL)
		delete[] data;
}

int PNGChunk::getLength()
{
	return length;
}

int PNGChunk::getType()
{
	return type;
}

unsigned char *PNGChunk::getData()
{
	return data;
}
