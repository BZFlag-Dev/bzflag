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

unsigned char PNGImageFile::pngHeader[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

PNGImageFile::PNGImageFile(std::istream* stream) : ImageFile(stream), valid(true) 
{
	char buffer[8];
	stream->read(buffer, 8);
	if (strncmp((char*)pngHeader, buffer, 8) != 0) {
		valid = false;
		return;
	}

	PNGChunk *c = PNGChunk::readChunk(stream);
	if (c->getType() != PNGChunk::IHDR) {
		valid = false;
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

	init(3, width, height);
}

PNGImageFile::~PNGImageFile()
{
	// do nothing
}

std::string				PNGImageFile::getExtension()
{
	return ".png";
}

bool					PNGImageFile::read(void* buffer)
{
	PNGChunk *c;
	
	c = PNGChunk::readChunk(getStream());
	while ((c->getType() != PNGChunk::IDAT) && (c->getType() != PNGChunk::IEND)) {
		if (c->getType() == PNGChunk::PLTE)
			readPalette(c);
		delete c;
		c = PNGChunk::readChunk(getStream());
	}

	int lineLen = 4 * this->getWidth() + 1;
	unsigned char *line = new unsigned char[lineLen];
	int err;
	z_stream stream;
	stream.next_out = line;
	stream.avail_out = lineLen;
	stream.zalloc = (alloc_func)NULL;
	stream.zfree = (free_func)NULL;

	while (c->getType() == PNGChunk::IDAT) {

		stream.next_in = c->getData();
		stream.avail_in = c->getLength();

		err = inflateInit(&stream);
		if (err != Z_OK) {
			delete[] line;
			delete c;
			return false;
		}

		err = inflate(&stream, Z_FINISH);
		while (err == Z_BUF_ERROR) {
			//Do something with line			

			stream.next_out = line;
			stream.avail_out = lineLen;
			err = inflate(&stream, Z_FINISH);
		}
		if (err != Z_STREAM_END) {
			delete[] line;
			delete c;
			return false;
		}
		inflateEnd(&stream);

		delete c;
		c = PNGChunk::readChunk(getStream());
	}

	delete[] line;


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

int PNGImageFile::decodeData(void *buffer, int bufferPos, PNGChunk *c)
{
	return bufferPos;
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
