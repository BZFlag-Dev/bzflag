/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_MEDIA_FILE_H
#define BZF_MEDIA_FILE_H

#include <string>
#include "common.h"
#include "bzfio.h"

// if HALF_RATE_AUDIO defined then use half the normal audio sample
// rate (and downsample the audio files to match).  this reduces the
// demands on the system.
// #define HALF_RATE_AUDIO


class MediaFile {
public:
	// close the media file.  this does *not* destroy the stream.
	virtual ~MediaFile();

	// read an image file.  use delete[] to release the returned
	// image.  returns NULL on failure.  images are stored RGBA,
	// left to right, bottom to top.
	static unsigned char* readImage(const std::string& filename,
							int* width, int* height);

	// read a sound file.  use delete[] to release the returned
	// audio.  returns NULL on failure.  sounds are stored
	// left/right.
	static float*		readSound(const std::string& filename,
							int* numFrames, int* rate);

protected:
	MediaFile(std::istream*);

	// get the stream
	std::istream*			getStream() const { return stream; }

	// return true if the stream is in a readable state
	bool				isOkay() const;

	// utility method to read raw data
	void				readRaw(void* buffer, uint32_t bytes);

	// utility method to skip data
	void				skip(uint32_t bytes);

	// utility methods to read 2 and 4 byte number into host byte order
	uint16_t			read16LE();
	uint16_t			read16BE();
	uint32_t			read32LE();
	uint32_t			read32BE();

	// utility methods to byte swap in place into host byte order.  all
	// return the byte swapped data.
	static uint16_t		swap16LE(uint16_t*);
	static uint16_t		swap16BE(uint16_t*);
	static uint32_t		swap32LE(uint32_t*);
	static uint32_t		swap32BE(uint32_t*);

private:
	std::istream*			stream;
};

#endif
// ex: shiftwidth=4 tabstop=4
