/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_IMAGE_FILE_H
#define BZF_IMAGE_FILE_H

#include "MediaFile.h"

class ImageFile : public MediaFile {
public:
	// close the image file.  this does *not* destroy the stream.
	virtual ~ImageFile();

	// note -- all concrete ImageFile types should have a method to
	// return the default file extension for files in the format:
	// static BzfString getExtension();

	// pixels are stored I, IA, RGB, or RGBA, depending on the number
	// of channels.  rows are stored left to right, bottom to top.
	// buffer must be at least getNumChannels() * getWidth() * getHeight()
	// bytes.
	virtual bool		read(void* buffer) = 0;

	// returns true if the stream was successfully opened as an image file
	bool				isOpen() const;

	// get information about the image file.  channels are 8 bits.
	int					getNumChannels() const;
	int					getWidth() const;
	int					getHeight() const;

protected:
	ImageFile(istream*);

	// save info about the stream.  called by the derived c'tor.
	// don't call this if the stream is not an audio file.
	void				init(int numChannels, int width, int height);

private:
	bool				open;
	int					numChannels;
	int					width, height;
};

#endif
