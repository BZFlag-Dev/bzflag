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

#ifdef WIN32
#include <winsock2.h>
#endif

// This code is 'inprogress' I'm reading chunk types that I don't find documented. TBD
// Not ready for use.


//
// PNGImageFile
//

unsigned char PNGImageFile::pngHeader[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

PNGImageFile::PNGImageFile(std::istream* stream) : ImageFile(stream), valid(true) 
{
	char buffer[8];
	stream->read( buffer, 8 );
	if (strncmp( (char*)pngHeader, buffer, 8 ) != 0) {
		valid = false;
		return;
	}

	PNGChunk *c = PNGChunk::readChunk( stream );
	if (c->getType() != PNGChunk::IHDR) {
		valid = false;
		delete c;
		return;
	}

	unsigned char* data = c->getData();
	int width, height;
	unsigned char bitDepth, colorDepth;
	data = (unsigned char *)nboUnpackInt( data, width );
	data = (unsigned char *)nboUnpackInt( data, height );
	data = (unsigned char *)nboUnpackUByte( data, bitDepth );
	data = (unsigned char *)nboUnpackUByte( data, colorDepth );

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
	
	do
	{
		c = PNGChunk::readChunk(getStream());
		if ((c->getType() == PNGChunk::IDAT) || (c->getType() == PNGChunk::IEND))
			break;
		delete c;
	} while (true);

	unsigned char *data = c->getData();


	delete c;
	return true;
}

int PNGChunk::IHDR = 'IHDR';
int PNGChunk::sRGB = 'sRGB';
int PNGChunk::PLTE = 'PLTE';
int PNGChunk::IDAT = 'IDAT';
int PNGChunk::IEND = 'IEND';

PNGChunk *PNGChunk::readChunk(std::istream *stream)
{
	PNGChunk *c = new PNGChunk();
	stream->read( (char *) &c->length, 4 );
	c->length = ntohl( c->length );
	stream->read( (char *) &c->type, 4 );
	c->type = ntohl( c->type );
	if (c->length > 0)
	{
		c->data = new unsigned char[c->length];
		stream->read( (char*) c->data, c->length );
	}
	stream->read( (char *) &c->crc, 4 );
	c->crc = ntohl( c->crc );
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
