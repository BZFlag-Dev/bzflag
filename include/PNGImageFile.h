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

#ifndef BZF_PNG_IMAGE_FILE_H
#define BZF_PNG_IMAGE_FILE_H

#include "ImageFile.h"

class PNGImageFile : public ImageFile {
public:
	PNGImageFile(std::istream*);
	virtual ~PNGImageFile();

	static std::string	getExtension();

	// ImageFile overrides
	virtual bool		read(void* buffer);
private:
	static unsigned char	pngHeader[8];
	bool					valid;
};

class PNGChunk
{
public:
	static PNGChunk *readChunk(std::istream *stream);
	~PNGChunk();
	int getLength();
	int getType();
	unsigned char* getData();

	static int		IHDR;
	static int		sRGB;
	static int		PLTE;
	static int		IDAT;
	static int		IEND;
private:
	PNGChunk();
	int				length;
	int				type;
	unsigned char	*data;
	int				crc;

};

#endif
